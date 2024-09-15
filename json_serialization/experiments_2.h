#pragma once
#include "autotelica_core/util/include/std_disambiguation.h"
#include "autotelica_core/util/include/asserts.h"
#include "autotelica_core/util/include/enum_to_string.h"
#include <string.h>
// for some reason, probably good, rapidjson uses their own size_t
// we are going to just make that size_type
#define RAPIDJSON_NO_SIZETYPEDEFINE
namespace rapidjson { typedef size_t size_t; }
#include "rapidjson/encodings.h"
#include "rapidjson/error/en.h"

// Compile time configuration

// constants, they help
#define _AF_RJSON_ASCII 0
#define _AF_RJSON_UTF8  1
#define _AF_RJSON_UTF16 2

// Encoding. Most of the time UTF8 will just work. 
#ifndef _AF_RJSON_ENCODING
#define _AF_RJSON_ENCODING _AF_RJSON_UTF8
#endif

// Default optmisation settings are all optimised.
// To make it all more verbose, define _AF_RJSON_VERBOSE
#ifdef _AF_RJSON_VERBOSE
#define _AF_RJSON_OPTIMISED false
#endif

#ifndef _AF_RJSON_OPTIMISED
#define _AF_RJSON_OPTIMISED true
#endif

#ifndef _AF_RJSON_READ_BUFFER_SIZE 
#define _AF_RJSON_READ_BUFFER_SIZE 4*65535
#endif

#if _AF_RJSON_OPTIMISED

// In terse mode, when target value is equal to default, we just don't write it
#ifndef		_AF_RJSON_TERSE
#define		_AF_RJSON_TERSE true
#endif

// Optimisation of strings maps means that keys in the map are used as JSON keys.
// Otherwise they are written with "key", "value" pairs like other maps.
#ifndef		_AF_RJSON_OPTIMISED_STRING_MAPS
#define		_AF_RJSON_OPTIMISED_STRING_MAPS true
#endif

// When Optimised Default Initialisation is turned off, values are not initialised 
// with defaults during JSON reads.
// Otherwise they are. 
#ifndef		_AF_RJSON_OPTIMISED_DEFAULT_INITIALISATION
#define		_AF_RJSON_OPTIMISED_DEFAULT_INITIALISATION true
#endif

// Checking if there are duplicate keys in object descriptions.
#ifndef		_AF_RJSON_VALIDATE_DUPLICATE_KEYS
#define		_AF_RJSON_VALIDATE_DUPLICATE_KEYS false
#endif

#else

// in terse mode, when target value is equal to default, we just don't write it
#ifndef		_AF_RJSON_TERSE
#define		_AF_RJSON_TERSE false
#endif

// Optimisation of strings maps means that keys in the map are used as JSON keys.
// Otherwise they are written with "key", "value" pairs like other maps.
#ifndef		_AF_RJSON_OPTIMISED_STRING_MAPS
#define		_AF_RJSON_OPTIMISED_STRING_MAPS false
#endif

// When Optimised Default Initialisation is turned off, values are not initialised 
// with defaults during JSON reads.
// Otherwise they are. 
#ifndef		_AF_RJSON_OPTIMISED_DEFAULT_INITIALISATION
#define		_AF_RJSON_OPTIMISED_DEFAULT_INITIALISATION false
#endif

// Checking if there are duplicate keys in object descriptions.
#ifndef		_AF_RJSON_VALIDATE_DUPLICATE_KEYS
#define		_AF_RJSON_VALIDATE_DUPLICATE_KEYS true
#endif

#endif

// Translating configuration to rapidjson constants and tag types.
#if _AF_RJSON_ENCODING == _AF_RJSON_ASCII
#define __AF_RAPIDJSON_ENCODING rapidjson::ASCII<>
#define AF_RJSON_TAG( name, value ) static const char* const name = value; 
#elif _AF_RJSON_ENCODING == _AF_RJSON_UTF8
#define __AF_RAPIDJSON_ENCODING rapidjson::UTF8<>
#define AF_RJSON_TAG( name, value ) static const char* const name = value; 
#elif _AF_RJSON_ENCODING == _AF_RJSON_UTF16
#define __AF_RAPIDJSON_ENCODING rapidjson::UTF16<>
#define AF_RJSON_TAG( name, value ) static const wchar_t* const name = Lvalue; 
#else
#error Unknown rapidjson encoding.
#endif
namespace autotelica{
namespace serialization {

// serialization stuff is always hierarchical, we gonna have some forward declarations
class object_description;

// af_serializable is how you make objects serializable
struct af_serializable {
	virtual std::shared_ptr<object_description> serialization() = 0;
};

// rapidjson interface implementation
namespace rjson_impl{

// we use SFINAE a lot here
using namespace autotelica::std_disambiguation;
struct af_rjson_handler_t; // we need the forward declaration to handle the types more prettily

// traits class to name all the types used later
struct af_rjson_types_t {
	using encoding_t = __AF_RAPIDJSON_ENCODING;
	using char_t = encoding_t::Ch;
	using string_t = std::basic_string<char_t>;
	using key_t = string_t;
	using handler_t = af_rjson_handler_t;
	using handler_p = af_rjson_handler_t*;
	using handlers_t = std::vector<std::pair<key_t, handler_p>>;

	static inline bool equal_tag(char_t const* s, size_t n, char_t const* t) {
		for (size_t i = 0; i < n; ++i, ++s, ++t)
			if (*t != *s) return false;
		return true;
	}

