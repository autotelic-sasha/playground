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


		// Predicates
		// Useful when composing std like sfinae conditions. 

		// All the types used with these predicates are assumed to be testing 
		// conditions with the result of the test exposed via a static boolean constant "value"
		// (like std meta programming library types do).
		// 
		// sometimes we just need a type
		template<bool value_v>
		struct const_t : public std::integral_constant<bool, value_v> {};

		// if_t enablest a definition if its argument evaluates to true.
		template <typename condition_t>
		using if_t = std::enable_if_t<condition_t::value, bool>;

		// using SFINAE with class specialisation is a lot like switch statements
		// see _AF_DECLARE_HAS_MEMBER at the bottom for example of use
		template<typename condition_t>
		using case_t = if_t<condition_t>;

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
		template<typename T>
		using is_bitset_t = std_disambiguation::is_bitset_t<T>;

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

		// a few handy little helpers
		template<typename T>
		using is_pointer_t = any_of_t<is_shared_ptr_t<T>, std::is_pointer<T>>;
		
		// map where key is not a string
		template<typename T>
		using is_non_string_map_t = all_of_t<is_mapish_t<T>,not_t<is_string_t<typename T::key_t>>>;
		// map where key is a string
		template<typename T>
		using is_string_map_t = all_of_t<is_mapish_t<T>,is_string_t<typename T::key_t>>;
		// is a pair where first type is not a string 
		template<typename T>
		using is_non_string_pair_t = all_of_t<is_pair_t<T>,not_t<is_string_t<typename T::first_type>>>;
		// a pair where first type is a string 
		template<typename T>
		using is_string_pair_t = all_of_t<is_pair_t<T>, is_string_t<typename T::first_type>>;
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
		using if_pointer_t = if_t<is_pointer_t<T>>;

		// choose if T is a bitset
		template<typename T>
		using if_bitset_t = if_t<is_bitset_t<T>>;


		// choose if T is a map not indexed by strings
		template<typename T>
		using if_non_string_map_t = if_t<is_non_string_map_t<T>>;

		// choose if T is a map indexed by strings
		template<typename T>
		using if_string_map_t = if_t<is_string_map_t<T>>;

		// choose if T is a pair where first type is not a string 
		template<typename T>
		using if_non_string_pair_t = if_t<is_non_string_pair_t<T>>;

		// choose if T is a pair where first type is a string 
		template<typename T>
		using if_string_pair_t = if_t<is_string_pair_t<T>>;

		// unused marks things as unused, so compilers don't moan
		template <typename... Args> inline void _unused(Args&&...) {}
	
		// function_signatures namespace contains utilities for inspecting functions
		// 
		// the syntax becomes boring, so the core functionlity is wrapped in macros:
		// 
		//		_af_number_of_arguments(function_)	is constexpr number of arguments a function takes
		//		_af_return_type(function_)			is the return type of a function
		//		_af_argument_type(function_, i)		is the type of the i-th argument
		//
		//		_af_signature(function_)				creates the type for signature_t trait
		//
		//  signature_t can be created for function pointers, member function pointers, 
		//	static member function pointers, functor objects, and lambdas
		//
		//	if created succesfully, it exposes:
		//		size_t arguments_n				constexpr number of arguments the function takes
		//		return_t						the return type of a function
		//		template<int i> argument_t		the type of the i-th argument
		//
		//	the signatures can be traversed in SAX like fashion, more details below and in examples.
		namespace function_signatures {

			// to deal with functors (including lambdas) we need a predicate to detect them
			template<typename T, typename switch_t = bool>
			struct is_functor_impl_t : std::false_type {};
			template<typename T >
			struct is_functor_impl_t<T,
				case_t<std::is_member_pointer<decltype(&T::operator())>>> : std::true_type {};
			template<typename T >
			struct is_functor_t : is_functor_impl_t<T>::type {};
			template<typename T> using if_is_functor_t = if_t<is_functor_t<T>>;
			template<typename T> using if_is_not_functor_t = if_t<not_t<is_functor_t<T>>>;

			// signature_t implements all the clever bits
			template<typename T> struct signature_t;

			// function pointers
			template<typename return_type>
			struct signature_t<return_type(*)(void)> {
				using return_t = return_type;
				static constexpr size_t arguments_n = 0;
			};
			template<typename return_type, typename ... argument_types>
			struct signature_t<return_type(*)(argument_types...)> {
				using return_t = return_type;
				static constexpr size_t arguments_n = sizeof...(argument_types);

				using argument_types_t = std::tuple<argument_types...>;

				template <int i>
				using argument_t = typename std::tuple_element<i, argument_types_t>::type;
			};

			// member function pointers
			template<typename return_type, typename class_type>
			struct signature_t<return_type(class_type::*)(void)> {
				using return_t = return_type;
				static constexpr size_t arguments_n = 0;
			};
			template<typename return_type, typename class_type, typename ... argument_types>
			struct signature_t<return_type(class_type::*)(argument_types...)> {
				using return_t = return_type;
				static constexpr size_t arguments_n = sizeof...(argument_types);

				using argument_types_t = std::tuple<argument_types...>;

				template <int i>
				using argument_t = typename std::tuple_element<i, argument_types_t>::type;
			};

			// member function pointers to const members
			template<typename return_type, typename class_type>
			struct signature_t<return_type(class_type::*)(void) const> {
				using return_t = return_type;
				static constexpr size_t arguments_n = 0;
			};

			template<typename return_type, typename class_type, typename ... argument_types>
			struct signature_t<return_type(class_type::*)(argument_types...) const> {
				using return_t = return_type;
				static constexpr size_t arguments_n = sizeof...(argument_types);

				using argument_types_t = std::tuple<argument_types...>;

				template <int i>
				using argument_t = typename std::tuple_element<i, argument_types_t>::type;
			};

			// SFINAE helpers to deduce the type of signature_t
			template<typename function_t, if_is_functor_t<function_t> = true>
			constexpr signature_t<decltype(&function_t::operator())> make_signature_t(function_t) {
				return signature_t<decltype(&function_t::operator())>();
			}

			template<typename function_t, if_is_not_functor_t<function_t> = true>
			constexpr signature_t<function_t> make_signature_t(function_t) {
				return signature_t<function_t>();
			}

			// the macros are basically the API to this funcionality
#define _af_signature(function_) decltype(autotelica::sfinae::function_signatures::make_signature_t(function_))
#define _af_number_of_arguments(function_) _af_signature(function_)::arguments_n
#define _af_return_type(function_) typename _af_signature(function_)::return_t
#define _af_argument_type(function_, i) typename _af_signature(function_)::argument_t<i>


			// SAX type signature information traversals
			// This works with a traverser object. 
			// A traverser object is any object that implements following methods:
			//
			// 
			// template<size_t i>
			// inline void number_of_arguments();
			//
			// template<typename return_t>
			// inline void return_type();
			//
			// template<typename argument_t, size_t i>
			// inline void argument_type();
			//
			// inline void done();
			//
			// return_type get_result() const;
			//
			// NB: return_type is whatever makes sense fot that particular traverser
			//
			template<typename signature_t, typename traverser_t, int i>
			struct traverse_arguments_t {
				static inline void traverse(traverser_t& traverser_) {
					using arg_t = typename signature_t::template argument_t<i - 1>;
					traverse_arguments_t<signature_t, traverser_t, i - 1>::traverse(traverser_);
					traverser_.template argument_type<arg_t, i>();

				}
			};
			template<typename signature_t, typename traverser_t>
			struct traverse_arguments_t<signature_t, traverser_t, 0> {
				static inline void traverse(traverser_t& /*unused*/) {
				}
			};

			template<typename signature_t, typename traverser_t>
			class traverse_signature_t {

				traverser_t& _traverser;


			public:
				traverse_signature_t(traverser_t& traverser_) :_traverser(traverser_) {
				}

				void traverse() {
					const size_t argsn = signature_t::arguments_n;
					_traverser.template number_of_arguments<argsn>();
					_traverser.template return_type<typename signature_t::return_t>();
					traverse_arguments_t<signature_t, traverser_t, argsn>::traverse(_traverser);
					_traverser.done();
				}
			};

			// to traverse a signature, implement a traverser, and then invoke traverse_signature
			template<typename signature_t, typename traverser_t>
			auto traverse_signature() -> typename _af_signature(&traverser_t::get_result)::return_t{
				traverser_t traverser_;
				traverse_signature_t<signature_t, traverser_t> t(traverser_);
				t.traverse();
				return traverser_.get_result();
			}

			template<typename signature_t, typename traverser_t>
			auto traverse_signature(traverser_t& traverser_) -> typename _af_signature(&traverser_t::get_result)::return_t{
				traverse_signature_t<signature_t, traverser_t> t(traverser_);
				t.traverse();
				return traverser_.get_result();
			}

		} // namespace function_signatures

	}
}

