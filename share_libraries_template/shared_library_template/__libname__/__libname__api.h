#pragma once
#include "{{libname}}_export_macros.h"

{{LIBNAME}}_EXPORT int add(int i, int j);

{{LIBNAME}}_EXPORT int subtract(int i, int j);

{{LIBNAME}}_EXPORT int mul(int i, int j);

class {{LIBNAME}}_EXPORT adder {
	int _i{ 0 };
	int _j{ 0 };
public:
	adder(int i_, int j_);
	int sum() const { return _i + _j; }
	int sum_product(int k);
};