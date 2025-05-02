#include "ClientMng.h"

ClientMng* ClientMng::instance = nullptr;
std::string ClientMng::receiveResponse()
{
    std::string response = serverConn->receive();
    for (Controller* ctrl : controllers)
    {
        if (ctrl->validateResponse(response))
        {
            ctrl->handleServerResponse(response);
            break;
        }
    }
    return response;
}