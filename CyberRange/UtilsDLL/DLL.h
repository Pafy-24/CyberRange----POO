#pragma once

#ifdef UTILS_DLL_EXPORTS
#define UTILS_API __declspec(dllexport)
#else
#define UTILS_API __declspec(dllimport)
#endif

#include <atomic>
#include <mutex>
#include <iostream>
#include <thread>
#include <string>

// Thread-safe message printing utility
// Export/import the global variables
extern UTILS_API std::mutex consoleMutex;
extern UTILS_API std::atomic<bool> serverRunning;

// For inline functions used across DLL boundaries, we need special handling
// Option 1: Make it non-inline with export/import
UTILS_API void printMessage(const std::string& message);

//// Option 2: Keep it inline but ensure it's properly exported
//// This is trickier as inline functions need their implementation visible everywhere
//#ifdef UTILS_DLL_EXPORTS
//// In DLL implementation, define the function normally
//inline void printMessageInline(const std::string& message) {
//    std::lock_guard<std::mutex> lock(consoleMutex);
//    std::cout << "[" << std::this_thread::get_id() << "] " << message << std::endl;
//}
//#else
//// When used outside the DLL, declare it as extern
//extern UTILS_API void printMessageInline(const std::string& message);
//#endif