#pragma once

#pragma warning(disable:4267)

#ifndef _AF_JSON_READ_BUFFER_SIZE 
#define _AF_JSON_READ_BUFFER_SIZE 4*65535
#endif

// Optimisation of strings maps means that keys in the map are used as JSON keys.
// Otherwise they are written with "key", "value" pairs like other maps.
#ifndef		_AF_JSON_OPTIMISED_STRING_MAPS
#define		_AF_JSON_OPTIMISED_STRING_MAPS true
#endif

#include "type_description.h"

#include "autotelica_core/util/include/asserts.h"
#include "autotelica_core/util/include/enum_to_string.h"
#include "autotelica_core/util/include/string_util.h"
#include <string.h>
#include <cstdint>
// for some reason, probably good, rapidjson uses their own size_t
// we are going to just make that size_type
#define RAPIDJSON_NO_SIZETYPEDEFINE
namespace rapidjson { typedef size_t SizeType; }
#include "rapidjson/encodings.h"
#include "rapidjson/error/en.h"
#include "rapidjson/encodedstream.h"
#include "rapidjson/reader.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/writer.h"
#include "rapidjson/schema.h"


namespace autotelica {
namespace json {
	using namespace serialization;
	using namespace type_description;
	using namespace sfinae;

	enum class json_encoding {
		utf8,
		utf16le,
		utf16be,
		utf32le,
		utf32be,
		detect
	};

namespace impl {
	// rapidjson has this weird thing about writers - there's no hierarchy
	// but we want to make things really easy to use, so we are going to pay 
	// the cost of virtual function calls (at least until we see too much damage in profilers)
	// so we have this little wrapper hierarchy
	struct writer_wrapper_t {
		virtual ~writer_wrapper_t() {}

		using char_t = traits::char_t;

		virtual bool Null() = 0;
		virtual bool Bool(bool b) = 0;
		virtual bool Int(int i) = 0;
		virtual bool Uint(unsigned i) = 0;
		virtual bool Int64(int64_t i) = 0;
		virtual bool Uint64(uint64_t i) = 0;
		virtual bool Double(double d) = 0;
		virtual bool RawNumber(const char_t* str, size_t length, bool copy) = 0;
		virtual bool String(const char_t* str, size_t length, bool copy) = 0;
		virtual bool StartObject() = 0;
		virtual bool Key(const char_t* str, size_t length, bool copy) = 0;
		virtual bool EndObject(size_t memberCount) = 0;
		virtual bool StartArray() = 0;
		virtual bool EndArray(size_t elementCount) = 0;
	};

	template<typename writer_t>
	struct writer_wrapper_impl_t : public writer_wrapper_t {
		writer_t& _writer;
		using char_t = traits::char_t;

		writer_wrapper_impl_t(writer_t& writer_) :_writer(writer_) {}

		bool Null() override { return _writer.Null(); }
		bool Bool(bool b) override { return _writer.Bool(b); }
		bool Int(int i) override { return _writer.Int(i); }
		bool Uint(unsigned i) override { return _writer.Uint(i); }
		bool Int64(int64_t i) override { return _writer.Int64(i); }
		bool Uint64(uint64_t i)  override { return _writer.Uint64(i); }
		bool Double(double d) override { return _writer.Double(d); }
		bool RawNumber(const char_t* str, size_t length, bool copy) override { return _writer.RawNumber(str, length, copy); }
		bool String(const char_t* str, size_t length, bool copy) override { return _writer.String(str, length, copy); }
		bool StartObject() override { return _writer.StartObject(); }
		bool Key(const char_t* str, size_t length, bool copy) override { return _writer.Key(str, length, copy); }
		bool EndObject(size_t memberCount) override { return _writer.EndObject(memberCount); }
		bool StartArray() override { return _writer.StartArray(); }
		bool EndArray(size_t elementCount) override { return _writer.EndArray(elementCount); }
	};

	// writing simple types
	namespace writing {
		inline void write(int const& value, writer_wrapper_t& writer) { writer.Int(value); }
		inline void write(unsigned const& value, writer_wrapper_t& writer) { writer.Uint(value); }
		inline void write(short const& value, writer_wrapper_t& writer) { writer.Int(value); }
		inline void write(unsigned short const& value, writer_wrapper_t& writer) { writer.Uint(value); }
		inline void write(long const& value, writer_wrapper_t& writer) { writer.Int64(static_cast<std::int64_t>(value)); }
		inline void write(unsigned long const& value, writer_wrapper_t& writer) { writer.Uint64(static_cast<std::uint64_t>(value)); }
		inline void write(long long const& value, writer_wrapper_t& writer) { writer.Int64(static_cast<std::int64_t>(value)); }
		inline void write(unsigned long long const& value, writer_wrapper_t& writer) { writer.Uint64(static_cast<std::uint64_t>(value)); }

		inline void write(float const& value, writer_wrapper_t& writer) { writer.Double(static_cast<double>(value)); }
		inline void write(double const& value, writer_wrapper_t& writer) { writer.Double(value); }


		inline void write(bool const& value, writer_wrapper_t& writer) { writer.Bool(value); }

		inline void write(traits::string_t const& value, writer_wrapper_t& writer) {
			writer.String(value.c_str(), value.length(), false);
		}
	}

	// base class  for rapidjson SAX handlers
	struct handler_t : public serialization_handler_t {
		using char_t = traits::char_t;

		bool _done;
		handler_t() :_done(false) {}
		virtual ~handler_t() {}
		
		inline bool set_done(bool done_ = true) { return (_done = done_); }
		inline bool is_done() const { return _done; } // signal to the controller that we are done loading this value

		virtual void prepare_for_loading() {
			set_done(false);
		}

		virtual bool Null() { AF_ERROR("Null was not expected."); return false; }
		virtual bool Bool(bool b) { AF_ERROR("Boolean value was not expected."); return false; }
		virtual bool Int(int i) { AF_ERROR("Int value was not expected.");  return false; }
		virtual bool Uint(unsigned i) { AF_ERROR("Uint value was not expected.");  return false; }
		virtual bool Int64(int64_t i) { AF_ERROR("Int64 value was not expected."); return false; }
		virtual bool Uint64(uint64_t i) { AF_ERROR("Uint64 value was not expected."); return false; }
		virtual bool Double(double d) { AF_ERROR("Double value was not expected.");  return false; }
		virtual bool RawNumber(const char_t* str, size_t length, bool copy) { AF_ERROR("RawNumber value was not expected."); return false; }
		virtual bool String(const char_t* str, size_t length, bool copy) { AF_ERROR("String value was not expected."); return false; }
		virtual bool StartObject() { AF_ERROR("Object start was not expected."); return false; }
		virtual bool Key(const char_t* str, size_t length, bool copy) { AF_ERROR("Object key was not expected."); return false; }
		virtual bool EndObject(size_t memberCount) { AF_ERROR("Object end was not expected."); return false; }
		virtual bool StartArray() { AF_ERROR("Array start was not expected."); return false; }
		virtual bool EndArray(size_t elementCount) { AF_ERROR("Array end was not expected."); return false; }

		virtual bool Missing(traits::key_t const& key) { AF_ERROR("Missing was not expected (key %).", key); return false; }

		// we need to know if the value will actually be written when writing objects
		// for terse mode to work as expected
		virtual bool will_write() const = 0; 

		virtual void write(writer_wrapper_t& writer_) const = 0;
	};
	using handler_p = std::shared_ptr<handler_t>;

	// typed base for handlers
	template<typename target_t>
	struct handler_value_t : public handler_t {

		using char_t = traits::char_t;
		using base_t = handler_t;
		using default_p = traits::default_p<target_t>;
		// To make construction easier, all instances have to implement a constructor that 
		// takes a contained default value pointer. Ugly but effective. 
		using default_contained_p = traits::default_contained_p<target_t>;

		target_t* _target; // owned by someone else, don't delete
		default_p	_default;

