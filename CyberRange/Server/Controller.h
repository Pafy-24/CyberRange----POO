#pragma once
#include <string>
#include "CLogger.h"
#include "Connection.h"
#include "Observable.h"

class Controller:public Observable {
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
