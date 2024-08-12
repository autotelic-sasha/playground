#pragma once
#include "{{libname}}_export_macros.h"

/// This is where you declare functions and classes that are to be exported from the library.
/// The silly examples will be annoying, you need to remember to delete them, 
/// but it's nice to see how it all fits together. 
/// The function and class declared here are implemented in {{libname}}_api.cpp and 
/// tested in {{libname}}_test.cpp.
//
// an example function
{{LIBNAME}}_EXPORT int {{libname}}_add(int i, int j);

// an example class
class {{LIBNAME}}_EXPORT {{libname}}_adder {
	int _i{ 0 };
	int _j{ 0 };
public:
	{{libname}}_adder(int i_, int j_);
	int sum();
};