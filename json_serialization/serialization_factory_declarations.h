#pragma once

// A serialization factory for every serialization type must be a class implementing 
// two public static functions:
//
//  using namespace autotelica::serialization;
//
//	template<target_t>
//	using default_value_t =  traits::default_types_t<target_t>::value_t;
// 
//	template<target_t>
//	using default_contained_t =  traits::default_types_t<target_t>::contained_t;
// 
//	template<typename target_t>
//	static serialization_handler_p make_handler(
//		target_t* target_,
//		default_value_t<target_t> const* default_value_,				// nullptr means no default
//		default_contained_t<target_t> const* contained_default_value_	// nullptr means no default
//	);
//
//
//
// template<typename object_t>
// static serialization_handler_p make_object_handler(
//		object_t& object_,
//		default_value_t<object_t> const* default_,							// nullptr means no default
//		handlers_t const& handlers_,
//		traits::setup_function_t pre_load_f_,				// nullptr means no function	
//		traits::setup_function_t post_load_f_,				// nullptr means no function	
//		traits::setup_function_t pre_save_f_,				// nullptr means no function	
//		traits::setup_function_t post_save_f_				// nullptr means no function	
//	);
// 
// Because types can be recursive, the handler for object types will also need a way to be constructed 
// from a type_description. Sorry, can't avoid recursion in the implementation when it comes this naturally.
//

// To add a new serialization type factory:
// 1) implement it as above
// 2) add an entry to the serialization_type_t enum
// 3) declare it using AF_DECLARE_SERIALIZATION_TYPE_FACTORY macro


enum class serialization_type_t : int {
	json,
	unknown // unknown must be the last
};

AF_DECLARE_SERIALIZATION_TYPE_FACTORY(serialization_type_t::json, autotelica::json::serialization_factory);
AF_DECLARE_SERIALIZATION_TYPE_FACTORY(serialization_type_t::unkown, impl::unknown_handlers_factory);

