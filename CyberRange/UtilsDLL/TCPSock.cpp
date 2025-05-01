#define SQL_NOUNICODEMAP
#include "pch.h"
#include "TCPSock.h"
#include <iostream>
#include <thread>
#include <mutex>
#include <openssl/err.h>
#include <sstream>
#include <windows.h>
#include <sqlext.h>
#include <sqltypes.h>
#include <sql.h>


TCPSock::TCPSock(const std::string& addr, int port)
    : address(addr), port(port), connected(false), isServer(false),
    timeout(sf::seconds(30)), tlsEnabled(false), ssl(nullptr, SSL_free),
    sslCtx(nullptr, SSL_CTX_free) {

    std::string msg = "Client socket created for " + addr + ":" + std::to_string(port);
    printMessage(msg);
}

TCPSock::TCPSock(int port)
    : address("0.0.0.0"), port(port), connected(false), isServer(true),
    timeout(sf::seconds(30)), tlsEnabled(false), ssl(nullptr, SSL_free),
    sslCtx(nullptr, SSL_CTX_free) {

    std::string msg = "Server socket created on port " + std::to_string(port);
    printMessage(msg);
}

TCPSock::TCPSock(std::unique_ptr<sf::TcpSocket> sock, const std::string& clientAddr, int clientPort)
    : address(clientAddr), port(clientPort),
    connected(false), isServer(false), timeout(sf::seconds(30)), tlsEnabled(false),
    ssl(nullptr, SSL_free), sslCtx(nullptr, SSL_CTX_free) {

    if (sock) {
        tcpSocket = std::make_unique<TcpSocketWithHandle>();
        printMessage("Client socket created for " + clientAddr + ":" + std::to_string(clientPort));

        connected = true;

        auto* rawSocket = sock.release();

        if (auto* socketWithHandle = dynamic_cast<TcpSocketWithHandle*>(rawSocket)) {
            tcpSocket.reset(socketWithHandle);
        }
        else {
            printMessage("Failed to transfer socket handle - creating new connection");

            sf::Socket::Status status = tcpSocket->connect(
                sf::IpAddress(clientAddr),
                static_cast<unsigned short>(clientPort),
                timeout
            );

            if (status != sf::Socket::Status::Done) {
                printMessage("Failed to create client connection to " + clientAddr + ":" + std::to_string(clientPort));
                connected = false;
            }

            delete rawSocket;
        }
    }
    else {
        printMessage("Invalid socket provided for client connection");
        connected = false;
    }
}

TCPSock::~TCPSock() {
    disconnect();
    stopServer();
    printMessage("Socket destroyed for " + address + ":" + std::to_string(port));
}

bool TCPSock::connect() {
    if (isServer || connected || tcpSocket) {
        printMessage("Cannot connect: socket already in use or is a server");
        return false;
    }

    tcpSocket = std::make_unique<TcpSocketWithHandle>();
    tcpSocket->setBlocking(true);

    printMessage("Connecting to " + address + ":" + std::to_string(port) + "...");
    sf::Socket::Status status = tcpSocket->connect(address, static_cast<unsigned short>(port), timeout);
    if (status != sf::Socket::Status::Done) {
        printMessage("Error connecting to server: " + std::to_string(static_cast<int>(status)));
        tcpSocket.reset();
        return false;
    }

    if (tlsEnabled && !setupTLS()) {
        printMessage("TLS setup failed");
        disconnect();
        return false;
    }

    connected = true;
    printMessage("Successfully connected to " + address + ":" + std::to_string(port));
    return true;
}

bool TCPSock::disconnect() {
    if (!connected) {
        return true;
    }

    printMessage("Disconnecting from " + address + ":" + std::to_string(port));
    cleanupTLS();
    tcpSocket.reset();
    connected = false;
    return true;
}

bool TCPSock::isConnected() const {
    return connected && tcpSocket;
}

