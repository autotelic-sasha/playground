#include <iostream>
#include "{{libname}}_examples.h"
#include "autotelica_core/util/include/test_runner_impl.h"

int main(int argc, const char* argv[])
{
	return autotelica::test_runner_impl::main_impl(argc, argv);
}