		handler_value_t(
			target_t* target_,
			default_p default_ = nullptr) :
			_target(target_),
			_default(default_){
		}

		inline bool set(target_t const& value_) { 
			AF_ASSERT(_target, "Target is not initialised.");
			*_target = value_; 
			return set_done(); 
		}
		inline void set_default(default_p default_) {
			_default = default_;
		}
		template<typename ConvertibleT>
		inline bool set(ConvertibleT const& value_) { 
			AF_ASSERT(_target, "Target is not initialised.");
			*_target = static_cast<target_t>(value_); 
			return set_done();
		}

		inline void reset(target_t* target_) {
			if (target_ == _target) return;
			_target = target_;
			prepare_for_loading();
		}

		inline bool is_set() const { return _target != nullptr; }

		inline target_t const& get() const { return *_target; }
		inline target_t& get() { return *_target; }
		inline bool set_default() {
			if (_default)
				_default->set_value(*_target);
			set_done();
			return true;
		}
		bool Missing(traits::key_t const& key) override {
#if !_AF_SERIALIZATION_TERSE		
			AF_ASSERT(_default, "Value for key & is missing when default value is not provided.", key);
#endif
			return set_default();
		}

		bool Null()  override {
#if !_AF_SERIALIZATION_TERSE			
			AF_ASSERT(_default, "Unexpected: null read when default value is not provided.");
#endif
			return set_default();
		}

		inline bool should_not_write() const {
#if _AF_SERIALIZATION_TERSE && !_AF_VERBOSE_WRITES_ALWAYS
			return (
				_default &&
				_default->equal_to_default(*_target));// double negatives are fun
#else
			return false;
#endif
		}
		bool will_write() const override {
			// double double negative, yay!
			return !should_not_write();
		}
	};
	template<typename target_t>
	using handler_value_p = std::shared_ptr<handler_value_t<target_t>>;



	// handlers create other handlers, so we need a forward declaration
	struct serialization_factory;


	// handler for integral types
	template<typename target_t>
	struct handler_integral_t : public handler_value_t<target_t> {

		using base_t = handler_value_t<target_t>;
		using default_p = typename base_t::default_p;
		using default_contained_p = typename base_t::default_contained_p;
		
		handler_integral_t(
				target_t* target_,
				default_p default_ = nullptr,
				default_contained_p unused_ = nullptr) :
			base_t(target_, default_) {
			_unused(unused_);
		}

		bool Bool(bool b)  override { return base_t::set(b); }
		bool Int(int i)  override { return base_t::set(i); }
		bool Uint(unsigned i)  override { return base_t::set(i); }
		bool Int64(int64_t i)  override { return base_t::set(i); }
		bool Uint64(uint64_t i) override { return base_t::set(i); }

		void write(writer_wrapper_t& writer_) const override {
			if (base_t::should_not_write()) return;
			writing::write(*base_t::_target, writer_);
		}

	};

	// handler for bitsets
	template<typename target_t>
	struct handler_bitset_t : public handler_value_t<target_t> {

		using base_t = handler_value_t<target_t>;
		using default_p = typename base_t::default_p;
		using default_contained_p = typename base_t::default_contained_p;

		handler_bitset_t(
			target_t* target_,
			default_p default_ = nullptr,
			default_contained_p unused_ = nullptr) :
			base_t(target_, default_) {
			_unused(unused_);
		}

		bool Int(int i)  override { return base_t::set(base_t::target_t(static_cast<unsigned long>(i))); }
		bool Uint(unsigned i)  override { return base_t::set(base_t::target_t(static_cast<unsigned long>(i))); }
		bool Int64(int64_t i)  override { return base_t::set(base_t::target_t(static_cast<unsigned long>(i))); }
		bool Uint64(uint64_t i) override { return base_t::set(base_t::target_t(static_cast<unsigned long>(i))); }

		void write(writer_wrapper_t& writer_) const override {
			if (base_t::should_not_write()) return;
			writing::write(base_t::_target->to_ulong(), writer_);
		}

	};

	// handler for floating types
	template<typename target_t>
	struct handler_floating_t : public handler_value_t<target_t> {

		using base_t = handler_value_t<target_t>;
		using default_p = typename base_t::default_p;
		using default_contained_p = typename base_t::default_contained_p;

		handler_floating_t(
				target_t* target_,
				default_p default_ = nullptr,
				default_contained_p unused_ = nullptr) :
			base_t(target_, default_) {
			_unused(unused_);
		}

		bool Double(double d) override { return base_t::set(d); }
		bool Int(int i)  override { return base_t::set(i); }
		bool Uint(unsigned i)  override { return base_t::set(i); }
		bool Int64(int64_t i)  override { return base_t::set(i); }
		bool Uint64(uint64_t i) override { return base_t::set(i); }

		void write(writer_wrapper_t& writer_) const override {
			if (base_t::should_not_write()) return;
			writing::write(*base_t::_target, writer_);
		}
	};

	// handler for strings
	template<typename target_t>
	struct handler_string_t : public handler_value_t<target_t> {

		using base_t = handler_value_t<target_t>;
		using default_p = typename base_t::default_p;
		using default_contained_p = typename base_t::default_contained_p;
		using char_t = traits::char_t;

		handler_string_t(
				target_t* target_,
				default_p default_ = nullptr,
				default_contained_p unused_ = nullptr) :
			base_t(target_, default_) {
			_unused(unused_);
		}

		// reader part
		bool String(const char_t* str, size_t length, bool copy) override {
			util::assign(base_t::_target, str, length);
			return base_t::set_done();
		}

		void write(writer_wrapper_t& writer_) const override {
			if (base_t::should_not_write()) return;
			writing::write(*base_t::_target, writer_);
		}
	};

	// handler for enumerations
	template<typename target_t>
	struct handler_enum_t : public handler_value_t<target_t> {

		using base_t = handler_value_t<target_t>;
		using default_p = typename base_t::default_p;
		using default_contained_p = typename base_t::default_contained_p;
		using char_t = traits::char_t;
		using string_t = traits::string_t;

		handler_enum_t(
				target_t* target_,
				default_p default_ = nullptr,
				default_contained_p unused_ = nullptr) :
			base_t(target_, default_) {
			_unused(unused_);
		}

		// reader part
		bool String(const char_t* str, size_t length, bool copy) override {
			using namespace autotelica::enum_to_string;
			*base_t::_target = to_enum<target_t>(str);
			return base_t::set_done();
		}
		void write(writer_wrapper_t& writer_) const override {
			if (base_t::should_not_write()) return;
			string_t out;
			using autotelica::enum_to_string;
			to_string(out, *base_t::_target);
			writing::write(out, writer_);
		}
	};

	// many handlers delegate to other handlers
	// for those, we need to keep track of the point when we started delegation
	template<typename target_t>
	struct handler_delegating_t : public handler_value_t<target_t> {
		using base_t = handler_value_t<target_t>;
		using default_p = typename base_t::default_p;
		using default_contained_p = typename base_t::default_contained_p;
		using contained_t = traits::default_contained_t<target_t>;
		using char_t = traits::char_t;

		bool _started_loading;// set to true when we delegate the first json call

		handler_delegating_t(
			target_t* target_,
			default_p default_ = nullptr) :
			base_t(target_, default_),
			_started_loading(false) {

		}

		inline bool has_started_loading() const { return _started_loading; }
		inline void set_started_loading(bool const started = true) { _started_loading = started; }

		void prepare_for_loading() override {
			base_t::set_done(false);
			set_started_loading(false);
		}

	};

	// handler for pointers
	template<typename target_t>
	struct handler_ptr_t : public handler_delegating_t<target_t> {

		using base_t = handler_delegating_t<target_t>;
		using default_p = typename base_t::default_p;
		using default_contained_p = typename base_t::default_contained_p;
		using contained_t = traits::default_contained_t<target_t>;
		using char_t = traits::char_t;
		using value_handler_t = handler_value_p<contained_t>;

		value_handler_t _value_handler;

