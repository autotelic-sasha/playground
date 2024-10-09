#pragma once
#include "type_description.h"
#include "serialization_factory.h"
#include "autotelica_core/util/include/asserts.h"
#include "autotelica_core/util/include/enum_to_string.h"
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

// Default optmisation settings are all optimised.
// To make it all more verbose, define _AF_JSON_VERBOSE
#ifdef _AF_JSON_VERBOSE
#define _AF_JSON_OPTIMISED false
#endif

#ifndef _AF_JSON_OPTIMISED
#define _AF_JSON_OPTIMISED true
#endif

#ifndef _AF_JSON_READ_BUFFER_SIZE 
#define _AF_JSON_READ_BUFFER_SIZE 4*65535
#endif
#if _AF_JSON_OPTIMISED

// In terse mode, when target value is equal to default, we just don't write it
#ifndef		_AF_JSON_TERSE
#define		_AF_JSON_TERSE true
#endif

// Optimisation of strings maps means that keys in the map are used as JSON keys.
// Otherwise they are written with "key", "value" pairs like other maps.
#ifndef		_AF_JSON_OPTIMISED_STRING_MAPS
#define		_AF_JSON_OPTIMISED_STRING_MAPS true
#endif

#else

// in terse mode, when target value is equal to default, we just don't write it
#ifndef		_AF_JSON_TERSE
#define		_AF_JSON_TERSE false
#endif

// Optimisation of strings maps means that keys in the map are used as JSON keys.
// Otherwise they are written with "key", "value" pairs like other maps.
#ifndef		_AF_JSON_OPTIMISED_STRING_MAPS
#define		_AF_JSON_OPTIMISED_STRING_MAPS false
#endif

#endif

namespace autotelica {
namespace json {
	using namespace serialization;
	using namespace type_description;
	using namespace sfinae;


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


	// handling defaults generically is a pain
	// we have a whole class to do it properly
	class default_value_t {
	public:
		virtual ~default_value_t() {}
	};

	template<typename target_t>
	class default_value_impl_t : public default_value_t {
		target_t const _default_value;
		inline bool set_target(target_t* target_) {
			*target_ = _default_value;
			return true;
		}
	public:
		default_value_impl_t(target_t const& default_value_) :
			_default_value(default_value_) {
		}
		inline bool null(target_t* target_) {
			return set_target(target_);
		}

		// TODO: do we need should_not_write?
		bool should_not_write(target_t const& target_) const {
			// double negatives here, but makes it easier to read code later
			// in terse mode, when _target is equal to default, we just don't write it
#if _AF_JSON_TERSE
			return (target_ == _default_value);
#else
			return false;
#endif
		}
	};
	template<target_t>
	using default_value_p = std::shared_ptr<default_value_impl_t<target_t>>;

	template<target_t>
	inline default_value_p<target_t> make_default(target_t const* target_) {
		if (!target_) return nullptr;
		return std::make_shared<default_value_impl_t<target_t>>(*target_);
	}

	// base class  for rapidjson SAX handlers
	struct handler_t : public serialization_handler_t {
		using char_t = traits::char_t;

		virtual ~handler_t() {}
		// TODO: do we need is_done?
		virtual bool is_done() const = 0; // signal to the controller that we are done loading this value
		
		// TODO: do we need handles_terse_storage?
		virtual bool handles_terse_storage() const { return false; }
		// TOTO: do we need prepare_for_loading?
		virtual void prepare_for_loading() { }

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

		virtual bool will_write() const = 0; // we need to know if the value will actually be written
		// for terse mode
		virtual void write(writer_wrapper_t& writer_) const = 0;
	};
	using handler_p = std::shared_ptr<handler_t>;

	// typed base for handlers
	template<typename target_t>
	struct handler_value_t : public handler_t {

		using char_t = traits::char_t;
		using base_t = handler_t;
		using default_t = typename traits::default_types_t<target_t>::value_t;
		// To make construction easier, all instances have to implement a constructor that 
		// takes a contained default value pointer. Ugly but effective. 
		using contained_default_t = typename traits::default_types_t<target_t>::contained_t;

		target_t* _target; // owned by someone else, don't delete
		default_value_p<default_t> _default;
		bool _done;

		handler_value_t(
			target_t* target_,
			default_value_p<default_t> default_ = nullptr) :
			_target(target_),
			_default(default_),
			_done(false) {
			AF_ASSERT(_target, "Target is not initialised.");
		}

		inline bool set(target_t const& value_) { *_target = value_; return done(); }

		template<typename ConvertibleT>
		inline bool set(ConvertibleT const& value_) { *_target = static_cast<target_t>(value_); return done(); }

		virtual void reset(target_t* target_) {
			_target = target_;
			_done = false;
		}
		void prepare_for_loading() override {
			reset(_target);
		}
		inline bool done() { return (_done = true); }

		bool is_done() const override { return _done; }
		bool handles_terse_storage() const override { return _default_h != nullptr; }
		inline bool is_set() const { return _target != nullptr; }

		inline target_t const& get() const { return *_target; }
		inline target_t& get() { return *_target; }

		bool Null()  override {
			AF_ASSERT(_default, "Unexpected: null read when default value is not provided.");
			done();
			return _default->null(_target);
		}