	static inline void assign(string_t* target, const char_t* src, size_t length) {
		target->assign(src, length);
	}
};

// SFINAE based type filter
#define _AF_RJSON_TYPE_FILTER( condition ) std::enable_if_t<condition, bool> = true

// creator functions all need to follow a certain form
#define _AF_RJSON_DECLARE_HANDLER_CREATOR( HandlerType, Condition ) \
	template< typename target_t, _AF_RJSON_TYPE_FILTER( Condition ) >\
	af_rjson_handler_value_t<target_t>* af_create_rjson_handler(\
			target_t* target_,\
			target_t::af_rjson_default_value_t* default_ = nullptr,\
			void* dummy = nullptr) {\
		AF_ASSERT(dummy == nullptr, "Wrong create function invoked.");
		return new HandlerType<target_t>(target_, default_);\
	}\


#define _AF_RJSON_DECLARE_CONTAINER_HANDLER_CREATOR( HandlerType, Condition ) \
	template< typename target_t, _AF_RJSON_TYPE_FILTER( Condition ) >\
	af_rjson_handler_value_t<target_t>* af_create_rjson_handler(\
			target_t* target_,\
			target_t::af_rjson_default_value_t* default_ = nullptr,\
			target_t::value_af_rjson_default_value_t* value_default_ = nullptr) {\
		return new HandlerType<target_t>(target_, default_, value_default_);\
	}


// rapidjson has this weird thing about writers - there's no hierarchy
// but we want to make things really easy to use, so we are going to pay 
// the cost of virtual function calls (at least until we see too much damage in profilers)
// so we have this little wrapper hierarchy
struct af_rjson_writer_wrapper_t {
	virtual ~af_rjson_writer_t() {}

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
struct af_rjson_writer_wrapper_impl_t {
	writer_t& _writer;

