#pragma once
#include "serialization_util.h"

#ifndef		_AF_SERIALIZATION_VALIDATE_DUPLICATE_KEYS
#define		_AF_SERIALIZATION_VALIDATE_DUPLICATE_KEYS false
#endif

namespace autotelica {
	namespace type_description {
		using namespace autotelica::sfinae;
		using namespace autotelica::serialization;

		// the api from and object to type_description layers is a single static function:
		// static type_description_t const& type_description()

		// Type descriptions can be used to produce handlers for many different kinds of 
		// serialization. 
		// We also want it to be fast and as much of it compile time deduced as we can make it be. 
		// So we use static polymorphism a lot (yep, that means sfinae). 
		// 
		// Some interaction is always going to be required, and the 
		// use of static polymorphism means that we need clever mechanics in place to 
		// be able to extend the framework with new serialization types. 
		// 
		// First we need a few concepts:
		//		We try to keep a list of supported data types to a small, generic set by grouping them into theoretical abstractions. 
		//			Namespace 'traits' contains a list of sfinae predicates to distinguish all supported groups of types.
		// 
		//		Serialization type is a the input/output data format. For example: JSON, CSV, Excel, flatbuffer ...
		// 
		//		A handlers is an object that deals with serialization of a given type. 
		//			They will be implemented using sfinae predicates mentioned above, one for each supported group of types.
		//			One set of handlers for each serialization type. 
		// 
		//		Defaulting is challenging to do generically. To be efficient, we can choose to not serialize 
		//			anything for the variables that are equal to their default values. We call this 'terse' serialisation. 
		//			We will assume that all target objects can be default constructed, and that the c++ can do a better job of 
		//			default initialization than we can. 
		//			It doesn't always make sense to do support terse serialization, it depends on the serialization type (not 
		//			all formats have sensible null values, nor do they all want to show them). It should never be an error to 
		//			ask for terse serialization though, it may just not have any effect.
		//			The users of our libraries can opt for non-terse serialization when they want to be verbose in their outputs,
		//			or when they make decisions about default initialisation efficiency. 
		// 
		//		Handler makers. When implementing handlers for serialization types, there is a convoluted step that 
		//			I cannot find a way to avoid without adding a lot more dynamic type resolutions and virtual function calls.
		//			The types of handlers that need to be created depend on the types of members that they are handling. 
		//			To be able to rely on static type based dispatching, we make use of a variation of abstract factory pattern. 
		//			For each serialization kind there should be a way to access a 'maker' function that is templated both on the 
		//			type of the member being serialized and the object that contains the member. 
		//			 
		



		template<typename T>
		using if_has_default_t = std::enable_if_t<
			!std::is_same<has_no_default_t, std::remove_cv<T>>::value,
			bool>;

		template<typename T>
		using if_has_no_default_t = std::enable_if_t<
			std::is_same<has_no_default_t, std::remove_cv<T>>::value,
			bool>;

		template<typename target_t, if_has_default_t<target_t> = true>
		default_value_p<target_t> make_default_value(target_t const& v) {
			return std::make_shared<default_value_t<target_t>>(v);
		}
		// all instances of values with no defaults are the same, so 
		// we use a static value here, to spare the heap a little 
		template<typename target_t, if_has_no_default_t<target_t> = true>
		default_value_p<target_t> make_default_value(target_t const& v) {
			static default_value_p<target_t> no_default(
				std::make_shared<default_value_t<target_t>>(v));
			return no_default;
		}

		// a member descriptions is a description of a type's data member
		// it has a key, a default value, and a pointer to member
		// there's a bit of a hierarchy to hoist the templated instances for storage
		// In particular, there is a version of abstract factory pattern implementation 
		// required for each serialization implementation, in order to pass the type information
		// from the member description to the method creating a handler for a particular data type.

		// type_member_description
		template<typename object_t, typename factory_t>
		struct member_description_t {
			using key_t = typename traits::key_t;

			key_t _key;

			key_t const& key() const { return _key; }
			member_description_t(key_t const& key_) :_key(key_) {}
			virtual ~member_description_t() {}

			virtual serialization_handler_p make_handler(object_t& object) const = 0;

		};

		template<typename object_t, typename factory_t>
		using member_description_p = std::shared_ptr<member_description_t<object_t, factory_t>>;

		template<typename object_t, typename target_t, typename factory_t>
		struct member_description_instance_t : public member_description_t<object_t, factory_t> {

