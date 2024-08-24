#pragma once
#include "autotelica_core/util/include/std_disambiguation.h"
#include "autotelica_core/util/include/asserts.h"
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


#ifndef _AF_RJSON_TERSE
#define _AF_RJSON_TERSE false
#define
#ifndef _AF_RJSON_OPTIMISED_STRING_MAPS
#define _AF_RJSON_OPTIMISED_STRING_MAPS false
#define
#ifndef _AF_RJSON_OPTIMISED_DEFAULT_INITIALISATION
#define _AF_RJSON_OPTIMISED_DEFAULT_INITIALISATION false
#define
#ifndef _AF_RJSON_ENCODING
#define _AF_RJSON_ENCODING _AF_RJSON_UTF8
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

struct af_json_char_t {
	using encoding_t = __AF_RAPIDJSON_ENCODING;
	using char_t = encoding_t::Ch;
	using string_t = std::basic_string<char_t>;
};

template<typename char_t>
inline bool equal_tag(const af_json_char_t::char_t* s, size_t n, const af_json_char_t::char_t* t) {
	for (size_t i = 0; i < n; ++i, ++s, ++t)
		if (*t != *s) return false;
	return true;
}
template<typename char_t>
inline bool string_assign(std::basic_string<char_t>* target, const char_t* src, size_t length) {
	target->assign(src, length);
}

class af_rjson_configuration {
	bool _terse;
	bool _optimised_string_maps;
	bool _optimised_default_initialisation;

	static af_rjson_configuration& instance() {
		static af_rjson_configuration _instance;
		return _instance;
	}
	af_rjson_configuration() :
		_terse(_AF_RJSON_TERSE),
		_optimised_string_maps(_AF_RJSON_OPTIMISED_STRING_MAPS),
		_optimised_default_initialisation(_AF_RJSON_OPTIMISED_DEFAULT_INITIALISATION){}
public:

	static bool terse() { return instance()._terse; }
	static void set_terse(bool terse_ = true) { instance()._terse = terse_; }

	static bool optimised_string_maps() { return instance()._optimised_string_maps; }
	static void set_optimised_string_maps(bool optimised_string_maps_ = true) { instance()._optimised_string_maps = optimised_string_maps_; }

	static bool optimised_default_initialisation() { return instance()._optimised_default_initialisation; }
	static void set_optimised_default_initialisation(bool optimised_default_initialisation_ = true) { instance()._optimised_default_initialisation = optimised_default_initialisation_; }
}



template<typename target_t>
default_value_handler {
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
		_target_initialised(false){
	}
	inline bool initialise(target_t* target_) const {

		return (optimised_default_initialisation() ||
				((af_rjson_configuration::terse() &&
					set_target(target_)));
	}
	inline bool null(target_* target_) const {
		return (optimised_default_initialisation() || 
				set_target(target_));
	}
	template<typename writer_t>
	bool write(target_t const& target_, writer_t& writer_) const {
		// in terse mode, when _target is equal to default, we just don't write it
		return (
			af_rjson_configuration::terse() &&
			target_ == _default_value));
	}
	void reset(target_t* target_) {
		_target_initialised = false;
		initialise(target_);
	}
};

template<typename target_t, typename writer_t>
inline void delegate_write(
	writer_t& writer_,
	target_t const& target_,
	default_value_handler<target_t> const* default_f_ = nullptr
) const {
	auto writer = create_writer<writer_t>(target_, default_f);
	key_writer.write(writer_);
}

template<typename target_t, typename writer_t>
void write(target_t const& value, writer_t& writer);

template< typename writer_t >
inline void write(int const& value, writer_t& writer) { writer.Int(value); }

template< typename writer_t >
inline void write(size_t const& value, writer_t& writer) { writer.Uint64(static_cast<uint64_t>(value)); }

// TODO: more write functions

// SFINAE based type filter
#define TypeFilter( condition ) std::enable_if_t<condition, bool> = true
#define CreateHandlers( HandlerType, Condition ) \
	template< typename TargetT, typename WriterT, TypeFilter( Condition ) >\
	af_rjson_handler_value_t<TargetT, WriterT>* create_handler(TargetT* target_, default_value_handler<TargetT, WriterT>* default_ = nullptr) {\
		return new HandlerType<TargetT, WriterT>(target_, default_);\
	}\
	template< typename TargetT, typename WriterT, TypeFilter( Condition ) >\
	af_rjson_handler_value_t<TargetT, WriterT>* create_handler(TargetT* target_, TargetT const& default_value_) {\
		return new HandlerType<TargetT, WriterT>(target_, default_value_);\
	}


struct af_rjson_handler_base_t {
	using char_type = encoding::Ch;
	virtual ~af_rjson_handler_t() {}
};
template<typename encoding = rapidjson::UTF8<>>
struct af_rjson_handler_t {
	using char_t = af_json_char_t::char_t;
	