int TCPSock::send(const std::string& data) {
    if (!isConnected()) {
        printMessage("Cannot send: not connected");
        return -1;
    }

    if (tlsEnabled && ssl) {
        int bytesSent = SSL_write(ssl.get(), data.c_str(), static_cast<int>(data.length()));
        if (bytesSent <= 0) {
            printMessage("SSL write error: " + std::string(ERR_error_string(ERR_get_error(), nullptr)));
            return -1;
        }
        printMessage("Sent " + std::to_string(bytesSent) + " bytes via SSL");
        return bytesSent;
    }

    std::size_t bytesSent = 0;
    sf::Socket::Status status = tcpSocket->send(data.c_str(), data.length(), bytesSent);
    if (status != sf::Socket::Status::Done) {
        printMessage("Error sending data: " + std::to_string(static_cast<int>(status)));
        return -1;
    }
    printMessage("Sent " + std::to_string(bytesSent) + " bytes");
    return static_cast<int>(bytesSent);
}

std::string TCPSock::receive() {
    if (!isConnected()) {
        printMessage("Cannot receive: not connected");
        return "";
    }

    char buffer[4096];
    std::size_t bytesRead = 0;

    if (tlsEnabled && ssl) {
        int result = SSL_read(ssl.get(), buffer, sizeof(buffer) - 1);
        if (result <= 0) {
            int error = SSL_get_error(ssl.get(), result);
            if (error == SSL_ERROR_ZERO_RETURN) {
                printMessage("SSL connection closed");
                connected = false;
            }
            else {
                printMessage("SSL read error: " + std::string(ERR_error_string(ERR_get_error(), nullptr)));
            }
            return "";
        }
        bytesRead = static_cast<std::size_t>(result);
    }
    else {
        sf::Socket::Status status = tcpSocket->receive(buffer, sizeof(buffer) - 1, bytesRead);
        if (status == sf::Socket::Status::Disconnected) {
            printMessage("Connection closed by peer");
            connected = false;
            return "";
        }
        if (status != sf::Socket::Status::Done) {
            printMessage("Error receiving data: " + std::to_string(static_cast<int>(status)));
            return "";
        }
    }

    buffer[bytesRead] = '\0';
    printMessage("Received " + std::to_string(bytesRead) + " bytes");
    return std::string(buffer, bytesRead);
}

bool TCPSock::bind(int bindPort) {
    if (connected || tcpListener) {
        printMessage("Cannot bind: socket already in use");
        return false;
    }

    if (bindPort > 0) {
        port = bindPort;
    }

    tcpListener = std::make_unique<sf::TcpListener>();
    tcpListener->setBlocking(true);

    printMessage("Binding to port " + std::to_string(port) + "...");
    sf::Socket::Status status = tcpListener->listen(static_cast<unsigned short>(port));
    if (status != sf::Socket::Status::Done) {
        printMessage("Error binding socket: " + std::to_string(static_cast<int>(status)));
        tcpListener.reset();
        return false;
    }

    isServer = true;
    printMessage("Successfully bound to port " + std::to_string(port));
    return true;
}

bool TCPSock::listen(int backlog) {
    if (!isServer || !tcpListener) {
        printMessage("Cannot listen: not a server socket");
        return false;
    }

    connected = true;
    printMessage("Listening for connections on port " + std::to_string(port));
    return true;
}

Connection* TCPSock::accept() {
    if (!isServer || !connected || !tcpListener) {
        printMessage("Cannot accept: not a listening server socket");
        return nullptr;
    }

    auto clientSocket = std::make_unique<TcpSocketWithHandle>();
    sf::Socket::Status status = tcpListener->accept(*clientSocket);

    if (status != sf::Socket::Status::Done) {
        if (thisServerRunning && status != sf::Socket::Status::NotReady) {
            printMessage("Error accepting connection: " + std::to_string(static_cast<int>(status)));
        }
        return nullptr;
    }

    sf::IpAddress clientAddr = clientSocket->getRemoteAddress();
    unsigned short clientPort = clientSocket->getRemotePort();

    printMessage("Accepted connection from " + clientAddr.toString() + ":" + std::to_string(clientPort));

    auto clientSock = new TCPSock(std::move(clientSocket), clientAddr.toString(), clientPort);

    if (tlsEnabled) {
        clientSock->tlsEnabled = true;

        if (!clientSock->setupTLS()) {
            printMessage("Failed to set up TLS for client connection");
            delete clientSock;
            return nullptr;
        }

        printMessage("TLS successfully set up for client connection");
    }

    return clientSock;
}
std::string TCPSock::getAddress() const {
    return address;
}