			using base_t = member_description_t<object_t, factory_t>;
			using key_t = typename base_t::key_t;
			using target_p = target_t object_t::*;
			using default_t = typename traits::default_types_t<target_t>::value_t;
			using contained_t = typename traits::default_types_t<target_t>::contained_t;
			using target_default_p = typename traits::default_p<target_t>;
			using target_contained_default_p = typename traits::default_contained_p<target_t>;

			target_p const _target;
			target_default_p const _default_value;
			target_contained_default_p const _contained_default_value;

			member_description_instance_t(
				key_t const& key_,
				target_p target_,
				target_default_p default_value_ = nullptr,
				target_contained_default_p contained_default_value_ = nullptr
			) :
				base_t(key_),
				_target(target_),
				_default_value(default_value_),
				_contained_default_value(contained_default_value_) {

			}

			serialization_handler_p make_handler(object_t& object_) const override {
				
				target_t* target = &(object_.*(_target));

				return factory_t::make_handler(
					target,
					_default_value,
					_contained_default_value);
			}

		};
		
		template<typename object_t, typename target_t, typename factory_t>
		using member_description_instance_p = std::shared_ptr<member_description_instance_t<object_t, target_t, factory_t>>;

		template<typename object_t, typename target_t, typename factory_t >
		member_description_p<object_t, factory_t> make_member_description(
			typename member_description_instance_t<object_t, target_t, factory_t>::key_t const& key_,
			typename member_description_instance_t<object_t, target_t, factory_t>::target_p target_,
			typename member_description_instance_t<object_t, target_t, factory_t>::target_default_p default_value_ = nullptr,
			typename member_description_instance_t<object_t, target_t, factory_t>::target_contained_default_p contained_default_value_ = nullptr
		) {
			return std::make_shared<member_description_instance_t<object_t, target_t, factory_t>>(
				key_, target_, default_value_, contained_default_value_);
		}


		// type description
		template<typename object_t, typename factory_t>
		class type_description_t {
		public:
			using key_t = typename traits::key_t;
			using default_t = typename traits::default_types_t<object_t>::value_t;
			using member_function_t = void (object_t::*)();
			using member_description_p = member_description_p<object_t, factory_t>;
			using member_descriptions_t = std::vector<member_description_p>;

			class object_description_t {
				using setup_function_t = traits::setup_function_t;
				using handlers_t = traits::handlers_t;
				using default_p = traits::default_p<object_t>;

				object_t& _object;
				default_p _default;
				handlers_t _handlers;
				setup_function_t _pre_load_f;
				setup_function_t _post_load_f;
				setup_function_t _pre_save_f;
				setup_function_t _post_save_f;
				friend class type_description_t;
			public:

				object_description_t(
					object_t& object_,
					default_p default_,
					handlers_t const& handlers_,
					setup_function_t pre_load_f_,
					setup_function_t post_load_f_,
					setup_function_t pre_save_f_,
					setup_function_t post_save_f_) :
					_object(object_), _default(default_), _handlers(handlers_),
					_pre_load_f(pre_load_f_), _post_load_f(post_load_f_),
					_pre_save_f(pre_save_f_), _post_save_f(post_save_f_) {

				}

				inline object_t& object() { return _object; }
				inline default_p const& default_() const { return _default; }
				inline handlers_t const& handlers() const { return _handlers; }
				inline setup_function_t const& pre_load_f() const { return _pre_load_f; }
				inline setup_function_t const& post_load_f() const { return _post_load_f; }
				inline setup_function_t const& pre_save_f() const { return _pre_save_f; }
				inline setup_function_t const& post_save_f() const { return _post_save_f; }

			};
			using object_description_p = std::shared_ptr<object_description_t>;
			inline object_description_p make_object_description(
				object_t& object_,
				traits::default_p<object_t> default_,
				traits::handlers_t const& handlers_,
				traits::setup_function_t pre_load_f_,
				traits::setup_function_t post_load_f_,
				traits::setup_function_t pre_save_f_,
				traits::setup_function_t post_save_f_
			) const {
				return std::make_shared<object_description_t>(
					object_,
					default_,
					handlers_,
					pre_load_f_,
					post_load_f_,
					pre_save_f_,
					post_save_f_);
			}
		protected:
			bool _done;
			member_function_t _pre_load_f;
			member_function_t _pre_save_f;
			member_function_t _post_load_f;
			member_function_t _post_save_f;
			member_descriptions_t _member_descriptions;

