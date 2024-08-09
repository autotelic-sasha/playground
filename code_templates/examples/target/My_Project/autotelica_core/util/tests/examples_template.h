#pragma once
#include "testing_util.h"
#include "THE_FILE_DECLARING_STUFF_YOURE_TESTING.H"
namespace autotelica {
    namespace examples {
        namespace NAMESPACE_NAME { 
            template< bool = true> // declaring it as a template is a way to work aroud c++ limitations about declaring things in headers
            void examples() {
                // code here will only be run in example runs
            }
            template< bool = true> // declaring it as a template is a way to work aroud c++ limitations about declaring things in headers
            void tests() {
                // code here will be run in test, examples and record mode
            }
        }
    }
}

// uncomment the line below to declare your test set
//AF_DECLARE_TEST_SET("NICE DESCRIPTION OF YOUR TEST", NAMESPACE_NAME, autotelica::examples::NAMESPACE_NAME::examples<>() , autotelica::examples::NAMESPACE_NAME::tests<>());

// remember to also register the test so you can run it nicely
// you do that by adding a line like this to test_registry
//AF_REGISTER_TEST_SET(NAMESPACE_NAME);

// also remember to delete these instructional comments when you are done
// it triggers some of us

