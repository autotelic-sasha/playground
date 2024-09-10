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
using namespace autotelica::std_disambiguation;


//  terse means don't write nulls OR defaults
//  reading:
//		default on null  - just do it, if default value is present
//		otherwise 
//			throw on null
//		
//		terse			 - initialise at start if default is present 
//
//	writing:
//		if not terse
//			write nullptr as null
//		if terse and default is present
//			null on default


#define _AF_RJSON_ASCII 0
#define _AF_RJSON_UTF8  1
#define _AF_RJSON_UTF16 2

#ifndef _AF_RJSON_ENCODING
#define _AF_RJSON_ENCODING _AF_RJSON_UTF8
#endif

#ifdef _AF_RJSON_VERBOSE
#define _AF_RJSON_OPTIMISED false
#endif

#ifndef _AF_RJSON_OPTIMISED
#define _AF_RJSON_OPTIMISED true
#endif

#if _AF_RJSON_OPTIMISED

#ifndef		_AF_RJSON_TERSE
#define		_AF_RJSON_TERSE true
#endif
#ifndef		_AF_RJSON_OPTIMISED_STRING_MAPS
#define		_AF_RJSON_OPTIMISED_STRING_MAPS true
#endif
#ifndef		_AF_RJSON_OPTIMISED_DEFAULT_INITIALISATION
#define		_AF_RJSON_OPTIMISED_DEFAULT_INITIALISATION true
#endif

#else

#ifndef		_AF_RJSON_TERSE
#define		_AF_RJSON_TERSE false
#endif
#ifndef		_AF_RJSON_OPTIMISED_STRING_MAPS
#define		_AF_RJSON_OPTIMISED_STRING_MAPS false
#endif
#ifndef		_AF_RJSON_OPTIMISED_DEFAULT_INITIALISATION
#define		_AF_RJSON_OPTIMISED_DEFAULT_INITIALISATION false
#endif

#endif

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

struct af_rjson_char_t {
	using encoding_t = __AF_RAPIDJSON_ENCODING;
	using char_t = encoding_t::Ch;
	using string_t = std::basic_string<char_t>;

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

#define _AF_RJSON_DECLARE_HANDLER_CREATORS( HandlerType, Condition ) \
	template< typename TargetT, _AF_RJSON_TYPE_FILTER( Condition ) >\
	af_rjson_handler_value_t<TargetT>* create_handler(\
			TargetT* target_,\
			TargetT::default_value_handler_t* default_ = nullptr) {\
		return new HandlerType<TargetT>(target_, default_);\
	}\
	template< typename TargetT, _AF_RJSON_TYPE_FILTER( Condition ) >\
	af_rjson_handler_value_t<TargetT>* create_handler(\
			TargetT* target_,\
			TargetT const& default_value_) {\
		return new HandlerType<TargetT>(target_, default_value_);\
	}

#define _AF_RJSON_DECLARE_CONTAINER_HANDLER_CREATORS( HandlerType, Condition ) \
	template< typename TargetT, _AF_RJSON_TYPE_FILTER( Condition ) >\
	af_rjson_handler_value_t<TargetT>* create_handler(\
			TargetT* target_,\
			TargetT::default_value_handler_t* default_ = nullptr) {\
		return new HandlerType<TargetT>(target_, default_, nullptr);\
	}\
	template< typename TargetT, _AF_RJSON_TYPE_FILTER( Condition ) >\
	af_rjson_handler_value_t<TargetT>* create_handler(\
			TargetT* target_,\
			value_default_value_handler_t* value_default_) {\
		return new HandlerType<TargetT>(target_, nullptr, value_default_);\
	}\
	template< typename TargetT, _AF_RJSON_TYPE_FILTER( Condition ) >\
	af_rjson_handler_value_t<TargetT>* create_handler(\
			TargetT* target_,\
			TargetT::default_value_handler_t* default_,
			value_default_value_handler_t* value_default_) {\
		return new HandlerType<TargetT>(target_, default_, value_default_);\
	}\
	template< typename TargetT, _AF_RJSON_TYPE_FILTER( Condition ) >\
	af_rjson_handler_value_t<TargetT>* create_handler(\
			TargetT* target_,\
			TargetT const& default_value_) {\
		return new HandlerType<TargetT>(target_, default_value_);\
	}\
	template< typename TargetT, _AF_RJSON_TYPE_FILTER(Condition) >\
	af_rjson_handler_value_t<TargetT>* create_handler(\
			TargetT* target_, \
			TargetT const& default_value_, 
			TargetT::value_t const& value_default_value_) {\
		return new HandlerType<TargetT>(target_, default_value_, value_default_value_);\
	}



struct af_rjson_serializable {
	using char_t = af_rjson_char_t::char_t;
	using key_t = af_rjson_char_t::string_t;
	using handler_t = af_rjson_handler;
	using handler_t_p = std::shared_ptr<af_rjson_handler>;
	using handlers_t = std::vector<std::pair<key_t, handler_t_p>>;
	// we use a vector here to preserve the order of elements

