#pragma once
#include "comparissons.h"
#include "testing_util.h"

namespace autotelica {
    namespace examples {
        namespace comparissons {
            template< bool = true> // declaring it as a template is a way to work aroud c++ limitations about declaring things in headers
            void examples() {
                // code here will only be run in example runs
            }
            template< bool = true> // declaring it as a template is a way to work aroud c++ limitations about declaring things in headers
            void tests(){
                size_t size_t_ = 1;
                int int_ = 1;
                float float_ = 1.1234f;
                double double_ = 1.1234;

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

                using namespace autotelica::comparissons;

                AF_TEST_RESULT(false, are_equal_f(float_, float_ + 2 * comparissons_config::float_epsilon()));
                AF_TEST_RESULT(false, are_equal_f(double_, double_ + 2 * comparissons_config::double_epsilon()));
                AF_TEST_RESULT(true, are_equal_f(size_t_, size_t(1)));

                AF_TEST_RESULT(true, are_equal_f(int_, 1));
                AF_TEST_RESULT(true, are_equal_f(float_, 1.1234f));
                AF_TEST_RESULT(true, are_equal_f(double_, 1.1234));

                AF_TEST_RESULT(true, are_equal_f(vector_, std::vector<double> { 1.23, 0.32435, 4.5656, 0.120981280932481 }));
                AF_TEST_RESULT(true, are_equal_f(vector_vector_, std::vector<std::vector<int>>{ { 1, 2, 3}, { 2,2,3 }, { 3,2,3,4 }, { 1,2,3,4,5 } }));
                AF_TEST_RESULT(true, are_equal_f(deque_, std::deque<int>{ 7, 5, 16, 8 }));
                AF_TEST_RESULT(true, are_equal_f(forward_list_, std::forward_list<std::string> { "7", "5", "16", "8" }));
                AF_TEST_RESULT(true, are_equal_f(list_, std::list<bool> { true, false, true, true }));

                AF_TEST_RESULT(true, are_equal_f(set_, std::set<int> { 1, 1, 2, 3 }));
                AF_TEST_RESULT(true, are_equal_f(multiset_, std::multiset<int> { 1, 1, 2, 3 }));
                AF_TEST_RESULT(true, are_equal_f(map_, std::map<std::string, int> { {"First", 2}, { "Second",3234324 }, { "Third",44 } }));
                AF_TEST_RESULT(true, are_equal_f(map_vectors_, std::map<std::string, std::vector<double>> { {"First", vector_}, { "Second",vector_ }, { "Third",vector_ } }));
                AF_TEST_RESULT(true, are_equal_f(multimap_, std::multimap<std::string, int> { {"First", 1}, { "First",2 }, { "Second",3 }, { "Second",3234324 }, { "Third",44 } }));

                AF_TEST_RESULT(true, are_equal_f(unordered_set_, std::unordered_set<int> { 1, 1, 2, 3 }));
                AF_TEST_RESULT(true, are_equal_f(unordered_multiset_, std::unordered_multiset<int> { 1, 1, 2, 3 }));
                AF_TEST_RESULT(true, are_equal_f(unordered_map_, std::unordered_map<std::string, int> { {"First", 2}, { "Second",3234324 }, { "Third",44 } }));
                AF_TEST_RESULT(true, are_equal_f(unordered_multimap_, std::unordered_multimap<std::string, int> { {"First", 1}, { "First",2 }, { "Second",3 }, { "Second",3234324 }, { "Third",44 } }));

                AF_TEST_RESULT(true, are_equal_f(string_, std::string{ "test string" }));
            }
        }
    }
}

AF_DECLARE_TEST_SET( "Comparissons", comparissons, 
    autotelica::examples::comparissons::examples<>(), autotelica::examples::comparissons::tests<>());

