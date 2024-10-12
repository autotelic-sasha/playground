#pragma once
#include <functional>
#include "autotelica_core/util/include/asserts.h"
#include "autotelica_core/util/include/sfinae_util.h"
#include "autotelica_core/util/include/enum_to_string.h"
// What kind of strings do you like? The default works for UTF8, why would you want anything else? 
#ifndef _AF_SERIALIZATION_CHAR_T
#define _AF_SERIALIZATION_CHAR_T char
#define _AF_CHAR_CONSTANT(VALUE) VALUE
// for wide character use, you have to define both of the ones below
//#define _AF_SERIALIZATION_CHAR_T wchar_t
//#define _AF_CHAR_CONSTANT(VALUE) L##VALUE
#endif

// In terse mode, when target value is equal to default, we just don't write it
#ifndef		_AF_SERIALIZATION_TERSE
#define		_AF_SERIALIZATION_TERSE false
#endif

namespace autotelica {
	namespace serialization {


		// has_no_default_t is a type tag to flag members that have no default values
		struct has_no_default_t {};
		constexpr has_no_default_t has_no_default;

		// to mark that a value has no default, we use a special value default_value_t<has_no_default_t>
		// it makes it prettier when we write type descriptions and also provides disambiguation 
		// between null default values and no-default values. 
		template<typename target_t>
		struct default_value_t {
			target_t const _default_value;

			default_value_t(target_t const& default_value_) :_default_value(default_value_) {}

			inline void set_value(target_t& target_) {
				target_ = _default_value;
			}

			// TODO: do we need should_not_write?
			bool should_not_write(target_t const& target_) const {
				// double negatives here, but makes it easier to read code later
				// in terse mode, when _target is equal to default, we just don't write it
#if _AF_SERIALIZATION_TERSE
				return (target_ == _default_value);
#else
				return false;
#endif
			}

		};
		template<typename target_t>
		using default_value_p = std::shared_ptr<default_value_t<target_t>>;

		// All handlers must be based on this. 
		struct serialization_handler_t {
			virtual ~serialization_handler_t() {}
		};
		using serialization_handler_p = std::shared_ptr<serialization_handler_t>;

		namespace util {
			namespace predicates {
				using namespace autotelica::sfinae;
				// declare predicates has_static_type_description and if_has_static_type_description, 
				// which checks if a type implements type description API
				_AF_DECLARE_HAS_STATIC_MEMBER(type_description);

				template<typename target_t>
				using is_serializable_object_t = has_static_type_description<target_t>;
				template<typename T>
				using if_serializable_object_t = if_t<is_serializable_object_t<T>>;

				template<typename target_t>
				using is_integral_t = std::is_integral<target_t>;
				template<typename target_t>
				using is_floating_point_t = std::is_floating_point<target_t>;
				template<typename target_t>
				using is_enum_t = std::is_enum<target_t>;

				template<typename target_t>
				using is_has_no_default_t = std::is_same<has_no_default_t, std::remove_cv_t<target_t>>;

				template<typename target_t>
				using if_has_no_default_t = if_t<is_has_no_default_t<target_t>>;

				template<typename target_t>
				using if_no_contained_default_t = if_t<
					any_of_t<
						is_integral_t<target_t>,
						is_floating_point_t<target_t>,
						is_string_t<target_t>,
						is_enum_t<target_t>,
						is_pair_t<target_t>,
						is_serializable_object_t<target_t>
					>
				>;
			}
		}
		namespace traits {
			// the type of characters used is compile-time configurable
			using char_t = _AF_SERIALIZATION_CHAR_T;
			using string_t = std::basic_string<char_t>;
			using key_t = string_t; // type of keys for tagged serialisation formats (json, xml etc)
			using tag_t = char_t const* const;

			using handlers_t = std::vector<std::pair<traits::key_t, serialization_handler_p>>;
			
			using setup_function_t = std::function<void()>;
			
			using namespace autotelica::sfinae;
			using namespace util::predicates;

			template<typename T, typename U = void>
			struct default_types_t : public std::false_type {};

			template<typename target_t>
			struct default_types_t< target_t, select_t<if_no_contained_default_t<target_t>> > : public std::true_type {
				using value_t = target_t;
				using contained_t = has_no_default_t;
			};

			template<typename target_t>
			struct default_types_t< target_t, select_t<if_pointer_t<target_t>>> : public std::true_type {
				using value_t = target_t;
				using contained_t = decltype(*(target_t()));
			};

			template<typename target_t>
			struct default_types_t< target_t, select_t<if_sequence_t<target_t>>> : public std::true_type {
				using value_t = target_t;
				using contained_t = typename target_t::value_type;
			};