		inline bool should_not_write() const {
			return (
				_default &&
				_default->should_not_write(*_target));
		}
		bool will_write() const override {
			// double negative, but makes code later easer to read
			return !should_not_write();
		}
	};
	template<typename target_t>
	using handler_value_p = std::shared_ptr<handler_value_t<target_t>>;


	// hadnlers create other handlers, so we need a forward declaration
	template<target_t, typename condition_t>
	handler_value_p<target_t> make_handler(
		target_t* target_,
		typename traits::default_types_t<target_t>::value_t const* default_,
		typename traits::default_types_t<target_t>::contained_t const* contained_default_
	);

	// handler for integral types
	template<typename target_t>
	struct handler_integral_t : public handler_value_t<target_t> {

		using base_t = handler_value_t<target_t>;
		using default_t = typename base_t::default_t;
		using contained_default_t = typename base_t::contained_default_t;
		
		handler_integral_t(
				target_t* target_,
				default_value_p<default_t> default_ = nullptr,
				default_value_p<contained_default_t> unused_ = nullptr) :
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

	// handler for floating types
	template<typename target_t>
	struct handler_floating_t : public handler_value_t<target_t> {

		using base_t = handler_value_t<target_t>;
		using default_t = typename base_t::default_t;
		using contained_default_t = typename base_t::contained_default_t;

		handler_floating_t(
				target_t* target_,
				default_value_p<default_t> default_ = nullptr,
				default_value_p<contained_default_t> unused_ = nullptr) :
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
		using default_t = typename base_t::default_t;
		using contained_default_t = typename base_t::contained_default_t;
		using char_t = traits::char_t;

		handler_string_t(
				target_t* target_,
				default_value_p<default_t> default_ = nullptr,
				default_value_p<contained_default_t> unused_ = nullptr) :
			base_t(target_, default_) {
			_unused(unused_);
		}

		// reader part
		bool String(const char_t* str, size_t length, bool copy) override {
			util::assign(base_t::_target, str, length);
			return base_t::done();
		}

		void write(writer_wrapper_t& writer_) const override {
			if (base_t::should_not_write()) return;
			writing::write(*base_t::_target, writer_);
		}
	};

	// handler for enumerations
	template<typename target_t>
	struct handler_enum_t : public handler_value_t<target_t> {

		using base_t = af_json_handler_value_t<target_t>;
		using default_t = typename base_t::default_t;
		using contained_default_t = typename base_t::contained_default_t;
		using char_t = traits::char_t;
		using string_t = traits::string_t;

		handler_enum_t(
				target_t* target_,
				default_value_p<default_t> default_ = nullptr,
				default_value_p<contained_default_t> unused_ = nullptr) :
			base_t(target_, default_) {
			_unused(unused_);
		}

		// reader part
		bool String(const char_t* str, size_t length, bool copy) override {
			using namespace autotelica::enum_to_string;
			*base_t::_target = to_enum<target_t>(str);
			return base_t::done();
		}
		void write(writer_wrapper_t& writer_) const override {
			if (base_t::should_not_write()) return;
			string_t out;
			using autotelica::enum_to_string;
			to_string(out, *base_t::_target);
			writing::write(out, writer_);
		}
	};

	// handler for pointers
	template<typename target_t>
	struct handler_ptr_t : public handler_value_t<target_t> {

		using base_t = handler_value_t<target_t>;
		using default_t = typename base_t::default_t;
		using contained_default_t = typename base_t::contained_default_t;
		using contained_t = typename traits::default_types_t<target_t>::contained_t;
		using char_t = traits::char_t;
		using value_handler_t = handler_value_p<contained_t>;

		value_handler_t _value_handler;
		default_value_p<contained_default_t> _value_default;

		handler_ptr_t(
				target_t* target_,
				default_value_p<default_t> default_ = nullptr,
				default_value_p<contained_default_t> unused_ = nullptr) :
			base_t(target_, default_),
			_value_handler(nullptr),
			_value_default(contained_default_) {
		}

		bool is_done() const override {
			return base_t::is_done() || // _done is set when null is read
				(_value_handler && _value_handler->is_done());
		}

		inline contained_t* value_ptr() {
			return &(*(*base_t::_target));
		}
		inline void initialise_target() {
			if (!(*base_t::_target))
				*base_t::_target = target_t(new contained_t());
		}
		void reset(target_t* target_) override {
			base_t::reset(target_);
			if (_value_handler)
				_value_handler->reset(value_ptr());
			else
				_value_handler = make_handler(value_ptr(), _value_default);
		}
		void prepare_for_loading() override {
			base_t::prepare_for_loading();
			_value_handler->prepare_for_loading();
		}

		template<typename... ParamsT>
		bool delegate_f(bool (value_handler_t::* mf)(ParamsT ...), ParamsT... ps) {
			initialise_target();
			if (!_value_handler)
				_value_handler = make_handler(value_ptr(), _value_default); // for shared_ptr this is _target->get(), but this should work for naked pointers too
			return (_value_handler->*mf)(ps...);
		}

		bool Null()  override {
			if (_value_handler) // if we are already reading the contained value, delegate
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
			_value_handler->write(writer_);
		}
	};

