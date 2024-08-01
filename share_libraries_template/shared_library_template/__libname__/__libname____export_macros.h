#pragma once
//#define {{LIBNAME}}_BUILD during the building of this file


#ifdef _MSC_VER
#	ifdef {{LIBNAME}}_BUILD
#		define {{LIBNAME}}_EXPORT __declspec(dllexport) 
#		define {{LIBNAME}}_LOCAL   
#	else
#		define {{LIBNAME}}_EXPORT __declspec(dllimport) 
#		define {{LIBNAME}}_LOCAL   
#	endif
#else
#	ifdef __GNUC__
#		ifdef {{LIBNAME}}_BUILD
#			define {{LIBNAME}}_EXPORT  __attribute__ ((visibility ("default")))
#			define {{LIBNAME}}_LOCAL   __attribute__ ((visibility ("hidden")))
#		else
#			define {{LIBNAME}}_EXPORT  
#			define {{LIBNAME}}_LOCAL   
#		endif
#	else
#		error Unknown compiler.
#	endif
#endif
