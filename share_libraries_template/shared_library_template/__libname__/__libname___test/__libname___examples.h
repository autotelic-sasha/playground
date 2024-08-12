
#include "autotelica_core/util/include/testing_util.h"
#include "autotelica_core/util/include/diagnostic_messages.h"
#include "{{libname}}_api.h"


namespace {{libname}} {
    template< bool = true> // declaring it as a template is a way to work aroud c++ limitations about declaring things in headers
    void examples() {
        // code here will only be run in example runs
        using namespace autotelica::diagnostic_messages;
        messages::message("Adding % to % gives %", 2, 2, add(2, 2));
        messages::message("Adding % to % gives % (using a class this time)", 3, 3, adder(3, 3).sum());
    }
    template< bool = true> // declaring it as a template is a way to work aroud c++ limitations about declaring things in headers
    void tests() {
        // code here will be run in test, examples and record mode
        AF_TEST_RESULT(4, add(2, 2));
        AF_TEST_RESULT(6, adder(3, 3).sum());
    }
}

AF_DECLARE_TEST_SET("{{libname}} tests", {{libname}},{{libname}}::examples<>() , {{libname}}::tests<>());