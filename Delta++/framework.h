#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>

#ifdef DPP_EXPORTS
#define DPP_API __declspec(dllexport)
#else
#define DPP_API __declspec(dllimport)
#endif