		handler_ptr_t(
				target_t* target_,
				default_p default_ = nullptr,
				default_contained_p contained_default_ = nullptr) :
			base_t(target_, default_),
			_value_handler(serialization_factory::make_handler(value_ptr(), contained_default_, nullptr)){
		}

		void prepare_for_loading() override {
			if (!(*base_t::_target))
				*base_t::_target = target_t(new contained_t());
			base_t::prepare_for_loading();
			_value_handler->reset(base_t::_target);
		}

		inline contained_t* value_ptr() {
			return &(*(*base_t::_target));// for shared_ptr this is _target->get(), but this should work for naked pointers too
		}

		template<typename... ParamsT>
		bool delegate_f(bool (value_handler_t::* mf)(ParamsT ...), ParamsT... ps) {
			base_t::set_started_loading();
			bool ret = (_value_handler->*mf)(ps...);
			base_t::set_done(_value_handler->is_done());
			return ret;
		}

		bool Null()  override {
			if (base_t::has_started_loading()) // if we are already reading the contained value, delegate
				return _value_handler->Null();
			return base_t::Null();
		}
		bool Bool(bool b) override { return delegate_f(&value_handler_t::Bool, b); }
		bool Int(int i) override { return delegate_f(&value_handler_t::Int, i); }
		bool Uint(unsigned i) override { return delegate_f(&value_handler_t::Uint, i); }
		bool Int64(int64_t i) override { return delegate_f(&value_handler_t::Int64, i); }
		bool Uint64(uint64_t i) override { return delegate_f(&value_handler_t::Uint64, i); }
		bool Double(double d) override { return delegate_f(&value_handler_t::Double, d); }
		bool RawNumber(const char_t* str, size_t length, bool copy) override { return delegate_f(&value_handler_t::RawNumber, str, length, copy); }
		bool String(const char_t* str, size_t length, bool copy) override { return delegate_f(&value_handler_t::String, str, length, copy); }
		bool StartObject()  override { return delegate_f(&value_handler_t::StartObject); }
		bool Key(const char_t* str, size_t length, bool copy) override { return delegate_f(&value_handler_t::Key, str, length, copy); }
		bool EndObject(size_t memberCount)  override { return delegate_f(&value_handler_t::EndObject, memberCount); }
		bool StartArray() override { return delegate_f(&value_handler_t::StartArray); }
		bool EndArray(size_t elementCount)  override { return delegate_f(&value_handler_t::EndArray, elementCount); }

		void write(writer_wrapper_t& writer_) const override {
			if (base_t::should_not_write()) return;
			_value_handler->reset(base_t::_target);
			_value_handler->write(writer_);
		}
	};

	// handler for sequences (lists and vectors)
	template<typename target_t>
	struct handler_sequence_t : public handler_delegating_t<target_t> {

		using base_t = handler_delegating_t<target_t>;
		using default_p = typename base_t::default_p;
		using default_contained_p = typename base_t::default_contained_p;
		using contained_t = typename target_t::value_type;
		using char_t = traits::char_t;
		using value_handler_t = handler_value_t<contained_t>;
		using value_handler_p = handler_value_p<contained_t>;

		value_handler_p _value_handler;
		const bool _as_object; // this is a little bit of a hack, so that we can use this same handler with maps

		handler_sequence_t(
				target_t* target_,
				default_p default_ = nullptr,
				default_contained_p contained_default_ = nullptr) :
			base_t(target_, default_),
			_value_handler(serialization_factory::make_handler<contained_t>(nullptr, contained_default_, nullptr)),
			_as_object(false){
		}
		handler_sequence_t(
			target_t* target_,
			bool as_object_,
			default_p default_ ,
			default_contained_p contained_default_ ) :
			base_t(target_, default_),
			_value_handler(serialization_factory::make_handler<contained_t>(nullptr, contained_default_, nullptr)),
			_as_object(as_object_) {
		}

		void prepare_for_loading() override {
			base_t::prepare_for_loading();
			_value_handler->prepare_for_loading();
		}
		virtual void set_next() {
			base_t::_target->emplace_back();
			contained_t& ret(base_t::_target->back());
			_value_handler->reset(&ret);
		}
		virtual void finish_loading_element() {
			_value_handler->reset(nullptr);
		}
		template<typename... ParamsT>
		inline bool delegate_f(bool (value_handler_t::* mf)(ParamsT ...), ParamsT... ps) {
			base_t::set_started_loading();
			if (!_value_handler->is_set())
				set_next();
			bool ret = ((*_value_handler).*mf)(ps...);
			if (_value_handler->is_done())
				finish_loading_element();
			return ret;
		}
		bool Null() override {
			if (!base_t::has_started_loading())
				return base_t::Null();
			return delegate_f(&value_handler_t::Null);
		}
		bool Bool(bool b) override { return delegate_f(&value_handler_t::Bool, b); }
		bool Int(int i) override { return delegate_f(&value_handler_t::Int, i); }
		bool Uint(unsigned i) override { return delegate_f(&value_handler_t::Uint, i); }
		bool Int64(int64_t i) override { return delegate_f(&value_handler_t::Int64, i); }
		bool Uint64(uint64_t i) override { return delegate_f(&value_handler_t::Uint64, i); }
		bool Double(double d) override { return delegate_f(&value_handler_t::Double, d); }
		bool RawNumber(const char_t* str, size_t length, bool copy) override { return delegate_f(&value_handler_t::RawNumber, str, length, copy); }
		bool String(const char_t* str, size_t length, bool copy) override { return delegate_f(&value_handler_t::String, str, length, copy); }
		bool StartObject() override { 
			if (_as_object && !base_t::has_started_loading()) {
				base_t::_target->clear();
				return true;
			}
			return delegate_f(&value_handler_t::StartObject);
		}
		bool Key(const char_t* str, size_t length, bool copy) { return delegate_f(&value_handler_t::String, str, length, copy); }
		bool EndObject(size_t memberCount) override { 
			if (_as_object && (!base_t::has_started_loading() || !_value_handler->is_set()))
				return base_t::set_done();

			return delegate_f(&value_handler_t::EndObject, memberCount); 
		}
		bool StartArray() override {
			if (!_as_object && !base_t::has_started_loading()) {
				base_t::_target->clear();
				return true;
			}
			return delegate_f(&value_handler_t::StartArray);
		}
		bool EndArray(size_t elementCount) override {
			if (!_as_object && (!base_t::has_started_loading() || !_value_handler->is_set()))
				return base_t::set_done(); 
			return delegate_f(&value_handler_t::EndArray, elementCount);
		}
		// writing 
		void write(writer_wrapper_t& writer_) const override {
			if (base_t::should_not_write()) return;
			if (_as_object)
				writer_.StartObject();
			else
				writer_.StartArray();
			for (auto& t : *(base_t::_target)) {
				_value_handler->reset(&t);
				_value_handler->write(writer_);
			}
			if (_as_object)
				writer_.EndObject(base_t::_target->size());
			else
				writer_.EndArray(base_t::_target->size());
			_value_handler->reset(nullptr);
		}
	};

	// handler for sets
	template<typename target_t>
	struct handler_setish_t : public handler_sequence_t<target_t> {

		using base_t = handler_sequence_t<target_t>;
		using default_p = typename base_t::default_p;
		using default_contained_p = typename base_t::default_contained_p;
		using contained_t = typename target_t::value_type;

		contained_t _current_value;

		handler_setish_t(
				target_t* target_,
				default_p default_ = nullptr,
				default_contained_p unused_ = nullptr) :
			base_t(target_, default_) {// sets canot contain default values
			_unused(unused_);
		}
		handler_setish_t(
			target_t* target_,
			bool as_object_,
			default_p default_ ,
			default_contained_p unused_ ) :
			base_t(target_, as_object_, default_, nullptr) {// sets canot contain default values
			_unused(unused_);
		}

