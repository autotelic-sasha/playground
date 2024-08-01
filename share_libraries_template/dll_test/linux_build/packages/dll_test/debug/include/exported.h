#pragma once
#include "dll_test_export_macros.h"

DLL_TEST_EXPORT int add(int i, int j);

DLL_TEST_EXPORT int subtract(int i, int j);

DLL_TEST_EXPORT int mul(int i, int j);

class DLL_TEST_EXPORT adder {
	int _i{ 0 };
	int _j{ 0 };
public:
	adder(int i_, int j_);
	int sum() const { return _i + _j; }
	int sum_product(int k);
};