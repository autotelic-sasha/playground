#pragma once
#include "autotelica_core/util/include/std_disambiguation.h"
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

// Compile time configuration


// Encoding. Most of the time UTF8 will just work. 
#ifndef _AF_SERIALIZATION_CHAR_T
#define _AF_SERIALIZATION_CHAR_T char
#endif

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

// When Optimised Default Initialisation is turned off, values are not initialised 
// with defaults during JSON reads.
// Otherwise they are. 
#ifndef		_AF_JSON_OPTIMISED_DEFAULT_INITIALISATION
#define		_AF_JSON_OPTIMISED_DEFAULT_INITIALISATION true
#endif

// Checking if there are duplicate keys in object descriptions.
#ifndef		_AF_JSON_VALIDATE_DUPLICATE_KEYS
#define		_AF_JSON_VALIDATE_DUPLICATE_KEYS false
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

// When Optimised Default Initialisation is turned off, values are not initialised 
// with defaults during JSON reads.
// Otherwise they are. 
#ifndef		_AF_JSON_OPTIMISED_DEFAULT_INITIALISATION
#define		_AF_JSON_OPTIMISED_DEFAULT_INITIALISATION false
#endif

// Checking if there are duplicate keys in object descriptions.
#ifndef		_AF_JSON_VALIDATE_DUPLICATE_KEYS
#define		_AF_JSON_VALIDATE_DUPLICATE_KEYS true
#endif

#endif

#define AF_JSON_TAG( name, value ) static const _AF_SERIALIZATION_CHAR_T* const name = value; 

namespace autotelica{
namespace serialization {
	using namespace autotelica::std_disambiguation;
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
struct af_json_handler_t; // we need the forward declaration to handle the types more prettily

// traits class to name all the types used later
struct af_json_types_t {
	using char_t = _AF_SERIALIZATION_CHAR_T;
	using string_t = std::basic_string<char_t>;
	using key_t = string_t;
	using handler_t = af_json_handler_t;
	using handler_p = af_json_handler_t*;
	using handlers_t = std::vector<std::pair<key_t, handler_p>>;

	static inline bool equal_tag(char_t const* s, size_t n, char_t const* t) {
		for (size_t i = 0; i < n; ++i, ++s, ++t)
			if (*t != *s) return false;
		return true;
	}
	static inline bool equal_tag(string_t const& s, string_t const& t) {
		return equal_tag(s.c_str(), s.size(), t.c_str());
	}
	static inline void assign(string_t* target, const char_t* src, size_t length) {
		target->assign(src, length);
	}
};

#define _AF_COMMA ,
// SFINAE based type filter
#define _AF_JSON_TYPE_FILTER( condition ) std::enable_if_t<condition, bool> = true

// creator functions all need to follow a certain form
#define _AF_JSON_DECLARE_HANDLER_CREATOR( HandlerType, Condition ) \
	template< typename target_t, _AF_JSON_TYPE_FILTER( Condition ) >\
	af_json_handler_value_t<target_t>* af_create_rjson_handler(\
			target_t* target_,\
			typename af_json_default_value<target_t>* default_ = nullptr,\
			void* dummy = nullptr) {\
		AF_ASSERT(dummy == nullptr, "Wrong create function invoked.");\
		return new HandlerType<target_t>(target_, default_);\
	}\


#define _AF_JSON_DECLARE_CONTAINER_HANDLER_CREATOR( HandlerType, Condition ) \
	template< typename target_t, _AF_JSON_TYPE_FILTER( Condition ) >\
	af_json_handler_value_t<target_t>* af_create_rjson_handler(\
			target_t* target_,\
			typename af_json_default_value<target_t>* default_ = nullptr,\
			typename target_t::value_af_json_default_value_t* value_default_ = nullptr) {\
		return new HandlerType<target_t>(target_, default_, value_default_);\
	}


// rapidjson has this weird thing about writers - there's no hierarchy
// but we want to make things really easy to use, so we are going to pay 
// the cost of virtual function calls (at least until we see too much damage in profilers)
// so we have this little wrapper hierarchy
struct af_json_writer_wrapper_t {
	virtual ~af_json_writer_wrapper_t() {}

	using char_t = rjson_impl::af_json_types_t::char_t;

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
struct af_json_writer_wrapper_impl_t : public af_json_writer_wrapper_t {
	writer_t& _writer;
	using char_t = rjson_impl::af_json_types_t::char_t;

	af_json_writer_wrapper_impl_t(writer_t& writer_):_writer(writer_){}

