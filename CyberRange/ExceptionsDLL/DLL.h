#pragma once
#ifdef EXCEPTIONS_DLL_EXPORTS
#define EXCEPTIONS_API __declspec(dllexport)
#else
#define EXCEPTIONS_API __declspec(dllimport)
#endif