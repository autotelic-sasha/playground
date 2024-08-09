#pragma once
#include "testing_util.h"
#include "string_util.h"
namespace autotelica {
    namespace examples {
        namespace string_util { 
            template< bool = true> // declaring it as a template is a way to work aroud c++ limitations about declaring things in headers
            void examples() {
                // code here will only be run in example runs
            }
            template< bool = true> // declaring it as a template is a way to work aroud c++ limitations about declaring things in headers
            void tests() {
                using namespace autotelica::string_util;

                AF_TEST_COMMENT("Replacing substrings.");
                AF_TEST_RESULT("Yes no No no no no nonono", replace("Yes no No yes yes no yesyesyes", "yes", "no"));
                AF_TEST_RESULT("Yes yes No yes yes yes yesyesyes", replace("Yes no No yes yes no yesyesyes", "no", "yes"));
                AF_TEST_RESULT("no no No no no no nonono", replace_nc("Yes no No yes yes no yesYESyes", "yes", "no"));
                AF_TEST_RESULT("Yes yes yes yes yes yes yesYESyes", replace_nc("Yes no No yes yes no yesYESyes", "no", "yes"));

                AF_TEST_COMMENT("Wildcard matching.");
                AF_TEST_RESULT(false, wildcard_match("Yes no No yes yes no .yesYESyes", ""));
                AF_TEST_RESULT(true, wildcard_match("Yes no No yes yes no .yesYESyes", "?es no * yes yes no .yesYESyes"));
                AF_TEST_RESULT(true, wildcard_match("Yes no No yes yes no .yesYESyes", "?es no * yes yes no .*"));
                AF_TEST_RESULT(false, wildcard_match("Yes no No yes yes no .yesYESyes", "?es no * yes yes no .*no"));
                AF_TEST_RESULT(true, wildcard_match("same", "same"));

                std::string s1("Yes no No yes yes no yesyesyes");
                std::string s2("no no no no no no");

                AF_TEST_COMMENT("Case insesitive comparissons of strings.");
                AF_TEST_RESULT(true, equal_nc(s1, s1));
                AF_TEST_RESULT(false, equal_nc(s1, s2));
                AF_TEST_RESULT(true, equal_nc(s1, s1));
                AF_TEST_RESULT(false, equal_nc(s1, s2));
                AF_TEST_RESULT(true, equal_nc(s1, s1));
                AF_TEST_RESULT(false, equal_nc(s1, s2));
                AF_TEST_RESULT(true, equal_nc(s1, s1));
                AF_TEST_RESULT(false, equal_nc(s1, s2));

                AF_TEST_RESULT(false, less_nc(s1, s1));
                AF_TEST_RESULT(true, less_nc(s2, s1));
                AF_TEST_RESULT(false, less_nc(s1, s1));
                AF_TEST_RESULT(false, less_nc(s1, s2));
                AF_TEST_RESULT(false, less_nc(s1, s1));
                AF_TEST_RESULT(true, less_nc(s2, s1));
                AF_TEST_RESULT(false, less_nc(s1, s1));
                AF_TEST_RESULT(false, less_nc(s1, s2));

                string_map_nc<int> m{ {"First",1}, {"second", 2}, {"THIRD", 3} };
                string_unordered_map_nc<int> um{ {"First",1}, {"second", 2}, {"THIRD", 3} };

                AF_TEST_COMMENT("Case insesitive string maps.");
                AF_TEST_RESULT(1, m["FIRST"]);
                AF_TEST_RESULT(1, m["first"]);
                AF_TEST_RESULT(3, m["third"]);
                AF_TEST_RESULT(1, um["FIRST"]);
                AF_TEST_RESULT(2, um["second"]);
                AF_TEST_RESULT(3, um["third"]);

                string_set_nc s{ "First", "second", "THIRD" };
                string_unordered_set_nc us{ "First", "second", "THIRD" };

                AF_TEST_COMMENT("Case insesitive string sets.");
                AF_TEST_RESULT(true, (s.find("first") != s.end()));
                AF_TEST_RESULT(false, (s.find("SECOND_") != s.end()));
                AF_TEST_RESULT(true, (s.find("third") != s.end()));
                AF_TEST_RESULT(true, (us.find("first") != us.end()));
                AF_TEST_RESULT(false, (us.find("SECOND_") != us.end()));
                AF_TEST_RESULT(true, (us.find("third") != us.end()));

                AF_TEST_COMMENT("String interpolation.");
                AF_TEST_RESULT(std::string("Test string formatted with a number 3 and a boolean true"),
                    af_format_string("Test % formatted with a number % and a boolean %", "string", 3, true));

                AF_TEST_COMMENT("Case conversions");
                AF_TEST_RESULT("YES YES YES YES YES YES YESYESYES", to_upper("Yes yes yes yes yes yes yesYESyes"));
                AF_TEST_RESULT("yes yes yes yes yes yes yesyesyes", to_lower("Yes yes yes yes yes yes yesYESyes"));

                AF_TEST_COMMENT("Trimming");
                AF_TEST_RESULT("Yes yes yes yes yes yes yesYESyes     ", ltrim("   Yes yes yes yes yes yes yesYESyes     "));
                AF_TEST_RESULT("   Yes yes yes yes yes yes yesYESyes", rtrim("   Yes yes yes yes yes yes yesYESyes     "));
                AF_TEST_RESULT("Yes yes yes yes yes yes yesYESyes", trim("   Yes yes yes yes yes yes yesYESyes     "));
                AF_TEST_RESULT("Yes yes yes yes yes yes yesYESyes###", ltrim("###Yes yes yes yes yes yes yesYESyes###", '#'));
                AF_TEST_RESULT("###Yes yes yes yes yes yes yesYESyes", rtrim("###Yes yes yes yes yes yes yesYESyes###", '#'));
                AF_TEST_RESULT("Yes yes yes yes yes yes yesYESyes", trim("###Yes yes yes yes yes yes yesYESyes###", '#'));

                AF_TEST_COMMENT("CSV");
                AF_TEST_RESULT("string: with \"quotes\",1,1.23,text,true", to_plain_csv_row_string("string: with \"quotes\"", 1, 1.23, "text", true));
                AF_TEST_RESULT("\"string:: with \"\"quotes\"\"\",1,1.23,text,true", to_excel_csv_row_string("string: with \"quotes\"", 1, 1.23, "text", true ));

                std::vector<int> vi{ 1,2,3,4,5 };
                std::vector<std::vector<std::string>> vs{ {"one_1", "two_1", "three_1"},{"one_2", "two_2", "three_2"},{"one_3", "two_3", "three_3"} };
                std::vector<double> sd{ 0, 0.123, 1.234, 2.45678 };
                AF_TEST_RESULT("1,2,3,4,5", to_csv(vi));
                AF_TEST_RESULT("one_1,two_1,three_1\none_2,two_2,three_2\none_3,two_3,three_3", to_csv(vs));
                AF_TEST_RESULT("0,0.123,1.234,2.45678", to_csv(sd));

                AF_TEST_RESULT(true, is_uppercase("UPPER"));
                AF_TEST_RESULT(false, is_uppercase("UpPeR"));
                AF_TEST_RESULT(true, is_lowercase("lower"));
                AF_TEST_RESULT(false, is_lowercase("loWer"));

                AF_TEST_RESULT(false, starts_with("short", "shorter"));
                AF_TEST_RESULT(true, starts_with("shorter", "shorter"));
                AF_TEST_RESULT(true, starts_with("shorter", "short"));
                AF_TEST_RESULT(false, ends_with("short", "shorter"));
                AF_TEST_RESULT(true, ends_with("shorter", "shorter"));
                AF_TEST_RESULT(true, ends_with("shorter", "ter"));

            }
        }
    }
}

AF_DECLARE_TEST_SET("StringUtil", string_util, 
      autotelica::examples::string_util::examples<>() , autotelica::examples::string_util::tests<>());