	// handler for sequences (lists and vectors)
	template<typename target_t>
	struct handler_sequence_t : public handler_value_t<target_t> {

		using base_t = handler_value_t<target_t>;
		using default_t = typename base_t::default_t;
		using contained_default_t = typename base_t::contained_default_t;
		using contained_t = typename traits::default_types_t<target_t>::contained_t;
		using char_t = traits::char_t;
		using value_handler_t = handler_value_p<contained_t>;

		value_handler_t _value_handler;

		handler_sequence_t(
				target_t* target_,
				default_value_t default_ = nullptr,
				contained_default_value_t contained_default_ = nullptr) :
			base_t(target_, default_),
			_value_handler(make_handler<contained_t>(nullptr, contained_default_)) {
		}

		void reset(target_t* target_) override {
			base_t::reset(target_);
			_value_handler->reset(nullptr);
		}
		inline void set_next() {
			base_t::_target->emplace_back({});
			contained_t& ret(base_t::_target->back());
			_value_handler->reset(&ret);
			_value_handler->prepare_for_loading();
		}
		template<typename... ParamsT>
		inline bool delegate_f(bool (value_handler_t::* mf)(ParamsT ...), ParamsT... ps) {
			if (!_value_handler->is_set())
				set_next();
			bool ret = (_value_handler->*mf)(ps...);
			if (_value_handler->is_done())
				_value_handler->reset(nullptr);
			return ret;
		}
		bool Null() override {
			if (!_value_handler->is_set())
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
		bool StartObject() override { return delegate_f(&value_handler_t::StartObject); }
		bool Key(const char_t* str, size_t length, bool copy) { return delegate_f(&value_handler_t::String, str, length, copy); }
		bool EndObject(size_t memberCount) override { return delegate_f(&value_handler_t::EndObject, memberCount); }
		bool StartArray() override {
			if (!_value_handler->is_set()) {
				set_next();
				return true;
			}
			return delegate_f(&value_handler_t::StartArray);
		}
		bool EndArray(size_t elementCount) override {
			if (!_value_handler->is_set())
				return base_t::done();
			return delegate_f(&value_handler_t::EndArray, elementCount);
		}

		void write(writer_wrapper_t& writer_) const override {
			if (base_t::should_not_write()) return;
			writer_->StartArray();
			for (auto const& t : base_t::_target) {
				_value_handler->reset(&t);
				_value_handler->write(writer_);
			}
			writer_->EndArray();
			_value_handler->reset(nullptr);
		}
	};

	// handler for sets
	template<typename target_t>
	struct handler_setish_t : public handler_sequence_t<target_t> {

		using base_t = handler_sequence_t<target_t>;
		using default_t = typename base_t::default_t;
		using contained_default_t = typename base_t::contained_default_t;
		using contained_t = typename traits::default_types_t<target_t>::contained_t;
		using char_t = traits::char_t;
		using value_handler_t = handler_value_p<contained_t>;

		contained_t _current_value;

		handler_setish_t(
				target_t* target_,
				default_value_t default_ = nullptr,
				contained_default_value_t unused_ = nullptr) :
			base_t(target_, default_, nullptr) {// sets canot contain default values
			_unused(unused_);
		}

		template<typename... ParamsT>
		inline bool delegate_f(bool (value_handler_t::* mf)(ParamsT ...), ParamsT... ps) {
			if (!base_t::_value_handler->is_set()) {
				_current_value = contained_t();
				base_t::_value_handler->reset(&_current_value);
				base_t::_value_handler->prepare_for_loading();
			}
			bool ret = (base_t::_value_handler->*mf)(ps...);
			if (base_t::_value_handler->is_done()) {
				base_t::_target->insert(_current_value);
				base_t::_value_handler->reset(nullptr);
			}
			return ret;
		}
		bool Null() override {
			if (!base_t::_value_handler->is_set())
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
		bool StartObject() override { return delegate_f(&value_handler_t::StartObject); }
		bool Key(const char_t* str, size_t length, bool copy) { return delegate_f(&value_handler_t::String, str, length, copy); }
		bool EndObject(size_t memberCount) override { return delegate_f(&value_handler_t::EndObject, memberCount); }
		bool StartArray() override {
			if (!base_t::_value_handler->is_set()) {
				base_t::_value_handler->reset(&_current_value);
				return true;
			}
			return delegate_f(&value_handler_t::StartArray);
		}
		bool EndArray(size_t elementCount) override {
			if (!base_t::_value_handler->is_set())
				return base_t::done();
			return delegate_f(&value_handler_t::EndArray, elementCount);
		}
	};

	// handling maps needs a bit of work
	// general maps are stored as arrays of pair objects: { "key" : key_value, "value" : value_value }
	// there is an optimisation for string maps
	// first we need an intermediate handler for these objects
	namespace pair_tags {
		static traits::tag_t tag_key = _AF_CHAR_CONSTANT("key");
		static traits::tag_t tag_value = _AF_CHAR_CONSTANT("value");
	};

	// handling pairs
	template< typename target_t>
	struct handler_pair_t : public handler_value_t<target_t> {

		using base_t = handler_value_t<target_t>;
		using default_t = typename base_t::default_t;
		using contained_default_t = typename base_t::contained_default_t;
		using char_t = traits::char_t;