	virtual ~af_rjson_handler_t(){}
	virtual bool is_done() const = 0; // signal to the controller that we are done loading this value
	virtual bool handles_terse_storage() const { return false; }
	virtual bool prepare_for_loading() { }

	virtual bool Null() { AF_ERROR("Null was not expected."); return false; }
	virtual bool Bool(bool b) { AF_ERROR("Boolean value was not expected."); return false;}
	virtual bool Int(int i) { AF_ERROR("Int value was not expected.");  return false; }
	virtual bool Uint(unsigned i) { AF_ERROR("Uint value was not expected.");  return false; }
	virtual bool Int64(int64_t i) { AF_ERROR("Int64 value was not expected."); return false; }
	virtual bool Uint64(uint64_t i) { AF_ERROR("Uint64 value was not expected."); return false;}
	virtual bool Double(double d) { AF_ERROR("Double value was not expected.");  return false; }
	virtual bool RawNumber(const char_t* str, size_t length, bool copy) { AF_ERROR("RawNumber value was not expected."); return false; }
	virtual bool String(const char_t* str, size_t length, bool copy) { AF_ERROR("String value was not expected."); return false; }
	virtual bool StartObject() { AF_ERROR("Object start was not expected."); return false; }
	virtual bool Key(const char_t* str, size_t length, bool copy) { AF_ERROR("Object key was not expected."); return false; }
	virtual bool EndObject(size_t memberCount) { AF_ERROR("Object end was not expected."); return false; }
	virtual bool StartArray() { AF_ERROR("Array start was not expected."); return false; }
	virtual bool EndArray(size_t elementCount) { AF_ERROR("Array end was not expected."); return false; }

	virtual void release() { delete this; }
};
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

template<typename target_t>
struct af_rjson_handler_value_t : public af_rjson_handler_t {
	
	using char_t = af_json_char_t::char_t;
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
};

template<typename target_t, typename writer_t>
struct af_rjson_writer_value_t {
	
	using default_value_handler_t = default_value_handler<target_t>;
	
	target_t const& _target;
	default_value_handler_t const* _default_f;

	af_rjson_writer_value_t(
		target_t const& target_,
		default_value_handler_t const* default_f_
	) : _target(_target), _default_f(default_f_){}

	inline bool default_write(writer_t& writer_) const { // returns true if default writer wrote
		return (
			_default_h &&
			_default_h->write(_target, writer_));
	}

	inline void write(writer_t& writer_) const {
		if (default_write(writer_)) return;
		write(_target, writer_);
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

CreateHandlers(af_rjson_handler_integral_t, std::is_integral<TargetT>::value )

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

CreateHandlers(af_rjson_handler_integral_t, std::is_floating_point<TargetT>::value)

template<typename target_t>
struct af_rjson_handler_string_t : public af_rjson_handler_value_t<target_t> {

	using char_t = af_json_char_t::char_t;
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
		string_assign(_target, str, length);
		return done();
	}

};

CreateHandlers(af_rjson_handler_integral_t, is_string<TargetT>::value)

template<typename target_t>
struct af_rjson_handler_ptr_t : public af_rjson_handler_value_t<target_t> {
	
	using char_t = af_json_char_t::char_t;
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
};

template<typename target_t, typename writer_t>
struct af_rjson_writer_ptr_t : public af_rjson_writer_value_t<target_t, writer_t> {

	using base_t = af_rjson_writer_value_t<target_t, writer_t>;
	using default_value_handler_t = default_value_handler<target_t>;

	af_rjson_writer_ptr_t(
		target_t const& target_,
		default_value_handler_t const* default_f_
	) : base_t(target_, default_f_) {}

	void write(writer_t& writer_) const {
		if (!(*_target)) // null pointers are always written as null
			writer.Null();
		else if (default_write(writer_))
			return;
		else 
			delegate_write(writer_, *_target, nullptr);
	}
};

CreateHandlers(af_rjson_handler_shared_ptr_t, is_shared_ptr<TargetT>::value || std::is_pointer<TargetT>::value)

template<typename target_t>
struct af_rjson_handler_sequence_t : public af_rjson_handler_value_t<target_t> {
	
	using char_t = af_json_char_t::char_t;
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
 };

template<typename target_t, typename writer_t>
struct af_rjson_writer_sequence_t : public af_rjson_writer_value_t<target_, writer_t> {

	using base_t = af_rjson_writer_value_t<target_t, writer_t>;
	using default_value_handler_t = default_value_handler<target_t>;
	using contained_t = target_t::value_type;
	using contained_default_value_handler_t = default_value_handler<contained_t>;

	contained_default_value_handler_t const* _contained_default_f;

