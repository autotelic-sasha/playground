#pragma once
#include "std_disambiguation.h"


namespace autotelica {
	namespace sfinae {
		// Trickery to make use of SFINAE - can be instantiated only if the listed types exist
		// C++ 17 has this built in (and calls it std::void_t)     
		template<typename ... Ts>
		using if_types_exist = void;

		template<typename T>
		using if_exists = void;

		template<typename T>
		using void_t_ = void;

		template<typename T>
		using select_t = void;


		// basic std types
		template<typename T>
		using is_shared_ptr_t = std_disambiguation::is_shared_ptr_t<T>;
		// is_string detects either std::string or std::wstring
		template<typename T>
		using is_string_t = std_disambiguation::is_string_t<T>;
		// is_astring detects only std::string
		template<typename T>
		using is_astring_t = std_disambiguation::is_astring_t<T>;
		// is_wstring detects only std::wstring
		template<typename T>
		using is_wstring_t = std_disambiguation::is_wstring_t<T>;
		template<typename T>
		using is_pair_t = std_disambiguation::is_pair_t<T>;
		template<typename T>
		using is_initializer_list_t = std_disambiguation::is_initializer_list_t<T>;
		template<typename T>
		using is_vector_t = std_disambiguation::is_vector_t<T>;
		template<typename T>
		using is_deque_t = std_disambiguation::is_deque_t<T>;
		template<typename T>
		using is_forward_list_t = std_disambiguation::is_forward_list_t<T>;
		template<typename T>
		using is_list_t = std_disambiguation::is_list_t<T>;
		template<typename T>
		using is_set_t = std_disambiguation::is_set_t<T>;
		template<typename T>
		using is_multiset_t = std_disambiguation::is_multiset_t<T>;
		template<typename T>
		using is_map_t = std_disambiguation::is_map_t<T>;
		template<typename T>
		using is_multimap_t = std_disambiguation::is_multimap_t<T>;
		template<typename T>
		using is_unordered_set_t = std_disambiguation::is_unordered_set_t<T>;
		template<typename T>
		using is_unordered_multiset_t = std_disambiguation::is_unordered_multiset_t<T>;
		template<typename T>
		using is_unordered_map_t = std_disambiguation::is_unordered_map_t<T>;
		template<typename T>
		using is_unordered_multimap_t = std_disambiguation::is_unordered_multimap_t<T>;
		template<typename T>
		using is_stack_t = std_disambiguation::is_stack_t<T>;
		template<typename T>
		using is_queue_t = std_disambiguation::is_queue_t<T>;
		template<typename T>
		using is_priority_queue_t = std_disambiguation::is_priority_queue_t<T>;

		// kinds of containers
		// is sequence detects vector or deque or forward_list or list
		template<typename T>
		using is_sequence_t = std_disambiguation::is_sequence_t<T>;
		// is setish detects set or multiset or unordered_set or unordered_multiset
		template<typename T>
		using is_setish_t = std_disambiguation::is_setish_t<T>;
		// is setish detects map or multimap or unordered_map or unordered_multimap
		template<typename T>
		using is_mapish_t = std_disambiguation::is_mapish_t<T>;
		// is associative detects anything that is mapish or setish
		template<typename T>
		using is_associative_t = std_disambiguation::is_associative_t<T>;
		// is_adaptor detects stack or queue or priority_queue
		template<typename T>
		using is_adaptor_t = std_disambiguation::is_adaptor_t<T>;
		// is_indexable is badly named - it detects containers through which you can 
		// navigate by adding a constant to begin() iterator
		// all sequences, anything setish, and strings
		template<typename T>
		using is_indexable_t = std_disambiguation::is_indexable_t<T>;
		// is_container detects anything indexable, anything mapish, and all adaptors
		template<typename T>
		using is_container_t = std_disambiguation::is_container_t<T>;
		// a value_grid is a sequence of sequences of strings or non-containers
		// basically a simple representation of a table
		// e.g. vector<vector<string>>>, list<vector<double>> etc
		template<typename T>
		using is_value_grid_t = std_disambiguation::is_value_grid_t<T>;


		// Selectors
		// Predicates to use a particular implementation of a templated function
		// for commonly used types.

