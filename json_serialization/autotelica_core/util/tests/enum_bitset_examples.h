#pragma once
#include "testing_util.h"
#include "enum_bitset.h"
#include "enum_to_string.h"
#include "std_pretty_printing.h"
#include <set>
namespace autotelica {
    namespace examples {
        namespace enum_bitsets {
            enum class ExampleEnumClass {
                value1,
                value2,
                value3,
                none
            };
            enum ExampleEnum {
                e_value1,
                e_value2,
                e_value3,
                none
            };
            AF_ENUM_TO_STRING(ExampleEnumClass,
                ExampleEnumClass::value1, "value1",
                ExampleEnumClass::value2, "value2",
                ExampleEnumClass::value3, "value3",
                ExampleEnumClass::none, "none"
            );
            AF_ENUM_TO_STRING_T(ExampleEnum, std::string,
                ExampleEnum::e_value1, "e_value1",
                ExampleEnum::e_value2, "e_value2",
                ExampleEnum::e_value3, "e_value3",
                ExampleEnum::e_value3, "none"
            );
            template< bool = true> // declaring it as a template is a way to work aroud c++ limitations about declaring things in headers
            void examples() {
                // code here will only be run in example runs

            }
            template< bool = true> // declaring it as a template is a way to work aroud c++ limitations about declaring things in headers
            void tests() {
                // code here will be run in test, examples and record mode
                using namespace autotelica::enum_bitsets;
                enum_bitset<ExampleEnumClass> bs1;
                AF_TEST_RESULT("0000", bs1.to_string());
                bs1.set(ExampleEnumClass::value1);
                AF_TEST_RESULT("0001", bs1.to_string());
                bool v = bs1[ExampleEnumClass::value1];
                AF_TEST_RESULT(true, v);
                AF_TEST_RESULT(false, bs1.test(ExampleEnumClass::value2));
                bs1.set();
                AF_TEST_RESULT("1111", bs1.to_string());
                bs1.reset();
                AF_TEST_RESULT("0000", bs1.to_string());
                bs1.flip();
                AF_TEST_RESULT("1111", bs1.to_string());
                bs1.flip(0);
                AF_TEST_RESULT("1110", bs1.to_string());
                bs1.flip(ExampleEnumClass::value2);
                AF_TEST_RESULT("1100", bs1.to_string());
                bs1.reset(ExampleEnumClass::value3);
                AF_TEST_RESULT("1000", bs1.to_string());
                AF_TEST_RESULT(true, bs1.any());
                AF_TEST_RESULT(false, bs1.all());
                bs1.set(ExampleEnumClass::none, false);
                AF_TEST_RESULT("0000", bs1.to_string());
                AF_TEST_RESULT(true, bs1.none());
                AF_TEST_RESULT(0, bs1.to_ulong());
                AF_TEST_RESULT(0, bs1.to_ullong());
                enum_bitset<ExampleEnumClass> bs2;
                AF_TEST_RESULT(0, bs2.to_ulong());
                enum_bitset<ExampleEnumClass> bs3({ExampleEnumClass::value1,ExampleEnumClass::value2 });
                std::set<ExampleEnumClass> enum_set({ ExampleEnumClass::value1, ExampleEnumClass::value2 });
                auto res = bs3.to_enum_set();
                using namespace autotelica::std_pretty_printing;
                AF_TEST_RESULT(pretty_s(enum_set), pretty_s(res));
                AF_TEST_RESULT("0011", bs3.to_string());
                auto bs4 = bs2 & bs3;
                AF_TEST_RESULT("0000", bs4.to_string());
                auto bs5 = bs2 | bs3;
                AF_TEST_RESULT("0011", bs5.to_string());
                auto bs6 = bs3 ^ bs3;
                AF_TEST_RESULT("0000", bs6.to_string());
                enum_bitset<ExampleEnumClass> bs7("1010");
                bs7 ^= bs3;
                AF_TEST_RESULT("1001", bs7.to_string());
                bs7 |= bs3;
                AF_TEST_RESULT("1011", bs7.to_string());
                bs7 &= bs3;
                AF_TEST_RESULT("0011", bs7.to_string());

                std::bitset<4> raw("0001");
                enum_bitset_ref<ExampleEnum> bsref1(raw);
                AF_TEST_RESULT(true, bsref1.test(ExampleEnum::e_value1));
                AF_TEST_RESULT(false, bsref1.test(ExampleEnum::e_value2));
                bsref1.set(ExampleEnum::e_value2);
                AF_TEST_RESULT(true, bsref1.test(ExampleEnum::e_value2));
                AF_TEST_RESULT("0011", bsref1.to_string());
                AF_TEST_RESULT(true, bsref1.any());
                AF_TEST_RESULT(false, bsref1.none());
                AF_TEST_RESULT(false, bsref1.all());
                bsref1.flip();
                AF_TEST_RESULT("1100", bsref1.to_string());
                AF_TEST_RESULT(true, bsref1.test(ExampleEnum::e_value3));
                bsref1.flip(ExampleEnum::e_value3);
                AF_TEST_RESULT("1000", bsref1.to_string());
                AF_TEST_RESULT(false, bsref1.test(ExampleEnum::e_value3));
                bsref1.reset();
                AF_TEST_RESULT("0000", bsref1.to_string());
                AF_TEST_RESULT(false, bsref1.any());
                AF_TEST_RESULT(true, bsref1.none());
                AF_TEST_RESULT(false, bsref1.all());

            }
        }
    }
}

AF_DECLARE_TEST_SET("EnumBitsets", enum_bitsets,
    autotelica::examples::enum_bitsets::examples<>(), autotelica::examples::enum_bitsets::tests<>());