// _AF_DECLARE_HAS_MEMBER declares a sfinae predicate 
// which is true if its type parameter has a member with a given name
// (when the member is a template, you must pass as further arguments types with which the template can be instantiated)
// examples:
//		_AF_DECLARE_HAS_MEMBER(f1)
//			has_f1_t<A> is std::true_type if class A implements a public member or a public static f
//			if_has_f1_t<A> is an alias for if_t<has_f1_t<A>>
// 
//		_AF_DECLARE_HAS_MEMBER(f2, int, int)
//			has_f2_t<A> is std::true_type if class A implements a public member template 
//			or a public static template f2 which can be instantiated with template parameters int and int
//			if_has_f2_t<A> is an alias for if_t<has_f2_t<A>>
// 
#define _AF_DECLARE_HAS_MEMBER(function_name, ...)\
	template<typename T, typename switch_t = bool>\
	struct has_##function_name##_impl_t : std::false_type {};\
	template<typename T>\
	struct has_##function_name##_impl_t<T,\
		case_t<std::is_function<decltype(T::function_name)>>> : std::true_type {};\
	template<typename T>\
	struct has_##function_name##_impl_t<T, \
		case_t<std::is_function<decltype(T::template function_name<__VA_ARGS__>)>>> : std::true_type {};\
	template<typename T >\
	struct has_##function_name##_impl_t<T, \
		case_t<std::is_member_pointer<decltype(&T::function_name)>>> : std::true_type {};\
	template<typename T >\
	struct has_##function_name##_impl_t<T, \
		case_t<std::is_member_pointer<decltype(&T::template function_name<__VA_ARGS__>)>>> : std::true_type {};\
	template<typename T>\
	struct has_##function_name##_t : has_##function_name##_impl_t<T>::type {};\
	template<typename T> using if_has_##function_name##_t = if_t<has_##function_name##_t<T>>;



// _AF_DECLARE_HAS_SUBTYPE declares a sfinae predicate 
// which is true if its type parameter has a subtype with the given name implemented
// example:
//		_AF_DECLARE_HAS_SUBTYPE(some_type);
//		has_some_type_t<A> is std::true_type if A has a subtype some_type
//		if_has_some_type_t<A> is an alias for if_t<has_some_type_t<A>>
#define _AF_DECLARE_HAS_SUBTYPE(subtype_t)\
	template<typename T, typename switch_t = bool>\
	struct has_##subtype_t##_impl_t : std::false_type {};\
	template<typename T>\
	struct has_##subtype_t##_impl_t<T, case_t<not_t<std::is_same<typename T::subtype_t, void>>>> : std::true_type {};\
	template<typename T>\
	struct has_##subtype_t##_t : has_##subtype_t##_impl_t<T>::type {};\
	template<typename T>\
	using if_has_##subtype_t##_t = if_t<has_##subtype_t##_t<T>>;