			template<typename target_t>
			struct default_types_t< target_t, select_t<if_setish_t<target_t>>> : public std::true_type {
				using value_t = target_t;
				using contained_t = typename target_t::value_type;
			};

			template<typename target_t>
			struct default_types_t< target_t, select_t<if_pair_t<target_t>>> : public std::true_type {
				using value_t = has_no_default_t;
				using contained_t = typename target_t::second_type;
			};

			template<typename target_t>
			struct default_types_t< target_t, select_t<if_mapish_t<target_t>>> : public std::true_type {
				using value_t = target_t;
				using contained_t = typename target_t::value_type;
			};

			template<typename target_t>
			struct default_types_t< target_t, select_t<if_has_no_default_t<target_t>> > : public std::true_type {
				using value_t = has_no_default_t;
				using contained_t = has_no_default_t;
			};


			template<typename target_t>
			using default_t = typename default_types_t<target_t>::value_t;
			
			template<typename target_t>
			using default_p = default_value_p<default_t<target_t>>;

			template<typename target_t>
			using default_contained_t =  typename default_types_t<target_t>::contained_t;

			template<typename target_t>
			using default_contained_p = default_value_p<default_contained_t<target_t>>;



		}
		namespace util {	
			// basic operations on keys (comparissons and assignments) 
			static inline bool equal_tag(traits::char_t const* s, size_t n, traits::char_t const* t) {
				for (size_t i = 0; i < n; ++i, ++s, ++t)
					if (*t != *s) return false;
				return true;
			}
			static inline bool equal_tag(traits::key_t const& s, traits::key_t const& t) {
				return equal_tag(s.c_str(), s.size(), t.c_str());
			}
			static inline void assign(traits::key_t* target, const traits::char_t* src, size_t length) {
				target->assign(src, length);
			}
		}
		
		// A serialization factory for every serialization type must be a class implementing 
		// two public static functions:
		//
		//  using namespace autotelica::serialization;
		//  using namespace autotelica::serialization::traits;
		// 
		//	template<typename target_t>
		//	static serialization_handler_p make_handler(
		//		target_t*								target_,
		//		traits::default_p<target_t>				default_,			// nullptr means no default
		//		traits::default_contained_p<target_t>>	contained_default_	// nullptr means no default
		//	);
		//
		//
		//
		// template<typename object_t>
		// static serialization_handler_p make_object_handler(
		//		object_t&								object_,
		//		traits::default_p<target_t>				default_value_,		// nullptr means no default
		//		handlers_t const&						handlers_,
		//		traits::setup_function_t				pre_load_f_,	// nullptr means no function	
		//		traits::setup_function_t				post_load_f_,	// nullptr means no function	
		//		traits::setup_function_t				pre_save_f_,	// nullptr means no function	
		//		traits::setup_function_t				post_save_f_	// nullptr means no function	
		//	);
		// 
		// Because types can be recursive, the handler for object types will also need a way to be constructed 
		// from a type_description. Sorry, can't avoid recursion in the implementation when it comes this naturally.


		namespace impl {
			// We use tagged static forwarding, wrapping these two functions in sfinae resolved templates.
			// It is all a bit messy, but we are going to wrap it into a macro to make it all simple to use.
#define AF_DECLARE_SERIALIZATION_TYPE_FACTORY(serialization_type_v, factory_t)\
	template<typename target_t, serialization_type_t id, std::enable_if_t<id == serialization_type_v, bool> = true>\
	inline serialization_handler_p make_handler(\
		target_t*								target_,\
		traits::default_p<target_t>				default_,\
		traits::default_contained_p<target_t>	contained_default_){\
			return make_handler(target_, default_, contained_default_);\
		}\
	template<typename object_t, serialization_type_t id, std::enable_if_t<id == serialization_type_v, bool> = true>\
	static serialization_handler_p make_object_handler(\
		object_t&					object_,\
		traits::default_p<object_t>	default_,\
		traits::handlers_t const&	handlers_,\
		traits::setup_function_t	pre_load_f_,\
		traits::setup_function_t	post_load_f_,\
		traits::setup_function_t	pre_save_f_,\
		traits::setup_function_t	post_save_f_){\
			return factory_t::make_object_handler(object_, default_, handlers_, pre_load_f_, post_load_f_, pre_save_f_, post_save_f_);\
		}
		
		// unknown_handlers_factory is there to catch errors in specifying required handlers
		struct unknown_handlers_factory {
			template<typename target_t>
			static serialization_handler_p make_handler(
				target_t*								target_, 
				traits::default_p<target_t>				default_, 
				traits::default_contained_p<target_t> contained_default_
			) {
				AF_ERROR("Unknown handler factory was invoked.");
				return nullptr;
			}

