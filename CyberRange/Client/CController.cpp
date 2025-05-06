#include "pch.h"
#include "CController.h"
#include <iostream>

CController::CController(const std::string& name)
    : controllerName(name) {}

std::string CController::getControllerName() 
{
    return controllerName;
}

void CController::handleServerResponse(const std::string& response) 
{
    std::cout << "[" << controllerName << "] Handling response: " << response << "\n";
}

bool CController::validateResponse(const std::string& response) 
{
    return response.find(controllerName) != std::string::npos;
}