	bool Null() override { return _writer.Null(); }
	bool Bool(bool b) override { return _writer.Bool(b); }
	bool Int(int i) override { return _writer.Int(i); }
	bool Uint(unsigned i) override { return _writer.Uint(i); }
	bool Int64(int64_t i) override { return _writer.Int64(i); }
	bool Uint64(uint64_t i)  override { return _writer.Uint64(i); }
	bool Double(double d) override { return _writer.Double(d); }
	bool RawNumber(const char_t * str, size_t length, bool copy) override { return _writer.RawNumber(str, length, copy); }
	bool String(const char_t* str, size_t length, bool copy) override { return _writer.String(str, length, copy); }
	bool StartObject() override { return _writer.StartObject(); }
	bool Key(const char_t * str, size_t length, bool copy) override { return _writer.Key(str, length, copy); }		
	bool EndObject(size_t memberCount) override { return _writer.EndObject(memberCount); }
	bool StartArray() override { return _writer.StartArray(); }
	bool EndArray(size_t elementCount) override { return _writer.EndArray(elementCount); }
};

// writing simple types
namespace writing {
	inline void write(int const& value, af_json_writer_wrapper_t& writer) { writer.Int(value); }
	inline void write(unsigned const& value, af_json_writer_wrapper_t& writer) { writer.Uint(value); }
	inline void write(short const& value, af_json_writer_wrapper_t& writer) { writer.Int(value); }
	inline void write(unsigned short const& value, af_json_writer_wrapper_t& writer) { writer.Uint(value); }
	inline void write(long const& value, af_json_writer_wrapper_t& writer) { writer.Int64(static_cast<std::int64_t>(value)); }
	inline void write(unsigned long const& value, af_json_writer_wrapper_t& writer) { writer.Uint64(static_cast<std::uint64_t>(value)); }
	inline void write(long long const& value, af_json_writer_wrapper_t& writer) { writer.Int64(static_cast<std::int64_t>(value)); }
	inline void write(unsigned long long const& value, af_json_writer_wrapper_t& writer) { writer.Uint64(static_cast<std::uint64_t>(value)); }
	//inline void write(size_t const& value, af_json_writer_wrapper_t& writer) { writer.Uint64(static_cast<std::uint64_t>(value)); }

	inline void write(float const& value, af_json_writer_wrapper_t& writer) { writer.Double(static_cast<double>(value)); }
	inline void write(double const& value, af_json_writer_wrapper_t& writer) { writer.Double(value); }

	//inline void write(long float const& value, af_json_writer_wrapper_t& writer) { AF_ERROR("long float serialisation to json is not supported"); }
	//inline void write(long double const& value, af_json_writer_wrapper_t& writer) { AF_ERROR("long double serialisation to json is not supported"); }

	inline void write(bool const& value, af_json_writer_wrapper_t& writer) { writer.Bool(value); }

	inline void write(rjson_impl::af_json_types_t::string_t const& value, af_json_writer_wrapper_t& writer) {
		writer.String(value.c_str(), value.length(), false);
	}
}

// handling defaults generically is a pain
// we have a whole class to do it properly
template<typename target_t>
class af_json_default_value{
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
	af_json_default_value(target_t const& default_value_) :
		_default_value(default_value_),
		_target_initialised(false) {
	}
	inline bool initialise(target_t* target_){
		return (_AF_JSON_OPTIMISED_DEFAULT_INITIALISATION ||
				((_AF_JSON_TERSE && set_target(target_))));
	}
	inline bool null(target_t* target_)  {
		return (_AF_JSON_OPTIMISED_DEFAULT_INITIALISATION ||
				set_target(target_));
	}

	bool write(target_t const& target_, af_json_writer_wrapper_t& writer_) const {
		// in terse mode, when _target is equal to default, we just don't write it
		return (
			_AF_JSON_TERSE &&
			target_ == _default_value);
	}

	void reset(target_t* target_) {
		_target_initialised = false;
		initialise(target_);
	}
};

// we use naked pointers internally a lot
// a couple of utility functions to make memory management consistent
template<typename ptr_t>
inline void release_handler_ptr(ptr_t* p) {
	if (p)
		p->release();
	p = nullptr;
}
template<typename ptr_t>
inline void release_ptr(ptr_t p) {
	if (p) {
		delete p;
		p = nullptr;
	}
}
// rapidjson SAX
// base class  for rapidjson SAX handlers
struct af_json_handler_t {
	using char_t = af_json_types_t::char_t;

	virtual ~af_json_handler_t() {}
	virtual bool is_done() const = 0; // signal to the controller that we are done loading this value
	virtual bool handles_terse_storage() const { return false; }
	virtual bool prepare_for_loading() { return true; }

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

	virtual void write(af_json_writer_wrapper_t& writer_) const = 0;
};

// base handler for most values
template<typename target_t>
struct af_json_handler_value_t : public af_json_handler_t {
	
	using char_t = af_json_types_t::char_t;
	using base_t = af_json_handler_t;
	using default_value_t = af_json_default_value<target_t>;

	target_t* _target; // owned by someone else, don't delete
	default_value_t* _default_h;
	bool _done;

	af_json_handler_value_t(
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
	inline bool set(ConvertibleT const& value_) { *_target = static_cast<target_t>( value_ ); return done(); }

	virtual void reset(target_t* target_) { 
		_target = target_; 
		if (_default_h)
			_default_h->initialise(_target);
		_done = false; 
	}
	bool prepare_for_loading() override {
		reset(_target);
		return true;
	}
	inline bool done() { return (_done = true); }

	bool is_done() const override { return _done; }
	bool handles_terse_storage() const override { return _default_h != nullptr; }
	inline bool is_set() const { return _target != nullptr; }

	inline target_t const& get() const { return *_target; }
	inline target_t& get() { return *_target; }

	bool Null()  override {
		AF_ASSERT(_default_h, "Unexpected: null read when default value is not provided.");
		return done() && _default_h->null(_target);
	}

	void release() override { 
		release_ptr(_default_h);
		base_t::release();
	}

	inline bool default_write(af_json_writer_wrapper_t& writer_) const {
		return (
			_default_h &&
			_default_h->write(*_target, writer_));
	}
	void write(af_json_writer_wrapper_t& writer_) const override{
		if (default_write(writer_)) return;
		writing::write(_target, writer_);
	}
};

// handler for integral types
template<typename target_t>
struct af_json_handler_integral_t : public af_json_handler_value_t<target_t> {