int TCPSock::getPort() const {
    return port;
}

bool TCPSock::enableTLS() {
    if (connected) {
        printMessage("Cannot enable TLS on already connected socket");
        return false;
    }
    tlsEnabled = true;
    printMessage("TLS enabled for " + address + ":" + std::to_string(port));
    return true;
}

bool TCPSock::isTLSEnabled() const {
    return tlsEnabled;
}

void TCPSock::setTimeout(int ms) {
    timeout = sf::milliseconds(ms);
    printMessage("Timeout set to " + std::to_string(ms) + " ms");
}

int TCPSock::getTimeout() const {
    return static_cast<int>(timeout.asMilliseconds());
}

bool TCPSock::setupTLS() {
    if (!tlsEnabled) {
        return false;
    }

    if (ssl) {
        return true;
    }

    printMessage("Setting up TLS for " + address + ":" + std::to_string(port) + "...");

    static bool sslInitialized = false;
    static std::mutex sslInitMutex;

    {
        std::lock_guard<std::mutex> lock(sslInitMutex);
        if (!sslInitialized) {
            SSL_load_error_strings();
            OpenSSL_add_ssl_algorithms();
            sslInitialized = true;
        }
    }

    const SSL_METHOD* method = isServer ? TLS_server_method() : TLS_client_method();
    sslCtx = std::unique_ptr<SSL_CTX, decltype(&SSL_CTX_free)>(
        SSL_CTX_new(method), SSL_CTX_free);

    if (!sslCtx) {
        printMessage("Failed to create SSL context: " + std::string(ERR_error_string(ERR_get_error(), nullptr)));
        return false;
    }

    SSL_CTX_set_options(sslCtx.get(), SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_TLSv1 | SSL_OP_NO_TLSv1_1);

    if (isServer) {
        
        const char* cert = certFile.empty() ? "server.crt" : certFile.c_str();
        const char* key = keyFile.empty() ? "server.key" : keyFile.c_str();

        printMessage("Loading certificate from: " + std::string(cert));
        printMessage("Loading private key from: " + std::string(key));

        if (SSL_CTX_use_certificate_file(sslCtx.get(), cert, SSL_FILETYPE_PEM) <= 0) {
            printMessage("Failed to load certificate: " + std::string(ERR_error_string(ERR_get_error(), nullptr)));
            return false;
        }

        if (SSL_CTX_use_PrivateKey_file(sslCtx.get(), key, SSL_FILETYPE_PEM) <= 0) {
            printMessage("Failed to load private key: " + std::string(ERR_error_string(ERR_get_error(), nullptr)));
            return false;
        }

        if (SSL_CTX_check_private_key(sslCtx.get()) != 1) {
            printMessage("Private key does not match certificate: " +
                std::string(ERR_error_string(ERR_get_error(), nullptr)));
            return false;
        }

        printMessage("Certificate and private key loaded successfully");
    }

    SSL_CTX_set_verify(sslCtx.get(), SSL_VERIFY_NONE, nullptr);

    ssl = std::unique_ptr<SSL, decltype(&SSL_free)>(SSL_new(sslCtx.get()), SSL_free);
    if (!ssl) {
        printMessage("Failed to create SSL object: " + std::string(ERR_error_string(ERR_get_error(), nullptr)));
        return false;
    }

    if (!tcpSocket) {
        printMessage("No valid socket for TLS setup");
        return false;
    }

    auto _tcpSocket = dynamic_cast<TcpSocketWithHandle*>(tcpSocket.get());
    if (!_tcpSocket) {
        printMessage("Failed to get socket handle for TLS");
        return false;
    }

    int sock = _tcpSocket->getHandle();
    if (sock < 0) {
        printMessage("Invalid socket handle: " + std::to_string(sock));
        return false;
    }

    if (SSL_set_fd(ssl.get(), sock) != 1) {
        printMessage("Failed to set SSL file descriptor: " + std::string(ERR_error_string(ERR_get_error(), nullptr)));
        return false;
    }

    printMessage("Performing SSL handshake...");
    int result;
    int retry_count = 0;
    const int max_retries = 3;

    while (retry_count < max_retries) {
        if (isServer) {
            result = SSL_accept(ssl.get());
        }
        else {
            result = SSL_connect(ssl.get());
        }

        if (result == 1) {
            break;
        }

        int error = SSL_get_error(ssl.get(), result);

        if (error == SSL_ERROR_WANT_READ || error == SSL_ERROR_WANT_WRITE) {
            retry_count++;
            printMessage("Retrying SSL handshake, attempt " + std::to_string(retry_count));
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        unsigned long errCode = ERR_get_error();
        char errBuffer[256];
        ERR_error_string_n(errCode, errBuffer, sizeof(errBuffer));

        printMessage("SSL handshake failed: " + std::string(errBuffer) + " (code: " + std::to_string(error) + ")");

        if (error == SSL_ERROR_SSL) {
            printMessage("Protocol error. Check if both sides are using SSL/TLS.");
        }
        else if (error == SSL_ERROR_SYSCALL) {
            printMessage("I/O error. Check if the connection was closed unexpectedly.");
        }

        return false;
    }

    if (result != 1) {
        printMessage("SSL handshake failed after " + std::to_string(max_retries) + " attempts");
        return false;
    }

    printMessage("SSL connection established using " +
        std::string(SSL_get_version(ssl.get())) +
        " with cipher " +
        std::string(SSL_get_cipher(ssl.get())));

    printMessage("TLS setup completed successfully for " + address + ":" + std::to_string(port));
    return true;
}

bool TCPSock::cleanupTLS() {
    if (ssl) {
        printMessage("Shutting down SSL connection...");
        SSL_shutdown(ssl.get());
    }
    ssl.reset();
    sslCtx.reset();
    return true;
}

bool TCPSock::runServer() {
    if (thisServerRunning) {
        printMessage("Server is already running");
        return false;
    }

    if (!bind(port) || !listen()) {
        printMessage("Failed to start server");
        return false;
    }

    thisServerRunning = true;
    std::thread serverThread([this]() {
        printMessage("TCP server started on port " + std::to_string(port));

        if (tcpListener) {
            tcpListener->setBlocking(false);
        }

        while (thisServerRunning) {
            auto conn = dynamic_cast<TCPSock*>(accept());
            if (!conn) {
                if (!thisServerRunning) break;
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }

            if (!conn->isConnected()) {
                printMessage("Discarding invalid connection");
                delete conn;
                continue;
            }

            printMessage("New client connection established from " + conn->getAddress() + ":" + std::to_string(conn->getPort()));

            std::unique_ptr<TCPSock> connPtr(conn);

            try 
            {
                std::thread clientThread(&TCPSock::handleClientRequest, this, std::move(connPtr));
                clientThread.detach();
            }
            catch (const std::exception& e) {
                printMessage("Failed to create client thread: " + std::string(e.what()));
            }
        }

        printMessage("TCP server stopped");
        });

    serverThread.detach();
    return true;
}

void TCPSock::stopServer() {
    if (thisServerRunning) {
        printMessage("Stopping server...");
        thisServerRunning = false;
        disconnect();
        tcpListener.reset();
        printMessage("Server stopped");
    }
}

bool TCPSock::isServerRunning() const {
    return thisServerRunning;
}

void TCPSock::handleClientRequest(std::unique_ptr<TCPSock> clientSock) {
    if (!clientSock || !clientSock->isConnected()) {
        printMessage("Invalid client connection");
        return;
    }

    std::string clientInfo = clientSock->getAddress() + ":" + std::to_string(clientSock->getPort());
    printMessage("Handling client request from " + clientInfo);

    clientSock->setTimeout(10000);

    try {
        while (clientSock->isConnected()) {
            std::string request = clientSock->receive();
            if (request.find("AUTH") == 0) {
                handleDBClient(std::move(clientSock), request);
            }
            else {
                clientSock->send("ERROR: Unknown protocol\n");
                clientSock->disconnect();
            }

            if (!clientSock->isConnected()) {
                printMessage("Client disconnected: " + clientInfo);
                break;
            }

            if (!request.empty()) {
                printMessage("Received " + std::to_string(request.length()) + " bytes from " + clientInfo);

                std::string response = "Server response: Received " + std::to_string(request.length()) + " bytes";
                int sent = clientSock->send(response);

                if (sent > 0) {
                    printMessage("Sent " + std::to_string(sent) + " bytes to " + clientInfo);
                }
                else {
                    printMessage("Failed to send response to " + clientInfo);
                    break;
                }
            }
            else {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        }
    }
    catch (const std::exception& e) {
        printMessage("Exception handling client connection " + clientInfo + ": " + std::string(e.what()));
    }

    clientSock->disconnect();
    printMessage("Client connection from " + clientInfo + " handled and closed");
}

void TCPSock::handleDBClient(std::unique_ptr<TCPSock> clientSock, const std::string& authLine)
{
    printMessage("Handling DBConn client");
    if (!clientSock || !clientSock->isConnected()) {
        printMessage("Invalid DB client");
        return;
    }

    std::string clientInfo = clientSock->getAddress() + ":" + std::to_string(clientSock->getPort());
    printMessage("Handling DBConn client from " + clientInfo);

    // === Parsare linie AUTH ===
    std::istringstream iss(authLine);
    std::string command, user, pass, dbname;
    iss >> command >> user >> pass >> dbname;

    if (command != "AUTH") {
        clientSock->send("ERROR: Expected AUTH command\n");
        return;
    }

    // === ODBC: conectare la baza de date ===
    SQLHENV hEnv;
    SQLHDBC hDbc;
    SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
    SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);
    SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);

    std::wstring connStr = L"DRIVER={SQL Server};SERVER=localhost;DATABASE=CyberRangeDB;UID=admin;PWD=adminhash;";

    SQLWCHAR outstr[1024];
    SQLSMALLINT outstrlen;

    SQLRETURN ret = SQLDriverConnectW(
        hDbc,
        NULL,
        (SQLWCHAR*)connStr.c_str(), // <== CONVERTIT la SQLWCHAR*
        SQL_NTS,
        outstr,
        sizeof(outstr),
        &outstrlen,
        SQL_DRIVER_COMPLETE
    );

    if (!SQL_SUCCEEDED(ret)) {
        clientSock->send("ERROR: Could not connect to database\n");
        printMessage("ODBC connection failed for " + dbname);
        SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
        SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
        return;
    }

    clientSock->send("OK: Authenticated and connected to DB\n");

    // === Primesc și execut comenzi SQL ===
    while (clientSock->isConnected()) {
        std::string query = clientSock->receive();
        if (query.empty()) break;

        SQLHSTMT hStmt;
        SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
        std::wstring wquery(query.begin(), query.end());
        SQLRETURN retExec = SQLExecDirectW(hStmt, (SQLWCHAR*)wquery.c_str(), SQL_NTS);

        if (SQL_SUCCEEDED(retExec)) {
            SQLSMALLINT columns;
            SQLNumResultCols(hStmt, &columns);

            std::string result;
            char buffer[1024];
            SQLLEN indicator;

            while (SQLFetch(hStmt) == SQL_SUCCESS) {
                for (int i = 1; i <= columns; ++i) {
                    SQLGetData(hStmt, i, SQL_C_CHAR, buffer, sizeof(buffer), &indicator);
                    result += std::string(buffer) + (i < columns ? " | " : "\n");
                }
            }

            if (result.empty()) result = "OK: Executed (no rows returned)\n";
            clientSock->send(result);
        }
        else {
            clientSock->send("ERROR: SQL execution failed\n");
        }

        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    }

    // === Cleanup ===
    SQLDisconnect(hDbc);
    SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
    SQLFreeHandle(SQL_HANDLE_ENV, hEnv);

    clientSock->disconnect();
    printMessage("DBConn client disconnected: " + clientInfo);
}