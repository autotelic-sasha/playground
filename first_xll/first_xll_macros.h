#pragma once
#include "framework.h"

#ifdef FIRSTXLL_EXPORTS
#define FIRSTXLL_API extern "C" __declspec(dllexport) 
#else
#define FIRSTXLL_API 
#endif