	bool Null() override { return _writer.Null(); }
	bool Bool(bool b) override { return _writer.Bool(b); }
	bool Int(int i) override { return _writer.Int(i); }
	bool Uint(unsigned i) override { return _writer.Uint(i); }
	bool Int64(int64_t i) override { return _writer.Int64(i); }
	bool Uint64(uint64_t i)  override { return _writer.Uint64(i); }
	bool Double(double d) override { return _writer.Double(i); }
	bool RawNumber(const char_t * str, size_t length, bool copy) override { return _writer.RawNumber(str, length, copy); }
	bool String(const char_t* str, size_t length, bool copy) override { return _writer.String(str, length, copy); }
	bool StartObject() override { return _writer.StartObject(); }
	bool Key(const char_t * str, size_t length, bool copy) override { return _writer.Key(str, length, copy); }		
	virtual bool EndObject(size_t memberCount) override { return _writer.EndObject(); }
	bool EndObject(size_t memberCount) override { return _writer.EndObject(memberCount); }
	bool StartArray() override { return _writer.StartArray(); }
	bool EndArray(size_t elementCount) override { return _writer.EndArray(elementCount); }
};

// writing simple types
namespace writing {
	inline void write(int const& value, af_rjson_writer_wrapper_t& writer) { writer.Int(value); }
	inline void write(unsigned const& value, af_rjson_writer_wrapper_t& writer) { writer.Uint(value); }
	inline void write(short const& value, af_rjson_writer_wrapper_t& writer) { writer.Int(value); }
	inline void write(unsigned short const& value, af_rjson_writer_wrapper_t& writer) { writer.Uint(value); }
	inline void write(long const& value, af_rjson_writer_wrapper_t& writer) { writer.Int64(static_cast<int64_t>(value); }
	inline void write(unsigned long const& value, af_rjson_writer_wrapper_t& writer) { writer.Uint64(static_cast<uint64_t>(value); }
	inline void write(long long const& value, af_rjson_writer_wrapper_t& writer) { writer.Int64(static_cast<int64_t>(value); }
	inline void write(unsigned long long const& value, af_rjson_writer_wrapper_t& writer) { writer.Uint64(static_cast<uint64_t>(value); }
	inline void write(size_t const& value, af_rjson_writer_wrapper_t& writer) { writer.Uint64(static_cast<uint64_t>(value)); }

	inline void write(float const& value, af_rjson_writer_wrapper_t& writer) { writer.Double(static_cast<double>(value)); }
	inline void write(double const& value, af_rjson_writer_wrapper_t& writer) { writer.Double(value); }

	inline void write(long float const& value, af_rjson_writer_wrapper_t& writer) { AF_ERROR("long float serialisation to json is not supported"); }
	inline void write(long double const& value, af_rjson_writer_wrapper_t& writer) { AF_ERROR("long double serialisation to json is not supported"); }

	inline void write(bool const& value, af_rjson_writer_wrapper_t& writer) { writer.Bool(value); }

	inline void write(af_rjson_types_t::string_t const& value, af_rjson_writer_wrapper_t& writer) {
		writer.String(value.c_str(), value.length(), false);
	}
}

// handling defaults generically is a pain
// we have a whole class to do it properly
template<typename target_t>
af_rjson_default_value{
	target_t const& _default_value;
	bool _target_initialised;
	inline bool set_target(target_t* target_) {
		if (!_target_initialised) {
			 *target_ = _default_value;
			_target_initialised = true;
		}
		return true;
	}
public:
	af_rjson_default_value(target_t const& default_value_) :
		_default_value(default_value_),
		_target_initialised(false) {
	}
	inline bool initialise(target_t* target_) const {
		return (_AF_RJSON_OPTIMISED_DEFAULT_INITIALISATION ||
				((_AF_RJSON_TERSE && set_target(target_)));
	}
	inline bool null(target_* target_) const {
		return (_AF_RJSON_OPTIMISED_DEFAULT_INITIALISATION ||
				set_target(target_));
	}

	bool write(target_t const& target_, af_rjson_writer_wrapper_t& writer_) const {
		// in terse mode, when _target is equal to default, we just don't write it
		return (
			_AF_RJSON_TERSE &&
			target_ == _default_value));
	}

	void reset(target_t* target_) {
		_target_initialised = false;
		initialise(target_);
	}
};

// we use naked pointers internally a lot
// a couple of utility functions to make memory management consistent
template<typename ptr_t>
inline void release_handler_ptr(ptr_t* p) const {
	if (p)
		p->release();
	p = nullptr;
}
template<typename ptr_t>
inline void release_ptr(ptr_t p) const {
	if (p) {
		delete p;
		p = nullptr;
	}
}
// rapidjson SAX
// base class  for rapidjson SAX handlers
struct af_rjson_handler_t {
	using char_t = af_rjson_types_t::char_t;

	virtual ~af_rjson_handler_t() {}
	virtual bool is_done() const = 0; // signal to the controller that we are done loading this value
	virtual bool handles_terse_storage() const { return false; }
	virtual bool prepare_for_loading() { }

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

	virtual void release() { delete this; }

	virtual void write(af_rjson_writer_wrapper_t& writer_) const = 0;
};

// base handler for most values
template<typename target_t>
struct af_rjson_handler_value_t : public af_rjson_handler_t {
	
	using char_t = af_rjson_types_t::char_t;
	using base_t = af_rjson_handler_t;
	using default_value_t = af_rjson_default_value<target_t>;

	target_t* _target; // owned by someone else, don't delete
	default_value_t* _default_h;
	bool _done;

	af_rjson_handler_value_t(
			target_t* target_, 
			default_value_t* default_ = nullptr) :
		_target(target_), 
		_default_h(default_),
		_done(false){
		AF_ASSERT(_target, "Target is not initialised.");
		if(_default_h)
			_default_h->initialise(_target);
	}

	inline bool set(target_t const& value_) { *_target = value_; return done(); }

	template<typename ConvertibleT>
	inline bool set(ConvertibleT const& value_) { *_target = target_t{ value_ }; return done(); }

	virtual void reset(target_t* target_) { 
		_target = target_; 
		if (_default_h)
			_default_h->initialise(_target);
		_done = false; 
	}
	void prepare_for_loading() override {
		reset(_target);
	}
	inline bool done() { return (_done = true); }

	bool is_done() const override { return _done; }
	bool handles_terse_storage() const override { return _default_h != nullptr; }
	inline bool is_set() const { return _target != nullptr; }

	inline target_type const& get() const { return *_target; }
	inline target_type& get() { return *_target; }

	bool Null()  override {
		AF_ASSERT(_default_h, "Unexpected: null read when default value is not provided.");
		return done() && _default_h->null(_target);
	}

	void release() override { 
		release_ptr(_default_h);
		base_t::release();
	}

	inline bool default_write(af_rjson_writer_wrapper_t& writer_) const {
		return (
			_default_h &&
			_default_h->write(_target, writer_));
	}
	void write(af_rjson_writer_wrapper_t& writer_) const override{
		if (default_write(writer_)) return;
		writing::write(_target, writer_);
	}
};

// handler for integral types
template<typename target_t>
struct af_rjson_handler_integral_t : public af_rjson_handler_value_t<target_t> {

	using base_t = af_rjson_handler_value_t<target_t>;
	using default_value_t = af_rjson_default_value<target_t>;

	af_rjson_handler_integral_t(
			target_t* target_, 
			default_value_t* default_ = nullptr) :
		base_t(target_, default_) {}

	bool Bool(bool b)  override { return set(b); }
	bool Int(int i)  override { return set(i); }
	bool Uint(unsigned i)  override { return set(i); }
	bool Int64(int64_t i)  override { return set(i); }
	bool Uint64(uint64_t i) override { return set(i); }

};

_AF_RJSON_DECLARE_HANDLER_CREATOR(af_rjson_handler_integral_t, std::is_integral<target_t>::value )

// handler for floating types
template<typename target_t>
struct af_rjson_handler_floating_t : public af_rjson_handler_value_t<target_t>{

	using base_t = af_rjson_handler_value_t<target_t>;
	using default_value_t = af_rjson_default_value<target_t>;

	af_rjson_handler_floating_t(
			target_t* target_, 
			default_value_t* default_ = nullptr) :
		base_t(target_, default_) {}

	bool Double(double d) override { return set(d); }
	bool Int(int i)  override { return set(i); }
	bool Uint(unsigned i)  override { return set(i); }
	bool Int64(int64_t i)  override { return set(i); }
	bool Uint64(uint64_t i) override { return set(i); }

};

_AF_RJSON_DECLARE_HANDLER_CREATOR(af_rjson_handler_integral_t, std::is_floating_point<target_t>::value)

// handler for strings
template<typename target_t>
struct af_rjson_handler_string_t : public af_rjson_handler_value_t<target_t> {

	using char_t = af_rjson_types_t::char_t;
	using base_t = af_rjson_handler_value_t<target_t>;
	using default_value_t = af_rjson_default_value<target_t>;

	af_rjson_handler_string_t(
			target_t* target_, 
			default_value_t* default_ = nullptr) :
		base_t(target_, default_) {}

	// reader part
	bool String(const char_t* str, size_t length, bool copy) override {
		af_rjson_types_t::assign(_target, str, length);
		return done();
	}

};

_AF_RJSON_DECLARE_HANDLER_CREATOR(af_rjson_handler_string_t, is_string<target_t>::value)

// handler for enumerations
template<typename target_t>
struct af_rjson_handler_enum_t : public af_rjson_handler_value_t<target_t> {

	using char_t = af_rjson_types_t::char_t;
	using base_t = af_rjson_handler_value_t<target_t>;
	using default_value_t = af_rjson_default_value<target_t>;
	using string_t = af_rjson_types_t::string_t;

	af_rjson_handler_enum_t(
		target_t* target_,
		default_value_t* default_ = nullptr) :
		base_t(target_, default_) {}

	// reader part
	bool String(const char_t* str, size_t length, bool copy) override {
		using autotelica::enum_to_string;
		*_target = to_enum(str);
		return done();
	}
	void write(af_rjson_writer_wrapper_t& writer_) const override {
		if (default_write(writer_)) return;
		string_t out;
		using autotelica::enum_to_string;
		to_string(out, *_target);
		writing::write(out, writer_);
	}
};

_AF_RJSON_DECLARE_HANDLER_CREATOR(af_rjson_handler_enum_t, std::is_enum<target_t>::value)

// handler for pointers
template<typename target_t>
struct af_rjson_handler_ptr_t : public af_rjson_handler_value_t<target_t> {
	
	using char_t = af_rjson_types_t::char_t;
	using base_t = af_rjson_handler_value_t<target_t>;
	using default_value_t = af_rjson_default_value<target_t>;
	using contained_t = target_t::element_type;
	using value_handler_t = af_rjson_handler_value_t<contained_t>;
	using contained_default_value_t = af_rjson_default_value<contained_t>;

	value_handler_t* _value_handler;
	contained_default_value_t* _value_default;

	af_rjson_handler_ptr_t(
			target_t* target_, 
			default_value_t* default_ = nullptr,
			contained_default_value_t* contained_default_f_ = nullptr) :
		base_t(target_, default_),
		_value_handler(nullptr),
		_value_default(contained_default_f_){
	}

	bool is_done() const override { 
		return base_t::is_done() || // _done is set when null is read
			(_value_handler && _value_handler->is_done()); } 
	
	inline contained_t* value_ptr() {
		return &(*(*_target));
	}
	inline void initialise_target() {
		if (!(*_target))
			*_target = target_t(new contained_t());
	}
	void reset(target_t* target_) override { 
		base_t::reset(target);
		if (!_value_handler)
			_value_handler->reset(value_ptr());
		else
			_value_handler = af_create_rjson_handler(value_ptr(), _value_default);
	}

	template<typename... ParamsT>
	bool delegate_f(bool (value_handler_t::* mf)(ParamsT ...), ParamsT... ps) {
		initialise_target();
		if(!_value_handler)
			_value_handler = af_create_rjson_handler(value_ptr(), _value_default); // for shared_ptr this is _target->get(), but this should work for naked pointers too
		return (_value_handler->*mf)(ps...);
	}

	bool Null()  override {
		if (_value_handler) // if we are already reading the contained value, delegate
			return _value_handler->Null(); 
		return _base_t::Null();
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

	void release() override {
		release_handler_ptr(_value_handler);
		release_ptr(_value_default);
		base_t::release();
	}

	void write(af_rjson_writer_wrapper_t& writer_) const override {
		if (default_write(writer_)) return;
		_value_handler->write(writer_);
	}
};

_AF_RJSON_DECLARE_CONTAINER_HANDLER_CREATOR(af_rjson_handler_shared_ptr_t, is_shared_ptr<target_t>::value || std::is_pointer<target_t>::value)

// handler for sequences (lists and vectors)
template<typename target_t>
struct af_rjson_handler_sequence_t : public af_rjson_handler_value_t<target_t> {
	
	using char_t = af_rjson_types_t::char_t;
	using base_t = af_rjson_handler_value_t<target_t>;
	using default_value_t = af_rjson_default_value<target_t>;
	using contained_t = target_t::value_type;
	using contained_default_value_t = af_rjson_default_value<contained_t>;
	using value_handler_t = af_rjson_handler_value_t<contained_t>;
	 
	value_handler_t* _value_handler;

	af_rjson_handler_sequence_t(
			target_t* target_, 
			default_value_t* default_f_ = nullptr,
			contained_default_value_t* contained_default_f_ = nullptr) :
		base_t(target_, default_),
		_value_handler(af_create_rjson_handler<contained_t>(nullptr, contained_default_f_)){
	}

	void reset(target_t* target_) override {
		base_t::reset(target_);
		_value_handler->reset(nullptr);
	}
	inline void set_next() {
		_target->emplace_back({});
		contained_t& ret(_target->back());
		_value_handler->reset(&ret);
	}
	template<typename... ParamsT>
	inline bool delegate_f(bool (value_handler_t::* mf)(ParamsT ...), ParamsT... ps) {
		if (!_value_handler->is_set()) 
			set_next();
		bool ret = (_value_handler->*mf)(ps...);
		if(_value_handler->is_done())
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
		if(!_value_handler->is_set())
			return done();
		return delegate_f(&value_handler_t::EndArray, elementCount);
	}

	 void release() override {
		 release_handler_ptr(_value_handler);
		 base_t::release();
	 }

	 void write(af_rjson_writer_wrapper_t& writer_) const override {
		 if (default_write(writer_)) return;
		 writer_->StartArray();
		 for (auto const& t : _target) {
			 _value_handler->reset(&t);
			 _value_handler->write(writer_);
		 }
		 writer_->EndArray();
		 _value_handler->reset(nullptr);
	 }
 };

_AF_RJSON_DECLARE_CONTAINER_HANDLER_CREATOR(af_rjson_handler_sequence_t, is_sequence<target_t>::value);

// handler for sets
 template<typename target_t>
 struct af_rjson_handler_setish_t : public af_rjson_handler_sequence_t<target_t> {

	 using char_t = af_rjson_types_t::char_t;
	 using base_t = af_rjson_handler_sequence_t<target_t>;
	 using default_value_t = af_rjson_default_value<target_t>;
	 using contained_t = target_t::value_type;
	 using value_handler_t = af_rjson_handler_value_t<contained_t>;

	 contained_t _current_value;

	 af_rjson_handler_setish_t(
			target_t* target_,
			default_value_t* default_ = nullptr) :
		 base_t(target_, default_, nullptr){// sets canot contain default values

	 }

	 template<typename... ParamsT>
	 inline bool delegate_f(bool (value_handler_t::*mf)(ParamsT ...), ParamsT... ps) {
		 if (!_value_handler->is_set()) {
			 _current_value = contained_t();
			 _value_handler->reset(&_current_value);
		 }
		 bool ret = (_value_handler->*mf)(ps...);
		 if (_value_handler->is_done()) {
			 _target->insert(_current_value);
			 _value_handler->reset(nullptr);
		 }
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
			 _value_handler->reset(&_current_value);
			 return true;
		 }
		 return delegate_f(&value_handler_t::StartArray);
	 }
	 bool EndArray(size_t elementCount) override {
		 if (!_value_handler->is_set())
			 return done();
		 return delegate_f(&value_handler_t::EndArray, elementCount);
	 }
 };
 
_AF_RJSON_DECLARE_HANDLER_CREATOR(af_rjson_handler_setish_t, is_setish<target_t>::value);

// handling maps needs a bit of work
// general maps are stored as arrays of pair objects: { "key" : key_value, "value" : value_value }
// there is an optimisation for string maps
// first we need an intermediate handler for these objects
namespace pair_tags{
	AF_RJSON_TAG(tag_key, "key");
	AF_RJSON_TAG(tag_value, "value");
};
// handling pairs
template< typename target_t>
struct af_rjson_handler_pair_t : public af_rjson_handler_value_t<target_t> {
	
	using char_t = af_rjson_types_t::char_t;
	using base_t = af_rjson_handler_value_t<target_t>;
	using key_t = target_t::first_type;
	using value_t = target_t::second_type;
	using key_handler_t = af_rjson_handler_value_t<key_t>;
	using value_handler_t = af_rjson_handler_value_t<value_t>;
	using current_handler_t = af_rjson_handler_t;
	using default_value_t = af_rjson_default_value <value_t> ;

	inline key_t* key() { return is_set()?(&(_target->first):nullptr; }
	inline value_t* value() { return is_set()?(&(_target->second)):nullptr;}
	
	key_handler_t* _key_handler;
	value_handler_t* _value_handler;
	current_handler_t* _current_handler; // switches between _key_handler, _value_handler, and nullptr

	af_rjson_handler_pair_t(
			target_t* target_,
			default_value_t* value_default_value_f = nullptr) :
		base_t(target_, nullptr), // maps cannot contain nulls
		_current_handler(nullptr) {

		_key_handler = af_create_rjson_handler<key_t>(key(), nullptr);
		_value_handler = af_create_rjson_handler<value_t>(value(), value_default_value_f);
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
	bool Uint(unsigned i) {	return delegate_f(&current_handler_t::Uint, i);	}
	bool Int64(int64_t i) { return delegate_f(&current_handler_t::Int64, i); }
	bool Uint64(uint64_t i) { return delegate_f(&current_handler_t::Uint64, i);	}
	bool Double(double d) { return delegate_f(&current_handler_t::Double, i); }
	bool String(const char_t* str, size_t length, bool copy) { return delegate_f(&current_handler_t::String, str, length, copy); }
	bool StartObject() {
		if (_current_handler)
			return delegate_f(&current_handler_t::StartObject);
		return true;
	}
	bool Key(const char_t* str, size_t length, bool copy) {
		if (_current_handler)
			return delegate_f(&current_handler_t::Key, str, length, copy);
		if (af_rjson_types_t::equal_tag(str, length, pair_tags::tag_key))
			_current_handler = _key_handler;
		else if (af_rjson_types_t::equal_tag(str, length, pair_tags::tag_value) == 0)
			_current_handler = _value_handler;
		else
			AF_ERROR("Unknown key value: %", str);
		return true;
	}
	bool EndObject(size_t memberCount) {
		if (_current_handler)
			return delegate_f(&current_handler_t::EndObject, memberCount);
		return done();
	}

	bool StartArray() { return delegate_f(&current_handler_t::StartArray); }
	bool EndArray(size_t elementCount) { return delegate_f(&current_handler_t::EndArray, elementCount); }
	
	void release() override {
		release_handler_ptr(_key_handler);
		release_handler_ptr(_value_handler);
		base_t::release();
	}

	void write(af_rjson_writer_wrapper_t& writer_) const override {
		if (default_write(writer_)) return;
		writer_->StartObject();
		writer_->Key(pair_tags::tag_key);
		_key_handler->write(writer_);
		writer_->Key(pair_tags::tag_value);
		_value_handler->write(writer_);
		writer_->EndObject();
	}
};
_AF_RJSON_DECLARE_HANDLER_CREATOR(af_rjson_handler_pair_t, is_pair<target_t>::value);
// handler for general maps
template<typename target_t>
struct af_rjson_handler_mappish_t : public af_rjson_handler_value_t<target_t> {
	// TODO: this is so similar to both sequence and settish
	// perhaps all they need is different construction?
	using char_t = af_rjson_types_t::char_t;
	using base_t = af_rjson_handler_value_t<target_t>;
	using default_value_t = af_rjson_default_value<target_t>;
	using value_t = target_t::mapped_type;
	using contained_t = target_t::value_type;
	using value_handler_t = af_json_pairs::af_rjson_handler_pair_t<contained_t>;
	using value_default_value_t = af_rjson_default_value <value_t>;

	contained_t _current_value;
	value_handler_t* _value_handler;

	af_rjson_handler_mappish_t(
			target_t* target_, 
			default_value_t* default_ = nullptr,
			value_default_value_t* value_default_f_ = nullptr) :
		base_t(target_, default_),
		_value_handler(af_create_rjson_handler<contained_t>(nullptr, value_default_f_){
	}

	void reset(target_t* target_) override {
		base_t::reset(target_);
		_value_handler->reset(nullptr);
		_current_value = contained_t();
	}
	template<typename... ParamsT>
	inline bool delegate_f(bool (value_handler_t::*mf)(ParamsT ...), ParamsT... ps) {
		if (!_value_handler->is_set()) {
			_current_value = contained_t();
			_value_handler->reset(&_current_value);
		}
		bool ret = (_value_handler->*mf)(ps...);
		if (_value_handler->is_done()) {
			_target.insert(_current_value);
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
		return delegate_f(&_value_handler::StartArray);
	}
	bool EndArray(size_t elementCount) override {
		if (!_value_handler->is_set())
			return done();
		return delegate_f(&_value_handler::EndArray, elementCount);
	}
	void release() override {
		release_handler_ptr(_value_handler);
		base_t::release();
	}

};

#if _AF_RJSON_OPTIMISED_STRING_MAPS
_AF_RJSON_DECLARE_CONTAINER_HANDLER_CREATOR(af_rjson_handler_mappish_t, (is_mappish<target_t>::value && !(is_string<target_t::key_t>::value)))
#else
_AF_RJSON_DECLARE_CONTAINER_HANDLER_CREATOR(af_rjson_handler_mappish_t, (is_mappish<target_t>::value))
#endif 
//opitimised format for maps with string keys
template<typename target_t>
struct af_rjson_handler_string_mappish_t : public af_rjson_handler_value_t<target_t> {

	using char_t = af_rjson_types_t::char_t;
	using base_t = af_rjson_handler_value_t<target_t>;
	using default_value_t = af_rjson_default_value<target_t>;
	using key_t = target_t::key_type; // this is some string type
	using value_t = target_t::mapped_type;
	using value_handler_t = af_rjson_handler_value_t <value_t> ;
	using value_default_value_t = af_rjson_default_value<value_t>;
	
	value_handler_t* _value_handler;

	af_rjson_handler_mappish_t(
			target_t* target_, 
			default_value_t* default_ = nullptr,
			value_default_value_t* value_default_f_ = nullptr) :
		base_t(target_, default_),
		_value_handler(af_create_rjson_handler<value_t>(nullptr, value_default_f_)){
	}
	
	void reset(target_t* target_) override {
		base_t::reset(target_);
		_value_handler.reset(nullptr);
	}

	inline contained_t* value_ptr(std::string const& key) {
		return &(*_target[key]);
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
			af_rjson_types_t::assign(&key, str, length);
			AF_ASSERT(_target->find(key) == _target->end(), "Duplicate key found in object (%)", key);
			_value_handler->reset(value_ptr(key));
			return true;
		}
		return delegate_f(&value_handler_t::Key, str, length, copy); 
	}
	bool EndObject(size_t memberCount) override {  
		if (!_value_handler->is_set())
			return done();
		return delegate_f(&value_handler_t::EndObject, memberCount);
	}
	bool StartArray() override {return delegate_f(&value_handler_t::StartArray);}
	bool EndArray(size_t elementCount) override {return delegate_f(&value_handler_t::EndArray, memberCount);}

	void release() override {
		release_handler_ptr(_value_handler);
		base_t::release();
	}

	void write(af_rjson_writer_wrapper_t& writer_) const override {
		if (default_write(writer_)) return;
		writer_->StartObject();
		for (auto const& t : _target) {
			writer_->Key(t.first);
			_value_handler->reset(&t.second);
			_value_handler->write(writer_);
		}
		writer_->EndObject();
		_value_handler->reset(nullptr);
	}
};

#if _AF_RJSON_OPTIMISED_STRING_MAPS
_AF_RJSON_DECLARE_CONTAINER_HANDLER_CREATOR(af_rjson_handler_mappish_t, (is_mappish<target_t>::value && is_string<target_t::key_t>::value))
#endif

// handler for objects
template<typename target_t>
struct af_rjson_handler_object_t : public af_rjson_handler_value_t<target_t> {

	using char_t = af_rjson_types_t::char_t;
	using base_t = af_rjson_handler_value_t<target_t>; 
	using default_value_t = af_rjson_default_value<target_t>;
	
	using key_t = af_rjson_types_t::key_t;
	using handler_t = af_rjson_types_t::handler_t;
	using handler_p = af_rjson_types_t::handler_p;
	using handlers_t = af_rjson_types_t::handlers_t;

	using pre_load_function_t = std::function<void()>;
	using pre_save_function_t = std::function<void()>;
	using post_load_function_t = std::function<void()>;
	using post_save_function_t = std::function<void()>;

	handlers_t	_handlers;
	handler_t_p _current_handler;
	pre_load_function_t _pre_load_f;
	pre_save_function_t _pre_save_f;
	post_load_function_t _post_load_f;
	post_save_function_t _post_save_f;

	af_rjson_handler_object_t(
			target_t* target_,
			default_value_t* default_ = nullptr,
			pre_load_function_t pre_load_f_ = nullptr,
			pre_save_function_t pre_save_f_ = nullptr,
			post_load_function_t post_load_f_ = nullptr,
			post_save_function_t post_save_f_ = nullptr,
			handlers_t const& handlers_ = {}) :
		base_t(target_, default_),
		_pre_load_f(pre_load_f_),
		_pre_save_f(pre_save_f_),
		_post_load_f(post_load_f_),
		_post_save_f(post_save_f_),
		_handlers(handlers_),
		_current_handler(nullptr){
	}

	void release() override {
		for (auto& h : _handlers)
			release_handler_ptr(h.second);
		_handlers = {};
		_current_handler = nullptr;
		base_t::release();
	}

	inline handler_t_p find_handler(const char_t* const key, size_t length) {
		for (auto& h : _handlers) {
			if (af_rjson_types_t::equal_tag(key, length, h.first))
				return h.second;
		}
		return nullptr;
	}

	bool validate_all_loaded() const {
		for (auto const& h : _handlers) {
			if (!h.second->is_done())
				if( !_AF_RJSON_TERSE || !h.second->handles_terse_storage())
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
		bool ret = (_current_handler->*mf)(ps...);
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
		if(_current_handler)
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
		return done(); 
	}
	bool StartArray() override { return delegate_f(&handler_t::StartArray); }
	bool EndArray(size_t elementCount) override { return delegate_f(&handler_t::EndArray(elementCount); }

	void write(af_rjson_writer_wrapper_t& writer_) const override {
		if (_pre_save_f)
			_pre_save_f();
		if (default_write(writer_)) return;
		writer_->StartObject();
		for (auto const& h : _handlers) {
			writer_->Key(h.first);
			t.second->write(writer_);
		}
		writer_->EndObject();
		if (_post_save_f)
			_post_save_f();

	}
};
} // namespace rjson_impl

// object descriptions
// Serilization is a two step process:
//		first we builld a generic object description
//		then out of that we create handlers for the target format
// A little slower than handcrafting things, but it does mean that we can use 
// same serialization framework for many serialization formats. 
struct has_no_default_t;
struct has_no_element_default_t;

constexpr has_no_default_t has_no_default;
constexpr has_no_element_default_t has_no_element_default;

template<target_t>
struct default_value_description {
	target_t const _default_value;

	af_rjson_default_value<target_t>* create_rjson_default() const {
		 return new af_rjson_default_value<target_t>(_default_value);
	}
};

struct member_description {
	using key_t = af_rjson_types_t::key_t;
	
	key_t _key;
	
	member_description(key_t const& _key) :_key(key_) {}

	key_t const& key() const { return _key; }

	virtual af_rjson_handler_t* create_rjson_handler() const = 0;
	virtual ~member_description() {}
};

template<typename target_t, typename target_element_t = void*>
struct member_description_impl : public member_description {
	//		is_sequence<target_t>::value || is_mappish<target_t>::value 
	using key_t = af_rjson_types_t::key_t;
	using target_p = target_t*;
	using target_default_p = default_value_description<target_t>*;
	using target_element_default_p = default_value_description<target_element_t>*;

	target_p _target;
	target_default_p _default_value;
	target_element_default_p _element_default_value;

	member_description_impl(
		key_t const* _key,
		target_p target_
		target_default_p default_value_,
		target_element_default_p element_default_value_
		) :
		_target(target_), 
		_default_value(_default_value),
		_element_default_value(element_default_value_){

	}
	~member_description_impl() {
		if (_default_value)
			delete _default_value;
		if (_element_default_value)
			delete _element_default_value;
	}
	af_rjson_handler_t* create_rjson_handler() const override {
		af_rjson_default_value<target_t>* _default(nullptr);
		if (_default_value)
			_default = _default_value->create_rjson_default();
		af_rjson_default_value<target_t>* _element_default(nullptr);
		if (_element_default_value)
			_element_default = _element_default_value->create_rjson_default();
		return af_create_rjson_handler(_target, _default, _element_default);
	}
};
template<typename target_t, typename element_t>
inline member_description* create_member_description(
	af_rjson_types_t::key_t const& key_,
	target_t* target_,
	target_t const& default_,
	element_t const& element_default_
) {
	return new member_description_impl<target_t, element_t>(
		key_,
		target_,
		new default_value_description_impl<target_t>(default_),
		new default_value_description_impl<element_t>(element_default_));
}
template<typename target_t>
inline member_description* create_member_description(
	af_rjson_types_t::key_t const& key_,
	target_t* target_,
	has_no_default_t const& unused_ = has_no_default,
	has_no_element_default_t const& unused_2_ = has_no_element_default
) {
	return new member_description_impl<target_t>(
		key_, 
		target_,
		nullptr,
		nullptr);
}
template<typename target_t>
inline member_description* create_member_description(
	af_rjson_types_t::key_t const& key_,
	target_t* target_,
	target_t const& default_,
	has_no_element_default_t const& unused_ = has_no_element_default
) {
	return new member_description_impl<target_t>(
		key_,
		target_,
		new default_value_description<target_t>(default_),
		nullptr);
}

class object_description  {

public:
	using key_t = af_rjson_types_t::key_t;
	using pre_load_function_t = std::function<void()>;
	using pre_save_function_t = std::function<void()>;
	using post_load_function_t = std::function<void()>;
	using post_save_function_t = std::function<void()>;
	using member_description_p = member_description*;
	using member_descriptions_t = std::vector<member_description_p>;
	using handlers_t = af_rjson_types_t::handlers_t;
private:
	bool _done;
	pre_load_function_t _pre_load_f;
	pre_save_function_t _pre_save_f;
	post_load_function_t _post_load_f;
	post_save_function_t _post_save_f;
	member_descriptions_t _member_descriptions;
	
	
	inline void validate_key(key_t const& key) {
#ifdef _AF_RJSON_VALIDATE_DUPLICATE_KEYS
		AF_ASSERT(!key.empty(), "Empty keys are not allowed");
		for (auto const& d : _member_descriptions) {
			AF_ASSERT(af_rjson_types_t::equal_keys(key, d->key()),
				"Key % is duplicated", key);
		}
#endif
	}
public:
	object_description() :_done(false) {
	}
	~object_description() {
		for (auto p : _member_descriptions)
			delete p;
		_member_descriptions = {};
	}

	inline object_description& before_loading(pre_load_function_t& f) {
		AF_ASSERT(!_done, "Cannot append description data once end_object is invoked.");
		_pre_load_f = f;
		return *this;
	}
	inline object_description& before_saving(pre_save_function_t& f) {
		AF_ASSERT(!_done, "Cannot append description data once end_object is invoked.");
		_pre_save_f = f;
		return *this;
	}
	inline object_description& after_loading(post_load_function_t& f) {
		AF_ASSERT(!_done, "Cannot append description data once end_object is invoked.");
		_post_load_f = f;
		return *this;
	}
	inline object_description& after_saving(post_save_function_t& f) {
		AF_ASSERT(!_done, "Cannot append description data once end_object is invoked.");
		_post_save_f = f;
		return *this;
	}
	template<typename target_t, typename element_t>
	inline object_description& member(
		key_t const& key_,
		target_t& target_,
		target_t const& default_,
		element_t const& element_default_
	) {
		AF_ASSERT(!_done, "Cannot append description data once end_object is invoked.");
		validate_key(key_);
		_member_descriptions.push_back(create_member_description(key_, &target_, default_, element_default_));
		return *this;
	}
	template<typename target_t>
	inline object_description& member(
		af_rjson_types_t::key_t const& key_,
		target_t& target_,
		has_no_default_t const& default_ = has_no_default,
		has_no_element_default_t const& element_default_ = has_no_element_default
	) {
		AF_ASSERT(!_done, "Cannot append description data once end_object is invoked.");
		validate_key(key_);
		_member_descriptions.push_back(create_member_description(key_, &target_, default_, element_default_));
		return *this;
	}
	template<typename target_t>
	inline object_description& member(
		af_rjson_types_t::key_t const& key_,
		target_t& target_,
		target_t const& default_,
		has_no_element_default_t const& element_default_ = has_no_element_default
	) {
		AF_ASSERT(!_done, "Cannot append description data once end_object is invoked.");
		validate_key(key_);
		_member_descriptions.push_back(create_member_description(key_, &target_, default_, element_default_));
		return *this;
	}
	inline object_description& end_object() {
		AF_ASSERT(!_done, "Cannot end_object more than once.");
		_done = true;
		return *this;
	}

	template<typename target_t>
	af_rjson_handler_t* create_rjson_handler(
		target_t* target_,
		target_t::af_rjson_default_value_t* default_ = nullptr) const {

			AF_ASSERT(_done, "Cannot create handlers before object description is done.");
			handlers_t _handlers;
			_handlers.resize(_member_descriptions.size());
			for (auto const& d : _member_descriptions)
				_handlers.push_back({ d->key(), d->create_rjson_handler() });

			return new af_rjson_handler_object_t(
				target_,
				default_,
				_pre_load_f,
				_pre_save_f,
				_post_load_f,
				_post_save_f,
				_handlers);
	}
};

namespace rjson_impl {
	template< typename target_t, _AF_RJSON_TYPE_FILTER(std::is_base_of<af_serializable, target_t>) >
	af_rjson_handler_t* af_create_rjson_handler(
		target_t* target_,
		target_t::af_rjson_default_value_t* default_ = nullptr,
		void* unused_ = nullptr) {
		return target_->serialisation()->create_rjson_handler(target_t, default_);
	}
}

// public interface for JSON serialization
namespace json {
	template<typename target_t, typename stream_t, _AF_RJSON_TYPE_FILTER(std::is_base_of<af_serializable, target_t>)>
	inline void load_from_stream(
			std::shared_ptr<target_t>& target, 
			stream_t& stream, 
			bool encoded = false) {
		using namespace rapidjson;
		using encoding_t = AutoUTFInputStream<unsigned, stream_t>;
		
		af_rjson_handler_t* handler = nullptr;
		try {
			Reader reader;
			handler = af_create_rjson_handler(target.get());
			bool parsed = false;
			if (encoded)
				parsed = reader.Parse(encoding_t(stream), *handler);
			else
				parsed = reader.Parse(stream, *handler);

			if (!encoded) {
				ParseErrorCode e = reader.GetParseErrorCode();
				size_t o = reader.GetErrorOffset();
				AF_ERROR("Error parsing JSON. Error is: % (at %, near '%...')",
					GetParseError_En(e), o, json.substr(o, 10));
			}
		}
		catch (...) {
			release_handler_ptr(handler);
			throw;
		}
		release_handler_ptr(handler);
	}



	template<typename target_t, _AF_RJSON_TYPE_FILTER(std::is_base_of<af_serializable, target_t>)>
	void load(
			std::shared_ptr<target_t>& target, 
			af_rjson_types_t::string_t const& json, 
			bool encoded = false) {
		using namespace rapidjson;
		StringStream ss(json.c_str());
		load_from_stream(target, ss, encoded);
	}

	template<typename target_t, _AF_RJSON_TYPE_FILTER(std::is_base_of<af_serializable, target_t>)>
	std::shared_ptr<target_t> load(
			af_rjson_types_t::string_t const& json,
			bool encoded = false) {
		std::shared_ptr<target_t> target(new target_t);
		load(target, json, encoded);
		return target;
	}


	template<typename target_t, _AF_RJSON_TYPE_FILTER(std::is_base_of<af_serializable, target_t>)>
	void load_from_file(
			std::shared_ptr<target_t>& target,
			af_rjson_types_t::string_t const& path,
			bool encoded = false) {
		using namespace rapidjson;
	// rapidjson documentation says this is how to open files
#ifdef _WIN32
		const char* const file_flags = "rb";
#else
		const char* const file_flags = "r";
#endif
		FILE* fp = fopen(path.c_str(), file_flags);
		char readBuffer[_AF_RJSON_READ_BUFFER_SIZE];
		FileReadStream fs(fp, readBuffer, sizeof(readBuffer));
		load_from_stream(target, fs, encoded);
		fclose(fp);
	}

	template<typename target_t, _AF_RJSON_TYPE_FILTER(std::is_base_of<af_serializable, target_t>)>
	std::shared_ptr<target_t> load_from_file(
		af_rjson_types_t::string_t const& path,
		bool encoded = false) {
		std::shared_ptr<target_t> target(new target_t);
		load(target, json, encoded);
		return target;
	}

	template<typename target_t, typename writer_t, _AF_RJSON_TYPE_FILTER(std::is_base_of<af_serializable, target_t>)>
	inline void write_to_writer(
			std::shared_ptr<target_t>& target,
			writer_t& writer) {
		using namespace rapidjson;
		af_rjson_handler_t* handler = nullptr;
		try {
			handler = af_create_rjson_handler(target.get());
			auto writer_wrapper = af_rjson_writer_wrapper_impl_t<writer_t>(writer);
			handler->write(writer_wrapper);
		}
		catch (...) {
			release_handler_ptr(handler);
			throw;
		}
		release_handler_ptr(handler);
	}

	template<typename target_t, typename stream_t, _AF_RJSON_TYPE_FILTER(std::is_base_of<af_serializable, target_t>)>
	inline void write_to_stream(
			std::shared_ptr<target_t>& target,
			stream_t& stream,
			bool pretty = false,
			bool encoded = false) {
		using namespace rapidjson;
		using encoding_t = AutoUTFOutputStream<unsigned, stream_t>;
		if (pretty) {
			if (encoding)
				write_to_writer(target, PrettyWriter<encoding_t>(encoding_t(stream)));
			else
				write_to_writer(target, PrettyWriter<stream_t>(stream));
		}
		else {
			if (encoding)
				write_to_writer(target, Writer<encoding_t>(encoding_t(stream)));
			else
				write_to_writer(target, Writer<stream_t>(stream));

		}
	}

	template<typename target_t, _AF_RJSON_TYPE_FILTER(std::is_base_of<af_serializable, target_t>)>
	inline void write(
			std::shared_ptr<target_t>& target,
			af_rjson_types_t::string_t& json,
			bool pretty = false,
			bool encoded = false) {
		using namespace rapidjson;
		StringBuffer ss;
		write_to_stream(target, ss, pretty, encoded);
		json = ss.GetString();
	}
	template<typename target_t, _AF_RJSON_TYPE_FILTER(std::is_base_of<af_serializable, target_t>)>
	inline af_rjson_types_t::string_t write(
			std::shared_ptr<target_t>& target,
			bool pretty = false,
			bool encoded = false) {
		using namespace rapidjson;
		StringBuffer ss;
		write_to_stream(target, ss, pretty, encoded);
		return ss.GetString();
	}
	template<typename target_t, _AF_RJSON_TYPE_FILTER(std::is_base_of<af_serializable, target_t>)>
	inline void write_to_file(
			std::shared_ptr<target_t>& target,
			af_rjson_types_t::string_t& path,
			bool pretty = false,
			bool encoded = false) {
		// rapidjson documentation says this is how to open files
#ifdef _WIN32
		const char* const file_flags = "wb";
#else
		const char* const file_flags = "w";
#endif
		FILE* fp = fopen(path.c_str(), file_flags);
		char writeBuffer[_AF_RJSON_READ_BUFFER_SIZE];
		FileWriteStream fs(fp, writeBuffer, sizeof(readBuffer));
		write_to_stream(target, fs, pretty, encoded);
		fclose(fp);
	}
}

}//namespace serialization
}//namespace autotelica