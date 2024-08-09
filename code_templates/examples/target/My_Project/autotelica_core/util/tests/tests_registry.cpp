#include "testing_util.h"
#include "testing_util_examples.h"
#include "std_disambiguation_examples.h"
#include "std_pretty_printing_examples.h"
#include "macro_magic_examples.h"
#include "string_util_examples.h"
#include "asserts_examples.h"
#include "comparissons_examples.h"
#include "diagnostic_messages_examples.h"
#include "cl_parsing_examples.h"

namespace autotelica {
	namespace tests {
		namespace registry {
			AF_REGISTER_TEST_SET(testing);
			AF_REGISTER_TEST_SET(std_disambiguation);
			AF_REGISTER_TEST_SET(std_pretty_printing);
			AF_REGISTER_TEST_SET(macro_magic);
			AF_REGISTER_TEST_SET(string_util);
			AF_REGISTER_TEST_SET(asserts);
			AF_REGISTER_TEST_SET(comparissons);
			AF_REGISTER_TEST_SET(diagnostic_messages);
			AF_REGISTER_TEST_SET(cl_parsing);
			// 
			//AF_REGISTER_TEST_SET(TEST_SET_NAMESPACE);
		}
	}
}