	using base_t = af_json_handler_value_t<target_t>;
	using default_value_t = af_json_default_value<target_t>;

	af_json_handler_integral_t(
			target_t* target_, 
			default_value_t* default_ = nullptr) :
		base_t(target_, default_) {}

	bool Bool(bool b)  override { return base_t::set(b); }
	bool Int(int i)  override { return base_t::set(i); }
	bool Uint(unsigned i)  override { return base_t::set(i); }
	bool Int64(int64_t i)  override { return base_t::set(i); }
	bool Uint64(uint64_t i) override { return base_t::set(i); }

};

_AF_JSON_DECLARE_HANDLER_CREATOR(af_json_handler_integral_t, std::is_integral<target_t>::value )

// handler for floating types
template<typename target_t>
struct af_json_handler_floating_t : public af_json_handler_value_t<target_t>{

	using base_t = af_json_handler_value_t<target_t>;
	using default_value_t = af_json_default_value<target_t>;

	af_json_handler_floating_t(
			target_t* target_, 
			default_value_t* default_ = nullptr) :
		base_t(target_, default_) {}

	bool Double(double d) override { return base_t::set(d); }
	bool Int(int i)  override { return base_t::set(i); }
	bool Uint(unsigned i)  override { return base_t::set(i); }
	bool Int64(int64_t i)  override { return base_t::set(i); }
	bool Uint64(uint64_t i) override { return base_t::set(i); }

};

_AF_JSON_DECLARE_HANDLER_CREATOR(af_json_handler_integral_t, std::is_floating_point<target_t>::value)

// handler for strings
template<typename target_t>
struct af_json_handler_string_t : public af_json_handler_value_t<target_t> {

	using char_t = af_json_types_t::char_t;
	using base_t = af_json_handler_value_t<target_t>;
	using default_value_t = af_json_default_value<target_t>;

	af_json_handler_string_t(
			target_t* target_, 
			default_value_t* default_ = nullptr) :
		base_t(target_, default_) {}

	// reader part
	bool String(const char_t* str, size_t length, bool copy) override {
		af_json_types_t::assign(base_t::_target, str, length);
		return base_t::done();
	}

};

_AF_JSON_DECLARE_HANDLER_CREATOR(af_json_handler_string_t, is_string_t<target_t>::value)

// handler for enumerations
template<typename target_t>
struct af_json_handler_enum_t : public af_json_handler_value_t<target_t> {

	using char_t = af_json_types_t::char_t;
	using base_t = af_json_handler_value_t<target_t>;
	using default_value_t = af_json_default_value<target_t>;
	using string_t = af_json_types_t::string_t;

	af_json_handler_enum_t(
		target_t* target_,
		default_value_t* default_ = nullptr) :
		base_t(target_, default_) {}

	// reader part
	bool String(const char_t* str, size_t length, bool copy) override {
		using namespace autotelica::enum_to_string;
		*base_t::_target = to_enum<target_t>(str);
		return base_t::done();
	}
	void write(af_json_writer_wrapper_t& writer_) const override {
		if (base_t::default_write(writer_)) return;
		string_t out;
		using autotelica::enum_to_string;
		to_string(out, *base_t::_target);
		writing::write(out, writer_);
	}
};

_AF_JSON_DECLARE_HANDLER_CREATOR(af_json_handler_enum_t, std::is_enum<target_t>::value)

// handler for pointers
template<typename target_t>
struct af_json_handler_ptr_t : public af_json_handler_value_t<target_t> {
	
	using char_t = af_json_types_t::char_t;
	using base_t = af_json_handler_value_t<target_t>;
	using default_value_t = af_json_default_value<target_t>;
	using contained_t = typename target_t::element_type;
	using value_handler_t = af_json_handler_value_t<contained_t>;
	using contained_default_value_t = af_json_default_value<contained_t>;

	value_handler_t* _value_handler;
	contained_default_value_t* _value_default;

	af_json_handler_ptr_t(
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
		return &(*(*base_t::_target));
	}
	inline void initialise_target() {
		if (!(*base_t::_target))
			*base_t::_target = target_t(new contained_t());
	}
	void reset(target_t* target_) override { 
		base_t::reset(target_);
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

	void release() override {
		release_handler_ptr(_value_handler);
		release_ptr(_value_default);
		base_t::release();
	}

	void write(af_json_writer_wrapper_t& writer_) const override {
		if (base_t::default_write(writer_)) return;
		_value_handler->write(writer_);
	}
};

_AF_JSON_DECLARE_CONTAINER_HANDLER_CREATOR(af_json_handler_ptr_t, is_shared_ptr_t<target_t>::value || std::is_pointer<target_t>::value)

// handler for sequences (lists and vectors)
template<typename target_t>
struct af_json_handler_sequence_t : public af_json_handler_value_t<target_t> {
	
	using char_t = af_json_types_t::char_t;
	using base_t = af_json_handler_value_t<target_t>;
	using default_value_t = af_json_default_value<target_t>;
	using contained_t = typename target_t::value_type;
	using contained_default_value_t = af_json_default_value<contained_t>;
	using value_handler_t = af_json_handler_value_t<contained_t>;
	 
	value_handler_t* _value_handler;

