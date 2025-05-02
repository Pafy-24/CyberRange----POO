#pragma once
#include <string>

class Controller {
public:
	virtual void handleRequest(const std::string& data, Connection* client = nullptr) {
		try {
			client->handleRequest(data, client);
		}
		catch (...) {
			try {
				client->send("ERROR: No handler for request");
			}
			catch (...) {
				std::cerr << "Failed to send error response: " << std::endl;
			}
		}
	}
};
