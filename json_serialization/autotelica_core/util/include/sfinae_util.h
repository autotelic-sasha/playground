#pragma once
#include "std_disambiguation.h"


namespace autotelica {
	namespace sfinae {
		// Trickery to make use of SFINAE - can be instantiated only if the listed types exist
		// C++ 17 has this built in (and calls it std::void_t)     
		template<typename ... Ts>
		using if_types_exist_t = void;

		template<typename T>
		using if_exists_t = void;

		template<typename T>
		using select_t = void; // sometimes this makes things more readable

		// Predicates
		// Useful when composing std like sfinae conditions. 

		// All the types used with these predicates are assumed to be testing 
		// conditions with the result of the test exposed via a static boolean constant "value"
		// (like std meta programming library types do).
		// 

		// if_t enablest a definition if its argument evaluates to true.
		template <typename condition_t>
		using if_t = std::enable_if_t<condition_t::value, bool>;

		// not_t is a simple negation
		template <typename condition_t>
		struct not_t {
			static const bool value = !(condition_t::value);
		};


		// all_of_t is only true if all the conditions passed to it are true.
		// In predicate calculus is it equivalend to "all" operator.
		// With a single argument it evaluates to whatever the value of that argument is.
		// With two arguments it is equivalent to boolean "and".
		// With multiple arguments it ends up being a left folding over the arguments with "and" operator. 
		template<typename... conditions_t>
		struct all_of_t {};

		template <typename condition_t>
		struct all_of_t<condition_t> {
			static const bool value = condition_t::value;
		};

		template <typename condition_t, typename... conditions_t >
		struct all_of_t<condition_t, conditions_t...> {
			static const bool value =
				condition_t::value && all_of_t<conditions_t...>::value
				;
		};

		// any_of_t is true if any of the conditions passed to it are true.
		// In predicate calculus is it equivalend to "exists" operator.
		// With a single argument it evaluates to whatever the value of that argument is.
		// With two arguments it is equivalent to boolean "or".
		// With multiple arguments it ends up being a left folding over the arguments with "or" operator. 
		template<typename... conditions_t>
		struct any_of_t {};

		template <typename condition_t>
		struct any_of_t<condition_t> {
			static const bool value = condition_t::value;
		};

		template <typename condition_t, typename... conditions_t >
		struct any_of_t<condition_t, conditions_t...> {
			static const bool value =
				condition_t::value || any_of_t<conditions_t...>::value
				;
		};

		// none_of_t is a shorthand for "not any",
		// it is only true if none of its arguments are true.
		// With a single argument it is equivalent to boolean "not".
		template<typename... conditions_t>
		using none_of_t = not_t<any_of_t<conditions_t...>>;

		// one_of_t is true if exactly one of the conditions passed to it are true.
		// With a single argument it evaluates to whatever the value of that argument is.
		// With two arguments it is equivalent to boolean "xor".
		// With multiple arguments it ends up being a left folding over the arguments with "xor" operator. 
		template<typename... conditions_t>
		struct one_of_t {};

		template <typename condition_t>
		struct one_of_t<condition_t> {
			static const bool value = condition_t::value;
		};

		template <typename condition_t, typename... conditions_t >
		struct one_of_t<condition_t, conditions_t...> {
			static const bool value =
				condition_t::value ?
				(none_of_t<conditions_t...>::value) :
				(one_of_t<conditions_t...>::value)
				;
		};

		// Testing for common types.

		// strings
		// is_string detects either std::string or std::wstring
		template<typename T>
		using is_string_t = std_disambiguation::is_string_t<T>;
		// is_astring detects only std::string
		template<typename T>
		using is_astring_t = std_disambiguation::is_astring_t<T>;
		// is_wstring detects only std::wstring
		template<typename T>
		using is_wstring_t = std_disambiguation::is_wstring_t<T>;
		// is_cstring_t detects c-style strings
		template<typename T, typename U = bool>
		struct is_cstring_impl : std::false_type {};

		template<typename T>
		struct is_cstring_impl<T,
			if_t<all_of_t<
			std::is_constructible<std::string, T>,
			not_t<is_astring_t<T>>
			>>> : std::true_type {};

		template<typename T>
		struct is_cstring_t : is_cstring_impl<T>::type {};
		// is_wcstring_t detects c-style wide strings
		template<typename T, typename U = bool>
		struct is_wcstring_impl : std::false_type {};

		template<typename T>
		struct is_wcstring_impl<T,
			if_t<all_of_t<
			std::is_constructible<std::wstring, T>,
			not_t<is_wstring_t<T>>
			>>> : std::true_type {};

		template<typename T>
		struct is_wcstring_t : is_wcstring_impl<T>::type {};