		using key_t = typename target_t::first_type;
		using value_t = typename target_t::second_type;
		using key_handler_t = handler_value_p<key_t>;
		using value_handler_t = handler_value_p<value_t>;
		using current_handler_t = handler_t;
		using current_handler_p = handler_p;

		inline key_t key() { return base_t::is_set() ? (&(base_t::_target->first)) : nullptr; }
		inline value_t value() { return base_t::is_set() ? (&(base_t::_target->second)) : nullptr; }

		key_handler_t _key_handler;
		value_handler_t _value_handler;
		current_handler_p _current_handler; // switches between _key_handler, _value_handler, and nullptr

		handler_pair_t(
				target_t* target_,
				default_value_t default_ = nullptr,
				contained_default_value_t unused_ = nullptr) :
			base_t(target_, default_) {
			_unused(unused_);
			_key_handler = make_handler<key_t>(key(), default_.first);
			_value_handler = make_handler<value_t>(value(), default_.second);
		}

		bool is_done() const {
			return _key_handler->is_done() && _value_handler->is_done();
		}
		void reset(target_t* target_) override {
			base_t::reset(target_);
			_key_handler->reset(&key());
			_value_handler->reset(&value());
			_current_handler = nullptr;
		}
		void prepare_for_loading() override {
			base_t::prepare_for_loading();
			_value_handler->prepare_for_loading();
		}
		template<typename... ParamsT>
		bool delegate_f(bool (current_handler_t::* mf)(ParamsT ...), ParamsT... ps) {
			AF_ASSERT(_current_handler, "Unexpected value while reading a map.");
			bool ret = (_current_handler->*mf)(ps...);
			if (_current_handler->is_done())
				_current_handler = nullptr;
			return ret;
		}

		bool Null() {
			AF_ASSERT(_current_handler, "Maps cannot contain nulls.");
			AF_ASSERT(_current_handler != _key_handler, "Keys in a map cannot be null.");
			return delegate_f(&current_handler_t::Null);
		}
		bool Bool(bool b) { return delegate_f(&current_handler_t::Bool, b); }
		bool Int(int i) { return delegate_f(&current_handler_t::Int, i); }
		bool Uint(unsigned i) { return delegate_f(&current_handler_t::Uint, i); }
		bool Int64(int64_t i) { return delegate_f(&current_handler_t::Int64, i); }
		bool Uint64(uint64_t i) { return delegate_f(&current_handler_t::Uint64, i); }
		bool Double(double d) { return delegate_f(&current_handler_t::Double, d); }
		bool String(const char_t* str, size_t length, bool copy) { return delegate_f(&current_handler_t::String, str, length, copy); }
		bool StartObject() {
			if (_current_handler)
				return delegate_f(&current_handler_t::StartObject);
			return true;
		}
		bool Key(const char_t* str, size_t length, bool copy) {
			if (_current_handler)
				return delegate_f(&current_handler_t::Key, str, length, copy);
			if (traits::equal_tag(str, length, pair_tags::tag_key))
				_current_handler = _key_handler;
			else if (traits::equal_tag(str, length, pair_tags::tag_value) == 0)
				_current_handler = _value_handler;
			else
				AF_ERROR("Unknown key value: %", str);
			_current_handler->prepare_for_loading();
			return true;
		}
		bool EndObject(size_t memberCount) {
			if (_current_handler)
				return delegate_f(&current_handler_t::EndObject, memberCount);
			return base_t::done();
		}

		bool StartArray() { return delegate_f(&current_handler_t::StartArray); }
		bool EndArray(size_t elementCount) { return delegate_f(&current_handler_t::EndArray, elementCount); }

		void write(writer_wrapper_t& writer_) const override {
			if (base_t::should_not_write()) return;
			writer_->StartObject();
			if (_value_handler->will_write()) {
				writer_->Key(pair_tags::tag_key);
				_key_handler->write(writer_);
				writer_->Key(pair_tags::tag_value);
				_value_handler->write(writer_);
			}
			writer_->EndObject();
		}
	};


	// handler for general maps
	template<typename target_t>
	struct handler_mappish_t : public handler_value_t<target_t> {
		// TODO: this is so similar to both sequence and settish
		// perhaps all they need is different construction?
		using base_t = handler_value_t<target_t>;
		using default_t = typename base_t::default_t;
		using contained_default_t = typename base_t::contained_default_t;
		using contained_t = typename traits::default_types_t<target_t>::contained_t;
		using char_t = traits::char_t;
		using value_handler_t = handler_pair_t<contained_t>;
		
		contained_t _current_value;
		value_handler_t _value_handler;

		handler_mappish_t(
				target_t* target_,
				default_value_t default_ = nullptr,
				contained_default_value_t contained_default_ = nullptr) :
			base_t(target_, default_),
			_value_handler(make_hadler<contained_t>(nullptr, contained_default_)) {
		}

