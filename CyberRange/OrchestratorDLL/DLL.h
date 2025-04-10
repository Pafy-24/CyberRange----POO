#pragma once
#ifdef ORCHESTRATOR_DLL_EXPORTS
#define ORCHESTRATOR_API __declspec(dllexport)
#else
#define ORCHESTRATOR_API __declspec(dllimport)
#endif