		// basic std types
		template<typename T>
		using is_shared_ptr_t = std_disambiguation::is_shared_ptr_t<T>;
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
		using if_string_t = if_t<is_string_t<T>>;
		template<typename T>
		using if_astring_t = if_t<is_astring_t<T>>;
		template<typename T>
		using if_wstring_t = if_t<is_wstring_t<T>>;
		template<typename T>
		using if_cstring_t = if_t<is_cstring_t<T>>;
		template<typename T>
		using if_wcstring_t = if_t<is_wcstring_t<T>>;
		template<typename T>
		using if_pair_t = if_t<is_pair_t<T>>;
		template<typename T>
		using if_initializer_list_t = if_t<is_initializer_list_t<T>>;
		template<typename T>
		using if_vector_t = if_t<is_vector_t<T>>;
		template<typename T>
		using if_deque_t = if_t<is_deque_t<T>>;
		template<typename T>
		using if_forward_list_t = if_t<is_forward_list_t<T>>;
		template<typename T>
		using if_list_t = if_t<is_list_t<T>>;
		template<typename T>
		using if_set_t = if_t<is_set_t<T>>;
		template<typename T>
		using if_multiset_t = if_t<is_multiset_t<T>>;
		template<typename T>
		using if_map_t = if_t<is_map_t<T>>;
		template<typename T>
		using if_multimap_t = if_t<is_multimap_t<T>>;
		template<typename T>
		using if_unordered_set_t = if_t<is_unordered_set_t<T>>;
		template<typename T>
		using if_unordered_multiset_t = if_t<is_unordered_multiset_t<T>>;
		template<typename T>
		using if_unordered_map_t = if_t<is_unordered_map_t<T>>;
		template<typename T>
		using if_unordered_multimap_t = if_t<is_unordered_multimap_t<T>>;
		template<typename T>
		using if_stack_t = if_t<is_stack_t<T>>;
		template<typename T>
		using if_queue_t = if_t<is_queue_t<T>>;
		template<typename T>
		using if_priority_queue_t = if_t<is_priority_queue_t<T>>;

		// kinds of containers
		// is sequence detects vector or deque or forward_list or list
		template<typename T>
		using if_sequence_t = if_t<is_sequence_t<T>>;
		// is setish detects set or multiset or unordered_set or unordered_multiset
		template<typename T>
		using if_setish_t = if_t<is_setish_t<T>>;
		// is setish detects map or multimap or unordered_map or unordered_multimap
		template<typename T>
		using if_mapish_t = if_t<is_mapish_t<T>>;
		// is associative detects anything that is mapish or setish
		template<typename T>
		using if_associative_t = if_t<is_associative_t<T>>;
		// is_adaptor detects stack or queue or priority_queue
		template<typename T>
		using if_adaptor_t = if_t<is_adaptor_t<T>>;
		// is_indexable is badly named - it detects containers through which you can 
		// navigate by adding a constant to begin() iterator
		// all sequences, anything setish, and strings
		template<typename T>
		using if_indexable_t = if_t<is_indexable_t<T>>;
		// is_container detects anything indexable, anything mapish, and all adaptors
		template<typename T>
		using if_container_t = if_t<is_container_t<T>>;
		// a value_grid is a sequence of sequences of strings or non-containers
		// basically a simple representation of a table
		// e.g. vector<vector<string>>>, list<vector<double>> etc
		template<typename T>
		using if_value_grid_t = if_t<is_value_grid_t<T>>;


		// choose if T is an integral type
		template<typename T>
		using if_integral_t = if_t<std::is_integral<T>>;

		// choose if T is a floating point type
		template<typename T>
		using if_floating_point_t = if_t<std::is_floating_point<T>>;

		// choose if T is an enum type
		template<typename T>
		using if_enum_t = if_t<std::is_enum<T>>;

		// choose if T is a pointer type
		template<typename T>
		using if_pointer_t = if_t<
			any_of_t<is_shared_ptr_t<T>, std::is_pointer<T>>>;

		// choose if T is a map not indexed by strings
		template<typename T>
		using if_non_string_map_t = if_t<
			all_of_t<
			is_mapish_t<T>,
			not_t<is_string_t<typename T::key_t>>>>;

		// choose if T is a map indexed by strings
		template<typename T>
		using if_string_map_t = if_t<
			all_of_t<
			is_mapish_t<T>,
			is_string_t<typename T::key_t>>>;


		// unused marks things as unused, so compilers don't moan
		template <typename... Args> inline void _unused(Args&&...) {}
	}
}

// _AF_DECLARE_HAS_STATIC_MEMBER declares a sfinae predicate 
// which is true if it's type parameter has a static function with a given name 
#define _AF_DECLARE_HAS_STATIC_MEMBER(function_name) \
	template<typename T, typename U = void>\
	struct has_static_##function_name##_impl : std::false_type {};\
	template<typename T>\
	struct has_static_##function_name##_impl<T, \
		std::is_function<decltype(T::function_name)>> : std::true_type {};\
	template<typename T>\
	struct has_static_##function_name## : has_static_##function_name##_impl<T>::type {};\
	template<typename T> using if_has_static_##function_name##_t = if_t<has_static_##function_name##<T>>;

// _AF_DECLARE_HAS_MEMBER declares a sfinae predicate 
// which is true if it's type parameter has a non-static member with a given name
#define _AF_DECLARE_HAS_MEMBER(function_name) \
	template<typename T, typename U = void>\
	struct has_##function_name##_impl : std::false_type {};\
	template<typename T>\
	struct has_##function_name##_impl<T, std::enable_if_t<\
		std::is_member_pointer<decltype(&T::function_name)>::value>> : std::true_type {};\
	template<typename T>\
	struct has_##function_name## : has_##function_name##_impl<T>::type {};\
	template<typename T> using if_has_##function_name##_t = if_t<has_##function_name##<T>>;