		template<typename T>
		using if_shared_ptr_t = std::enable_if_t<is_shared_ptr_t<T>::value, bool>;
		// is_string detects either std::string or std::wstring
		template<typename T>
		using if_string_t = std::enable_if_t<is_string_t<T>::value, bool>;
		// is_astring detects only std::string
		template<typename T>
		using if_astring_t = std::enable_if_t<is_astring_t<T>::value, bool>;
		// is_wstring detects only std::wstring
		template<typename T>
		using if_wstring_t = std::enable_if_t<is_wstring_t<T>::value, bool>;
		template<typename T>
		using if_pair_t = std::enable_if_t<is_pair_t<T>::value, bool>;
		template<typename T>
		using if_initializer_list_t = std::enable_if_t<is_initializer_list_t<T>::value, bool>;
		template<typename T>
		using if_vector_t = std::enable_if_t<is_vector_t<T>::value, bool>;
		template<typename T>
		using if_deque_t = std::enable_if_t<is_deque_t<T>::value, bool>;
		template<typename T>
		using if_forward_list_t = std::enable_if_t<is_forward_list_t<T>::value, bool>;
		template<typename T>
		using if_list_t = std::enable_if_t<is_list_t<T>::value, bool>;
		template<typename T>
		using if_set_t = std::enable_if_t<is_set_t<T>::value, bool>;
		template<typename T>
		using if_multiset_t = std::enable_if_t<is_multiset_t<T>::value, bool>;
		template<typename T>
		using if_map_t = std::enable_if_t<is_map_t<T>::value, bool>;
		template<typename T>
		using if_multimap_t = std::enable_if_t<is_multimap_t<T>::value, bool>;
		template<typename T>
		using if_unordered_set_t = std::enable_if_t<is_unordered_set_t<T>::value, bool>;
		template<typename T>
		using if_unordered_multiset_t = std::enable_if_t<is_unordered_multiset_t<T>::value, bool>;
		template<typename T>
		using if_unordered_map_t = std::enable_if_t<is_unordered_map_t<T>::value, bool>;
		template<typename T>
		using if_unordered_multimap_t = std::enable_if_t<is_unordered_multimap_t<T>::value, bool>;
		template<typename T>
		using if_stack_t = std::enable_if_t<is_stack_t<T>::value, bool>;
		template<typename T>
		using if_queue_t = std::enable_if_t<is_queue_t<T>::value, bool>;
		template<typename T>
		using if_priority_queue_t = std::enable_if_t<is_priority_queue_t<T>::value, bool>;

		// kinds of containers
		// is sequence detects vector or deque or forward_list or list
		template<typename T>
		using if_sequence_t = std::enable_if_t<is_sequence_t<T>::value, bool>;
		// is setish detects set or multiset or unordered_set or unordered_multiset
		template<typename T>
		using if_setish_t = std::enable_if_t<is_setish_t<T>::value, bool>;
		// is setish detects map or multimap or unordered_map or unordered_multimap
		template<typename T>
		using if_mapish_t = std::enable_if_t<is_mapish_t<T>::value, bool>;
		// is associative detects anything that is mapish or setish
		template<typename T>
		using if_associative_t = std::enable_if_t<is_associative_t<T>::value, bool>;
		// is_adaptor detects stack or queue or priority_queue
		template<typename T>
		using if_adaptor_t = std::enable_if_t<is_adaptor_t<T>::value, bool>;
		// is_indexable is badly named - it detects containers through which you can 
		// navigate by adding a constant to begin() iterator
		// all sequences, anything setish, and strings
		template<typename T>
		using if_indexable_t = std::enable_if_t<is_indexable_t<T>::value, bool>;
		// is_container detects anything indexable, anything mapish, and all adaptors
		template<typename T>
		using if_container_t = std::enable_if_t<is_container_t<T>::value, bool>;
		// a value_grid is a sequence of sequences of strings or non-containers
		// basically a simple representation of a table
		// e.g. vector<vector<string>>>, list<vector<double>> etc
		template<typename T>
		using if_value_grid_t = std::enable_if_t<is_value_grid_t<T>::value, bool>;


		// choose if T is an integral type
		template<typename T>
		using if_integral_t = std::enable_if_t<
			std::is_integral<T>::value, bool>;

		// choose if T is a floating point type
		template<typename T>
		using if_floating_point_t = std::enable_if_t<
			std::is_floating_point<T>::value, bool>;

		// choose if T is an enum type
		template<typename T>
		using if_enum_t = std::enable_if_t<
			std::is_enum<T>::value, bool>;

		// choose if T is a pointer type
		template<typename T>
		using if_pointer_t = std::enable_if_t<
			is_shared_ptr_t<T>::value || std::is_pointer<T>::value, bool>;

		// choose if T is a map not indexed by strings
		template<typename T>
		using if_non_string_map_t = std::enable_if_t<
			(is_mapish_t<T>::value && !(is_string_t<typename T::key_t>::value)), bool>;

		// choose if T is a map indexed by strings
		template<typename T>
		using if_string_map_t = std::enable_if_t<
			(is_mapish_t<T>::value && is_string_t<typename T::key_t>::value), bool>;

	
		// unused marks things as unused, so compilers don't moan
		template <typename... Args> inline void _unused(Args&&...) {}

	}


}

// _AF_DECLARE_HAS_STATIC_MEMBER declares a sfinae predicate 
// which is true if it's type parameter has a static function with a given name 
#define _AF_DECLARE_HAS_STATIC_MEMBER(function_name) \
	template<typename T, typename U = void>\
	struct af_has_##function_name##_impl : std::false_type {};\
	template<typename T>\
	struct af_has_##function_name##_impl<T, \
		std::is_function<decltype(T::function_name)>> : std::true_type {};\
	template<typename T>\
	struct af_has_##function_name## : af_has_##function_name##_impl<T>::type {};

// _AF_DECLARE_HAS_MEMBER declares a sfinae predicate 
// which is true if it's type parameter has a non-static member with a given name
#define _AF_DECLARE_HAS_MEMBER(function_name) \
	template<typename T, typename U = void>\
	struct af_has_##function_name##_impl : std::false_type {};\
	template<typename T>\
	struct af_has_##function_name##_impl<T, std::enable_if_t<\
		std::is_member_pointer<decltype(&T::function_name)>::value>> : std::true_type {};\
	template<typename T>\
	struct af_has_##function_name## : af_has_##function_name##_impl<T>::type {};
