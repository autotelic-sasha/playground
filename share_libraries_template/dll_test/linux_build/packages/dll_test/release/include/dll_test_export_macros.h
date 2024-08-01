#pragma once
//#define DLL_TEST_BUILD


#ifdef _MSC_VER
#	ifdef DLL_TEST_BUILD
#		define DLL_TEST_EXPORT __declspec(dllexport) 
#		define DLL_TEST_LOCAL   
#	else
#		define DLL_TEST_EXPORT __declspec(dllimport) 
#		define DLL_TEST_LOCAL   
#	endif
#else
#	ifdef __GNUC__
#		ifdef DLL_TEST_BUILD
#			define DLL_TEST_EXPORT  __attribute__ ((visibility ("default")))
#			define DLL_TEST_LOCAL   __attribute__ ((visibility ("hidden")))
#		else
#			define DLL_TEST_EXPORT  
#			define DLL_TEST_LOCAL   
#		endif
#	else
#		error Unknown compiler.
#	endif
#endif