	af_rjson_writer_sequence_t(
		target_t const& target_,
		default_value_handler_t const* default_f_,
		contained_default_value_handler_t const* contained_default_f_ = nullptr
	) : base_t(target_, default_f_),
		_contained_default_f(contained_default_f_){}

	void write(writer_t& writer_) const {
		if(default_write(writer_)) return;
		writer_->StartArray();
		for (auto const& t : _target)
			delegate_write(writer_, t, _contained_default_f);
		writer_->EndArray();
	}
};

CreateHandlers(af_rjson_handler_sequence_t, is_sequence<TargetT>::value);

 template<typename target_t>
 struct af_rjson_handler_setish_t : public af_rjson_handler_sequence_t<target_t> {

	 using char_t = af_json_char_t::char_t;
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
 template<typename target_t, typename writer_t>
 using af_rjson_writer_settish_t = af_rjson_writer_sequence_t<target_, writer_t>;

 CreateHandlers(af_rjson_handler_setish_t, is_setish<TargetT>::value);

// general maps are stored as arrays of pair objects: { "key" : key_value, "value" : value_value }
// there is an optimisation for string maps
// first we need an intermediate handler for these objects
namespace {
	AF_RJSON_TAG(tag_key, "key");
	AF_RJSON_TAG(tag_value, "value");
};
// reader and writer are different for these pairs
template< typename target_t>
struct af_rjson_handler_pair_t : public af_rjson_handler_value_t<target_t> {
	
	using char_t = af_json_char_t::char_t;
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
		_key_handler->reset(&key());
		_value_handler->reset(&value());
		if (equal_tag(str, length, tag_key))
			_current_handler = _key_handler;
		else if (equal_tag(str, length, tag_value) == 0)
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

};

template< typename target_t, typename writer_t>
struct af_rjson_writer_contained_t : public af_rjson_writer_value_t<target_t, writer_t> {

	using base_t = af_rjson_writer_value_t<target_t, writer_t>;
	using default_value_handler_t = default_value_handler<target_t>;
	using contained_t = target_t::second_type;
	using contained_default_value_handler_t = default_value_handler<contained_t>;

	contained_default_value_handler_t const* _contained_default_f;

	af_rjson_writer_contained_t(
		target_t const& target_,
		default_value_handler_t const* default_f_,
		contained_default_value_handler_t const* contained_default_f_ = nullptr
	) : base_t(target_, default_f_),
		_contained_default_f(contained_default_f_) {}

	void write(WriterT& writer_) const {
		if (default_write(writer_)) return;
		writer_->StartObject();
		writer_->Key(tag_key);
		delegate_write(writer_, _target.first, nullptr);
		writer_->Key(tag_value);
		delegate_write(writer_, _target.second, _contained_default_f);
		writer_->EndObject();
	}
};

template<typename target_t>
struct af_rjson_handler_mappish_t : public af_rjson_handler_value_t<target_t> {
	// TODO: this is so similar to both sequence and settish
	// perhaps all they need is different construction?
	using char_t = af_json_char_t::char_t;
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

template<typename target_t, typename writer_t>
using af_rjson_writer_mappish_t = af_rjson_writer_sequence_t<target_, writer_t>;

CreateHandlers(af_rjson_handler_mappish_t, (is_mappish<TargetT>::value && !(is_string<TargetT>::value)))

//opitimised format for maps with string keys
template<typename target_t>
struct af_rjson_handler_string_mappish_t : public af_rjson_handler_value_t<target_t> {

	using char_t = af_json_char_t::char_t;
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
			string_assign(&key, str, length);
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
};

template<typename target_t, typename writer_t>
struct af_rjson_writer_string_mappish_t : public af_rjson_writer_value_t<target_, writer_t> {
	
	using base_t = af_rjson_writer_value_t<target_t, writer_t>;
	using default_value_handler_t = default_value_handler<target_t>;
	using value_t = target_t::mapped_type;
	using value_default_value_handler_t = default_value_handler<value_t>;

	value_default_value_handler_t const* _value_default_f;

	af_rjson_writer_string_mappish_t(
		target_t const& target_,
		default_value_handler_t const* default_f_,
		value_default_value_handler_t const* value_default_f_ = nullptr
	) : base_t(target_, default_f_),
		_value_default_f(value_default_f_){}