		void reset(target_t* target_) override {
			base_t::reset(target_);
			_value_handler->reset(nullptr);
			_current_value = contained_t();
		}
		template<typename... ParamsT>
		inline bool delegate_f(bool (value_handler_t::* mf)(ParamsT ...), ParamsT... ps) {
			if (!_value_handler->is_set()) {
				_current_value = contained_t();
				_value_handler->reset(&_current_value);
				_value_handler->prepare_for_loading();
			}
			bool ret = (_value_handler->*mf)(ps...);
			if (_value_handler->is_done()) {
				base_t::_target.insert(_current_value);
				_value_handler->reset(nullptr);
			}
			return ret;
		}
		bool Null() {
			if (!_value_handler->is_set())
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
		bool StartObject() override { return delegate_f(&value_handler_t::StartObject); }
		bool Key(const char_t* str, size_t length, bool copy) { return delegate_f(&value_handler_t::Key, str, length, copy); }
		bool EndObject(size_t memberCount) override { return delegate_f(&value_handler_t::EndObject, memberCount); }
		bool StartArray() override {
			if (!_value_handler->is_set()) {
				_value_handler->reset(&_current_value);
				return true;
			}
			return delegate_f(&value_handler_t::StartArray);
		}
		bool EndArray(size_t elementCount) override {
			if (!_value_handler->is_set())
				return base_t::done();
			return delegate_f(&value_handler_t::EndArray, elementCount);
		}

		void write(af_json_writer_wrapper_t& writer_) const override {
			if (base_t::should_not_write()) return;
			writer_->StartObject();
			for (auto const& t : base_t::_target) {
				_value_handler->reset(&t);
				_value_handler->write(writer_);
			}
			writer_->EndObject();
			_value_handler->reset(nullptr);
		}
	};

	template<typename target_t>
	struct handler_string_mappish_t : public handler_value_t<target_t> {

		using base_t = handler_value_t<target_t>;
		using default_t = typename base_t::default_t;
		using contained_default_t = typename base_t::contained_default_t;
		using contained_t = typename traits::default_types_t<target_t>::contained_t;
		using char_t = traits::char_t;
		using key_t = typename target_t::key_type; // this is some string type
		using value_handler_t = handler_value_p<contained_t>;

		value_handler_t _value_handler;

		handler_string_mappish_t(
				target_t* target_,
				default_value_t default_ = nullptr,
				contained_default_value_t contained_default_ = nullptr) :
			base_t(target_, default_),
			_value_handler(make_handler<contained_t>(nullptr, contained_default_)) {
		}

		void reset(target_t* target_) override {
			base_t::reset(target_);
			_value_handler.reset(nullptr);
		}

		template<typename... ParamsT>
		inline bool delegate_f(bool (value_handler_t::* mf)(ParamsT ...), ParamsT... ps) {
			AF_ASSERT(_value_handler->is_set(), "Unexpected value read while reading a string map.");
			bool ret = (_value_handler->*mf)(ps...);
			if (_value_handler->is_done())
				_value_handler->reset(nullptr);
			return ret;
		}

		bool Null() {
			if (!_value_handler->is_set())
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
			if (!_value_handler->is_set())
				return true;
			return delegate_f(&value_handler_t::StartObject);
		}
		bool Key(const char_t* str, size_t length, bool copy) {
			if (!_value_handler->is_set()) {
				key_t key;
				traits::assign(&key, str, length);
				AF_ASSERT(base_t::_target->find(key) == base_t::_target->end(), "Duplicate key found in object (%)", key);
				_value_handler->reset(value_ptr(key));
				_value_handler->prepare_for_loading();
				return true;
			}
			return delegate_f(&value_handler_t::Key, str, length, copy);
		}
		bool EndObject(size_t memberCount) override {
			if (!_value_handler->is_set())
				return base_t::done();
			return delegate_f(&value_handler_t::EndObject, memberCount);
		}
		bool StartArray() override { return delegate_f(&value_handler_t::StartArray); }
		bool EndArray(size_t elementCount) override { return delegate_f(&value_handler_t::EndArray, elementCount); }

		void write(writer_wrapper_t& writer_) const override {
			if (base_t::should_not_write()) return;
			writer_->StartObject();
			for (auto const& t : base_t::_target) {
				if (_value_handler->will_write()) {
					writer_->Key(t.first);
					_value_handler->reset(&t.second);
					_value_handler->write(writer_);
				}
			}
			writer_->EndObject();
			_value_handler->reset(nullptr);
		}
	};

	
	// handler for objects
	template<typename target_t>
	struct handler_object_t : public handler_value_t<target_t> {

		using base_t = handler_value_t<target_t>;
		using default_t = typename base_t::default_t;
		using contained_default_t = typename base_t::contained_default_t;
		using contained_t = typename traits::default_types_t<target_t>::contained_t;
		using char_t = traits::char_t;
		using key_t = typename target_t::key_type; // this is some string type
		

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
				default_value_p<default_t> default_ = nullptr,
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
				_handlers.push_back({ h.first, std::static_pointer_cast<handler_t>(h.second) });
		}

		void prepare_for_loading() override {
			base_t::prepare_for_loading();
			for (auto& h : _handlers)
				h.second->prepare_for_loading();
		}
		inline handler_p find_handler(const char_t* const key, size_t length) {
			for (auto& h : _handlers) {
				if (traits::equal_tag(key, length, h.first.c_str()))
					return h.second;
			}
			return nullptr;
		}

