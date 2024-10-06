#pragma once
#include "testing_util.h"
#include "enum_to_string.h"
namespace autotelica {
    namespace examples {
        namespace enum_to_string { 
            enum class ExampleEnumClass{
                value1,
                value2,
                value3
            };
            enum ExampleEnum{
                e_value1,
                e_value2,
                e_value3
            };
            enum class ExampleEnumClassW {
                w_value1,
                w_value2,
                w_value3
            };

            AF_ENUM_TO_STRING(ExampleEnumClass,
                ExampleEnumClass::value1, "value1",
                ExampleEnumClass::value2, "value2",
                ExampleEnumClass::value3, "value3"
            );
            AF_ENUM_TO_STRING_T(ExampleEnum, std::wstring,
                ExampleEnum::e_value1, L"e_value1",
                ExampleEnum::e_value2, L"e_value2",
                ExampleEnum::e_value3, L"e_value3"
            );
            AF_ENUM_TO_STRING_W(ExampleEnumClassW,
                ExampleEnumClassW::w_value1, L"w_value1",
                ExampleEnumClassW::w_value2, L"w_value2",
                ExampleEnumClassW::w_value3, L"w_value3"
            );


            template< bool = true> // declaring it as a template is a way to work aroud c++ limitations about declaring things in headers
            void examples() {
                // code here will only be run in example runs

            }
            template< bool = true> // declaring it as a template is a way to work aroud c++ limitations about declaring things in headers
            void tests() {
                // code here will be run in test, examples and record mode
                using namespace autotelica::enum_to_string;
                AF_TEST_RESULT(L"value1", to_wstring(ExampleEnumClass::value1));
                AF_TEST_RESULT("value1", to_string(ExampleEnumClass::value1));
                AF_TEST_RESULT("value2", to_string(ExampleEnumClass::value2));
                AF_TEST_RESULT("value3", to_string(ExampleEnumClass::value3));

                AF_TEST_RESULT("value1", to_string(to_enum<ExampleEnumClass>("value1")));
                AF_TEST_RESULT(L"value1", to_wstring(to_enum<ExampleEnumClass>(L"value1"))); 
                AF_TEST_RESULT("value2", to_string(to_enum<ExampleEnumClass>("value2")));
                AF_TEST_RESULT("value3", to_string(to_enum<ExampleEnumClass>("value3")));

                AF_TEST_RESULT("value1", to_string(to_enum<ExampleEnumClass>(std::string("value1"))));
                AF_TEST_RESULT("value2", to_string(to_enum<ExampleEnumClass>(std::string("value2"))));
                AF_TEST_RESULT("value3", to_string(to_enum<ExampleEnumClass>(std::string("value3"))));

                AF_TEST_RESULT(L"e_value1", to_wstring(ExampleEnum::e_value1));
                AF_TEST_RESULT(L"e_value2", to_wstring(ExampleEnum::e_value2));
                AF_TEST_RESULT(L"e_value3", to_wstring(ExampleEnum::e_value3));

                AF_TEST_RESULT(L"w_value1", to_wstring(ExampleEnumClassW::w_value1));
                AF_TEST_RESULT(L"w_value2", to_wstring(ExampleEnumClassW::w_value2));
                AF_TEST_RESULT("w_value3", to_string(ExampleEnumClassW::w_value3));
                
                ExampleEnumClass e;
                to_enum(e, "value1");
                AF_TEST_RESULT(ExampleEnumClass::value1, e);
                to_enum(e, L"value3");
                AF_TEST_RESULT(ExampleEnumClass::value3, e);

                std::string res;
                to_string(res, ExampleEnumClass::value1);
                AF_TEST_RESULT("value1", res);
                std::wstring wres;
                to_string(wres, ExampleEnumClass::value3);
                AF_TEST_RESULT(L"value3", wres);
            }
        }
    }
}

AF_DECLARE_TEST_SET("EnumToString", enum_to_string, 
    autotelica::examples::enum_to_string::examples<>() , autotelica::examples::enum_to_string::tests<>());