		void set_next() override{
			_current_value = contained_t();
			base_t::_value_handler->reset(&_current_value);
		}
		void finish_loading_element() override {
			base_t::_target->insert(_current_value);
			base_t::_value_handler->reset(nullptr);
		}
	};


	// pairs can be stored more efficiently if the first type is a string
	// we will add a special handler for this

	template<typename target_t>
	struct handler_string_pair_t : public handler_delegating_t<target_t> {
		using base_t = handler_delegating_t<target_t>;
		using default_p = typename base_t::default_p;
		using default_contained_p = typename base_t::default_contained_p;
		using contained_t = traits::default_contained_t<target_t>;
		using char_t = traits::char_t;
		using key_t = traits::key_t;
		using value_t = typename target_t::second_type;
		using value_handler_t = handler_value_p<value_t>;

		using key_p = traits::key_t*;
		using value_p = typename target_t::second_type*;

		inline key_p key() { return base_t::is_set() ? (&(base_t::_target->first)) : nullptr; }
		inline value_p value() { return base_t::is_set() ? (&(base_t::_target->second)) : nullptr; }
		
		value_handler_t _value_handler;

		handler_string_pair_t(
			target_t* target_,
			default_p default_ = nullptr,
			default_contained_p unused_ = nullptr) :
				base_t(target_, default_),
				_value_handler(serialization_factory::make_handler(value(), nullptr, nullptr)) {
			_unused(unused_);
		}
		void prepare_for_loading() override {
			base_t::prepare_for_loading();
			if(key())
				key()->clear();
			_value_handler->prepare_for_loading();
		}

		inline void initialise_loading() {
			base_t::set_started_loading();
		}
		template<typename... ParamsT>
		bool delegate_f(bool (value_handler_t::* mf)(ParamsT ...), ParamsT... ps) {
			AF_ASSERT(base_t::has_started_loading(), "Reading value before the key in a string pair.");
			bool ret = (_value_handler->*mf)(ps...);
			base_t::set_done(_value_handler->is_done());
			return ret;
		}

		bool Null()  override {
			if (base_t::has_started_loading()) // if we are already reading the contained value, delegate
				return _value_handler->Null();
			return base_t::Null();
		}
		bool Bool(bool b) override { return delegate_f(&value_handler_t::Bool, b); }
		bool Int(int i) override { return delegate_f(&value_handler_t::Int, i); }
		bool Uint(unsigned i) override { return delegate_f(&value_handler_t::Uint, i); }
		bool Int64(int64_t i) override { return delegate_f(&value_handler_t::Int64, i); }
		bool Uint64(uint64_t i) override { return delegate_f(&value_handler_t::Uint64, i); }
		bool Double(double d) override { return delegate_f(&value_handler_t::Double, d); }
		bool RawNumber(const char_t* str, size_t length, bool copy) override { return delegate_f(&value_handler_t::RawNumber, str, length, copy); }
		bool String(const char_t* str, size_t length, bool copy) override { return delegate_f(&value_handler_t::String, str, length, copy); }
		bool StartObject()  override { 
			if (base_t::has_started_loading())
				return delegate_f(&value_handler_t::StartObject); 
			return true;
		}
		bool Key(const char_t* str, size_t length, bool copy) override { 
			if (base_t::has_started_loading())
				return delegate_f(&value_handler_t::Key, str, length, copy); 
			util::assign(key(), str, length);
			base_t::set_started_loading();
			return true;
		}
		bool EndObject(size_t memberCount)  override { 
			if (base_t::is_done())
				return true;
			return delegate_f(&value_handler_t::EndObject, memberCount); 
		}
		bool StartArray() override { return delegate_f(&value_handler_t::StartArray); }
		bool EndArray(size_t elementCount)  override { return delegate_f(&value_handler_t::EndArray, elementCount); }

		void write(writer_wrapper_t& writer_) const override {
			if (base_t::should_not_write()) return;
			writer_->StartObject();
			writing::write(*key(), writer_);
			_value_handler->reset(value());
			if(_value_handler->should_not_write())
				writer_->Null();
			else
				_value_handler->write(writer_);
			writer_->EndObject();
		}

	};


	namespace pair_tags {
		static traits::tag_t tag_key = _AF_CHAR_CONSTANT("key");
		static traits::tag_t tag_value = _AF_CHAR_CONSTANT("value");
	};

	// handling pairs
	template< typename target_t>
	struct handler_pair_t : public handler_delegating_t<target_t> {

		using base_t = handler_delegating_t<target_t>;
		using default_p = typename base_t::default_p;
		using default_contained_p = typename base_t::default_contained_p;
		using char_t = traits::char_t;

		using key_t = typename target_t::first_type;
		using value_t = typename target_t::second_type;
		using key_handler_p = handler_value_p<key_t>;
		using value_handler_p = handler_value_p<value_t>;
		using current_handler_t = handler_t;
		using current_handler_p = handler_p;

		inline key_t key() { return base_t::is_set() ? (&(base_t::_target->first)) : nullptr; }
		inline value_t value() { return base_t::is_set() ? (&(base_t::_target->second)) : nullptr; }

		key_handler_p _key_handler;
		value_handler_p _value_handler;
		current_handler_p _current_handler;
		bool _loaded_key;

		handler_pair_t(
				target_t* target_,
				default_p default_ = nullptr,
				default_contained_p unused_ = nullptr) :
			base_t(target_, default_),
			_key_handler(serialization_factory::make_handler(value(), nullptr, nullptr)),
			_value_handler(serialization_factory::make_handler(value(), nullptr, nullptr)),
			_current_handler(nullptr),
			_loaded_key(false){
			
			_unused(unused_);
		}

		void prepare_for_loading() override {
			base_t::prepare_for_loading();
			_key_handler->reset(key());
			_value_handler->reset(value());
			_loaded_key = false;
		}

		
		template<typename... ParamsT>
		bool delegate_f(bool (current_handler_t::* mf)(ParamsT ...), ParamsT... ps) {
			AF_ASSERT(_current_handler, "Unexpected value while reading a pair.");
			bool ret = (_current_handler->*mf)(ps...);
			if (_current_handler->is_done()) {
				base_t::set_done(_key_handler->is_done() && _value_handler->is_done());
				_current_handler = nullptr;
			}
			return ret;
		}

		bool Null() {
			if (base_t::has_started_loading())
				return delegate_f(&current_handler_t::Null);
			return base_t::Null();
		}
		bool Bool(bool b) { return delegate_f(&current_handler_t::Bool, b); }
		bool Int(int i) { return delegate_f(&current_handler_t::Int, i); }
		bool Uint(unsigned i) { return delegate_f(&current_handler_t::Uint, i); }
		bool Int64(int64_t i) { return delegate_f(&current_handler_t::Int64, i); }
		bool Uint64(uint64_t i) { return delegate_f(&current_handler_t::Uint64, i); }
		bool Double(double d) { return delegate_f(&current_handler_t::Double, d); }
		bool String(const char_t* str, size_t length, bool copy) { return delegate_f(&current_handler_t::String, str, length, copy); }
		bool StartObject() {
			if (base_t::has_started_loading())
				return delegate_f(&current_handler_t::StartObject);
			base_t::set_started_loading();
			return true;
		}
		bool Key(const char_t* str, size_t length, bool copy) {
			if (_current_handler)
				return delegate_f(&current_handler_t::Key, str, length, copy);
			if (util::equal_tag(str, length, pair_tags::tag_key))
				_current_handler = _key_handler;
			else if (util::equal_tag(str, length, pair_tags::tag_value) == 0)
				_current_handler = _value_handler;
			else
				AF_ERROR("Unknown key value: %", str);
			_current_handler->prepare_for_loading();
			return true;
		}
		bool EndObject(size_t memberCount) {
			if (_current_handler)
				return delegate_f(&current_handler_t::EndObject, memberCount);
			return base_t::set_done();
		}

		bool StartArray() { return delegate_f(&current_handler_t::StartArray); }
		bool EndArray(size_t elementCount) { return delegate_f(&current_handler_t::EndArray, elementCount); }

