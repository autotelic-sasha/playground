#include "asserts_examples.h"
#include "cl_parsing_examples.h"
#include "comparissons_examples.h"
#include "diagnostic_messages_examples.h"
#include "macro_magic_examples.h"
#include "std_disambiguation_examples.h"
#include "std_pretty_printing_examples.h"
#include "string_util_examples.h"
#include "testing_util_examples.h"
#include "timing_examples.h"
#include "enum_to_string_examples.h"

#include "test_runner_impl.h"
int main(int argc, const char* argv[])
{
	return autotelica::test_runner_impl::main_impl(argc, argv);
}