		bool validate_all_loaded() const {
			for (auto const& h : _handlers) {
				if (!h.second->is_done())
					if (!_AF_JSON_TERSE || !h.second->handles_terse_storage())
						return false;
			}
			return true;
		}

		void reset(target_t* target_) override {
			base_t::reset(target_);
			_current_handler = nullptr;
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
			AF_ASSERT(validate_all_loaded(), "Not all object members are loaded.");
			if (_post_load_f)
				_post_load_f();
			return base_t::done();
		}
		bool StartArray() override { return delegate_f(&handler_t::StartArray); }
		bool EndArray(size_t elementCount) override { return delegate_f(&handler_t::EndArray, elementCount); }

		void write(writer_wrapper_t& writer_) const override {
			if (_pre_save_f)
				_pre_save_f();
			if (base_t::should_not_write()) return;
			writer_.StartObject();
			for (auto const& h : _handlers) {
				if (h.second->will_write()) {
					writer_.Key(h.first.c_str(), h.first.size(), false);
					h.second->write(writer_);
				}
			}
			writer_.EndObject(_handlers.size());
			if (_post_save_f)
				_post_save_f();

		}
	};



	namespace handler_traits {
		using namespace serialization::util::predicates;


		template<typename target_t, typename condition_t = void>
		struct handler_types {
			using handler_type = void;
			using sfinae_condition_t = condition_t;
		};

#define _JSON_HANDLER_TRAIT(HandlerT, ConditionT) \
		template<typename target_t>\
		struct handler_types<target_t, ConditionT> {\
			using handler_type = HandlerT<target_t>;\
			using sfinae_condition_t = ConditionT;\
		};


		_JSON_HANDLER_TRAIT(handler_integral_t, if_integral_t<target_t>);
		_JSON_HANDLER_TRAIT(handler_floating_t, if_floating_point_t<target_t>);
		_JSON_HANDLER_TRAIT(handler_string_t, if_string_t<target_t>);
		_JSON_HANDLER_TRAIT(handler_enum_t, if_enum_t<target_t>);
		_JSON_HANDLER_TRAIT(handler_ptr_t, if_pointer_t<target_t>);
		_JSON_HANDLER_TRAIT(handler_sequence_t, if_sequence_t<target_t>);
		_JSON_HANDLER_TRAIT(handler_setish_t, if_setish_t<target_t>);
		_JSON_HANDLER_TRAIT(handler_pair_t, if_pair_t<target_t>);
#if _AF_JSON_OPTIMISED_STRING_MAPS
		_JSON_HANDLER_TRAIT(handler_mappish_t, if_non_string_map_t<target_t>);
		_JSON_HANDLER_TRAIT(handler_string_mappish_t, if_string_map_t<target_t>);
#else
		_JSON_HANDLER_TRAIT(handler_mappish_t, if_mappish_t<target_t>);
#endif

	}
	// creator functions all need to follow a certain form
	template<target_t, typename handler_traits::handler_types<target_t>::sfinae_condition_t = true>
	handler_value_p<target_t> make_handler(
		target_t* target_,
		typename traits::default_types_t<target_t>::value_t const* default_ = nullptr,
		typename traits::default_types_t<target_t>::contained_t const* contained_default_ = nullptr
	) {
		using handler_type = handler_traits::handler_types<target_t>::handler_type;

		return std::make_shared<handler_type>(
			target_, 
			make_default(default_), 
			make_default(contained_default_));
	}

	// we need a special maker for objects
	template<target_t, util::predicates::if_serializable_object_t<target_t> = true>
	handler_value_p<target_t> make_handler(
		target_t* target_,
		typename traits::default_types_t<target_t>::value_t const* default_ = nullptr,
		typename traits::default_types_t<target_t>::contained_t const* contained_default_ = nullptr
	) {
		using handler_type = handler_traits::handler_types<target_t>::handler_type;
		auto& description =
			static_cast<type_description_instance_t<target_t>>(target->type_description());

		return description.make_handler(
			serialization_type_t::json,
			*target_,
			default_);
	}

	template<typename object_t>
	static serialization_handler_p make_object_handler(
		object_t& object_,
		default_value_t<object_t> const* default_,							// nullptr means no default
		handlers_t const& handlers_,
		traits::setup_function_t pre_load_f_,				// nullptr means no function	
		traits::setup_function_t post_load_f_,				// nullptr means no function	
		traits::setup_function_t pre_save_f_,				// nullptr means no function	
		traits::setup_function_t post_save_f_				// nullptr means no function	
	) {
		return std::make_shared< handler_object_t>(
			&object_,
			default_,
			pre_load_f_,
			post_load_f_,
			pre_save_f_,
			post_save_f_,
			handlers_);
	}



} // namespace impl

struct serialization_factory {
	using namespace autotelica::serialization;
		
	template<target_t>
	using default_value_t =  traits::default_types_t<target_t>::value_t;
		 
	template<target_t>
	using default_contained_t =  traits::default_types_t<target_t>::contained_t;
		 


	template<typename target_t>
	inline static serialization_handler_p make_handler(
		target_t* target_,
		default_value_t<target_t> const* default_value_,				// nullptr means no default
		default_contained_t<target_t> const* contained_default_value_	// nullptr means no default
	) {
		return impl::make_handler(target_, default_value_, contained_default_value_);
	}
		
		
		