	virtual handlers_t& handlers() = 0;
};

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

	inline void write(af_rjson_char_t::string_t const& value, af_rjson_writer_wrapper_t& writer) {
		writer.String(value.c_str(), value.length(), false);
	}
}

template<typename target_t>
default_value_handler{
	target_t const _default_value;
	bool _target_initialised;
	inline bool set_target(target_t* target_) {
		if (!_target_initialised) {
			 *target_ = _default_value;
			_target_initialised = true;
		}
		return true;
	}
public:
	default_value_handler(target_t const& default_value_) :
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

struct af_rjson_handler_t {
	using char_t = af_rjson_char_t::char_t;

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

protected:
	// utility functions used by derived classes
	// makes memory management a little more consistent
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

};

template<typename target_t>
struct af_rjson_handler_value_t : public af_rjson_handler_t {
	
	using char_t = af_rjson_char_t::char_t;
	using base_t = af_rjson_handler_t;
	using default_value_handler_t = default_value_handler<target_t>;

	target_t* _target; // owned by someone else, don't delete
	default_value_handler_t* _default_h;
	bool _done;

	af_rjson_handler_value_t(
			target_t* target_, 
			default_value_handler_t* default_ = nullptr) :
		_target(target_), 
		_default_h(default_),
		_done(false){
		AF_ASSERT(_target, "Target is not initialised.");
		if(_default_h)
			_default_h->initialise(_target);
	}

	af_rjson_handler_value_t(
			target_t* target_, 
			target_t const& default_value_) :
		_target(target_),
		_default_h(new default_value_handler<target_t>(default_value_),
		_done(false) {
		AF_ASSERT(_target, "Target is not initialised.");
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

template<typename target_t>
struct af_rjson_handler_integral_t : public af_rjson_handler_value_t<target_t> {

	using base_t = af_rjson_handler_value_t<target_t>;
	using default_value_handler_t = default_value_handler<target_t>;

	af_rjson_handler_integral_t(
			target_t* target_, 
			default_value_handler_t* default_ = nullptr) :
		base_t(target_, default_) {}
	
	af_rjson_handler_integral_t(
			target_t* target_, 
			target_t const& default_value_) :
		bsae_t(target_, default_value_){}

	bool Bool(bool b)  override { return set(b); }
	bool Int(int i)  override { return set(i); }
	bool Uint(unsigned i)  override { return set(i); }
	bool Int64(int64_t i)  override { return set(i); }
	bool Uint64(uint64_t i) override { return set(i); }

};

_AF_RJSON_DECLARE_HANDLER_CREATORS(af_rjson_handler_integral_t, std::is_integral<TargetT>::value )

template<typename target_t>
struct af_rjson_handler_floating_t : public af_rjson_handler_value_t<target_t>{

	using base_t = af_rjson_handler_value_t<target_t>;
	using default_value_handler_t = default_value_handler<target_t>;

	af_rjson_handler_floating_t(
			target_t* target_, 
			default_value_handler_t* default_ = nullptr) :
		base_t(target_, default_) {}

	af_rjson_handler_floating_t(	
			target_t* target_, 
			target_t const& default_value_) :
		base_t(target_, default_value_) {}

	bool Double(double d) override { return set(d); }
	bool Int(int i)  override { return set(i); }
	bool Uint(unsigned i)  override { return set(i); }
	bool Int64(int64_t i)  override { return set(i); }
	bool Uint64(uint64_t i) override { return set(i); }

};

_AF_RJSON_DECLARE_HANDLER_CREATORS(af_rjson_handler_integral_t, std::is_floating_point<TargetT>::value)

template<typename target_t>
struct af_rjson_handler_string_t : public af_rjson_handler_value_t<target_t> {

	using char_t = af_rjson_char_t::char_t;
	using base_t = af_rjson_handler_value_t<target_t>;
	using default_value_handler_t = default_value_handler<target_t>;

	af_rjson_handler_string_t(
			target_t* target_, 
			default_value_handler_t* default_ = nullptr) :
		base_t(target_, default_) {}

	af_rjson_handler_string_t(
			target_t* target_, 
			target_t const& default_value_) :
		base_t(target_, default_value_) {}

	// reader part
	bool String(const char_t* str, size_t length, bool copy) override {
		af_rjson_char_t::assign(_target, str, length);
		return done();
	}

};

_AF_RJSON_DECLARE_HANDLER_CREATORS(af_rjson_handler_string_t, is_string<TargetT>::value)

template<typename target_t>
struct af_rjson_handler_enum_t : public af_rjson_handler_value_t<target_t> {

	using char_t = af_rjson_char_t::char_t;
	using base_t = af_rjson_handler_value_t<target_t>;
	using default_value_handler_t = default_value_handler<target_t>;
	using string_t = af_rjson_char_t::string_t;

	af_rjson_handler_string_t(
		target_t* target_,
		default_value_handler_t* default_ = nullptr) :
		base_t(target_, default_) {}

	af_rjson_handler_string_t(
		target_t* target_,
		target_t const& default_value_) :
		base_t(target_, default_value_) {}

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

_AF_RJSON_DECLARE_HANDLER_CREATORS(af_rjson_handler_enum_t, std::is_enum<TargetT>::value)


template<typename target_t>
struct af_rjson_handler_ptr_t : public af_rjson_handler_value_t<target_t> {
	
	using char_t = af_rjson_char_t::char_t;
	using base_t = af_rjson_handler_value_t<target_t>;
	using default_value_handler_t = default_value_handler<target_t>;
	using contained_t = target_t::element_type;
	using value_handler_t = af_rjson_handler_value_t<contained_t>;

	value_handler_t* _value_handler;

	af_rjson_handler_ptr_t(
			target_t* target_, 
			default_value_handler_t* default_ = nullptr) :
		base_t(target_, default_),
		_value_handler(nullptr){
	}
	
	af_rjson_handler_ptr_t(
			target_t* target_,
			target_t const& default_value_) :
		base_t(target_, default_value_),
		_value_handler(nullptr){
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
			_value_handler = create_handler(value_ptr());
	}

	template<typename... ParamsT>
	bool delegate_f(bool (value_handler_t::* mf)(ParamsT ...), ParamsT... ps) {
		initialise_target();
		if(!_value_handler)
			_value_handler = create_handler(value_ptr()); // for shared_ptr this is _target->get(), but this should work for naked pointers too
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
		base_t::release();
	}

	void write(af_rjson_writer_wrapper_t& writer_) const override {
		if (default_write(writer_)) return;
		_value_handler->write(writer_);
	}
};

_AF_RJSON_DECLARE_HANDLER_CREATORS(af_rjson_handler_shared_ptr_t, is_shared_ptr<TargetT>::value || std::is_pointer<TargetT>::value)

template<typename target_t>
struct af_rjson_handler_sequence_t : public af_rjson_handler_value_t<target_t> {
	
	using char_t = af_rjson_char_t::char_t;
	using base_t = af_rjson_handler_value_t<target_t>;
	using default_value_handler_t = default_value_handler<target_t>;
	using contained_t = target_t::value_type;
	using contained_default_value_handler_t = default_value_handler<contained_t>;
	using value_handler_t = af_rjson_handler_value_t<contained_t>;
	 
	value_handler_t* _value_handler;

	af_rjson_handler_sequence_t(
			target_t* target_, 
			default_value_handler_t* default_f_ = nullptr,
			contained_default_value_handler_t* contained_default_f_ = nullptr) :
		base_t(target_, default_),
		_value_handler(create_handler<contained_t>(nullptr, contained_default_f_)){
	}
	
	af_rjson_handler_sequence_t(
			target_t* target_,
			target_t const& default_value_) :
		base_t(target_, default_value_),
		_value_handler(create_handler<contained_t>(nullptr)){
	}

	af_rjson_handler_sequence_t(
			target_t* target_,
			target_t const& default_value_,
			contained_t const& contained_default_value) :
		base_t(target_, default_value_),
		_value_handler(create_handler<contained_t>(nullptr, contained_default_value)) {
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

_AF_RJSON_DECLARE_CONTAINER_HANDLER_CREATORS(af_rjson_handler_sequence_t, is_sequence<TargetT>::value);

 template<typename target_t>
 struct af_rjson_handler_setish_t : public af_rjson_handler_sequence_t<target_t> {

	 using char_t = af_rjson_char_t::char_t;
	 using base_t = af_rjson_handler_sequence_t<target_t>;
	 using default_value_handler_t = default_value_handler<target_t>;
	 using contained_t = target_t::value_type;
	 using value_handler_t = af_rjson_handler_value_t<contained_t>;

	 contained_t _current_value;

	 af_rjson_handler_setish_t(
			target_t* target_,
			 default_value_handler_t* default_ = nullptr) :
		 base_t(target_, default_, nullptr){// sets canot contain default values

	 }
	 af_rjson_handler_setish_t(
			target_t* target_, 
			target_t const& default_value_) :
		 base_t(target_, default_value_) {
	 }
	 void reset(target_t* target_) override {
		 base_t::reset(target);
		 _value_handler->reset(nullptr);
		 _contained_value = contained_t();
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
 
_AF_RJSON_DECLARE_HANDLER_CREATORS(af_rjson_handler_setish_t, is_setish<TargetT>::value);

// general maps are stored as arrays of pair objects: { "key" : key_value, "value" : value_value }
// there is an optimisation for string maps
// first we need an intermediate handler for these objects
namespace pair_tags{
	AF_RJSON_TAG(tag_key, "key");
	AF_RJSON_TAG(tag_value, "value");
};
// reader and writer are different for these pairs
template< typename target_t>
struct af_rjson_handler_pair_t : public af_rjson_handler_value_t<target_t> {
	
	using char_t = af_rjson_char_t::char_t;
	using base_t = af_rjson_handler_value_t<target_t>;
	using key_t = target_t::first_type;
	using value_t = target_t::second_type;
	using key_handler_t = af_rjson_handler_value_t<key_t>;
	using value_handler_t = af_rjson_handler_value_t<value_t>;
	using current_handler_t = af_rjson_handler_t;
	using default_value_handler_t = default_value_handler <value_t> ;

	inline key_t* key() { return is_set()?(&(_target->first):nullptr; }
	inline value_t* value() { return is_set()?(&(_target->second)):nullptr;}
	
	key_handler_t* _key_handler;
	value_handler_t* _value_handler;
	current_handler_t* _current_handler; // switches between _key_handler, _value_handler, and nullptr

	af_rjson_handler_pair_t(
			target_t* target_,
			default_value_handler_t* value_default_value_f = nullptr) :
		base_t(target_, nullptr), // maps cannot contain nulls
		_current_handler(nullptr) {

		_key_handler = create_handler<key_t>(key(), nullptr);
		_value_handler = create_handler<value_t>(value(), value_default_value_f);
	}

	af_rjson_handler_pair_t(
			target_t* target_,
			value_t const& value_default_value) :
		base_t(target_, nullptr), // maps cannot contain nulls
		_current_handler(nullptr) {

		_key_handler = create_handler<key_t>(key(), nullptr);
		_value_handler = create_handler<value_t>(value(), value_default_value);
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
		if (af_rjson_char_t::equal_tag(str, length, pair_tags::tag_key))
			_current_handler = _key_handler;
		else if (af_rjson_char_t::equal_tag(str, length, pair_tags::tag_value) == 0)
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

template<typename target_t>
struct af_rjson_handler_mappish_t : public af_rjson_handler_value_t<target_t> {
	// TODO: this is so similar to both sequence and settish
	// perhaps all they need is different construction?
	using char_t = af_rjson_char_t::char_t;
	using base_t = af_rjson_handler_value_t<target_t>;
	using default_value_handler_t = default_value_handler<target_t>;
	using value_t = target_t::mapped_type;
	using contained_t = target_t::value_type;
	using value_handler_t = af_json_pairs::af_rjson_handler_pair_t<contained_t>;
	using value_default_value_handler_t = default_value_handler <value_t>;

	contained_t _current_value;
	value_handler_t* _value_handler;

	af_rjson_handler_mappish_t(
			target_t* target_, 
			default_value_handler_t* default_ = nullptr,
			value_default_value_handler_t* value_default_f_ = nullptr) :
		base_t(target_, default_),
		_value_handler(create_handler<contained_t>(nullptr, value_default_f_){
	}
	af_rjson_handler_mappish_t(
			target_t* target_,
			target_t const& default_value_) :
		base_t(target_, default_value_),
		_value_handler(create_handler<contained_t>(nullptr){
	}
	af_rjson_handler_mappish_t(
			target_t* target_,
			target_t const& default_value_,
			value_t const& value_default_value_) :
		base_t(target_, default_value_),
		_value_handler(create_handler<contained_t>(nullptr, value_default_value_){
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
		af_rjson_handler_value_t<target_>::release();
	}

};

#if _AF_RJSON_OPTIMISED_STRING_MAPS
_AF_RJSON_DECLARE_CONTAINER_HANDLER_CREATORS(af_rjson_handler_mappish_t, (is_mappish<TargetT>::value && !(is_string<TargetT::key_t>::value)))
#else
_AF_RJSON_DECLARE_CONTAINER_HANDLER_CREATORS(af_rjson_handler_mappish_t, (is_mappish<TargetT>::value))
#endif 
//opitimised format for maps with string keys
template<typename target_t>
struct af_rjson_handler_string_mappish_t : public af_rjson_handler_value_t<target_t> {

	using char_t = af_rjson_char_t::char_t;
	using base_t = af_rjson_handler_value_t<target_t>;
	using default_value_handler_t = default_value_handler<target_t>;
	using key_t = target_t::key_type; // this is some string type
	using value_t = target_t::mapped_type;
	using value_handler_t = af_rjson_handler_value_t <value_t> ;
	using value_default_value_handler_t = default_value_handler<value_t>;
	
	value_handler_t* _value_handler;

	af_rjson_handler_mappish_t(
			target_t* target_, 
			default_value_handler_t* default_ = nullptr,
			value_default_value_handler_t* value_default_f_ = nullptr) :
		base_t(target_, default_),
		_value_handler(create_handler<value_t>(nullptr, value_default_f_)){
	}

	af_rjson_handler_mappish_t(
			target_t* target_,
			target_t const& default_value_) :
		base_t(target_, default_value_),
		_value_handler(create_handler<value_t>(nullptr)){
	}

	af_rjson_handler_mappish_t(
			target_t* target_,
			target_t const& default_value_, 
			value_t const& value_default_value) :
		base_t(target_, default_value_),
		_value_handler(create_handler<value_t>(nullptr, value_default_value)) {
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
			af_rjson_char_t::assign(&key, str, length);
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
_AF_RJSON_DECLARE_CONTAINER_HANDLER_CREATORS(af_rjson_handler_mappish_t, (is_mappish<TargetT>::value && is_string<TargetT::key_t>::value))
#endif

template<typename target_t>
struct af_rjson_handler_object_t : public af_rjson_handler_value_t<target_t> {

	using char_t = af_rjson_char_t::char_t;
	using base_t = af_rjson_handler_value_t<target_t>; 
	using default_value_handler_t = default_value_handler<target_t>;
	using key_t = af_rjson_serializable::key_t;
	using handler_t = af_rjson_serializable::handler_t;
	using handler_t_p = af_rjson_serializable::handler_t_p;
	using handlers_t = af_rjson_serializable::handlers_t;

	handler_t_p _current_handler;

	af_rjson_handler_object_t(
			target_t* target_,
			default_value_handler_t* default_ = nullptr,
			handlers_t const& handlers_ = {}) :
		base_t(target_, default_)
		_current_handler(nullptr){
	}
	af_rjson_handler_object_t(
			target_t* target_,
			target_t const& default_value_) :
		base_t(target_, default_value_),
		_current_handler(nullptr) {
	}

	inline handlers_t& handlers() {
		return _target->handlers();
	}
	inline handler_t_p find_handler(const char_t* const key, size_t length) {
		for (auto& h : _handlers) {
			if (af_rjson_char_t::equal_tag(key, length, h.first))
				return h.second;
		}
		return nullptr;
	}

	bool validate_all_loaded() const {
		for (auto const& h : handlers()) {
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
		return true;
	}
	bool Key(const char_t* str, size_t length, bool copy) { 
		if(_current_handler)
			return delegate_f(&handler_t::String, str, length, copy);

		_current_handler = find_handler(str, length);
		AF_ASSERT(_current_handler, "Unexpected key when loading object %", str);
		return true;
	}
	bool EndObject(size_t memberCount) override {
		if (_current_handler)
			return _current_handler->EndObject(memberCount);
		AF_ASSERT(validate_all_loaded(), "Not all object members are loaded.");
		return done(); 
	}
	bool StartArray() override { return delegate_f(&handler_t::StartArray); }
	bool EndArray(size_t elementCount) override { return delegate_f(&handler_t::EndArray(elementCount); }

	void write(af_rjson_writer_wrapper_t& writer_) const override {
		if (default_write(writer_)) return;
		writer_->StartObject();
		for (auto const& h : handlers()) {
			writer_->Key(h.first);
			t.second->write(writer_);
		}
		writer_->EndObject();
	}
};

