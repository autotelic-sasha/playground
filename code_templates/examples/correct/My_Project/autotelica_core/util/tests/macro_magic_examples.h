#pragma once
#include "testing_util.h"
#include "macro_magic.h"
namespace autotelica {
    namespace examples {
        namespace macro_magic { 
            std::string f(const char* a, const char* b, const char* c){
                std::stringstream s;
                s << a << ", " << b << ", " << c;
                std::cout << s.str().c_str() << std::endl; 
                return s.str();
            }
            template< bool = true> // declaring it as a template is a way to work aroud c++ limitations about declaring things in headers
            void examples() {
                // code here will only be run in example runs
            }
            template< bool = true> // declaring it as a template is a way to work aroud c++ limitations about declaring things in headers
            void tests() {
                std::string v("1");
               
                AF_TEST_COMMENT("STRINGIFY_VA_ARGS stringifies arguments passed to a variadic template.");


#define MM_VARIADIC_TEMPLATE(...)  STRINGIFY_VA_ARGS(__VA_ARGS__)

                AF_TEST_RESULT(v.c_str(), STRINGIFY_VA_ARGS(1));
                AF_TEST_RESULT("(1, 2, 3)", STRINGIFY_VA_ARGS((1, 2, 3)));
                AF_TEST_RESULT("(1, 2, 3, ({ 1,2,3 }))", STRINGIFY_VA_ARGS((1, 2, 3, ({ 1,2,3 }))));

                AF_TEST_RESULT(v.c_str(), MM_VARIADIC_TEMPLATE(1));
                AF_TEST_RESULT("1, 2, 3", MM_VARIADIC_TEMPLATE(1, 2, 3));
                AF_TEST_RESULT("(1, 2, 3, ({ 1,2,3 }))", MM_VARIADIC_TEMPLATE((1, 2, 3, ({ 1,2,3 }))));

                AF_TEST_COMMENT("NAME_WITH_LINE creates a variable name with __LINE__ appended to the name");
                auto l = []() {int NAME_WITH_LINE(v) = 1; return NAME_WITH_LINE(v) + 1; }; l();
                AF_TEST_RESULT(2, l() );
            }
        }
    }
}


AF_DECLARE_TEST_SET("MacroMagic", macro_magic, 
    autotelica::examples::macro_magic::examples<>() , autotelica::examples::macro_magic::tests<>());