		void write(writer_wrapper_t& writer_) const override {
			if (base_t::should_not_write()) return;
			_key_handler->reset(key());
			_value_handler->reset(value());
			writer_->StartObject();
			writer_->Key(pair_tags::tag_key);
			if (_key_handler->should_not_write()) 
				writer_->Null();
			else
				_key_handler->write(writer_);
			
			writer_->Key(pair_tags::tag_value);
			if (_value_handler->should_not_write())
				writer_->Null();
			else
				_value_handler->write(writer_);
			writer_->EndObject();
		}
	};


	// handler for general maps
	template<typename target_t>
	struct handler_mapish_t : public handler_setish_t<target_t> {

		using base_t = handler_setish_t<target_t>;
		using default_p = typename base_t::default_p;
		using default_contained_p = typename base_t::default_contained_p;

		handler_mapish_t(
			target_t* target_,
			default_p default_ = nullptr,
			default_contained_p unused_ = nullptr) :
			base_t(target_, true, default_, nullptr) {// mapps canot contain default values
			_unused(unused_);
		}

	};
	
	// handler for objects
	template<typename target_t>
	struct handler_object_t : public handler_value_t<target_t> {

		using base_t = handler_value_t<target_t>;
		using default_p = typename base_t::default_p;
		using contained_t = typename traits::default_contained_t<target_t>;
		using char_t = traits::char_t;
		using key_t = traits::key_t; // this is some string type
		

		using handlers_t = std::vector<std::pair<key_t, handler_p>>;

		using pre_load_function_t = traits::setup_function_t;
		using pre_save_function_t = traits::setup_function_t;
		using post_load_function_t = traits::setup_function_t;
		using post_save_function_t = traits::setup_function_t;

		handlers_t	_handlers;
		handler_p _current_handler;
		pre_load_function_t _pre_load_f;
		pre_save_function_t _pre_save_f;
		post_load_function_t _post_load_f;
		post_save_function_t _post_save_f;

		handler_object_t(
				target_t* target_,
				default_p default_ = nullptr,
				pre_load_function_t pre_load_f_ = nullptr,
				pre_save_function_t pre_save_f_ = nullptr,
				post_load_function_t post_load_f_ = nullptr,
				post_save_function_t post_save_f_ = nullptr,
				traits::handlers_t const& handlers_ = {}) :
			base_t(target_, default_),
			_pre_load_f(pre_load_f_),
			_pre_save_f(pre_save_f_),
			_post_load_f(post_load_f_),
			_post_save_f(post_save_f_),
			_current_handler(nullptr) {

			_handlers.reserve(handlers_.size());
			for (auto const& h : handlers_)
				_handlers.push_back(
					{ h.first, std::static_pointer_cast<handler_t>(h.second) });
		}

		void prepare_for_loading() override {
			base_t::prepare_for_loading();
			for (auto& h : _handlers)
				h.second->prepare_for_loading();
			_current_handler = nullptr;
		}
		inline handler_p find_handler(const char_t* const key, size_t length) {
			for (auto& h : _handlers) {
				if (util::equal_tag(key, length, h.first.c_str()))
					return h.second;
			}
			return nullptr;
		}

		inline bool validate_all_loaded() {
#if _AF_SERIALIZATION_TERSE
			return true;
#else
			for (auto const& h : _handlers) {
				if (!h.second->is_done())
					h.second->Missing(h.first);
			}
			return true;
#endif
		}

		template<typename... ParamsT>
		inline bool delegate_f(bool (handler_t::* mf)(ParamsT ...), ParamsT... ps) {
			AF_ASSERT(_current_handler, "Unexpected value when loading object");
			bool ret = ((*_current_handler).*mf)(ps...);
			if (_current_handler->is_done())
				_current_handler = nullptr;
			return ret;
		}
		bool Null() override {
			if (_current_handler)
				return delegate_f(&handler_t::Null);
			return base_t::Null();
		}
		bool Bool(bool b) override { return delegate_f(&handler_t::Bool, b); }
		bool Int(int i) override { return delegate_f(&handler_t::Int, i); }
		bool Uint(unsigned i) override { return delegate_f(&handler_t::Uint, i); }
		bool Int64(int64_t i) override { return delegate_f(&handler_t::Int64, i); }
		bool Uint64(uint64_t i) override { return delegate_f(&handler_t::Uint64, i); }
		bool Double(double d) override { return delegate_f(&handler_t::Double, d); }
		bool RawNumber(const char_t* str, size_t length, bool copy) override { return delegate_f(&handler_t::RawNumber, str, length, copy); }
		bool String(const char_t* str, size_t length, bool copy) override { return delegate_f(&handler_t::String, str, length, copy); }
		bool StartObject() override {
			if (_current_handler)
				return delegate_f(&handler_t::StartObject);
			if (_pre_load_f)
				_pre_load_f();
			return true;
		}
		bool Key(const char_t* str, size_t length, bool copy) {
			if (_current_handler)
				return delegate_f(&handler_t::String, str, length, copy);

			_current_handler = find_handler(str, length);
			AF_ASSERT(_current_handler, "Unexpected key (%) when loading object.", str);
			return true;
		}
		bool EndObject(size_t memberCount) override {
			if (_current_handler)
				return _current_handler->EndObject(memberCount);
			validate_all_loaded();
			if (_post_load_f)
				_post_load_f();
			return base_t::set_done();
		}
		bool StartArray() override { return delegate_f(&handler_t::StartArray); }
		bool EndArray(size_t elementCount) override { return delegate_f(&handler_t::EndArray, elementCount); }

		void write(writer_wrapper_t& writer_) const override {
			if (_pre_save_f)
				_pre_save_f();
			if (base_t::should_not_write()) return;
			writer_.StartObject();
			for (auto const& h : _handlers) {
#if _AF_SERIALIZATION_TERSE && !_AF_VERBOSE_WRITES_ALWAYS
				if (h.second->will_write()) {
					writer_.Key(h.first.c_str(), h.first.size(), false);
					h.second->write(writer_);
				}
#else
				writer_.Key(h.first.c_str(), h.first.size(), false);
				h.second->write(writer_);
#endif
			}
			writer_.EndObject(_handlers.size());
			if (_post_save_f)
				_post_save_f();

		}
	};



	namespace handler_traits {
		using namespace serialization::traits::predicates;


		template<typename target_t, typename = void>
		struct handler_types {
		};

#define _JSON_HANDLER_TRAIT(HandlerT, ConditionT) \
		template<typename target_t>\
		struct handler_types<target_t, select_t<ConditionT>> {\
			using handler_type = HandlerT<target_t>;\
			using sfinae_condition_t = ConditionT;\
		};


		_JSON_HANDLER_TRAIT(handler_integral_t, if_integral_t<target_t>);
		_JSON_HANDLER_TRAIT(handler_bitset_t, if_bitset_t<target_t>);
		_JSON_HANDLER_TRAIT(handler_floating_t, if_floating_point_t<target_t>);
		_JSON_HANDLER_TRAIT(handler_string_t, if_string_t<target_t>);
		_JSON_HANDLER_TRAIT(handler_enum_t, if_enum_t<target_t>);
		_JSON_HANDLER_TRAIT(handler_ptr_t, if_pointer_t<target_t>);
		_JSON_HANDLER_TRAIT(handler_sequence_t, if_sequence_t<target_t>);
		_JSON_HANDLER_TRAIT(handler_setish_t, if_setish_t<target_t>);
#if _AF_JSON_OPTIMISED_STRING_MAPS
		_JSON_HANDLER_TRAIT(handler_pair_t, if_non_string_pair_t<target_t>);
		_JSON_HANDLER_TRAIT(handler_string_pair_t, if_string_pair_t<target_t>);
#else
		_JSON_HANDLER_TRAIT(handler_pair_t, if_non_string_pair_t<target_t>);
#endif
		_JSON_HANDLER_TRAIT(handler_mapish_t, if_mapish_t<target_t>);

	}

	
	// creator functions all need to follow a certain form
	template<typename target_t, typename handler_traits::handler_types<target_t>::sfinae_condition_t = true>
	handler_value_p<target_t> make_json_handler(
		target_t*								target_,
		traits::default_p<target_t>				default_ = nullptr,
		traits::default_contained_p<target_t>	contained_default_ = nullptr
	) {
		using handler_type = typename handler_traits::handler_types<target_t>::handler_type;

		return std::make_shared<handler_type>(
			target_, 
			default_, 
			contained_default_);
	}

