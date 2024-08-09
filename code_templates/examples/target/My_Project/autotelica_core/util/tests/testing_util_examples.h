#pragma once
#include "testing_util.h"

namespace autotelica {
    namespace examples {
        namespace testing { 
            static int f_that_throws(int i = 0) {
                throw std::runtime_error("some error description");
                return 1;
            }
            static int f_that_doesnt_throw(int i = 0) {
                return i + 1;
            }
            template< bool = true> // declaring it as a template is a way to work aroud c++ limitations about declaring things in headers
            void examples() {
                // code here will only be run in example runs
            }
            template< bool = true> // declaring it as a template is a way to work aroud c++ limitations about declaring things in headers
            void tests() {
                //AF_TEST_FUNCTION(f_that_throws, 1);
                AF_TEST_FUNCTION(f_that_doesnt_throw, 1);
                AF_TEST_FUNCTION([](int i) {  return i + 1; }, 1);
                //AF_TEST_FUNCTION_NO_PARAMS(f_that_throws);
                AF_TEST(f_that_doesnt_throw());

                AF_TEST_THROWS(f_that_throws());
                //AF_TEST_FUNCTION_THROWS_NO_PARAMS(f_that_doesnt_throw);

                AF_TEST_FUNCTION_THROWS(f_that_throws, 1);
                AF_TEST_THROWS({ throw std::runtime_error("Snippet threw an error"); });
                int c = 1;
                auto error_lambda = []() {throw std::runtime_error("c is too big"); return 0; };
                AF_TEST_THROWS(((c < 1) ? 0 : error_lambda()));

                AF_TEST_RESULT(1, c);

                AF_TEST_FUNCTION_RESULT(2, f_that_doesnt_throw, 1);
                //AF_TEST_FUNCTION_RESULT(3, f_that_doesnt_throw, 1);
                AF_TEST_FUNCTION_RESULT(2, [](int i) {  return i + 1; }, 1);
                //AF_TEST_FUNCTION_RESULT(1, [](int i) {  return i + 1; }, 1);
                //AF_TEST_FUNCTION_RESULT_NO_PARAMS(0, f_that_throws);
                AF_TEST_RESULT(1, f_that_doesnt_throw());
                //AF_TEST_FUNCTION_RESULT_NO_PARAMS(2, f_that_doesnt_throw);
            }
        }
    }
}

AF_DECLARE_TEST_SET("TestingTesting", testing, 
    autotelica::examples::testing::examples<>() , autotelica::examples::testing::tests<>());