			template<typename object_t>
			static serialization_handler_p make_object_handler(
				object_t&					object_, 
				traits::default_p<object_t> default_,
				traits::handlers_t const&	handlers_, 
				traits::setup_function_t	pre_load_f_, 
				traits::setup_function_t	post_load_f_,
				traits::setup_function_t	pre_save_f_, 
				traits::setup_function_t	post_save_f_
			) {
				AF_ERROR("Unknown handler factory was invoked.");
				return nullptr;
			}
		};
		
		}

		// Serialization types declarations.
#include "serialization_factory_declarations.h"
		
		namespace impl {
			
			template<
				typename target_t,
				serialization_type_t serialization_type_,
				std::enable_if_t<(static_cast<int>(serialization_type_) < 0), bool> = true>
			inline serialization_handler_p dispatch_make_handler(
				serialization_type_t const				serialization_type_v,
				target_t*								target_,
				traits::default_p<target_t>				default_,
				traits::default_contained_p<target_t> contained_default_
			) {
				AF_ERROR("Unrecognised serialisation type value: %", serialization_type_v);
				return nullptr;
			}

			template<
				typename target_t,
				serialization_type_t serialization_type_,
				std::enable_if_t<(static_cast<int>(serialization_type_) >= 0), bool> = true>
			inline serialization_handler_p dispatch_make_handler(
				serialization_type_t const				serialization_type_v,
				target_t*								target_,
				traits::default_p<target_t>				default_,
				traits::default_contained_p<target_t> contained_default_
			) {

				const auto prev = (static_cast<serialization_type_t>(static_cast<int>(serialization_type_) - 1));

				return (serialization_type_v == serialization_type_) ?
					make_handler<target_t, serialization_type_>(target_, default_, contained_default_) :
					dispatch_make_handler<target_t, prev>(serialization_type_v, target_, default_, contained_default_);
			}

			template<
				typename object_t,
				serialization_type_t serialization_type_,
				std::enable_if_t<(static_cast<int>(serialization_type_) < 0), bool> = true>
			inline serialization_handler_p dispatch_make_object_handler(
				serialization_type_t const	 serialization_type_v,
				object_t&					object_,
				traits::default_p<object_t> default_,
				traits::handlers_t const&	handlers_,
				traits::setup_function_t	pre_load_f_,
				traits::setup_function_t	post_load_f_,
				traits::setup_function_t	pre_save_f_,
				traits::setup_function_t	post_save_f_
			) {
				AF_ERROR("Unrecognised serialisation type value: %", serialization_type_v);
				return nullptr;
			}
			template<
				serialization_type_t serialization_type_,
				typename object_t,
				std::enable_if_t<(static_cast<int>(serialization_type_) >= 0), bool> = true>
			inline serialization_handler_p dispatch_make_object_handler(
				serialization_type_t const	  serialization_type_v,
				object_t&					 object_,
				traits::default_p<object_t > default_,
				traits::handlers_t const&	 handlers_,
				traits::setup_function_t	 pre_load_f_,
				traits::setup_function_t	 post_load_f_,
				traits::setup_function_t	 pre_save_f_,
				traits::setup_function_t	 post_save_f_
			) {
				const auto prev = (static_cast<serialization_type_t>(static_cast<int>(serialization_type_) - 1));

				return (serialization_type_v == serialization_type_) ?
					make_object_handler<object_t, serialization_type_>(
						object_, default_, handlers_, pre_load_f_, post_load_f_, pre_save_f_, post_save_f_) :
					dispatch_make_object_handler<object_t, prev>(serialization_type_v,
						object_, default_, handlers_, pre_load_f_, post_load_f_, pre_save_f_, post_save_f_);

			}

		}
		
		namespace serializations_factory {
			template<typename target_t>
			inline serialization_handler_p make_handler(
				serialization_type_t const				serialization_type_v,
				target_t*								target_,
				traits::default_p<target_t>				default_,
				traits::default_contained_p<target_t> contained_default_
			) {
				return impl::dispatch_make_handler<target_t, serialization_type_t::unknown>(
					serialization_type_v, target_, default_, contained_default_);
			}

			template<typename object_t>
			inline serialization_handler_p make_object_handler(
				serialization_type_t const	serialization_type_v,
				object_t&					object_,
				traits::default_p<object_t> default_,
				traits::handlers_t const&	handlers_,
				traits::setup_function_t	pre_load_f_,
				traits::setup_function_t	post_load_f_,
				traits::setup_function_t	pre_save_f_,
				traits::setup_function_t	post_save_f_
			) {
				return impl::dispatch_make_object_handler<object_t, serialization_type_t::unknown>(serialization_type_v,
					object_, default_, handlers_, pre_load_f_, post_load_f_, pre_save_f_, post_save_f_);
			}
		}
	}
}
