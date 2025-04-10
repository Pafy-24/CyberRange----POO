#pragma once
#include "CController.h"

class LogClientController : public CController {
public:
    LogClientController();
    void fetchLogs();
};