	template<typename object_t>
	inline static serialization_handler_p make_object_handler(
		object_t& object_,
		default_value_t<object_t> const* default_,							// nullptr means no default
		handlers_t const& handlers_,
		traits::setup_function_t pre_load_f_,				// nullptr means no function	
		traits::setup_function_t post_load_f_,				// nullptr means no function	
		traits::setup_function_t pre_save_f_,				// nullptr means no function	
		traits::setup_function_t post_save_f_				// nullptr means no function	
	) {
		return impl::make_object_handler(
			object_, 
			default_, 
			handlers_, 
			pre_load_f_, 
			post_load_f_, 
			pre_save_f_, 
			post_save_f_);
	}
};

// TODO: schema validation
// TODO: bitset serialization
enum class json_encoding {
	utf8,
	utf16le,
	utf16be,
	utf32le,
	utf32be,
	detect
};


namespace detail {
	// reading and writing traits
	template<typename stream_t, json_encoding encoding>
	struct json_writing;

	template<typename stream_t> // source is always utf8
	struct json_writing<stream_t, json_encoding::utf8> {
		using input_encoding_t = rapidjson::UTF8<_AF_SERIALIZATION_CHAR_T>;
		using output_encoding_t = rapidjson::UTF8<_AF_SERIALIZATION_CHAR_T>;
		using output_stream_t = stream_t;
		using writer_t = rapidjson::Writer<output_stream_t, input_encoding_t>;
		using pretty_writer_t = rapidjson::PrettyWriter<output_stream_t, input_encoding_t>;
		using reader_stream_t = stream_t;

		static inline writer_t writer(stream_t& stream, bool put_bom = false) { return writer_t(stream); }
		static inline pretty_writer_t pretty_writer(stream_t& stream, bool put_bom = false) { return pretty_writer_t(stream); }
	};

#define __AF_JSON_WRITING_TRAIT(OUTPUT_ENCODING_ENUM, OUTPUT_ENCODING_JSON)\
	template<typename stream_t> \
	struct json_writing<stream_t, json_encoding::OUTPUT_ENCODING_ENUM> {\
		using input_encoding_t = rapidjson::UTF8<_AF_SERIALIZATION_CHAR_T>;\
		using output_encoding_t = rapidjson::OUTPUT_ENCODING_JSON<_AF_SERIALIZATION_CHAR_T>;\
		using output_stream_t = rapidjson::EncodedOutputStream<output_encoding_t, stream_t>;\
		using writer_t = rapidjson::Writer< output_stream_t, input_encoding_t, output_encoding_t>;\
		using pretty_writer_t = rapidjson::Writer< output_stream_t, input_encoding_t, output_encoding_t>;\
		static inline writer_t writer(stream_t& stream, bool put_bom = false) { return writer_t(output_stream_t(stream, put_bom)); }\
		static inline pretty_writer_t pretty_writer(stream_t& stream, bool put_bom = false) { return pretty_writer_t(output_stream_t(stream, put_bom)); }\
	};

	__AF_JSON_WRITING_TRAIT(utf16le, UTF16LE);
	__AF_JSON_WRITING_TRAIT(utf16be, UTF16BE);
	__AF_JSON_WRITING_TRAIT(utf32le, UTF32LE);
	__AF_JSON_WRITING_TRAIT(utf32be, UTF32BE);

	template<typename stream_t, json_encoding encoding>
	struct json_reading;

	template<typename stream_t>
	struct json_reading<stream_t, json_encoding::utf8> {
		using input_encoding_t = rapidjson::UTF8<_AF_SERIALIZATION_CHAR_T>;
		using input_stream_t = stream_t;
		static inline input_stream_t input_stream(stream_t& stream) { return stream; }
	};

#define __AF_JSON_READING_TRAIT(INPUT_ENCODING_ENUM, INPUT_ENCODING_JSON)\
template<typename stream_t> \
struct json_reading<stream_t, json_encoding::INPUT_ENCODING_ENUM> {\
	using input_encoding_t = rapidjson::INPUT_ENCODING_JSON<_AF_SERIALIZATION_CHAR_T>;\
	using input_stream_t = rapidjson::EncodedInputStream<input_encoding_t, stream_t>;\
	static inline input_stream_t input_stream(stream_t& stream) { return input_stream_t(stream); }\
};

	__AF_JSON_READING_TRAIT(utf16le, UTF16LE);
	__AF_JSON_READING_TRAIT(utf16be, UTF16BE);
	__AF_JSON_READING_TRAIT(utf32le, UTF32LE);
	__AF_JSON_READING_TRAIT(utf32be, UTF32BE);

	template<typename stream_t>
	struct json_reading<stream_t, json_encoding::detect> {
		using input_stream_t = rapidjson::AutoUTFInputStream<_AF_SERIALIZATION_CHAR_T, stream_t>;