	void write(writer_t& writer_) const {
		if (default_write(writer_)) return;
		writer_->StartObject();
		for (auto const& t : _target) {
			writer_->Key(t.first);
			delegate_write(writer_, t.second, _value_default_f);
		}
		writer_->EndObject();
	}
};

template< typename TargetT, typename WriterT, TypeFilter((is_mappish<TargetT>::value && is_string<TargetT>::value)) >
af_rjson_handler_value_t<TargetT, WriterT>* create_handler(TargetT* target_, default_value_handler_t* default_ = nullptr) {
	if(af_rjson_configuration::optimised_string_maps())
		return new af_rjson_handler_string_mappish_t<TargetT, WriterT>(target_, default_);
	else
		return new af_rjson_handler_mappish_t<TargetT, WriterT>(target_, default_);
}

template< typename TargetT, typename WriterT, TypeFilter((is_mappish<TargetT>::value && is_string<TargetT>::value)) >
af_rjson_handler_value_t<TargetT, WriterT>* create_handler(TargetT* target_, TargetT const& default_value_) {
	if (af_rjson_configuration::optimised_string_maps())
		return new af_rjson_handler_string_mappish_t<TargetT, WriterT>(target_, default_value_);
	else
		return new af_rjson_handler_mappish_t<TargetT, WriterT>(target_, default_value_);
}



template<typename target_t>
struct af_rjson_handler_object_t : public af_rjson_handler_value_t<target_t> {

	using char_t = af_json_char_t::char_t;
	using base_t = af_rjson_handler_value_t<target_t>; 
	using default_value_handler_t = default_value_handler<target_t>;
	using key_t = std::basic_string<Ch>;
	using handler_t = af_rjson_handler;
	using handlers_t = std::vector<std::pair<key_t, handler_t*>>;
	// we use a vector here to preserve the order of elements
	handlers_t _handlers;

	handler_t* _current_handler;
	af_rjson_handler_object_t(
			target_t* target_,
			default_value_handler_t* default_ = nullptr,
			handlers_t const& handlers_ = {}) :
		base_t(target_, default_),
		_handlers(handlers_),
		_current_handler(nullptr){
	}
	af_rjson_handler_object_t(
			target_t* target_,
			target_t const& default_value_,
			handlers_t const& handlers_ = {}) :
		base_t(target_, default_value_),
		_handlers(handlers_),
		_current_handler(nullptr) {
	}

	inline handler_t* find_handler(const Ch* const key, size_t length) {
		for (auto& h : _handlers) {
			if (strncmp(key, h.first, length) == 0)
				return h.second;
		}
		return nullptr;
	}
	inline add_handler(key_t const& key_, handler_t* handler_) {
		AF_ASSERT(!key_.empty(), "Empty key in object serialization.");
		AF_ASSERT(handler_, "Null handler in object serialization.");
		AF_ASSERT(find_handler(key_) == nullptr, "Duplicate key in object serialization: %", key_);
		_handlers.push_back(std::make_pair(key_, handler_));
	}

	bool validate_all_loaded() const {
		for (auto const& h : _handlers) {
			if (!h.second->is_done())
				if( !af_rjson_configuration::terse() || !h.second->handles_terse_storage())
					return false;
		}
		return true;
	}

	void reset(target_t* target_) override { 
		base_t::reset(target_);
		_current_handler = nullptr;
		for (auto const& h : _handlers) {

		}
	}


	template<typename... ParamsT>
	inline bool delegate_f(bool (handler_t::* mf)(ParamsT ...), ParamsT... ps) {
		
		return (_current_handler->*mf)(ps...);
	}
	bool Null() override {
		if (_current_handler)
			return _current_handler->Null();
		return af_rjson_handler_value_t<TargetT, WriterT>::Null();
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
		if (_current_handler && !_current_handler->is_done())
			return _current_handler->StartObject();
		return true;
	}
	bool Key(const char_t* str, size_t length, bool copy) { 
		if (_current_handler && !_current_handler->is_done())
			return _current_handler->Key(std, length, copy);
		_current_key.assign(str);
	}
	bool EndObject(size_t memberCount) override {
		if (_current_handler && !_current_handler->is_done())
			return _current_handler->EndObject(memberCount);
		AF_ASSERT(validate_all_loaded(), "Not all object members are loaded.");
		done();
		return true; 
	}
	bool StartArray() override { return delegate_f(&handler_t::StartArray); }
	bool EndArray(size_t elementCount) override { return delegate_f(&handler_t::EndArray(elementCount); }

	void release() override {
		for (auto const& h : _handlers)
			h.second->release();
		base_t::release();
	}
};

template<typename target_t, typename writer_t>
struct af_rjson_writer_object_t : public af_rjson_writer_value_t<target_, writer_t> {

	using default_value_handler_t = default_value_handler<target_t>;

	af_rjson_writer_object_t(
		target_t const& target_,
		default_value_handler_t const* default_f_
	) : af_rjson_writer_value_t<target_, writer_t>(target_, default_f_) {}

	void write(writer_t& writer_) const {
		if (default_write(writer_)) return;

		writer_->StartObject();
		for (auto const& h : _target) {
			writer_->Key(h.first);
			delegate_write(writer_, t.second->get(), nullptr);
		}
		writer_->EndObject();
	}
};