	// we need a special makers for objects
	template<typename target_t, typename object_description_t>
	inline handler_value_p<target_t> from_object_description(
			target_t*					target_,
			traits::default_p<target_t>	default_,
			object_description_t const& object_description_) {
		return std::make_shared< handler_object_t<target_t> >(
			target_,
			default_,
			object_description_.pre_load_f(),
			object_description_.post_load_f(),
			object_description_.pre_save_f(),
			object_description_.post_save_f(),
			object_description_.handlers());
	}
	
	template<typename target_t, typename type_description_inst_t>
	inline handler_value_p<target_t> from_type_description(
		target_t* target_,
		traits::default_p<target_t>	default_,
		type_description_inst_t const& type_description_) {

		auto object_description = 
			type_description_.to_impl<target_t>.make_object_description(*target_, default_ ? default_->value() : nullptr);
		return from_object_description(target_, default_, *object_description);
	}

	namespace predicates {
		using namespace traits::predicates;
		
		_AF_DECLARE_HAS_MEMBER(json_handler);
		// predicates for choosing the right way to access type description
		template<typename target_t>
		using if_use_json_handler_t = if_t<
			has_json_handler_t<target_t>
		>;


		template<typename target_t>
		using if_use_type_description_factory_t = if_t<
			all_of_t<
				not_t<has_json_handler_t<target_t>>,
				has_type_description_factory_t<target_t>
			>
		>;

		template<typename target_t>
		using if_use_object_description_t = if_t<
			all_of_t<
				not_t<has_json_handler_t<target_t>>,
				not_t<has_type_description_factory_t<target_t>>,
				has_object_description_t<target_t>
			>
		>;

		template<typename target_t>
		using if_use_type_description_t = if_t<
			all_of_t<
				not_t<has_json_handler_t<target_t>>,
				not_t<has_type_description_factory_t<target_t>>,
				not_t<has_object_description_t<target_t>>,
				has_type_description_t<target_t>
			>
		>;
	}

	template<typename target_t, predicates::if_use_json_handler_t<target_t> = true>
	inline handler_value_p<target_t> make_json_handler(
		target_t* target_,
		traits::default_p<target_t>				default_ = nullptr,
		traits::default_contained_p<target_t> 	contained_default_ = nullptr
	) {
		_unused(contained_default_);
		return target_->json_handler(default_);
	}

	template<typename target_t, predicates::if_use_type_description_t<target_t> = true>
	inline handler_value_p<target_t> make_json_handler(
		target_t*								target_,
		traits::default_p<target_t>				default_ = nullptr,
		traits::default_contained_p<target_t> 	contained_default_ = nullptr
	) {
		_unused(contained_default_);
		auto& td = target_->template type_description<serialization_factory>();

		return from_type_description(target_, default_, td);
	}

	// optimised version of making, for many small objects we want to avoid hitting the heap too much
	template<typename target_t, predicates::if_use_object_description_t<target_t> = true>
	inline handler_value_p<target_t> make_json_handler(
		target_t* target_,
		traits::default_p<target_t>				default_ = nullptr,
		traits::default_contained_p<target_t>	contained_default_ = nullptr
	) {
		_unused(contained_default_);
		auto object_description = target_->template object_description<serialization_factory>();
		
		return from_object_description(target_, default_, *object_description);
	}
	template<typename target_t, predicates::if_use_type_description_factory_t<target_t> = true>
	inline handler_value_p<target_t> make_json_handler(
		target_t* target_,
		traits::default_p<target_t>				default_ = nullptr,
		traits::default_contained_p<target_t>	contained_default_ = nullptr
	) {
		_unused(contained_default_);
		auto factory = target_->type_description_factory();
		auto object_factory = std::static_pointer_cast<type_description_factory_instance_t<target_t>>(factory);
		auto object_description = object_factory->template object_description<serialization_factory>();

		return from_object_description(target_, default_, *object_description);
	}

	// serialization factory is passed through to the type_description hierarchy
	struct serialization_factory {
		template<typename target_t>
		static inline handler_value_p<target_t> make_handler(
			target_t* target_,
			traits::default_p<target_t>				default_ = nullptr,
			traits::default_contained_p<target_t>	contained_default_ = nullptr
		) {
			return make_json_handler<target_t>(target_, default_, contained_default_);
		}
	};

// TODO: maybe tuple, variant, ranges ...  serialization 

	namespace encoding_traits {
		// reading and writing traits
		template<typename stream_t, json_encoding encoding>
		struct writing;

		template<typename stream_t> // source is always utf8
		struct writing<stream_t, json_encoding::utf8> {
			using input_encoding_t = rapidjson::UTF8<_AF_SERIALIZATION_CHAR_T>;
			using output_encoding_t = rapidjson::UTF8<_AF_SERIALIZATION_CHAR_T>;
			using output_stream_t = stream_t;
			using writer_t = rapidjson::Writer<output_stream_t, input_encoding_t>;
			using pretty_writer_t = rapidjson::PrettyWriter<output_stream_t, input_encoding_t>;
			using schema_t = rapidjson::SchemaDocument;
			using validating_writer_t = rapidjson::GenericSchemaValidator<schema_t, writer_t>;
			using validating_pretty_writer_t = rapidjson::GenericSchemaValidator<rapidjson::SchemaDocument, pretty_writer_t>;

			static inline writer_t writer(stream_t& stream, bool put_bom = false) { return writer_t(stream); }
			static inline pretty_writer_t pretty_writer(stream_t& stream, bool put_bom = false) { return pretty_writer_t(stream); }

		};

#define __AF_JSON_WRITING_TRAIT(OUTPUT_ENCODING_ENUM, OUTPUT_ENCODING_JSON)\
	template<typename stream_t> \
	struct writing<stream_t, json_encoding::OUTPUT_ENCODING_ENUM> {\
		using input_encoding_t = rapidjson::UTF8<_AF_SERIALIZATION_CHAR_T>;\
		using output_encoding_t = rapidjson::OUTPUT_ENCODING_JSON<_AF_SERIALIZATION_CHAR_T>;\
		using output_stream_t = rapidjson::EncodedOutputStream<output_encoding_t, stream_t>;\
		using writer_t = rapidjson::Writer< output_stream_t, input_encoding_t, output_encoding_t>;\
		using pretty_writer_t = rapidjson::PrettyWriter< output_stream_t, input_encoding_t, output_encoding_t>;\
		using schema_t = rapidjson::SchemaDocument;\
		using validating_writer_t = rapidjson::GenericSchemaValidator<schema_t, writer_t>;\
		using validating_pretty_writer_t = rapidjson::GenericSchemaValidator<schema_t, pretty_writer_t>;\
		static inline writer_t writer(stream_t& stream, bool put_bom = false) { return writer_t(output_stream_t(stream, put_bom)); }\
		static inline pretty_writer_t pretty_writer(stream_t& stream, bool put_bom = false) { return pretty_writer_t(output_stream_t(stream, put_bom)); }\
		};

		__AF_JSON_WRITING_TRAIT(utf16le, UTF16LE);
		__AF_JSON_WRITING_TRAIT(utf16be, UTF16BE);
		__AF_JSON_WRITING_TRAIT(utf32le, UTF32LE);
		__AF_JSON_WRITING_TRAIT(utf32be, UTF32BE);

		template<typename stream_t, json_encoding encoding>
		struct reading;