	af_json_handler_sequence_t(
			target_t* target_, 
			default_value_t* default_ = nullptr,
			contained_default_value_t* contained_default_ = nullptr) :
		base_t(target_, default_),
		_value_handler(af_create_rjson_handler<contained_t>(nullptr, contained_default_)){
	}

	void reset(target_t* target_) override {
		base_t::reset(target_);
		_value_handler->reset(nullptr);
	}
	inline void set_next() {
		base_t::_target->emplace_back({});
		contained_t& ret(base_t::_target->back());
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
			return base_t::done();
		return delegate_f(&value_handler_t::EndArray, elementCount);
	}

	 void release() override {
		 release_handler_ptr(_value_handler);
		 base_t::release();
	 }

	 void write(af_json_writer_wrapper_t& writer_) const override {
		 if (base_t::default_write(writer_)) return;
		 writer_->StartArray();
		 for (auto const& t : base_t::_target) {
			 _value_handler->reset(&t);
			 _value_handler->write(writer_);
		 }
		 writer_->EndArray();
		 _value_handler->reset(nullptr);
	 }
 };

_AF_JSON_DECLARE_CONTAINER_HANDLER_CREATOR(af_json_handler_sequence_t, is_sequence_t<target_t>::value);

// handler for sets
 template<typename target_t>
 struct af_json_handler_setish_t : public af_json_handler_sequence_t<target_t> {

	 using char_t = af_json_types_t::char_t;
	 using base_t = af_json_handler_sequence_t<target_t>;
	 using default_value_t = af_json_default_value<target_t>;
	 using contained_t = typename target_t::value_type;
	 using value_handler_t = af_json_handler_value_t<contained_t>;

	 contained_t _current_value;

	 af_json_handler_setish_t(
			target_t* target_,
			default_value_t* default_ = nullptr) :
		 base_t(target_, default_, nullptr){// sets canot contain default values

	 }

