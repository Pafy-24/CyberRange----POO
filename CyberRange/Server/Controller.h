#pragma once
#include <string>

class Controller {
public:
	virtual void handleRequest(const std::string& data, Connection* client = nullptr) {
		try {
			client->handleRequest(data, client);
		}
		catch (const std::exception& e) {
			try {
				client->send("ERROR: No handler for request");
			}
			catch (...) {
				std::cerr << "Failed to send error response: " << e.what() << std::endl;
			}
		}
	}
};
