#pragma once
#include "testing_util.h"
#include "std_disambiguation.h"
#include <array>
namespace autotelica {
	namespace examples {
		namespace std_disambiguation { 
			template< bool = true> // declaring it as a template is a way to work aroud c++ limitations about declaring things in headers
			void examples() {
				// code here will only be run in example runs
			}
			template< bool = true> // declaring it as a template is a way to work aroud c++ limitations about declaring things in headers
			void tests() {
				using namespace autotelica::std_disambiguation;

				std::array<int, 3> array_{ {1,2,3} };
				std::vector<double> vector_{ 1.23, 0.32435, 4.5656, 0.120981280932481 };
				std::vector<std::vector<int>> vector_vector_{ { 1,2,3}, {2,2,3}, {3,2,3,4}, {1,2,3,4,5} };
				std::deque<int> deque_ = { 7, 5, 16, 8 };
				std::forward_list<std::string> forward_list_ = { "7", "5", "16", "8" };
				std::list<bool> list_ = { true, false, true, true };

				std::set<int> set_{ 1,1,2,3 };
				std::multiset<int> multiset_{ 1,1,2,3 };
				std::map<std::string, int> map_{ {"First",2}, {"Second",3234324}, {"Third",44} };
				std::multimap<std::string, int> multimap_{ {"First",1}, {"First",2}, {"Second",3}, {"Second",3234324}, {"Third",44} };

				std::unordered_set<int> unordered_set_{ 1,1,2,3 };
				std::unordered_multiset<int> unordered_multiset_{ 1,1,2,3 };
				std::unordered_map<std::string, int> unordered_map_{ {"First",2}, {"Second",3234324}, {"Third",44} };
				std::unordered_multimap<std::string, int> unordered_multimap_{ {"First",1}, {"First",2}, {"Second",3}, {"Second",3234324}, {"Third",44} };

				std::stack<int> stack_{ deque_ };
				std::queue<int> queue_{ deque_ };
				std::priority_queue<int> priority_queue_{ array_.begin(), array_.end() };

				std::string string_{ "test string" };
				std::wstring wstring_{ L"test string" };

				std::shared_ptr<int> pi_;
				// shared_ptr
				AF_TEST_COMMENT("shared_ptr");
				AF_TEST_RESULT(true, is_shared_ptr_f(pi_));
				AF_TEST_RESULT(false, is_shared_ptr_f(array_));

				// vector
				AF_TEST_COMMENT("vector");
				AF_TEST_RESULT(false, is_vector_f(array_));
				AF_TEST_RESULT(true, is_vector_f(vector_));
				AF_TEST_RESULT(true, is_vector_f(vector_vector_));
				AF_TEST_RESULT(false, is_vector_f(deque_));
				AF_TEST_RESULT(false, is_vector_f(forward_list_));
				AF_TEST_RESULT(false, is_vector_f(list_));
				AF_TEST_RESULT(false, is_vector_f(set_));
				AF_TEST_RESULT(false, is_vector_f(multiset_));
				AF_TEST_RESULT(false, is_vector_f(map_));
				AF_TEST_RESULT(false, is_vector_f(multimap_));
				AF_TEST_RESULT(false, is_vector_f(unordered_set_));
				AF_TEST_RESULT(false, is_vector_f(unordered_multiset_));
				AF_TEST_RESULT(false, is_vector_f(unordered_map_));
				AF_TEST_RESULT(false, is_vector_f(unordered_multimap_));
				AF_TEST_RESULT(false, is_vector_f(stack_));
				AF_TEST_RESULT(false, is_vector_f(queue_));
				AF_TEST_RESULT(false, is_vector_f(priority_queue_));
				AF_TEST_RESULT(false, is_vector_f(string_));
				AF_TEST_RESULT(false, is_vector_f(123));
				AF_TEST_RESULT(false, is_vector_f(string_.begin()));
				AF_TEST_RESULT(false, is_vector_f(vector_.begin()));
				AF_TEST_RESULT(true, is_vector_f(*vector_vector_.begin()));

				// deque
				AF_TEST_COMMENT("deque");
				AF_TEST_RESULT(false, is_deque_f(array_));
				AF_TEST_RESULT(false, is_deque_f(vector_));
				AF_TEST_RESULT(false, is_deque_f(vector_vector_));
				AF_TEST_RESULT(true, is_deque_f(deque_));
				AF_TEST_RESULT(false, is_deque_f(forward_list_));
				AF_TEST_RESULT(false, is_deque_f(list_));
				AF_TEST_RESULT(false, is_deque_f(set_));
				AF_TEST_RESULT(false, is_deque_f(multiset_));
				AF_TEST_RESULT(false, is_deque_f(map_));
				AF_TEST_RESULT(false, is_deque_f(multimap_));
				AF_TEST_RESULT(false, is_deque_f(unordered_set_));
				AF_TEST_RESULT(false, is_deque_f(unordered_multiset_));
				AF_TEST_RESULT(false, is_deque_f(unordered_map_));
				AF_TEST_RESULT(false, is_deque_f(unordered_multimap_));
				AF_TEST_RESULT(false, is_deque_f(stack_));
				AF_TEST_RESULT(false, is_deque_f(queue_));
				AF_TEST_RESULT(false, is_deque_f(priority_queue_));
				AF_TEST_RESULT(false, is_deque_f(string_));
				AF_TEST_RESULT(false, is_deque_f(123));
				AF_TEST_RESULT(false, is_deque_f(string_.begin()));
				AF_TEST_RESULT(false, is_deque_f(vector_.begin()));
				AF_TEST_RESULT(false, is_deque_f(*vector_vector_.begin()));

				// forward_list
				AF_TEST_COMMENT("forward_list");
				AF_TEST_RESULT(false, is_forward_list_f(array_));
				AF_TEST_RESULT(false, is_forward_list_f(vector_));
				AF_TEST_RESULT(false, is_forward_list_f(vector_vector_));
				AF_TEST_RESULT(false, is_forward_list_f(deque_));
				AF_TEST_RESULT(true, is_forward_list_f(forward_list_));
				AF_TEST_RESULT(false, is_forward_list_f(list_));
				AF_TEST_RESULT(false, is_forward_list_f(set_));
				AF_TEST_RESULT(false, is_forward_list_f(multiset_));
				AF_TEST_RESULT(false, is_forward_list_f(map_));
				AF_TEST_RESULT(false, is_forward_list_f(multimap_));
				AF_TEST_RESULT(false, is_forward_list_f(unordered_set_));
				AF_TEST_RESULT(false, is_forward_list_f(unordered_multiset_));
				AF_TEST_RESULT(false, is_forward_list_f(unordered_map_));
				AF_TEST_RESULT(false, is_forward_list_f(unordered_multimap_));
				AF_TEST_RESULT(false, is_forward_list_f(stack_));
				AF_TEST_RESULT(false, is_forward_list_f(queue_));
				AF_TEST_RESULT(false, is_forward_list_f(priority_queue_));
				AF_TEST_RESULT(false, is_forward_list_f(string_));
				AF_TEST_RESULT(false, is_forward_list_f(123));
				AF_TEST_RESULT(false, is_forward_list_f(string_.begin()));
				AF_TEST_RESULT(false, is_forward_list_f(vector_.begin()));
				AF_TEST_RESULT(false, is_forward_list_f(*vector_vector_.begin()));

				// list
				AF_TEST_COMMENT("list");
				AF_TEST_RESULT(false, is_list_f(array_));
				AF_TEST_RESULT(false, is_list_f(vector_));
				AF_TEST_RESULT(false, is_list_f(vector_vector_));
				AF_TEST_RESULT(false, is_list_f(deque_));
				AF_TEST_RESULT(false, is_list_f(forward_list_));
				AF_TEST_RESULT(true, is_list_f(list_));
				AF_TEST_RESULT(false, is_list_f(set_));
				AF_TEST_RESULT(false, is_list_f(multiset_));
				AF_TEST_RESULT(false, is_list_f(map_));
				AF_TEST_RESULT(false, is_list_f(multimap_));
				AF_TEST_RESULT(false, is_list_f(unordered_set_));
				AF_TEST_RESULT(false, is_list_f(unordered_multiset_));
				AF_TEST_RESULT(false, is_list_f(unordered_map_));
				AF_TEST_RESULT(false, is_list_f(unordered_multimap_));
				AF_TEST_RESULT(false, is_list_f(stack_));
				AF_TEST_RESULT(false, is_list_f(queue_));
				AF_TEST_RESULT(false, is_list_f(priority_queue_));
				AF_TEST_RESULT(false, is_list_f(string_));
				AF_TEST_RESULT(false, is_list_f(123));
				AF_TEST_RESULT(false, is_list_f(string_.begin()));
				AF_TEST_RESULT(false, is_list_f(vector_.begin()));
				AF_TEST_RESULT(false, is_list_f(*vector_vector_.begin()));

				// set
				AF_TEST_COMMENT("set");
				AF_TEST_RESULT(false, is_set_f(array_));
				AF_TEST_RESULT(false, is_set_f(vector_));
				AF_TEST_RESULT(false, is_set_f(vector_vector_));
				AF_TEST_RESULT(false, is_set_f(deque_));
				AF_TEST_RESULT(false, is_set_f(forward_list_));
				AF_TEST_RESULT(false, is_set_f(list_));
				AF_TEST_RESULT(true, is_set_f(set_));
				AF_TEST_RESULT(false, is_set_f(multiset_));
				AF_TEST_RESULT(false, is_set_f(map_));
				AF_TEST_RESULT(false, is_set_f(multimap_));
				AF_TEST_RESULT(false, is_set_f(unordered_set_));
				AF_TEST_RESULT(false, is_set_f(unordered_multiset_));
				AF_TEST_RESULT(false, is_set_f(unordered_map_));
				AF_TEST_RESULT(false, is_set_f(unordered_multimap_));
				AF_TEST_RESULT(false, is_set_f(stack_));
				AF_TEST_RESULT(false, is_set_f(queue_));
				AF_TEST_RESULT(false, is_set_f(priority_queue_));
				AF_TEST_RESULT(false, is_set_f(string_));
				AF_TEST_RESULT(false, is_set_f(123));
				AF_TEST_RESULT(false, is_set_f(string_.begin()));
				AF_TEST_RESULT(false, is_set_f(vector_.begin()));
				AF_TEST_RESULT(false, is_set_f(*vector_vector_.begin()));

				// multiset
				AF_TEST_COMMENT("multiset");
				AF_TEST_RESULT(false, is_multiset_f(array_));
				AF_TEST_RESULT(false, is_multiset_f(vector_));
				AF_TEST_RESULT(false, is_multiset_f(vector_vector_));
				AF_TEST_RESULT(false, is_multiset_f(deque_));
				AF_TEST_RESULT(false, is_multiset_f(forward_list_));
				AF_TEST_RESULT(false, is_multiset_f(list_));
				AF_TEST_RESULT(false, is_multiset_f(set_));
				AF_TEST_RESULT(true, is_multiset_f(multiset_));
				AF_TEST_RESULT(false, is_multiset_f(map_));
				AF_TEST_RESULT(false, is_multiset_f(multimap_));
				AF_TEST_RESULT(false, is_multiset_f(unordered_set_));
				AF_TEST_RESULT(false, is_multiset_f(unordered_multiset_));
				AF_TEST_RESULT(false, is_multiset_f(unordered_map_));
				AF_TEST_RESULT(false, is_multiset_f(unordered_multimap_));
				AF_TEST_RESULT(false, is_multiset_f(stack_));
				AF_TEST_RESULT(false, is_multiset_f(queue_));
				AF_TEST_RESULT(false, is_multiset_f(priority_queue_));
				AF_TEST_RESULT(false, is_multiset_f(string_));
				AF_TEST_RESULT(false, is_multiset_f(123));
				AF_TEST_RESULT(false, is_multiset_f(string_.begin()));
				AF_TEST_RESULT(false, is_multiset_f(vector_.begin()));
				AF_TEST_RESULT(false, is_multiset_f(*vector_vector_.begin()));

				// map
				AF_TEST_COMMENT("map");
				AF_TEST_RESULT(false, is_map_f(array_));
				AF_TEST_RESULT(false, is_map_f(vector_));
				AF_TEST_RESULT(false, is_map_f(vector_vector_));
				AF_TEST_RESULT(false, is_map_f(deque_));
				AF_TEST_RESULT(false, is_map_f(forward_list_));
				AF_TEST_RESULT(false, is_map_f(list_));
				AF_TEST_RESULT(false, is_map_f(set_));
				AF_TEST_RESULT(false, is_map_f(multiset_));
				AF_TEST_RESULT(true, is_map_f(map_));
				AF_TEST_RESULT(false, is_map_f(multimap_));
				AF_TEST_RESULT(false, is_map_f(unordered_set_));
				AF_TEST_RESULT(false, is_map_f(unordered_multiset_));
				AF_TEST_RESULT(false, is_map_f(unordered_map_));
				AF_TEST_RESULT(false, is_map_f(unordered_multimap_));
				AF_TEST_RESULT(false, is_map_f(stack_));
				AF_TEST_RESULT(false, is_map_f(queue_));
				AF_TEST_RESULT(false, is_map_f(priority_queue_));
				AF_TEST_RESULT(false, is_map_f(string_));
				AF_TEST_RESULT(false, is_map_f(123));
				AF_TEST_RESULT(false, is_map_f(string_.begin()));
				AF_TEST_RESULT(false, is_map_f(vector_.begin()));
				AF_TEST_RESULT(false, is_map_f(*vector_vector_.begin()));

				// multimap
				AF_TEST_COMMENT("multimap");
				AF_TEST_RESULT(false, is_multimap_f(array_));
				AF_TEST_RESULT(false, is_multimap_f(vector_));
				AF_TEST_RESULT(false, is_multimap_f(vector_vector_));
				AF_TEST_RESULT(false, is_multimap_f(deque_));
				AF_TEST_RESULT(false, is_multimap_f(forward_list_));
				AF_TEST_RESULT(false, is_multimap_f(list_));
				AF_TEST_RESULT(false, is_multimap_f(set_));
				AF_TEST_RESULT(false, is_multimap_f(multiset_));
				AF_TEST_RESULT(false, is_multimap_f(map_));
				AF_TEST_RESULT(true, is_multimap_f(multimap_));
				AF_TEST_RESULT(false, is_multimap_f(unordered_set_));
				AF_TEST_RESULT(false, is_multimap_f(unordered_multiset_));
				AF_TEST_RESULT(false, is_multimap_f(unordered_map_));
				AF_TEST_RESULT(false, is_multimap_f(unordered_multimap_));
				AF_TEST_RESULT(false, is_multimap_f(stack_));
				AF_TEST_RESULT(false, is_multimap_f(queue_));
				AF_TEST_RESULT(false, is_multimap_f(priority_queue_));
				AF_TEST_RESULT(false, is_multimap_f(string_));
				AF_TEST_RESULT(false, is_multimap_f(123));
				AF_TEST_RESULT(false, is_multimap_f(string_.begin()));
				AF_TEST_RESULT(false, is_multimap_f(vector_.begin()));
				AF_TEST_RESULT(false, is_multimap_f(*vector_vector_.begin()));

				// unordered_set
				AF_TEST_COMMENT("unordered_set");
				AF_TEST_RESULT(false, is_unordered_set_f(array_));
				AF_TEST_RESULT(false, is_unordered_set_f(vector_));
				AF_TEST_RESULT(false, is_unordered_set_f(vector_vector_));
				AF_TEST_RESULT(false, is_unordered_set_f(deque_));
				AF_TEST_RESULT(false, is_unordered_set_f(forward_list_));
				AF_TEST_RESULT(false, is_unordered_set_f(list_));
				AF_TEST_RESULT(false, is_unordered_set_f(set_));
				AF_TEST_RESULT(false, is_unordered_set_f(multiset_));
				AF_TEST_RESULT(false, is_unordered_set_f(map_));
				AF_TEST_RESULT(false, is_unordered_set_f(multimap_));
				AF_TEST_RESULT(true, is_unordered_set_f(unordered_set_));
				AF_TEST_RESULT(false, is_unordered_set_f(unordered_multiset_));
				AF_TEST_RESULT(false, is_unordered_set_f(unordered_map_));
				AF_TEST_RESULT(false, is_unordered_set_f(unordered_multimap_));
				AF_TEST_RESULT(false, is_unordered_set_f(stack_));
				AF_TEST_RESULT(false, is_unordered_set_f(queue_));
				AF_TEST_RESULT(false, is_unordered_set_f(priority_queue_));
				AF_TEST_RESULT(false, is_unordered_set_f(string_));
				AF_TEST_RESULT(false, is_unordered_set_f(123));
				AF_TEST_RESULT(false, is_unordered_set_f(string_.begin()));
				AF_TEST_RESULT(false, is_unordered_set_f(vector_.begin()));
				AF_TEST_RESULT(false, is_unordered_set_f(*vector_vector_.begin()));

				// unordered_multiset
				AF_TEST_COMMENT("unordered_multiset");
				AF_TEST_RESULT(false, is_unordered_multiset_f(array_));
				AF_TEST_RESULT(false, is_unordered_multiset_f(vector_));
				AF_TEST_RESULT(false, is_unordered_multiset_f(vector_vector_));
				AF_TEST_RESULT(false, is_unordered_multiset_f(deque_));
				AF_TEST_RESULT(false, is_unordered_multiset_f(forward_list_));
				AF_TEST_RESULT(false, is_unordered_multiset_f(list_));
				AF_TEST_RESULT(false, is_unordered_multiset_f(set_));
				AF_TEST_RESULT(false, is_unordered_multiset_f(multiset_));
				AF_TEST_RESULT(false, is_unordered_multiset_f(map_));
				AF_TEST_RESULT(false, is_unordered_multiset_f(multimap_));
				AF_TEST_RESULT(false, is_unordered_multiset_f(unordered_set_));
				AF_TEST_RESULT(true, is_unordered_multiset_f(unordered_multiset_));
				AF_TEST_RESULT(false, is_unordered_multiset_f(unordered_map_));
				AF_TEST_RESULT(false, is_unordered_multiset_f(unordered_multimap_));
				AF_TEST_RESULT(false, is_unordered_multiset_f(stack_));
				AF_TEST_RESULT(false, is_unordered_multiset_f(queue_));
				AF_TEST_RESULT(false, is_unordered_multiset_f(priority_queue_));
				AF_TEST_RESULT(false, is_unordered_multiset_f(string_));
				AF_TEST_RESULT(false, is_unordered_multiset_f(123));
				AF_TEST_RESULT(false, is_unordered_multiset_f(string_.begin()));
				AF_TEST_RESULT(false, is_unordered_multiset_f(vector_.begin()));
				AF_TEST_RESULT(false, is_unordered_multiset_f(*vector_vector_.begin()));

				// unordered map
				AF_TEST_COMMENT("unordered map");
				AF_TEST_RESULT(false, is_unordered_map_f(array_));
				AF_TEST_RESULT(false, is_unordered_map_f(vector_));
				AF_TEST_RESULT(false, is_unordered_map_f(vector_vector_));
				AF_TEST_RESULT(false, is_unordered_map_f(deque_));
				AF_TEST_RESULT(false, is_unordered_map_f(forward_list_));
				AF_TEST_RESULT(false, is_unordered_map_f(list_));
				AF_TEST_RESULT(false, is_unordered_map_f(set_));
				AF_TEST_RESULT(false, is_unordered_map_f(multiset_));
				AF_TEST_RESULT(false, is_unordered_map_f(map_));
				AF_TEST_RESULT(false, is_unordered_map_f(multimap_));
				AF_TEST_RESULT(false, is_unordered_map_f(unordered_set_));
				AF_TEST_RESULT(false, is_unordered_map_f(unordered_multiset_));
				AF_TEST_RESULT(true, is_unordered_map_f(unordered_map_));
				AF_TEST_RESULT(false, is_unordered_map_f(unordered_multimap_));
				AF_TEST_RESULT(false, is_unordered_map_f(stack_));
				AF_TEST_RESULT(false, is_unordered_map_f(queue_));
				AF_TEST_RESULT(false, is_unordered_map_f(priority_queue_));
				AF_TEST_RESULT(false, is_unordered_map_f(string_));
				AF_TEST_RESULT(false, is_unordered_map_f(123));
				AF_TEST_RESULT(false, is_unordered_map_f(string_.begin()));
				AF_TEST_RESULT(false, is_unordered_map_f(vector_.begin()));
				AF_TEST_RESULT(false, is_unordered_map_f(*vector_vector_.begin()));

				// unordered_multimap
				AF_TEST_COMMENT("unordered_multimap");
				AF_TEST_RESULT(false, is_unordered_multimap_f(array_));
				AF_TEST_RESULT(false, is_unordered_multimap_f(vector_));
				AF_TEST_RESULT(false, is_unordered_multimap_f(vector_vector_));
				AF_TEST_RESULT(false, is_unordered_multimap_f(deque_));
				AF_TEST_RESULT(false, is_unordered_multimap_f(forward_list_));
				AF_TEST_RESULT(false, is_unordered_multimap_f(list_));
				AF_TEST_RESULT(false, is_unordered_multimap_f(set_));
				AF_TEST_RESULT(false, is_unordered_multimap_f(multiset_));
				AF_TEST_RESULT(false, is_unordered_multimap_f(map_));
				AF_TEST_RESULT(false, is_unordered_multimap_f(multimap_));
				AF_TEST_RESULT(false, is_unordered_multimap_f(unordered_set_));
				AF_TEST_RESULT(false, is_unordered_multimap_f(unordered_multiset_));
				AF_TEST_RESULT(false, is_unordered_multimap_f(unordered_map_));
				AF_TEST_RESULT(true, is_unordered_multimap_f(unordered_multimap_));
				AF_TEST_RESULT(false, is_unordered_multimap_f(stack_));
				AF_TEST_RESULT(false, is_unordered_multimap_f(queue_));
				AF_TEST_RESULT(false, is_unordered_multimap_f(priority_queue_));
				AF_TEST_RESULT(false, is_unordered_multimap_f(string_));
				AF_TEST_RESULT(false, is_unordered_multimap_f(123));
				AF_TEST_RESULT(false, is_unordered_multimap_f(string_.begin()));
				AF_TEST_RESULT(false, is_unordered_multimap_f(vector_.begin()));
				AF_TEST_RESULT(false, is_unordered_multimap_f(*vector_vector_.begin()));

				// stack
				AF_TEST_COMMENT("stack");
				AF_TEST_RESULT(false, is_stack_f(array_));
				AF_TEST_RESULT(false, is_stack_f(vector_));
				AF_TEST_RESULT(false, is_stack_f(vector_vector_));
				AF_TEST_RESULT(false, is_stack_f(deque_));
				AF_TEST_RESULT(false, is_stack_f(forward_list_));
				AF_TEST_RESULT(false, is_stack_f(list_));
				AF_TEST_RESULT(false, is_stack_f(set_));
				AF_TEST_RESULT(false, is_stack_f(multiset_));
				AF_TEST_RESULT(false, is_stack_f(map_));
				AF_TEST_RESULT(false, is_stack_f(multimap_));
				AF_TEST_RESULT(false, is_stack_f(unordered_set_));
				AF_TEST_RESULT(false, is_stack_f(unordered_multiset_));
				AF_TEST_RESULT(false, is_stack_f(unordered_map_));
				AF_TEST_RESULT(false, is_stack_f(unordered_multimap_));
				AF_TEST_RESULT(true, is_stack_f(stack_));
				AF_TEST_RESULT(false, is_stack_f(queue_));
				AF_TEST_RESULT(false, is_stack_f(priority_queue_));
				AF_TEST_RESULT(false, is_stack_f(string_));
				AF_TEST_RESULT(false, is_stack_f(123));
				AF_TEST_RESULT(false, is_stack_f(string_.begin()));
				AF_TEST_RESULT(false, is_stack_f(vector_.begin()));
				AF_TEST_RESULT(false, is_stack_f(*vector_vector_.begin()));

				// queue
				AF_TEST_COMMENT("queue");
				AF_TEST_RESULT(false, is_queue_f(array_));
				AF_TEST_RESULT(false, is_queue_f(vector_));
				AF_TEST_RESULT(false, is_queue_f(vector_vector_));
				AF_TEST_RESULT(false, is_queue_f(deque_));
				AF_TEST_RESULT(false, is_queue_f(forward_list_));
				AF_TEST_RESULT(false, is_queue_f(list_));
				AF_TEST_RESULT(false, is_queue_f(set_));
				AF_TEST_RESULT(false, is_queue_f(multiset_));
				AF_TEST_RESULT(false, is_queue_f(map_));
				AF_TEST_RESULT(false, is_queue_f(multimap_));
				AF_TEST_RESULT(false, is_queue_f(unordered_set_));
				AF_TEST_RESULT(false, is_queue_f(unordered_multiset_));
				AF_TEST_RESULT(false, is_queue_f(unordered_map_));
				AF_TEST_RESULT(false, is_queue_f(unordered_multimap_));
				AF_TEST_RESULT(false, is_queue_f(stack_));
				AF_TEST_RESULT(true, is_queue_f(queue_));
				AF_TEST_RESULT(false, is_queue_f(priority_queue_));
				AF_TEST_RESULT(false, is_queue_f(string_));
				AF_TEST_RESULT(false, is_queue_f(123));
				AF_TEST_RESULT(false, is_queue_f(string_.begin()));
				AF_TEST_RESULT(false, is_queue_f(vector_.begin()));
				AF_TEST_RESULT(false, is_queue_f(*vector_vector_.begin()));

				// priority_queue
				AF_TEST_COMMENT("priority_queue");
				AF_TEST_RESULT(false, is_priority_queue_f(array_));
				AF_TEST_RESULT(false, is_priority_queue_f(vector_));
				AF_TEST_RESULT(false, is_priority_queue_f(vector_vector_));
				AF_TEST_RESULT(false, is_priority_queue_f(deque_));
				AF_TEST_RESULT(false, is_priority_queue_f(forward_list_));
				AF_TEST_RESULT(false, is_priority_queue_f(list_));
				AF_TEST_RESULT(false, is_priority_queue_f(set_));
				AF_TEST_RESULT(false, is_priority_queue_f(multiset_));
				AF_TEST_RESULT(false, is_priority_queue_f(map_));
				AF_TEST_RESULT(false, is_priority_queue_f(multimap_));
				AF_TEST_RESULT(false, is_priority_queue_f(unordered_set_));
				AF_TEST_RESULT(false, is_priority_queue_f(unordered_multiset_));
				AF_TEST_RESULT(false, is_priority_queue_f(unordered_map_));
				AF_TEST_RESULT(false, is_priority_queue_f(unordered_multimap_));
				AF_TEST_RESULT(false, is_priority_queue_f(stack_));
				AF_TEST_RESULT(false, is_priority_queue_f(queue_));
				AF_TEST_RESULT(true, is_priority_queue_f(priority_queue_));
				AF_TEST_RESULT(false, is_priority_queue_f(string_));
				AF_TEST_RESULT(false, is_priority_queue_f(123));
				AF_TEST_RESULT(false, is_priority_queue_f(string_.begin()));
				AF_TEST_RESULT(false, is_priority_queue_f(vector_.begin()));
				AF_TEST_RESULT(false, is_priority_queue_f(*vector_vector_.begin()));

				// string
				AF_TEST_COMMENT("string");
				AF_TEST_RESULT(false, is_string_f(array_));
				AF_TEST_RESULT(false, is_string_f(vector_));
				AF_TEST_RESULT(false, is_string_f(vector_vector_));
				AF_TEST_RESULT(false, is_string_f(deque_));
				AF_TEST_RESULT(false, is_string_f(forward_list_));
				AF_TEST_RESULT(false, is_string_f(list_));
				AF_TEST_RESULT(false, is_string_f(set_));
				AF_TEST_RESULT(false, is_string_f(multiset_));
				AF_TEST_RESULT(false, is_string_f(map_));
				AF_TEST_RESULT(false, is_string_f(multimap_));
				AF_TEST_RESULT(false, is_string_f(unordered_set_));
				AF_TEST_RESULT(false, is_string_f(unordered_multiset_));
				AF_TEST_RESULT(false, is_string_f(unordered_map_));
				AF_TEST_RESULT(false, is_string_f(unordered_multimap_));
				AF_TEST_RESULT(false, is_string_f(stack_));
				AF_TEST_RESULT(false, is_string_f(queue_));
				AF_TEST_RESULT(false, is_string_f(priority_queue_));
				AF_TEST_RESULT(true, is_string_f(string_));
				AF_TEST_RESULT(false, is_string_f(123));
				AF_TEST_RESULT(false, is_string_f(string_.begin()));
				AF_TEST_RESULT(false, is_string_f(vector_.begin()));
				AF_TEST_RESULT(false, is_string_f(*vector_vector_.begin()));

				// sequence
				AF_TEST_COMMENT("sequence");
				AF_TEST_RESULT(false, is_sequence_f(array_));
				AF_TEST_RESULT(true, is_sequence_f(vector_));
				AF_TEST_RESULT(true, is_sequence_f(vector_vector_));
				AF_TEST_RESULT(true, is_sequence_f(deque_));
				AF_TEST_RESULT(true, is_sequence_f(forward_list_));
				AF_TEST_RESULT(true, is_sequence_f(list_));
				AF_TEST_RESULT(false, is_sequence_f(set_));
				AF_TEST_RESULT(false, is_sequence_f(multiset_));
				AF_TEST_RESULT(false, is_sequence_f(map_));
				AF_TEST_RESULT(false, is_sequence_f(multimap_));
				AF_TEST_RESULT(false, is_sequence_f(unordered_set_));
				AF_TEST_RESULT(false, is_sequence_f(unordered_multiset_));
				AF_TEST_RESULT(false, is_sequence_f(unordered_map_));
				AF_TEST_RESULT(false, is_sequence_f(unordered_multimap_));
				AF_TEST_RESULT(false, is_sequence_f(stack_));
				AF_TEST_RESULT(false, is_sequence_f(queue_));
				AF_TEST_RESULT(false, is_sequence_f(priority_queue_));
				AF_TEST_RESULT(false, is_sequence_f(string_));
				AF_TEST_RESULT(false, is_sequence_f(123));
				AF_TEST_RESULT(false, is_sequence_f(string_.begin()));
				AF_TEST_RESULT(false, is_sequence_f(vector_.begin()));
				AF_TEST_RESULT(true, is_sequence_f(*vector_vector_.begin()));

				// setish
				AF_TEST_COMMENT("setish");
				AF_TEST_RESULT(false, is_setish_f(array_));
				AF_TEST_RESULT(false, is_setish_f(vector_));
				AF_TEST_RESULT(false, is_setish_f(vector_vector_));
				AF_TEST_RESULT(false, is_setish_f(deque_));
				AF_TEST_RESULT(false, is_setish_f(forward_list_));
				AF_TEST_RESULT(false, is_setish_f(list_));
				AF_TEST_RESULT(true, is_setish_f(set_));
				AF_TEST_RESULT(true, is_setish_f(multiset_));
				AF_TEST_RESULT(false, is_setish_f(map_));
				AF_TEST_RESULT(false, is_setish_f(multimap_));
				AF_TEST_RESULT(true, is_setish_f(unordered_set_));
				AF_TEST_RESULT(true, is_setish_f(unordered_multiset_));
				AF_TEST_RESULT(false, is_setish_f(unordered_map_));
				AF_TEST_RESULT(false, is_setish_f(unordered_multimap_));
				AF_TEST_RESULT(false, is_setish_f(stack_));
				AF_TEST_RESULT(false, is_setish_f(queue_));
				AF_TEST_RESULT(false, is_setish_f(priority_queue_));
				AF_TEST_RESULT(false, is_setish_f(string_));
				AF_TEST_RESULT(false, is_setish_f(123));
				AF_TEST_RESULT(false, is_setish_f(string_.begin()));
				AF_TEST_RESULT(false, is_setish_f(vector_.begin()));
				AF_TEST_RESULT(false, is_setish_f(*vector_vector_.begin()));

				// mapish
				AF_TEST_COMMENT("mapish");
				AF_TEST_RESULT(false, is_mapish_f(array_));
				AF_TEST_RESULT(false, is_mapish_f(vector_));
				AF_TEST_RESULT(false, is_mapish_f(vector_vector_));
				AF_TEST_RESULT(false, is_mapish_f(deque_));
				AF_TEST_RESULT(false, is_mapish_f(forward_list_));
				AF_TEST_RESULT(false, is_mapish_f(list_));
				AF_TEST_RESULT(false, is_mapish_f(set_));
				AF_TEST_RESULT(false, is_mapish_f(multiset_));
				AF_TEST_RESULT(true, is_mapish_f(map_));
				AF_TEST_RESULT(true, is_mapish_f(multimap_));
				AF_TEST_RESULT(false, is_mapish_f(unordered_set_));
				AF_TEST_RESULT(false, is_mapish_f(unordered_multiset_));
				AF_TEST_RESULT(true, is_mapish_f(unordered_map_));
				AF_TEST_RESULT(true, is_mapish_f(unordered_multimap_));
				AF_TEST_RESULT(false, is_mapish_f(stack_));
				AF_TEST_RESULT(false, is_mapish_f(queue_));
				AF_TEST_RESULT(false, is_mapish_f(priority_queue_));
				AF_TEST_RESULT(false, is_mapish_f(string_));
				AF_TEST_RESULT(false, is_mapish_f(123));
				AF_TEST_RESULT(false, is_mapish_f(string_.begin()));
				AF_TEST_RESULT(false, is_mapish_f(vector_.begin()));
				AF_TEST_RESULT(false, is_mapish_f(*vector_vector_.begin()));

				// associative
				AF_TEST_COMMENT("associative");
				AF_TEST_RESULT(false, is_associative_f(array_));
				AF_TEST_RESULT(false, is_associative_f(vector_));
				AF_TEST_RESULT(false, is_associative_f(vector_vector_));
				AF_TEST_RESULT(false, is_associative_f(deque_));
				AF_TEST_RESULT(false, is_associative_f(forward_list_));
				AF_TEST_RESULT(false, is_associative_f(list_));
				AF_TEST_RESULT(true, is_associative_f(set_));
				AF_TEST_RESULT(true, is_associative_f(multiset_));
				AF_TEST_RESULT(true, is_associative_f(map_));
				AF_TEST_RESULT(true, is_associative_f(multimap_));
				AF_TEST_RESULT(true, is_associative_f(unordered_set_));
				AF_TEST_RESULT(true, is_associative_f(unordered_multiset_));
				AF_TEST_RESULT(true, is_associative_f(unordered_map_));
				AF_TEST_RESULT(true, is_associative_f(unordered_multimap_));
				AF_TEST_RESULT(false, is_associative_f(stack_));
				AF_TEST_RESULT(false, is_associative_f(queue_));
				AF_TEST_RESULT(false, is_associative_f(priority_queue_));
				AF_TEST_RESULT(false, is_associative_f(string_));
				AF_TEST_RESULT(false, is_associative_f(123));
				AF_TEST_RESULT(false, is_associative_f(string_.begin()));
				AF_TEST_RESULT(false, is_associative_f(vector_.begin()));
				AF_TEST_RESULT(false, is_associative_f(*vector_vector_.begin()));

				// adaptor
				AF_TEST_COMMENT("adaptor");
				AF_TEST_RESULT(false, is_adaptor_f(array_));
				AF_TEST_RESULT(false, is_adaptor_f(vector_));
				AF_TEST_RESULT(false, is_adaptor_f(vector_vector_));
				AF_TEST_RESULT(false, is_adaptor_f(deque_));
				AF_TEST_RESULT(false, is_adaptor_f(forward_list_));
				AF_TEST_RESULT(false, is_adaptor_f(list_));
				AF_TEST_RESULT(false, is_adaptor_f(set_));
				AF_TEST_RESULT(false, is_adaptor_f(multiset_));
				AF_TEST_RESULT(false, is_adaptor_f(map_));
				AF_TEST_RESULT(false, is_adaptor_f(multimap_));
				AF_TEST_RESULT(false, is_adaptor_f(unordered_set_));
				AF_TEST_RESULT(false, is_adaptor_f(unordered_multiset_));
				AF_TEST_RESULT(false, is_adaptor_f(unordered_map_));
				AF_TEST_RESULT(false, is_adaptor_f(unordered_multimap_));
				AF_TEST_RESULT(true, is_adaptor_f(stack_));
				AF_TEST_RESULT(true, is_adaptor_f(queue_));
				AF_TEST_RESULT(true, is_adaptor_f(priority_queue_));
				AF_TEST_RESULT(false, is_adaptor_f(string_));
				AF_TEST_RESULT(false, is_adaptor_f(123));
				AF_TEST_RESULT(false, is_adaptor_f(string_.begin()));
				AF_TEST_RESULT(false, is_adaptor_f(vector_.begin()));
				AF_TEST_RESULT(false, is_adaptor_f(*vector_vector_.begin()));

				// indexable
				AF_TEST_COMMENT("indexable");
				AF_TEST_RESULT(false, is_indexable_f(array_));
				AF_TEST_RESULT(true, is_indexable_f(vector_));
				AF_TEST_RESULT(true, is_indexable_f(vector_vector_));
				AF_TEST_RESULT(true, is_indexable_f(deque_));
				AF_TEST_RESULT(true, is_indexable_f(forward_list_));
				AF_TEST_RESULT(true, is_indexable_f(list_));
				AF_TEST_RESULT(true, is_indexable_f(set_));
				AF_TEST_RESULT(true, is_indexable_f(multiset_));
				AF_TEST_RESULT(false, is_indexable_f(map_));
				AF_TEST_RESULT(false, is_indexable_f(multimap_));
				AF_TEST_RESULT(true, is_indexable_f(unordered_set_));
				AF_TEST_RESULT(true, is_indexable_f(unordered_multiset_));
				AF_TEST_RESULT(false, is_indexable_f(unordered_map_));
				AF_TEST_RESULT(false, is_indexable_f(unordered_multimap_));
				AF_TEST_RESULT(false, is_indexable_f(stack_));
				AF_TEST_RESULT(false, is_indexable_f(queue_));
				AF_TEST_RESULT(false, is_indexable_f(priority_queue_));
				AF_TEST_RESULT(true, is_indexable_f(string_));
				AF_TEST_RESULT(false, is_indexable_f(123));
				AF_TEST_RESULT(false, is_indexable_f(string_.begin()));
				AF_TEST_RESULT(false, is_indexable_f(vector_.begin()));
				AF_TEST_RESULT(true, is_indexable_f(*vector_vector_.begin()));

				// container
				AF_TEST_COMMENT("container");
				AF_TEST_RESULT(false, is_container_f(array_));
				AF_TEST_RESULT(true, is_container_f(vector_));
				AF_TEST_RESULT(true, is_container_f(vector_vector_));
				AF_TEST_RESULT(true, is_container_f(deque_));
				AF_TEST_RESULT(true, is_container_f(forward_list_));
				AF_TEST_RESULT(true, is_container_f(list_));
				AF_TEST_RESULT(true, is_container_f(set_));
				AF_TEST_RESULT(true, is_container_f(multiset_));
				AF_TEST_RESULT(true, is_container_f(map_));
				AF_TEST_RESULT(true, is_container_f(multimap_));
				AF_TEST_RESULT(true, is_container_f(unordered_set_));
				AF_TEST_RESULT(true, is_container_f(unordered_multiset_));
				AF_TEST_RESULT(true, is_container_f(unordered_map_));
				AF_TEST_RESULT(true, is_container_f(unordered_multimap_));
				AF_TEST_RESULT(true, is_container_f(stack_));
				AF_TEST_RESULT(true, is_container_f(queue_));
				AF_TEST_RESULT(true, is_container_f(priority_queue_));
				AF_TEST_RESULT(true, is_container_f(string_));
				AF_TEST_RESULT(false, is_container_f(123));
				AF_TEST_RESULT(false, is_container_f(string_.begin()));
				AF_TEST_RESULT(false, is_container_f(vector_.begin()));
				AF_TEST_RESULT(true, is_container_f(*vector_vector_.begin()));

				// value_grid
				AF_TEST_COMMENT("value_grid");
				AF_TEST_RESULT(false, is_value_grid_f(array_));
				AF_TEST_RESULT(false, is_value_grid_f(vector_));
				AF_TEST_RESULT(true, is_value_grid_f(vector_vector_));
				AF_TEST_RESULT(false, is_value_grid_f(deque_));
				AF_TEST_RESULT(false, is_value_grid_f(forward_list_));
				AF_TEST_RESULT(false, is_value_grid_f(list_));
				AF_TEST_RESULT(false, is_value_grid_f(set_));
				AF_TEST_RESULT(false, is_value_grid_f(multiset_));
				AF_TEST_RESULT(false, is_value_grid_f(map_));
				AF_TEST_RESULT(false, is_value_grid_f(multimap_));
				AF_TEST_RESULT(false, is_value_grid_f(unordered_set_));
				AF_TEST_RESULT(false, is_value_grid_f(unordered_multiset_));
				AF_TEST_RESULT(false, is_value_grid_f(unordered_map_));
				AF_TEST_RESULT(false, is_value_grid_f(unordered_multimap_));
				AF_TEST_RESULT(false, is_value_grid_f(stack_));
				AF_TEST_RESULT(false, is_value_grid_f(queue_));
				AF_TEST_RESULT(false, is_value_grid_f(priority_queue_));
				AF_TEST_RESULT(false, is_value_grid_f(string_));
				AF_TEST_RESULT(false, is_value_grid_f(123));
				AF_TEST_RESULT(false, is_value_grid_f(string_.begin()));
				AF_TEST_RESULT(false, is_value_grid_f(vector_.begin()));
				AF_TEST_RESULT(false, is_value_grid_f(*vector_vector_.begin()));

			}
		}
	}
}

AF_DECLARE_TEST_SET("StdDisambiguation", std_disambiguation, 
	autotelica::examples::std_disambiguation::examples<>() , autotelica::examples::std_disambiguation::tests<>());