	 template<typename... ParamsT>
	 inline bool delegate_f(bool (value_handler_t::*mf)(ParamsT ...), ParamsT... ps) {
		 if (!base_t::_value_handler->is_set()) {
			 _current_value = contained_t();
			 base_t::_value_handler->reset(&_current_value);
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
 
_AF_JSON_DECLARE_HANDLER_CREATOR(af_json_handler_setish_t, is_setish_t<target_t>::value);

// handling maps needs a bit of work
// general maps are stored as arrays of pair objects: { "key" : key_value, "value" : value_value }
// there is an optimisation for string maps
// first we need an intermediate handler for these objects
namespace pair_tags{
	AF_JSON_TAG(tag_key, "key");
	AF_JSON_TAG(tag_value, "value");
};
// handling pairs
template< typename target_t>
struct af_json_handler_pair_t : public af_json_handler_value_t<target_t> {
	
	using char_t = af_json_types_t::char_t;
	using base_t = af_json_handler_value_t<target_t>;
	using key_t = typename target_t::first_type;
	using value_t = typename target_t::second_type;
	using key_handler_t = af_json_handler_value_t<key_t>;
	using value_handler_t = af_json_handler_value_t<value_t>;
	using current_handler_t = af_json_handler_t;
	using default_value_t = af_json_default_value <value_t> ;

	inline key_t* key() { return base_t::is_set()?(&(base_t::_target->first)):nullptr; }
	inline value_t* value() { return base_t::is_set()?(&(base_t::_target->second)):nullptr;}
	
	key_handler_t* _key_handler;
	value_handler_t* _value_handler;
	current_handler_t* _current_handler; // switches between _key_handler, _value_handler, and nullptr

	af_json_handler_pair_t(
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
		if (af_json_types_t::equal_tag(str, length, pair_tags::tag_key))
			_current_handler = _key_handler;
		else if (af_json_types_t::equal_tag(str, length, pair_tags::tag_value) == 0)
			_current_handler = _value_handler;
		else
			AF_ERROR("Unknown key value: %", str);
		return true;
	}
	bool EndObject(size_t memberCount) {
		if (_current_handler)
			return delegate_f(&current_handler_t::EndObject, memberCount);
		return base_t::done();
	}

	bool StartArray() { return delegate_f(&current_handler_t::StartArray); }
	bool EndArray(size_t elementCount) { return delegate_f(&current_handler_t::EndArray, elementCount); }
	
	void release() override {
		release_handler_ptr(_key_handler);
		release_handler_ptr(_value_handler);
		base_t::release();
	}

	void write(af_json_writer_wrapper_t& writer_) const override {
		if (base_t::default_write(writer_)) return;
		writer_->StartObject();
		writer_->Key(pair_tags::tag_key);
		_key_handler->write(writer_);
		writer_->Key(pair_tags::tag_value);
		_value_handler->write(writer_);
		writer_->EndObject();
	}
};
_AF_JSON_DECLARE_HANDLER_CREATOR(af_json_handler_pair_t, is_pair_t<target_t>::value);
// handler for general maps
template<typename target_t>
struct af_json_handler_mappish_t : public af_json_handler_value_t<target_t> {
	// TODO: this is so similar to both sequence and settish
	// perhaps all they need is different construction?
	using char_t = af_json_types_t::char_t;
	using base_t = af_json_handler_value_t<target_t>;
	using default_value_t = af_json_default_value<target_t>;
	using value_t = typename target_t::mapped_type;
	using contained_t = typename target_t::value_type;
	using value_handler_t = af_json_handler_pair_t<contained_t>;
	using value_default_value_t = af_json_default_value <value_t>;

	contained_t _current_value;
	value_handler_t* _value_handler;

	af_json_handler_mappish_t(
			target_t* target_, 
			default_value_t* default_ = nullptr,
			value_default_value_t* value_default_f_ = nullptr) :
		base_t(target_, default_),
		_value_handler(af_create_rjson_handler<contained_t>(nullptr, value_default_f_)){
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
	void release() override {
		release_handler_ptr(_value_handler);
		base_t::release();
	}

};

#if _AF_JSON_OPTIMISED_STRING_MAPS
_AF_JSON_DECLARE_CONTAINER_HANDLER_CREATOR(af_json_handler_mappish_t, (is_mapish_t<target_t>::value && !(is_string_t<typename target_t::key_t>::value)))
#else
_AF_JSON_DECLARE_CONTAINER_HANDLER_CREATOR(af_json_handler_mappish_t, (is_mapish_t<target_t>::value))
#endif 
//opitimised format for maps with string keys
template<typename target_t>
struct af_json_handler_string_mappish_t : public af_json_handler_value_t<target_t> {

	using char_t = af_json_types_t::char_t;
	using base_t = af_json_handler_value_t<target_t>;
	using default_value_t = af_json_default_value<target_t>;
	using key_t = typename target_t::key_type; // this is some string type
	using value_t = typename target_t::mapped_type;
	using value_handler_t = af_json_handler_value_t <value_t> ;
	using value_default_value_t = af_json_default_value<value_t>;
	
	value_handler_t* _value_handler;

	af_json_handler_string_mappish_t(
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
			af_json_types_t::assign(&key, str, length);
			AF_ASSERT(base_t::_target->find(key) == base_t::_target->end(), "Duplicate key found in object (%)", key);
			_value_handler->reset(value_ptr(key));
			return true;
		}
		return delegate_f(&value_handler_t::Key, str, length, copy); 
	}
	bool EndObject(size_t memberCount) override {  
		if (!_value_handler->is_set())
			return base_t::done();
		return delegate_f(&value_handler_t::EndObject, memberCount);
	}
	bool StartArray() override {return delegate_f(&value_handler_t::StartArray);}
	bool EndArray(size_t elementCount) override {return delegate_f(&value_handler_t::EndArray, elementCount);}

	void release() override {
		release_handler_ptr(_value_handler);
		base_t::release();
	}

	void write(af_json_writer_wrapper_t& writer_) const override {
		if (base_t::default_write(writer_)) return;
		writer_->StartObject();
		for (auto const& t : base_t::_target) {
			writer_->Key(t.first);
			_value_handler->reset(&t.second);
			_value_handler->write(writer_);
		}
		writer_->EndObject();
		_value_handler->reset(nullptr);
	}
};

#if _AF_JSON_OPTIMISED_STRING_MAPS
_AF_JSON_DECLARE_CONTAINER_HANDLER_CREATOR(af_json_handler_mappish_t, (is_mapish_t<target_t>::value && is_string_t<typename target_t::key_t>::value))
#endif

// handler for objects
template<typename target_t>
struct af_json_handler_object_t : public af_json_handler_value_t<target_t> {

	using char_t = af_json_types_t::char_t;
	using base_t = af_json_handler_value_t<target_t>; 
	using default_value_t = af_json_default_value<target_t>;
	
	using key_t = af_json_types_t::key_t;
	using handler_t = af_json_types_t::handler_t;
	using handler_p = af_json_types_t::handler_p;
	using handlers_t = af_json_types_t::handlers_t;

	using pre_load_function_t = std::function<void()>;
	using pre_save_function_t = std::function<void()>;
	using post_load_function_t = std::function<void()>;
	using post_save_function_t = std::function<void()>;

	handlers_t	_handlers;
	handler_p _current_handler;
	pre_load_function_t _pre_load_f;
	pre_save_function_t _pre_save_f;
	post_load_function_t _post_load_f;
	post_save_function_t _post_save_f;

	af_json_handler_object_t(
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

	inline handler_p find_handler(const char_t* const key, size_t length) {
		for (auto& h : _handlers) {
			if (af_json_types_t::equal_tag(key, length, h.first.c_str()))
				return h.second;
		}
		return nullptr;
	}

	bool validate_all_loaded() const {
		for (auto const& h : _handlers) {
			if (!h.second->is_done())
				if( !_AF_JSON_TERSE || !h.second->handles_terse_storage())
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
		return base_t::done();
	}
	bool StartArray() override { return delegate_f(&handler_t::StartArray); }
	bool EndArray(size_t elementCount) override { return delegate_f(&handler_t::EndArray,elementCount); }

	void write(af_json_writer_wrapper_t& writer_) const override {
		if (_pre_save_f)
			_pre_save_f();
		if (base_t::default_write(writer_)) return;
		writer_.StartObject();
		for (auto const& h : _handlers) {
			writer_.Key(h.first.c_str(), h.first.size(), false);
			h.second->write(writer_);
		}
		writer_.EndObject(_handlers.size());
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
struct has_no_default_t {};
struct has_no_element_default_t {};

constexpr has_no_default_t has_no_default;
constexpr has_no_element_default_t has_no_element_default;

template<typename target_t>
struct default_value_description {
	target_t const _default_value;

	rjson_impl::af_json_default_value<target_t>* create_rjson_default() const {
		 return new rjson_impl::af_json_default_value<target_t>(_default_value);
	}
};

struct member_description {
	using key_t = typename rjson_impl::af_json_types_t::key_t;
	
	key_t _key;
	
	member_description(key_t const& key_) :_key(key_) {}

	key_t const& key() const { return _key; }

	virtual rjson_impl::af_json_handler_t* create_rjson_handler() const = 0;
	virtual ~member_description() {}
};



template<typename target_t, typename target_element_t = void*>
struct member_description_impl : public member_description {
private:
	// resolving the type of element defaults, helps to avoid excessive overloading
	template<typename target_element_t>
	struct resolve_default_t {
		using default_t = rjson_impl::af_json_default_value<target_element_t>*;
	};
	template<>
	struct resolve_default_t<void*> {
		using default_t = void*;
	};
public:	
	//		is_sequence<target_t>::value || is_mappish<target_t>::value 
	using key_t = typename rjson_impl::af_json_types_t::key_t;
	using target_p = target_t*;
	using target_default_p = default_value_description<target_t>*;
	using target_element_default_p = default_value_description<target_element_t>*;

	target_p _target;
	target_default_p _default_value;
	target_element_default_p _element_default_value;

	member_description_impl(
		key_t const& key_,
		target_p target_,
		target_default_p default_value_,
		target_element_default_p element_default_value_
		) :
		member_description(key_),
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
	rjson_impl::af_json_handler_t* create_rjson_handler() const override {
		rjson_impl::af_json_default_value<target_t>* _default(nullptr);
		if (_default_value)
			_default = _default_value->create_rjson_default();
		typename resolve_default_t<target_element_t>::default_t _element_default(nullptr);
		//rjson_impl::af_json_default_value<target_element_t>* _element_default(nullptr);
		if (_element_default_value)
			_element_default = _element_default_value->create_rjson_default();
		return af_create_rjson_handler(_target, _default, _element_default);
	}
};
template<typename target_t, typename element_t>
inline member_description* create_member_description(
	typename rjson_impl::af_json_types_t::key_t const& key_,
	target_t* target_,
	target_t const& default_,
	element_t const& element_default_
) {
	return new member_description_impl<target_t, element_t>(
		key_,
		target_,
		new default_value_description<target_t>(default_),
		new default_value_description<element_t>(element_default_));
}
template<typename target_t>
inline member_description* create_member_description(
	typename rjson_impl::af_json_types_t::key_t const& key_,
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
	typename rjson_impl::af_json_types_t::key_t const& key_,
	target_t* target_,
	target_t const& default_,
	has_no_element_default_t const& unused_ = has_no_element_default
) {
	return new member_description_impl<target_t>(
		key_,
		target_,
		new default_value_description<target_t>{ default_ },
		nullptr);
}

class object_description  {

public:
	using key_t = typename rjson_impl::af_json_types_t::key_t;
	using pre_load_function_t = std::function<void()>;
	using pre_save_function_t = std::function<void()>;
	using post_load_function_t = std::function<void()>;
	using post_save_function_t = std::function<void()>;
	using member_description_p = member_description*;
	using member_descriptions_t = std::vector<member_description_p>;
	using handlers_t = typename rjson_impl::af_json_types_t::handlers_t;
private:
	bool _done;
	pre_load_function_t _pre_load_f;
	pre_save_function_t _pre_save_f;
	post_load_function_t _post_load_f;
	post_save_function_t _post_save_f;
	member_descriptions_t _member_descriptions;
	
	
	inline void validate_key(key_t const& key) {
#ifdef _AF_JSON_VALIDATE_DUPLICATE_KEYS
		AF_ASSERT(!key.empty(), "Empty keys are not allowed");
		auto test_key = [&](member_description_p d) { return rjson_impl::af_json_types_t::equal_tag(key, d->key()); };
		for (auto const& d : _member_descriptions) {
			AF_ASSERT(test_key(d),"Key % is duplicated", key);
		}
#endif
	}
public:
	object_description() :_done(false) {
	}
	~object_description() {
		for (member_description_p p : _member_descriptions)
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
		typename rjson_impl::af_json_types_t::key_t const& key_,
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
		typename rjson_impl::af_json_types_t::key_t const& key_,
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
	typename rjson_impl::af_json_handler_t* create_rjson_handler(
		target_t* target_,
		typename rjson_impl::af_json_handler_object_t<target_t>::default_value_t* default_ = nullptr) const {

			AF_ASSERT(_done, "Cannot create handlers before object description is done.");
			handlers_t _handlers;
			_handlers.resize(_member_descriptions.size());
			for (auto const& d : _member_descriptions)
				_handlers.push_back({ d->key(), d->create_rjson_handler() });

			return new rjson_impl::af_json_handler_object_t<target_t>(
				target_,
				default_,
				_pre_load_f,
				_pre_save_f,
				_post_load_f,
				_post_save_f,
				_handlers);
	}
};
class object_description_p {

public:
	using key_t = typename rjson_impl::af_json_types_t::key_t;
	using pre_load_function_t = std::function<void()>;
	using pre_save_function_t = std::function<void()>;
	using post_load_function_t = std::function<void()>;
	using post_save_function_t = std::function<void()>;
	using member_description_p = member_description*;
	using member_descriptions_t = std::vector<member_description_p>;
	using handlers_t = typename rjson_impl::af_json_types_t::handlers_t;
private:
	std::shared_ptr<object_description> _description;
public:
	object_description_p() :_description(new object_description()){
	}

	inline object_description_p& before_loading(pre_load_function_t& f) {
		_description->before_loading(f);
		return *this;
	}
	inline object_description_p& before_saving(pre_save_function_t& f) {
		_description->before_saving(f);
		return *this;
	}
	inline object_description_p& after_loading(post_load_function_t& f) {
		_description->after_loading(f);
		return *this;
	}
	inline object_description_p& after_saving(post_save_function_t& f) {
		_description->after_saving(f);
		return *this;
	}
	template<typename target_t, typename element_t>
	inline object_description_p& member(
		key_t const& key_,
		target_t& target_,
		target_t const& default_,
		element_t const& element_default_
	) {
		_description->member(key_, target_, default_, element_default_);
		return *this;
	}
	template<typename target_t>
	inline object_description_p& member(
		typename rjson_impl::af_json_types_t::key_t const& key_,
		target_t& target_,
		has_no_default_t const& default_ = has_no_default,
		has_no_element_default_t const& element_default_ = has_no_element_default
	) {
		_description->member(key_, target_, default_, element_default_);
		return *this;
	}
	template<typename target_t>
	inline object_description_p& member(
		typename rjson_impl::af_json_types_t::key_t const& key_,
		target_t& target_,
		target_t const& default_,
		has_no_element_default_t const& element_default_ = has_no_element_default
	) {
		_description->member(key_, target_, default_, element_default_);
		return *this;
	}
	inline object_description_p& end_object() {
		_description->end_object();
		return *this;
	}

	inline operator std::shared_ptr<object_description>() {
		return _description;
	}
};

namespace json {
	enum class json_encoding {
		utf8,
		utf16le,
		utf16be,
		utf32le,
		utf32be,
		detect
	};
}

namespace rjson_impl {
	template< typename target_t, _AF_JSON_TYPE_FILTER(std::is_base_of<af_serializable _AF_COMMA target_t>::value) >
	af_json_handler_t* af_create_rjson_handler(
		target_t* target_,
		typename target_t::af_json_default_value_t* default_ = nullptr,
		void* unused_ = nullptr) {
		return target_->serialisation()->create_rjson_handler(target_t, default_);
	}
	
	template<typename stream_t, json::json_encoding encoding>
	struct rjson_writing;

	template<typename stream_t> // source is always utf8
	struct rjson_writing<stream_t, json::json_encoding::utf8> {
		using input_encoding_t = rapidjson::UTF8<_AF_SERIALIZATION_CHAR_T>;
		using output_encoding_t = rapidjson::UTF8<_AF_SERIALIZATION_CHAR_T>;
		using output_stream_t = stream_t;
		using writer_t = rapidjson::Writer<output_stream_t, input_encoding_t>;
		using pretty_writer_t = rapidjson::PrettyWriter<output_stream_t, input_encoding_t>;
		using reader_stream_t = stream_t;

		static inline writer_t writer(stream_t& stream, bool put_bom = false) { return writer_t(stream); }
		static inline pretty_writer_t pretty_writer(stream_t& stream, bool put_bom = false) { return pretty_writer_t(stream);}
	};

#define __AF_RJSON_WRITING_TRAIT(OUTPUT_ENCODING_ENUM, OUTPUT_ENCODING_RJSON)\
	template<typename stream_t> \
	struct rjson_writing<stream_t, json::json_encoding::OUTPUT_ENCODING_ENUM> {\
		using input_encoding_t = rapidjson::UTF8<_AF_SERIALIZATION_CHAR_T>;\
		using output_encoding_t = rapidjson::OUTPUT_ENCODING_RJSON<_AF_SERIALIZATION_CHAR_T>;\
		using output_stream_t = rapidjson::EncodedOutputStream<output_encoding_t, stream_t>;\
		using writer_t = rapidjson::Writer< output_stream_t, input_encoding_t, output_encoding_t>;\
		using pretty_writer_t = rapidjson::Writer< output_stream_t, input_encoding_t, output_encoding_t>;\
		static inline writer_t writer(stream_t& stream, bool put_bom = false) { return writer_t(output_stream_t(stream, put_bom)); }\
		static inline pretty_writer_t pretty_writer(stream_t& stream, bool put_bom = false) { return pretty_writer_t(output_stream_t(stream, put_bom)); }\
	};

__AF_RJSON_WRITING_TRAIT(utf16le, UTF16LE)
__AF_RJSON_WRITING_TRAIT(utf16be, UTF16BE)
__AF_RJSON_WRITING_TRAIT(utf32le, UTF32LE)
__AF_RJSON_WRITING_TRAIT(utf32be, UTF32BE)

template<typename stream_t, json::json_encoding encoding>
struct rjson_reading;

#define __AF_RJSON_READING_TRAIT(OUTPUT_ENCODING_ENUM, OUTPUT_ENCODING_RJSON)\
template<typename stream_t> \
struct rjson_reading<stream_t, json::json_encoding::OUTPUT_ENCODING_ENUM> {\
	using input_encoding_t = rapidjson::OUTPUT_ENCODING_RJSON<_AF_SERIALIZATION_CHAR_T>;\
	using input_stream_t = rapidjson::EncodedInputStream<input_encoding_t, stream_t>;\
	static inline input_stream_t input_stream(stream_t& stream) { return input_stream_t(stream); }\
};

__AF_RJSON_READING_TRAIT(utf16le, UTF16LE)
__AF_RJSON_READING_TRAIT(utf16be, UTF16BE)
__AF_RJSON_READING_TRAIT(utf32le, UTF32LE)
__AF_RJSON_READING_TRAIT(utf32be, UTF32BE)

template<typename stream_t> 
struct rjson_writing<stream_t, json::json_encoding::detect> {
		using input_stream_t = rapidjson::AutoUTFInputStream<_AF_SERIALIZATION_CHAR_T, stream_t>; 
		
		static inline input_stream_t input_stream(stream_t& stream) { return input_stream_t(stream); }
};

}

// public interface for JSON serialization
namespace json {
	// TODO: schema validation
#define _AF_IS_SERIALIZABLE(TARGET_T) std::enable_if_t<std::is_base_of<typename autotelica::serialization::af_serializable, typename TARGET_T>::value, bool> = true 

	template<json_encoding encoding = json_encoding::utf8>
	struct loader {
		template<
			typename target_t,
			typename stream_t,
			_AF_JSON_TYPE_FILTER(std::is_base_of<af_serializable _AF_COMMA target_t>::value)>
		static void from_stream(
			std::shared_ptr<target_t>& target,
			stream_t& stream) {
			using namespace rapidjson;
			using namespace rjson_impl;
			rjson_impl::af_json_handler_t* handler = nullptr;
			try {
				Reader reader;
				handler = af_create_rjson_handler(target.get());
				bool parsed = false;
				auto actual_stream = rjson_writing<stream_t, encoding>::input_stream(stream);
				parsed = reader.Parse(actual_stream, *handler);

				ParseErrorCode e = reader.GetParseErrorCode();
				size_t o = reader.GetErrorOffset();
				AF_ERROR("Error parsing JSON. Error is: % (at %, near '%...')",
					GetParseError_En(e), o, json.substr(o, 10));
			}
			catch (...) {
				release_handler_ptr(handler);
				throw;
			}
			release_handler_ptr(handler);
		}
		template<
			typename target_t,
			_AF_IS_SERIALIZABLE(target_t)>
		static void from_string(
			std::shared_ptr<target_t>& target,
			typename rjson_impl::af_json_types_t::string_t const& json) {
			using namespace rapidjson;
			StringStream ss(json.c_str());
			from_stream(target, ss);
		}

		template<
			typename target_t,
			_AF_IS_SERIALIZABLE(target_t)>
		static std::shared_ptr<target_t> from_string(
			typename rjson_impl::af_json_types_t::string_t const& json) {
			std::shared_ptr<target_t> target(new target_t);
			from_string(target, json);
			return target;
		}


		template<typename target_t, _AF_IS_SERIALIZABLE(target_t)>
		static void from_file(
			std::shared_ptr<target_t>& target,
			typename rjson_impl::af_json_types_t::string_t const& path) {
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

		template<typename target_t, _AF_IS_SERIALIZABLE(target_t)>
		static std::shared_ptr<target_t> from_file(
			typename rjson_impl::af_json_types_t::string_t const& path,
			bool encoded = false) {
			std::shared_ptr<target_t> target(new target_t);
			from_file(target, json, encoded);
			return target;
		}
	};
	
	template<json_encoding encoding = json_encoding::utf8>
	struct writer {
		template<typename target_t, typename writer_t, _AF_IS_SERIALIZABLE(target_t)>
		static void to_writer(
			std::shared_ptr<target_t>& target,
			writer_t& writer) {
			using namespace rapidjson;
			using namespace rjson_impl;
			af_json_handler_t* handler = nullptr;
			try {
				handler = target->serialization()->create_rjson_handler(const_cast<target_t*>(target.get()));
				auto writer_wrapper = af_json_writer_wrapper_impl_t<writer_t>{ writer };
				handler->write(writer_wrapper);
			}
			catch (...) {
				release_handler_ptr(handler);
				throw;
			}
			release_handler_ptr(handler);
		}

		template<typename target_t, typename stream_t, _AF_IS_SERIALIZABLE(target_t)>
		static void to_stream(
			std::shared_ptr<target_t>& target,
			stream_t& stream,
			bool pretty = false,
			bool put_bom = false) {
			using namespace rapidjson;
			using writer_factory_t = rjson_impl::rjson_writing<stream_t, encoding>;
			if (pretty) {
				auto writer = writer_factory_t::pretty_writer(stream, put_bom);
				to_writer(target, writer);
			}
			else {
				auto writer = writer_factory_t::writer(stream, put_bom);
				to_writer(target, writer);
			}
		}
		template<typename target_t, _AF_IS_SERIALIZABLE(target_t)>
		static void to_string(
			std::shared_ptr<target_t>& target,
			typename rjson_impl::af_json_types_t::string_t& json,
			bool pretty = false,
			bool put_bom = false) {
			using namespace rapidjson;
			StringBuffer ss;
			to_stream(target, ss, pretty, put_bom);
			json = ss.GetString();
		}
		template<typename target_t, _AF_IS_SERIALIZABLE(target_t)>
		static rjson_impl::af_json_types_t::string_t to_string(
			std::shared_ptr<target_t>& target,
			bool pretty = false,
			bool put_bom = false) {
			using namespace rapidjson;
			StringBuffer ss;
			to_stream(target, ss, pretty, put_bom);
			return ss.GetString();
		}
		template<typename target_t, _AF_IS_SERIALIZABLE(target_t)>
		static void to_file(
			std::shared_ptr<target_t>& target,
			typename rjson_impl::af_json_types_t::string_t& path,
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
	};
}

}//namespace serialization
}//namespace autotelica