		template<typename stream_t>
		struct reading<stream_t, json_encoding::utf8> {
			using input_encoding_t = rapidjson::UTF8<_AF_SERIALIZATION_CHAR_T>;
			using input_stream_t = stream_t;
			using schema_t = rapidjson::SchemaDocument;
			static inline input_stream_t input_stream(stream_t& stream) { return stream; }
		};

#define __AF_JSON_READING_TRAIT(INPUT_ENCODING_ENUM, INPUT_ENCODING_JSON)\
	template<typename stream_t> \
	struct reading<stream_t, json_encoding::INPUT_ENCODING_ENUM> {\
		using input_encoding_t = rapidjson::INPUT_ENCODING_JSON<_AF_SERIALIZATION_CHAR_T>;\
		using input_stream_t = rapidjson::EncodedInputStream<input_encoding_t, stream_t>;\
		using schema_t = rapidjson::SchemaDocument;\
		static inline input_stream_t input_stream(stream_t& stream) { return input_stream_t(stream); }\
	};

		__AF_JSON_READING_TRAIT(utf16le, UTF16LE);
		__AF_JSON_READING_TRAIT(utf16be, UTF16BE);
		__AF_JSON_READING_TRAIT(utf32le, UTF32LE);
		__AF_JSON_READING_TRAIT(utf32be, UTF32BE);

		template<typename stream_t>
		struct reading<stream_t, json_encoding::detect> {
			using input_stream_t = rapidjson::AutoUTFInputStream<_AF_SERIALIZATION_CHAR_T, stream_t>;

			static inline input_stream_t input_stream(stream_t& stream) { return input_stream_t(stream); }
		};
	}

	class json_file {
		using read_stream_t = rapidjson::FileReadStream;
		using write_stream_t = rapidjson::FileWriteStream;
#ifdef _WIN32
		const char* const write_flags = "wb";
		const char* const read_flags = "rb";
#else
		const char* const write_flags = "w";
		const char* const read_flags = "r";
#endif		
		char _buffer[_AF_JSON_READ_BUFFER_SIZE];
		FILE* _fp;
		bool const _reading;
	public:
		json_file(typename traits::string_t const& path_, bool reading_) : _reading(reading_) {
			if (_reading)
				_fp = fopen(path_.c_str(), read_flags);
			else
				_fp = fopen(path_.c_str(), write_flags);

		}

		inline read_stream_t&& read_stream() {
			AF_ASSERT(_reading, "File was open for writing but a read stream is reaquired.");
			return read_stream_t(_fp, _buffer, sizeof(_buffer));
		}
		inline write_stream_t&& write_stream() {
			AF_ASSERT(!_reading, "File was open for reading but a write stream is reaquired.");
			return write_stream_t(_fp, _buffer, sizeof(_buffer));
		}
		~json_file() {
			fclose(_fp);
		}
	};

	static void report_parsing_error(rapidjson::Reader const& reader) {
		using namespace rapidjson;
		ParseErrorCode e = reader.GetParseErrorCode();
		size_t o = reader.GetErrorOffset();
		AF_ERROR("Error parsing JSON. Error is: % (near %)",
			GetParseError_En(e), o);
	}

} // namespace impl

template<typename target_t>
using json_handler_p = impl::handler_value_p<target_t>;

template<typename target_t>
using default_p = traits::default_p<target_t>;

using json_serialization_factory = impl::serialization_factory;

template<typename target_t, typename type_description_t>
inline json_handler_p<target_t> make_json_handler_from_type_description(
		target_t* target_,
		traits::default_p<target_t>	default_,
		type_description_t const& type_description_) {
	return impl::from_type_description(target_, default_, type_description_);
}

template<json_encoding encoding_v = json_encoding::utf8>
struct dom {
	
	using document_t = rapidjson::Document;
	
	template<typename stream_t>
	static document_t from_stream(stream_t& stream_) {
		using namespace impl;
		using namespace rapidjson;
		document_t d;
		auto actual_stream = encoding_traits::reading<stream_t, encoding_v>::input_stream(stream_);
		d.ParseStream(actual_stream);
		if (d.HasParseError()) {
			AF_ERROR("Error parsing JSON schema. Error is: % (near %)",
				GetParseError_En(d.GetParseError()), static_cast<unsigned>(d.GetErrorOffset()));
		}
		return d;
	}

	inline static document_t from_string(typename traits::string_t const& schema_) {
		using namespace rapidjson;
		StringStream ss(schema_.c_str());
		return from_stream(ss);
	}

	inline static document_t from_file(typename traits::string_t const& path_) {
		impl::json_file file(path_, true);
		auto stream = file.read_stream();
		return from_stream(stream);
	}

};

template<json_encoding encoding_v = json_encoding::utf8>
class schema {
public:
	using dom_t = dom<encoding_v>;
	using schema_t = rapidjson::SchemaDocument;
	using resolver_base_t = rapidjson::IRemoteSchemaDocumentProvider;
	using validator_t = rapidjson::SchemaValidator;

	template<typename handler_t>
	using handler_validator_t = rapidjson::GenericSchemaValidator<schema_t, handler_t>;

private:
	class schema_ref_resolver : public resolver_base_t {
		std::string _root;
		std::vector<schema_t*> _ref_schemas;
		
		inline std::string normalize_path(std::string const& path) const {
			using namespace string_util;
			return replace(path, "\\", "/");
		}
		std::string extract_local_path(std::string const& path) const {
			using namespace string_util;
			const std::string file_uri_prefix("file://");
			std::string ret = normalize_path(path);
			if (starts_with(path, file_uri_prefix))
				ret = ret.substr(file_uri_prefix.size());
			return ret;
		}

		std::string construct_full_path(std::string const& path) const {
			using namespace string_util;
			std::string ret = extract_local_path(path);
			AF_ASSERT(!ret.empty(), "Empty path referenced.");
			if (_root.empty())
				return ret;
			if (starts_with(ret, "/") ||
				(ret.size() > 1 && ret[1] == ':')
				)// absolute path
				return ret;
			if (ends_with(_root, "/"))
				return _root + ret;
			return _root + "/" + ret;
		}
	public:
		schema_ref_resolver(std::string const& root_) : _root(normalize_path(root_)){}
		~schema_ref_resolver() {
			for (auto sd : _ref_schemas)
				delete sd;
		}
		virtual const schema_t* GetRemoteDocument(const char* uri, size_t length) {
			// Resolve the uri and returns a pointer to that schema.
			auto full_path = construct_full_path(uri);
			schema_t* ret(new schema_t(dom<encoding_v>::from_file(full_path), 
				full_path.c_str(), full_path.size(),
				this));
			_ref_schemas.push_back(ret);
			return ret;
		}
	};

	schema_ref_resolver _resolver;
	schema_t _schema;
	validator_t _validator;

	schema(typename dom_t::document_t document, std::string const& root_ = "") : 
		_resolver(root_),
		_schema(document, &_resolver),
		_validator(_schema)
	{}


	template<typename validator_tt>
	static void check_validation_errors(validator_tt const& validator_) {
		using namespace rapidjson;
		using namespace impl;
		if (validator_.IsValid()) {
			return;
		}
		StringBuffer sb;
		auto writer = encoding_traits::writing<StringBuffer, encoding_v>::pretty_writer(sb);
		validator_.GetError().Accept(writer);
		AF_WARNING_T(typename traits::string_t, "Validation failed:\n%", sb.GetString());
		AF_ERROR("JSON Validation Error");
	}
	inline void check_errors() const {
		check_validation_errors(_validator);
	}
	template<typename stream_t>
	void validate_stream(stream_t& stream) {
		using namespace rapidjson;
		using namespace impl;
		auto actual_stream = encoding_traits::reading<stream_t, encoding_v>::input_stream(stream);
		Reader reader;
		_validator.Reset();
		if (!reader.Parse(actual_stream, _validator)) {
			ParseErrorCode e = reader.GetParseErrorCode();
			size_t o = reader.GetErrorOffset();
			AF_ERROR("Error parsing JSON during validation. Error is: % (near %)",
				GetParseError_En(e), o);
		}
		check_errors(_validator);
	}

public:

	inline schema_t const& get_schema() const { return _schema; }
	inline schema_t& get_schema() { return _schema; }
	inline validator_t validator() const { return _validator; }

	template<typename stream_t>
	inline static std::shared_ptr<schema> from_stream(stream_t& stream, std::string const& root_ = "") {
		return std::make_shared<schema_t>(dom_t::from_stream(stream), root_);
	}
	template<typename stream_t>
	inline static std::shared_ptr<schema> from_string(typename traits::string_t const& schema_, std::string const& root_ = "") {
		return std::make_shared<schema_t>(dom_t::from_string(schema_), root_);
	}
	template<typename stream_t>
	inline static std::shared_ptr<schema> from_file(typename traits::string_t const& path_, std::string const& root_ = "") {
		return std::make_shared<schema_t>(dom_t::from_file(path_), root_);
	}

	template<typename stream_t, typename handler_t>
	void parse_with_validation(stream_t& stream_, handler_t& handler_) {
		using namespace rapidjson;
		using namespace impl;
		handler_validator_t<handler_t > validator(_schema, handler_);
		Reader reader;
		if (reader.Parse(stream_, validator))
			impl::report_parsing_error(reader);
		check_validation_errors(validator);
	}
	void validate_string(typename traits::string_t const& json_) {
		using namespace rapidjson;
		StringStream ss(json.c_str());
		validate_stream(ss);
	}
	void validate_file(typename traits::string_t const& path_) {
		impl::json_file file(path_, true);
		auto stream = file.read_stream();
		validate_stream(stream);
	}
};
template<json_encoding encoding_v = json_envoding::utf8>
using schema_p = std::shared_ptr<schema<encoding_v>>;

template<json_encoding encoding_v = json_encoding::utf8>
struct reader {


	template<typename target_t, typename stream_t>
	static void from_stream(
			target_t& target_,
			stream_t& stream_,
			schema_p<encoding_v> schema_ = nullptr) {
		using namespace rapidjson;
		using namespace impl;
		
		Reader reader;
		auto handler = impl::make_json_handler(&target_);
		handler->prepare_for_loading();
		using reader_factory_t = encoding_traits::reading<stream_t, encoding_v>;
		auto actual_stream = reader_factory_t::input_stream(stream_);
		if (schema_) {
			schema_->parse_with_validation(actual_stream, *handler);
		}
		else {
			Reader reader;
			if (!reader.Parse(actual_stream, *handler))
				impl::report_parsing_error(reader);
		}
	}

	template<typename target_t>
	inline static void from_string(
			target_t& target_,
			typename traits::string_t const& json_,
			schema_p<encoding_v> schema_ = nullptr) {
		using namespace rapidjson;
		StringStream ss(json_.c_str());
		from_stream(target_, ss, schema_);
	}

	template<typename target_t>
	inline static void from_string(
			std::shared_ptr<target_t>& target_,
			typename traits::string_t const& json_,
			schema_p<encoding_v> schema_ = nullptr) {
		from_string(*target_, json_, schema_);
	}

	template<typename target_t>
	inline static target_t from_string(
			typename traits::string_t const& json_,
			schema_p<encoding_v> schema_ = nullptr) {
		target_t target;
		from_string(target, json_, schema_);
		return target;
	}

	template<typename target_t>
	inline static void from_file(
			target_t& target_,
			typename traits::string_t const& path_,
			schema_p<encoding_v> schema_ = nullptr) {
		impl::json_file file(path_, true);
		auto stream = file.read_stream();
		from_stream(target_, stream, schema_);
	}
	template<typename target_t>
	inline static void from_file(
			std::shared_ptr<target_t>& target_,
			typename traits::string_t const& path_,
			schema_p<encoding_v> schema_ = nullptr) {
		if (!target_)
			target_ = std::make_shared<target_t>();
		from_file(*target_, path_, schema_);
	}

	template<typename target_t>
	inline static target_t from_file(
			typename traits::string_t const& path_,
			bool encoded_ = false,
			schema_p<encoding_v> schema_ = nullptr) {
		target_t target;
		from_file(target, path_, encoded_, schema_);
		return target;
	}
};

template<json_encoding encoding_v = json_encoding::utf8>
struct writer {

	template<typename target_t, typename writer_t>
	static void to_writer(
		target_t& target_,
		writer_t& writer_) {
		using namespace rapidjson;
		using namespace impl;
		auto handler = impl::make_json_handler(&target_);
		auto writer_wrapper = writer_wrapper_impl_t<writer_t>{ writer_ };
		handler->write(writer_wrapper);
	}

	template<typename target_t, typename stream_t>
	static void to_stream(
		target_t& target_,
		stream_t& stream_,
		bool pretty_ = false,
		schema_p<encoding_v> schema_ = nullptr, 
		bool put_bom_ = false) {
		using namespace rapidjson;
		using namespace impl;

		using writer_factory_t = encoding_traits::writing<stream_t, encoding_v>;
		if (pretty_) {
			auto writer = writer_factory_t::pretty_writer(stream_, put_bom_);
			if (schema_) {
				typename writer_factory_t::validating_pretty_writer_t v_writer(
					schema_->get_schema(), writer);
				to_writer(target_, v_writer);
			}
			else
				to_writer(target_, writer);
		}
		else {
			auto writer = writer_factory_t::writer(stream_, put_bom_);
			if (schema_) {
				typename writer_factory_t::validating_writer_t v_writer(
					schema_->get_schema(), writer);
				to_writer(target_, v_writer);
			}
			else
				to_writer(target_, writer);
		}
	}

	template<typename target_t>
	inline static void to_string(
			target_t& target_,
			typename traits::string_t& json_,
			bool pretty_ = false,
			schema_p<encoding_v> schema_ = nullptr,
			bool put_bom_ = false) {
		using namespace rapidjson;
		StringBuffer ss;
		to_stream(target_, ss, pretty_, schema_, put_bom_);
		json = ss.GetString();
	}

	template<typename target_t>
	inline static void to_string(
			std::shared_ptr<target_t>& target_,
			typename traits::string_t& json_,
			bool pretty_ = false,
			schema_p<encoding_v> schema_ = nullptr,
			bool put_bom_ = false) {
		to_string(*target_, json_, pretty_, schema_, put_bom_);
	}

	template<typename target_t>
	inline static traits::string_t to_string(
			target_t& target_,
			bool pretty_ = false,
			schema_p<encoding_v> schema_ = nullptr,
			bool put_bom_ = false) {
		using namespace rapidjson;
		StringBuffer ss;
		to_stream(target_, ss, pretty_, schema_, put_bom_);
		return ss.GetString();
	}

	template<typename target_t>
	inline static traits::string_t to_string(
			std::shared_ptr<target_t>& target_,
			bool pretty_ = false,
			schema_p<encoding_v> schema_ = nullptr,
			bool put_bom_ = false) {
		return to_string(*target_, pretty_, schema_, put_bom_);
	}

	template<typename target_t>
	static void to_file(
			target_t& target_,
			typename traits::string_t const& path_,
			bool pretty_ = false,
			schema_p<encoding_v> schema_ = nullptr,
			bool put_bom_ = false) {
		// rapidjson documentation says this is how to open files
		impl::json_file file(path_, false);
		auto stream = file.write_stream();
		to_stream(target_, stream, pretty_, schema_, put_bom_);
	}

	template<typename target_t>
	inline static void to_file(
			std::shared_ptr<target_t>& target_,
			typename traits::string_t& path_,
			bool pretty_ = false,
			schema_p<encoding_v> schema_ = nullptr,
			bool put_bom_ = false) {
		to_file(*target_, path_, pretty_, schema_, put_bom_);
	}

};

} // namespace json
} // namespace autotelica