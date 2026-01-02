#pragma once
#include "testing_util.h"
#include "std_pretty_printing.h"

namespace autotelica {
    namespace examples {
        namespace std_pretty_printing { 
            template< bool = true> // declaring it as a template is a way to work aroud c++ limitations about declaring things in headers
            void examples() {
                // code here will only be run in example runs
            }
            void string_tests() {
                using namespace autotelica::std_pretty_printing;

                std::vector<double> vector_{ 1.23, 0.32435, 4.5656, 0.120981280932481 };
                std::vector<std::vector<int>> vector_vector_{ { 1,2,3}, {2,2,3}, {3,2,3,4}, {1,2,3,4,5} };
                std::deque<int> deque_ = { 7, 5, 16, 8 };
                std::forward_list<std::string> forward_list_ = { "7", "5", "16", "8" };
                std::list<bool> list_ = { true, false, true, true };

                std::set<int> set_{ 1,1,2,3 };
                std::multiset<int> multiset_{ 1,1,2,3 };
                std::map<std::string, int> map_{ {"First",2}, {"Second",3234324}, {"Third",44} };
                std::map<std::string, std::vector<double>> map_vectors_{ {"First",vector_}, {"Second",vector_}, {"Third",vector_} };
                std::multimap<std::string, int> multimap_{ {"First",1}, {"First",2}, {"Second",3}, {"Second",3234324}, {"Third",44} };

                std::unordered_set<int> unordered_set_{ 1,1,2,3 };
                std::unordered_multiset<int> unordered_multiset_{ 1,1,2,3 };
                std::unordered_map<std::string, int> unordered_map_{ {"First",2}, {"Second",3234324}, {"Third",44} };
                std::unordered_multimap<std::string, int> unordered_multimap_{ {"First",1}, {"First",2}, {"Second",3}, {"Second",3234324}, {"Third",44} };

                std::stack<int> stack_{ deque_ };
                std::queue<int> queue_{ deque_ };
                std::priority_queue<double> priority_queue_{ vector_.begin(), vector_.end() };

                std::string string_{ "test string" };

                std::vector<std::vector<std::string>> string_grid{
                    {"Title Column1", "Title Column2", "Title Column3"},
                    {"Row1 Column1", "Row1Column2", "Row1Column3"},
                    {"Row2 Column1", "Row2 Column2", "Row3 Column3"},
                    {"Row3 Column1", "Row3 Column2", "Row3 Column3"},

                };

                // pretty printing
                AF_TEST_COMMENT("pretty printing");
                AF_TEST_RESULT(std::string("123"), pretty_s(123));
                AF_TEST_RESULT(std::string("0.1234"), pretty_s(0.1234));
                AF_TEST_RESULT(std::string("[1,2,3]"), pretty_s<int[3]>({ 1,2,3 }));
                AF_TEST_RESULT(std::string("[[1,2,3,4],[2,2,3,4],[3,2,3,4]]"), pretty_s<int[3][4]>({ {1,2,3,4},{2,2,3,4},{3,2,3,4} }));
                AF_TEST_RESULT(std::string("raw string"), pretty_s("raw string"));
                AF_TEST_RESULT(std::string("[1.23,0.32435,4.5656,0.120981]"), pretty_s(vector_));
                AF_TEST_RESULT(std::string("[[1,2,3],[2,2,3],[3,2,3,4],[1,2,3,4,5]]"), pretty_s(vector_vector_));
                AF_TEST_RESULT(std::string("[7,5,16,8]"), pretty_s(deque_));
                AF_TEST_RESULT(std::string("[\"7\",\"5\",\"16\",\"8\"]"), pretty_s(forward_list_));
                AF_TEST_RESULT(std::string("[true,false,true,true]"), pretty_s(list_));
                AF_TEST_RESULT(std::string("{1,2,3}"), pretty_s(set_));
                AF_TEST_RESULT(std::string("{1,1,2,3}"), pretty_s(multiset_));
                AF_TEST_RESULT(std::string("{{\"First\",2},{\"Second\",3234324},{\"Third\",44}}"), pretty_s(map_));
                AF_TEST_RESULT(std::string("{{\"First\",[1.23,0.32435,4.5656,0.120981]},{\"Second\",[1.23,0.32435,4.5656,0.120981]},{\"Third\",[1.23,0.32435,4.5656,0.120981]}}"), pretty_s(map_vectors_));
                AF_TEST_RESULT(std::string("{{\"First\",1},{\"First\",2},{\"Second\",3},{\"Second\",3234324},{\"Third\",44}}"), pretty_s(multimap_));
#ifdef __GNUG__
                // gcc sorts unorederd containers differently
                AF_TEST_RESULT(std::string("{3,2,1}"), pretty_s(unordered_set_));
                AF_TEST_RESULT(std::string("{3,2,1,1}"), pretty_s(unordered_multiset_));
                AF_TEST_RESULT(std::string("{{\"Third\",44},{\"Second\",3234324},{\"First\",2}}"), pretty_s(unordered_map_));
                AF_TEST_RESULT(std::string("{{\"Third\",44},{\"Second\",3234324},{\"Second\",3},{\"First\",2},{\"First\",1}}"), pretty_s(unordered_multimap_));
#else
                AF_TEST_RESULT(std::string("{1,2,3}"), pretty_s(unordered_set_));
                AF_TEST_RESULT(std::string("{1,1,2,3}"), pretty_s(unordered_multiset_));
                AF_TEST_RESULT(std::string("{{\"First\",2},{\"Second\",3234324},{\"Third\",44}}"), pretty_s(unordered_map_));
                AF_TEST_RESULT(std::string("{{\"First\",1},{\"First\",2},{\"Second\",3},{\"Second\",3234324},{\"Third\",44}}"), pretty_s(unordered_multimap_));
#endif
                AF_TEST_RESULT(std::string("[8,16,5,7]"), pretty_s(stack_));
                AF_TEST_RESULT(std::string("[7,5,16,8]"), pretty_s(queue_));
                AF_TEST_RESULT(std::string("[4.5656,1.23,0.32435,0.120981]"), pretty_s(priority_queue_));
                AF_TEST_RESULT(std::string("\"test string\""), pretty_s(string_));

                // table printing
                AF_TEST_COMMENT("table printing");
                AF_TEST_RESULT(std::string("0 : 123\n"), table_s(123));
                AF_TEST_RESULT(std::string("0 : 0.1234\n"), table_s(0.1234));
                AF_TEST_RESULT(std::string("0 : raw string\n"), table_s("raw string"));
                AF_TEST_RESULT(std::string("0 :     1.23\n1 :  0.32435\n2 :   4.5656\n3 : 0.120981\n"), table_s(vector_));
                AF_TEST_RESULT(std::string("1 | 2 | 3 |   |  \n2 | 2 | 3 |   |  \n3 | 2 | 3 | 4 |  \n1 | 2 | 3 | 4 | 5\n"), table_s(vector_vector_));
                AF_TEST_RESULT(std::string("0 :  7\n1 :  5\n2 : 16\n3 :  8\n"), table_s(deque_));
                AF_TEST_RESULT(std::string("0 :  \"7\"\n1 :  \"5\"\n2 :  \"16\"\n3 :  \"8\"\n"), table_s(forward_list_));
                AF_TEST_RESULT(std::string("0 : true\n1 : false\n2 : true\n3 : true\n"), table_s(list_));
                AF_TEST_RESULT(std::string("0 : 1\n1 : 2\n2 : 3\n"), table_s(set_));
                AF_TEST_RESULT(std::string("0 : 1\n1 : 1\n2 : 2\n3 : 3\n"), table_s(multiset_));
                AF_TEST_RESULT(std::string("First  :       2\nSecond : 3234324\nThird  :      44\n"), table_s(map_));
                AF_TEST_RESULT(std::string("First  : [1.23,0.32435,4.5656,0.120981]\nSecond : [1.23,0.32435,4.5656,0.120981]\nThird  : [1.23,0.32435,4.5656,0.120981]\n"), table_s(map_vectors_));
                AF_TEST_RESULT(std::string("First  :       1\nFirst  :       2\nSecond :       3\nSecond : 3234324\nThird  :      44\n"), table_s(multimap_));
#ifdef __GNUG__
                // gcc sorts unorederd containers differently
                AF_TEST_RESULT(std::string("0 : 3\n1 : 2\n2 : 1\n"), table_s(unordered_set_));
                AF_TEST_RESULT(std::string("0 : 3\n1 : 2\n2 : 1\n3 : 1\n"), table_s(unordered_multiset_));
                AF_TEST_RESULT(std::string("Third  :      44\nSecond : 3234324\nFirst  :       2\n"), table_s(unordered_map_));
                AF_TEST_RESULT(std::string("Third  :      44\nSecond : 3234324\nSecond :       3\nFirst  :       2\nFirst  :       1\n"), table_s(unordered_multimap_));
#else
                AF_TEST_RESULT(std::string("0 : 1\n1 : 2\n2 : 3\n"), table_s(unordered_set_));
                AF_TEST_RESULT(std::string("0 : 1\n1 : 1\n2 : 2\n3 : 3\n"), table_s(unordered_multiset_));
                AF_TEST_RESULT(std::string("First  :       2\nSecond : 3234324\nThird  :      44\n"), table_s(unordered_map_));
                AF_TEST_RESULT(std::string("First  :       1\nFirst  :       2\nSecond :       3\nSecond : 3234324\nThird  :      44\n"), table_s(unordered_multimap_));
#endif
                AF_TEST_RESULT(std::string("0 :  8\n1 : 16\n2 :  5\n3 :  7\n"), table_s(stack_));
                AF_TEST_RESULT(std::string("0 :  7\n1 :  5\n2 : 16\n3 :  8\n"), table_s(queue_));
                AF_TEST_RESULT(std::string("0 :   4.5656\n1 :     1.23\n2 :  0.32435\n3 : 0.120981\n"), table_s(priority_queue_));
                AF_TEST_RESULT(std::string("0 : \"test string\"\n"), table_s(string_));

                AF_TEST_RESULT("Title Column1 | Title Column2 | Title Column3\nRow1 Column1  | Row1Column2   | Row1Column3  \nRow2 Column1  | Row2 Column2  | Row3 Column3 \nRow3 Column1  | Row3 Column2  | Row3 Column3 \n", table_s(string_grid));
                AF_TEST_RESULT("Title Column1 | Title Column2 | Title Column3\n================================================\nRow1 Column1  | Row1Column2   | Row1Column3  \nRow2 Column1  | Row2 Column2  | Row3 Column3 \nRow3 Column1  | Row3 Column2  | Row3 Column3 \n", table_s(string_grid, true));
            }
            void wstring_tests() {
                using namespace autotelica::std_pretty_printing;

                std::vector<double> vector_{ 1.23, 0.32435, 4.5656, 0.120981280932481 };
                std::vector<std::vector<int>> vector_vector_{ { 1,2,3}, {2,2,3}, {3,2,3,4}, {1,2,3,4,5} };
                std::deque<int> deque_ = { 7, 5, 16, 8 };
                std::forward_list<std::wstring> forward_list_ = { L"7", L"5", L"16", L"8" };
                std::list<bool> list_ = { true, false, true, true };

                std::set<int> set_{ 1,1,2,3 };
                std::multiset<int> multiset_{ 1,1,2,3 };
                std::map<std::wstring, int> map_{ {L"First",2}, {L"Second",3234324}, {L"Third",44} };
                std::map<std::wstring, std::vector<double>> map_vectors_{ {L"First",vector_}, {L"Second",vector_}, {L"Third",vector_} };
                std::multimap<std::wstring, int> multimap_{ {L"First",1}, {L"First",2}, {L"Second",3}, {L"Second",3234324}, {L"Third",44} };

                std::unordered_set<int> unordered_set_{ 1,1,2,3 };
                std::unordered_multiset<int> unordered_multiset_{ 1,1,2,3 };
                std::unordered_map<std::wstring, int> unordered_map_{ {L"First",2}, {L"Second",3234324}, {L"Third",44} };
                std::unordered_multimap<std::wstring, int> unordered_multimap_{ {L"First",1}, {L"First",2}, {L"Second",3}, {L"Second",3234324}, {L"Third",44} };

                std::stack<int> stack_{ deque_ };
                std::queue<int> queue_{ deque_ };
                std::priority_queue<double> priority_queue_{ vector_.begin(), vector_.end() };

                std::wstring string_{ L"test string" };

                std::vector<std::vector<std::wstring>> string_grid{
                    {L"Title Column1", L"Title Column2", L"Title Column3"},
                    {L"Row1 Column1", L"Row1Column2", L"Row1Column3"},
                    {L"Row2 Column1", L"Row2 Column2", L"Row3 Column3"},
                    {L"Row3 Column1", L"Row3 Column2", L"Row3 Column3"},

                };

                // pretty printing
                AF_TEST_COMMENT(L"pretty printing");
                AF_TEST_RESULT(std::wstring(L"123"), pretty_w(123));
                AF_TEST_RESULT(std::wstring(L"0.1234"), pretty_w(0.1234));
                AF_TEST_RESULT(std::wstring(L"[1,2,3]"), pretty_w<int[3]>({ 1,2,3 }));
                AF_TEST_RESULT(std::wstring(L"[[1,2,3,4],[2,2,3,4],[3,2,3,4]]"), pretty_w<int[3][4]>({ {1,2,3,4},{2,2,3,4},{3,2,3,4} }));
                AF_TEST_RESULT(std::wstring(L"raw string"), pretty_w(L"raw string"));
                AF_TEST_RESULT(std::wstring(L"[1.23,0.32435,4.5656,0.120981]"), pretty_w(vector_));
                AF_TEST_RESULT(std::wstring(L"[[1,2,3],[2,2,3],[3,2,3,4],[1,2,3,4,5]]"), pretty_w(vector_vector_));
                AF_TEST_RESULT(std::wstring(L"[7,5,16,8]"), pretty_w(deque_));
                AF_TEST_RESULT(std::wstring(L"[\"7\",\"5\",\"16\",\"8\"]"), pretty_w(forward_list_));
                AF_TEST_RESULT(std::wstring(L"[true,false,true,true]"), pretty_w(list_));
                AF_TEST_RESULT(std::wstring(L"{1,2,3}"), pretty_w(set_));
                AF_TEST_RESULT(std::wstring(L"{1,1,2,3}"), pretty_w(multiset_));
                AF_TEST_RESULT(std::wstring(L"{{\"First\",2},{\"Second\",3234324},{\"Third\",44}}"), pretty_w(map_));
                AF_TEST_RESULT(std::wstring(L"{{\"First\",[1.23,0.32435,4.5656,0.120981]},{\"Second\",[1.23,0.32435,4.5656,0.120981]},{\"Third\",[1.23,0.32435,4.5656,0.120981]}}"), pretty_w(map_vectors_));
                AF_TEST_RESULT(std::wstring(L"{{\"First\",1},{\"First\",2},{\"Second\",3},{\"Second\",3234324},{\"Third\",44}}"), pretty_w(multimap_));
#ifdef __GNUG__
                // gcc sorts unorederd containers differently
                AF_TEST_RESULT(std::wstring(L"{3,2,1}"), pretty_w(unordered_set_));
                AF_TEST_RESULT(std::wstring(L"{3,2,1,1}"), pretty_w(unordered_multiset_));
                AF_TEST_RESULT(std::wstring(L"{{\"Third\",44},{\"Second\",3234324},{\"First\",2}}"), pretty_w(unordered_map_));
                AF_TEST_RESULT(std::wstring(L"{{\"Third\",44},{\"Second\",3234324},{\"Second\",3},{\"First\",2},{\"First\",1}}"), pretty_w(unordered_multimap_));
#else
                AF_TEST_RESULT(std::wstring(L"{1,2,3}"), pretty_w(unordered_set_));
                AF_TEST_RESULT(std::wstring(L"{1,1,2,3}"), pretty_w(unordered_multiset_));
                AF_TEST_RESULT(std::wstring(L"{{\"First\",2},{\"Second\",3234324},{\"Third\",44}}"), pretty_w(unordered_map_));
                AF_TEST_RESULT(std::wstring(L"{{\"First\",1},{\"First\",2},{\"Second\",3},{\"Second\",3234324},{\"Third\",44}}"), pretty_w(unordered_multimap_));
#endif
                AF_TEST_RESULT(std::wstring(L"[8,16,5,7]"), pretty_w(stack_));
                AF_TEST_RESULT(std::wstring(L"[7,5,16,8]"), pretty_w(queue_));
                AF_TEST_RESULT(std::wstring(L"[4.5656,1.23,0.32435,0.120981]"), pretty_w(priority_queue_));
                AF_TEST_RESULT(std::wstring(L"\"test string\""), pretty_w(string_));

                // table printing
                AF_TEST_COMMENT(L"table printing");
                AF_TEST_RESULT(std::wstring(L"0 : 123\n"), table_w(123));
                AF_TEST_RESULT(std::wstring(L"0 : 0.1234\n"), table_w(0.1234));
                AF_TEST_RESULT(std::wstring(L"0 : raw string\n"), table_w(L"raw string"));
                AF_TEST_RESULT(std::wstring(L"0 :     1.23\n1 :  0.32435\n2 :   4.5656\n3 : 0.120981\n"), table_w(vector_));
                AF_TEST_RESULT(std::wstring(L"1 | 2 | 3 |   |  \n2 | 2 | 3 |   |  \n3 | 2 | 3 | 4 |  \n1 | 2 | 3 | 4 | 5\n"), table_w(vector_vector_));
                AF_TEST_RESULT(std::wstring(L"0 :  7\n1 :  5\n2 : 16\n3 :  8\n"), table_w(deque_));
                AF_TEST_RESULT(std::wstring(L"0 :  \"7\"\n1 :  \"5\"\n2 :  \"16\"\n3 :  \"8\"\n"), table_w(forward_list_));
                AF_TEST_RESULT(std::wstring(L"0 : true\n1 : false\n2 : true\n3 : true\n"), table_w(list_));
                AF_TEST_RESULT(std::wstring(L"0 : 1\n1 : 2\n2 : 3\n"), table_w(set_));
                AF_TEST_RESULT(std::wstring(L"0 : 1\n1 : 1\n2 : 2\n3 : 3\n"), table_w(multiset_));
                AF_TEST_RESULT(std::wstring(L"First  :       2\nSecond : 3234324\nThird  :      44\n"), table_w(map_));
                AF_TEST_RESULT(std::wstring(L"First  : [1.23,0.32435,4.5656,0.120981]\nSecond : [1.23,0.32435,4.5656,0.120981]\nThird  : [1.23,0.32435,4.5656,0.120981]\n"), table_w(map_vectors_));
                AF_TEST_RESULT(std::wstring(L"First  :       1\nFirst  :       2\nSecond :       3\nSecond : 3234324\nThird  :      44\n"), table_w(multimap_));
#ifdef __GNUG__
                // gcc sorts unorederd containers differently
                AF_TEST_RESULT(std::wstring(L"0 : 3\n1 : 2\n2 : 1\n"), table_w(unordered_set_));
                AF_TEST_RESULT(std::wstring(L"0 : 3\n1 : 2\n2 : 1\n3 : 1\n"), table_w(unordered_multiset_));
                AF_TEST_RESULT(std::wstring(L"Third  :      44\nSecond : 3234324\nFirst  :       2\n"), table_w(unordered_map_));
                AF_TEST_RESULT(std::wstring(L"Third  :      44\nSecond : 3234324\nSecond :       3\nFirst  :       2\nFirst  :       1\n"), table_w(unordered_multimap_));
#else
                AF_TEST_RESULT(std::wstring(L"0 : 1\n1 : 2\n2 : 3\n"), table_w(unordered_set_));
                AF_TEST_RESULT(std::wstring(L"0 : 1\n1 : 1\n2 : 2\n3 : 3\n"), table_w(unordered_multiset_));
                AF_TEST_RESULT(std::wstring(L"First  :       2\nSecond : 3234324\nThird  :      44\n"), table_w(unordered_map_));
                AF_TEST_RESULT(std::wstring(L"First  :       1\nFirst  :       2\nSecond :       3\nSecond : 3234324\nThird  :      44\n"), table_w(unordered_multimap_));
#endif
                AF_TEST_RESULT(std::wstring(L"0 :  8\n1 : 16\n2 :  5\n3 :  7\n"), table_w(stack_));
                AF_TEST_RESULT(std::wstring(L"0 :  7\n1 :  5\n2 : 16\n3 :  8\n"), table_w(queue_));
                AF_TEST_RESULT(std::wstring(L"0 :   4.5656\n1 :     1.23\n2 :  0.32435\n3 : 0.120981\n"), table_w(priority_queue_));
                AF_TEST_RESULT(std::wstring(L"0 : \"test string\"\n"), table_w(string_));

                AF_TEST_RESULT(L"Title Column1 | Title Column2 | Title Column3\nRow1 Column1  | Row1Column2   | Row1Column3  \nRow2 Column1  | Row2 Column2  | Row3 Column3 \nRow3 Column1  | Row3 Column2  | Row3 Column3 \n", table_w(string_grid));
                AF_TEST_RESULT(L"Title Column1 | Title Column2 | Title Column3\n================================================\nRow1 Column1  | Row1Column2   | Row1Column3  \nRow2 Column1  | Row2 Column2  | Row3 Column3 \nRow3 Column1  | Row3 Column2  | Row3 Column3 \n", table_w(string_grid, true));
            }
            template< bool = true> // declaring it as a template is a way to work aroud c++ limitations about declaring things in headers
            void tests() {
                string_tests();
                wstring_tests();
            }
        }
    }
}

AF_DECLARE_TEST_SET("PrettyPrinting", std_pretty_printing, 
    autotelica::examples::std_pretty_printing::examples<>() , autotelica::examples::std_pretty_printing::tests<>());