			inline traits::setup_function_t wrap_function(object_t& object, member_function_t f) const {
				return f ? [&]() {(object.*f)(); } : traits::setup_function_t();
			}

			inline void validate_key(key_t const& key) {
#ifdef _AF_SERIALIZATION_VALIDATE_DUPLICATE_KEYS
				AF_ASSERT(!key.empty(), "Empty keys are not allowed");
				auto test_key = [&](member_description_p d) { return !util::equal_tag(key, d->key()); };
				for (auto const& d : _member_descriptions) {
					AF_ASSERT(test_key(d), "Key % is duplicated", key);
				}
#endif
			}
		public:
			type_description_t() :
				_done(false),
				_pre_load_f(nullptr),
				_pre_save_f(nullptr),
				_post_load_f(nullptr),
				_post_save_f(nullptr)
			{
			}

			bool done() const { return _done; }
			member_function_t const& pre_load_f() const { return _pre_load_f; }
			member_function_t const& pre_save_f() const { return _pre_save_f; }
			member_function_t const& post_load_f() const { return _post_load_f; }
			member_function_t const& post_save_f() const { return _post_save_f; }
			member_descriptions_t const& member_descriptions() const { return _member_descriptions; }

			inline type_description_t& before_loading(member_function_t& f) {
				AF_ASSERT(!_done, "Cannot append description data once end_object is invoked.");
				_pre_load_f = f;
				return *this;
			}
			inline type_description_t& before_saving(member_function_t& f) {
				AF_ASSERT(!_done, "Cannot append description data once end_object is invoked.");
				_pre_save_f = f;
				return *this;
			}
			inline type_description_t& after_loading(member_function_t& f) {
				AF_ASSERT(!_done, "Cannot append description data once end_object is invoked.");
				_post_load_f = f;
				return *this;
			}
			inline type_description_t& after_saving(member_function_t& f) {
				AF_ASSERT(!_done, "Cannot append description data once end_object is invoked.");
				_post_save_f = f;
				return *this;
			}
			template<typename target_t, typename element_t>
			inline type_description_t& member(
				key_t const& key_,
				target_t object_t::* target_,
				target_t const& default_,
				element_t const& element_default_
			) {
				AF_ASSERT(!_done, "Cannot append description data once end_object is invoked.");
				validate_key(key_);
				auto member_description(make_member_description<object_t, target_t, factory_t>(
					key_, target_, &default_, &element_default_));
				_member_descriptions.push_back(member_description);
				return *this;
			}
			template<typename target_t>
			inline type_description_t& member(
				typename traits::key_t const& key_,
				target_t object_t::* target_,
				has_no_default_t const& default_ = has_no_default,
				has_no_default_t const& element_default_ = has_no_default
			) {
				AF_ASSERT(!_done, "Cannot append description data once end_object is invoked.");
				validate_key(key_);
				auto member_description(make_member_description<object_t, target_t, factory_t>(
					key_, target_));
				_member_descriptions.push_back(member_description);
				return *this;
			}

			template<typename target_t>
			inline type_description_t& member(
				typename traits::key_t const& key_,
				target_t object_t::* target_,
				target_t const& default_,
				has_no_default_t const& element_default_ = has_no_default
			) {
				AF_ASSERT(!_done, "Cannot append description data once end_object is invoked.");
				validate_key(key_);
				auto member_description(make_member_description<object_t, target_t, factory_t>(
					key_, target_, make_default_value(default_)));
				_member_descriptions.push_back(member_description);
				return *this;
			}

			inline type_description_t& end_object() {
				AF_ASSERT(!_done, "Cannot end_object more than once.");
				_done = true;
				return *this;
			}

			object_description_p make_object_description(
				object_t& object,
				default_t const* default_ = nullptr
			) const {
				auto od = make_object_description(
					object,
					make_default(default_),
					traits::handlers_t(),
					wrap_function(object, _pre_load_f),
					wrap_function(object, _post_load_f),
					wrap_function(object, _pre_save_f),
					wrap_function(object, _post_save_f)
				);
				od->_handlers.reserve(_member_descriptions.size());
				for (auto const d : _member_descriptions)
					od->_handlers.push_back({
						d->key(),
						(d->make_handler(object)) });
				return od;
			}

		};
		template<typename object_t, typename factory_t>
		type_description_t<object_t, factory_t> begin_object() {
			return type_description_t<object_t, factory_t>();
		}
	}// namespace type_description

}// namespace autotelica