		static inline input_stream_t input_stream(stream_t& stream) { return input_stream_t(stream); }
	};
}


template<json_encoding encoding = json_encoding::utf8>
struct reader {
	template<typename target_t, typename stream_t>
	static void from_stream(
			target_t& target,
			stream_t& stream) {
		using namespace rapidjson;
		using namespace json_impl;
		using namespace detail;
		Reader reader;
		auto handler = json_handlers_factory::make_object_handler(target);
		handler->prepare_for_loading();
		bool parsed = false;
		auto actual_stream = json_reading<stream_t, encoding>::input_stream(stream);
		parsed = reader.Parse(actual_stream, *handler);

		if (!parsed) {
			ParseErrorCode e = reader.GetParseErrorCode();
			size_t o = reader.GetErrorOffset();
			AF_ERROR("Error parsing JSON. Error is: % (at %)",
				GetParseError_En(e), o);
		}
	}

	template<typename target_t>
	inline static void from_string(
			target_t& target,
			typename traits::string_t const& json) {
		using namespace rapidjson;
		StringStream ss(json.c_str());
		from_stream(target, ss);
	}

	template<typename target_t>
	inline static void from_string(
			std::shared_ptr<target_t>& target,
			typename traits::string_t const& json) {
		from_string(*target, json);
	}

	template<typename target_t>
	inline static target_t from_string(
			typename traits::string_t const& json) {
		target_t target;
		from_string(target, json);
		return target;
	}

	template<typename target_t>
	inline static void from_file(
			target_t& target,
			typename traits::string_t const& path) {
		using namespace rapidjson;
		// rapidjson documentation says this is how to open files
#ifdef _WIN32
		const char* const file_flags = "rb";
#else
		const char* const file_flags = "r";
#endif
		FILE* fp = fopen(path.c_str(), file_flags);
		char readBuffer[_AF_JSON_READ_BUFFER_SIZE];
		FileReadStream fs(fp, readBuffer, sizeof(readBuffer));
		from_stream(target, fs);
		fclose(fp);
	}
	template<typename target_t>
	inline static void from_file(
		std::shared_ptr<target_t>& target,
		typename traits::string_t const& path) {
		if (!target)
			target = std::make_shared<target_t>();
		from_file(*target, path);
	}

	template<typename target_t>
	inline static target_t from_file(
			typename traits::string_t const& path,
			bool encoded = false) {
		target_t target;
		from_file(target, json, encoded);
		return target;
	}
};

template<json_encoding encoding = json_encoding::utf8>
struct writer {

	template<typename target_t, typename writer_t>
	static void to_writer(
		target_t& target,
		writer_t& writer) {
		using namespace rapidjson;
		using namespace json_impl;
		auto handler = json_handlers_factory::make_object_handler(target);
		auto writer_wrapper = af_json_writer_wrapper_impl_t<writer_t>{ writer };
		handler->write(writer_wrapper);
	}

	template<typename target_t, typename stream_t>
	static void to_stream(
		target_t& target,
		stream_t& stream,
		bool pretty = false,
		bool put_bom = false) {
		using namespace rapidjson;
		using namespace detail;

		using writer_factory_t = json_writing<stream_t, encoding>;
		if (pretty) {
			auto writer = writer_factory_t::pretty_writer(stream, put_bom);
			to_writer(target, writer);
		}
		else {
			auto writer = writer_factory_t::writer(stream, put_bom);
			to_writer(target, writer);
		}
	}

	template<typename target_t>
	inline static void to_string(
			target_t& target,
			typename traits::string_t& json,
			bool pretty = false,
			bool put_bom = false) {
		using namespace rapidjson;
		StringBuffer ss;
		to_stream(target, ss, pretty, put_bom);
		json = ss.GetString();
	}

	template<typename target_t>
	inline static void to_string(
			std::shared_ptr<target_t>& target,
			typename traits::string_t& json,
			bool pretty = false,
			bool put_bom = false) {
		to_string(*target, json, pretty, put_bom);
	}

	template<typename target_t>
	inline static traits::string_t to_string(
			target_t& target,
			bool pretty = false,
			bool put_bom = false) {
		using namespace rapidjson;
		StringBuffer ss;
		to_stream(target, ss, pretty, put_bom);
		return ss.GetString();
	}

	template<typename target_t>
	inline static traits::string_t to_string(
			std::shared_ptr<target_t>& target,
			bool pretty = false,
			bool put_bom = false) {
		return to_string(*target, pretty, put_bom);
	}

	template<typename target_t>
	static void to_file(
			target_t& target,
			typename traits::string_t& path,
			bool pretty = false,
			bool put_bom = false) {
		// rapidjson documentation says this is how to open files
		using namespace rapidjson;
#ifdef _WIN32
		const char* const file_flags = "wb";
#else
		const char* const file_flags = "w";
#endif
		FILE* fp = fopen(path.c_str(), file_flags);
		char writeBuffer[_AF_JSON_READ_BUFFER_SIZE];
		FileWriteStream fs(fp, writeBuffer, sizeof(writeBuffer));
		to_stream(target, fs, pretty, put_bom);
		fclose(fp);
	}

	template<typename target_t>
	inline static void to_file(
			std::shared_ptr<target_t>& target,
			typename traits::string_t& path,
			bool pretty = false,
			bool put_bom = false) {
		to_file(*target, path, pretty, put_bom);
	}
};

} // namespace json
} // namespace autotelica