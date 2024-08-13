
#include "autotelica_core/util/include/testing_util.h"
#include "autotelica_core/util/include/diagnostic_messages.h"
#include "{{appname}}_api.h"


namespace {{appname}} {
    template< bool = true> // declaring it as a template is a way to work aroud c++ limitations about declaring things in headers
    void examples() {
        // code here will only be run in example runs
        using namespace autotelica::diagnostic_messages;
        messages::message("Adding % to % gives %", 2, 2, 2+2);
    }
    template< bool = true> // declaring it as a template is a way to work aroud c++ limitations about declaring things in headers
    void tests() {
        // code here will be run in test, examples and record mode
        AF_TEST_RESULT(4, 2+2);
    }
}

AF_DECLARE_TEST_SET("{{appname}} tests", {{appname}},{{appname}}::examples<>() , {{appname}}::tests<>());