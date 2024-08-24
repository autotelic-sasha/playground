#pragma once
#include "autotelica_core/util/include/std_disambiguation.h"
#include "autotelica_core/util/include/asserts.h"
#include <string.h>
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


struct af_json_serializable {
	template<typename WriterT>
	virtual 
};

#ifndef _AF_JSON_TERSE
#define _AF_JSON_TERSE false
#define
#ifndef _AF_JSON_OPTIMISED_STRING_MAPS
#define _AF_JSON_OPTIMISED_STRING_MAPS false
#define


class af_json_configuration {
	bool _terse;
	bool _optimised_string_maps;

	static af_json_configuration& instance() {
		static af_json_configuration _instance;
		return _instance;
	}
	af_json_configuration() :
		_terse(_AF_JSON_TERSE),
		_optimised_string_maps(_AF_JSON_OPTIMISED_STRING_MAPS){}
public:
	static bool terse() { return instance()._terse; }
	static void set_terse(bool terse_ = true) { instance()._terse = terse_; }

	static bool optimised_string_maps() { return instance()._optimised_string_maps; }
	static void set_optimised_string_maps(bool optimised_string_maps_ = true) { instance()._optimised_string_maps = optimised_string_maps_; }

}

template<typename target_t>
default_value_handler {
	target_t const _default_value;
	bool _target_initialised;
	inline target_t& ref(void* target_) {
		return *(static_cast<target_t*>(target_));
	}
	inline bool set_target(void* target_) {
		if (!_target_initialised) {
			 ref(target_) = _default_value;
			_target_initialised = true;
		}
		return true;
	}
public:
	default_value_handler(target_t const& default_value_) :
		_default_value(default_value_),
		_target_initialised(false){
	}
	inline bool initialise(void* target_) const {
		return (af_json_configuration::terse() && 
				set_target(target_));
	}
	inline bool null(void* target_) const {
		return set_target(target_);
	}
	template<typename writer_t>
	bool write(target_t const& target_, writer_t& writer_) const {
		// in terse mode, when _target is equal to default, we just don't write it
		return (
			af_json_configuration::terse() &&
			target_ == _default_value));
	}
};


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
	virtual ~af_rjson_handler_t() {}
};

struct af_rjson_handler_t {
	
	virtual ~af_rjson_handler_t(){}
	virtual bool is_done() const = 0; // signal to the controller that we are done loading this value
	virtual bool has_default() const = 0;
	virtual void reset_state() = 0;
	// reader part
	virtual bool Null() { AF_ERROR("Null was not expected."); return false; }
	virtual bool Bool(bool b) { AF_ERROR("Boolean value was not expected."); return false;}
	virtual bool Int(int i) { AF_ERROR("Int value was not expected.");  return false; }
	virtual bool Uint(unsigned i) { AF_ERROR("Uint value was not expected.");  return false; }
	virtual bool Int64(int64_t i) { AF_ERROR("Int64 value was not expected."); return false; }
	virtual bool Uint64(uint64_t i) { AF_ERROR("Uint64 value was not expected."); return false;}
	virtual bool Double(double d) { AF_ERROR("Double value was not expected.");  return false; }
	virtual bool RawNumber(const Ch* str, SizeType length, bool copy) { AF_ERROR("RawNumber value was not expected."); return false; }
	virtual bool String(const Ch* str, SizeType length, bool copy) { AF_ERROR("String value was not expected."); return false; }
	virtual bool StartObject() { AF_ERROR("Object start was not expected."); return false; }
	virtual bool Key(const Ch* str, SizeType length, bool copy) { AF_ERROR("Object key was not expected."); return false; }
	virtual bool EndObject(SizeType memberCount) { AF_ERROR("Object end was not expected."); return false; }
	virtual bool StartArray() { AF_ERROR("Array start was not expected."); return false; }
	virtual bool EndArray(SizeType elementCount) { AF_ERROR("Array end was not expected."); return false; }

	virtual void release() { delete this; }
};

template<typename target_t>
struct af_rjson_handler_value_t : public af_rjson_handler_t {

	using default_value_handler_t = default_value_handler<target_t>;
	
	target_t* _target;
	default_value_handler_t* _default_h;
	bool _done;

	af_rjson_handler_value_t(target_t* target_, default_value_handler_t* default_ = nullptr) :
		_target(target_), 
		_default_h(default_),
		_done(false){}

	af_rjson_handler_value_t(target_t* target_, target_t const& default_value_) :
			_target(target_),
			_default_h(new default_value_handler<target_t>(default_value_),
			_done(false) {
		AF_ASSERT(_target, "Target is not initialised.");
		_default_h->initialise(_target);
	}

	inline void done() { _done = true; }
	bool is_done() const override { return _done; }
	bool has_default() const override { return _default_h != nullptr; }
	
	virtual void reset(target_t* target_) { _target = target_; _done = false; }
	void reset_state() override { _done = false; }

	inline bool is_set() const { return _target != nullptr; }

	inline bool set(target_t const& value_) { *_target = value_; done(); return true; }

	template<typename ConvertibleT>
	inline bool set(ConvertibleT const& value_) { *_target = target_t{ value_ }; done(); return true; }

	inline target_type const& get() const { return *_target; }
	inline target_type& get() { return *_target; }

	bool Null()  override {
		AF_ASSERT(_default_h, "Unexpected: null read when default value is not provided.");
		bool ret = _default_h->null(_target);
		done();
		return ret;
	}

	virtual void release() { 
		if (_target) {
			delete _target;
			_target = nullptr;
		}
		if (_default_h) {
			delete _default_h;
			_default_h = nullptr;
		}
		af_rjson_handler_t<WriterT>::release();
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
			_handler._default_h &&
			_handler._default_h->write(_target, writer_));
	}
	template<typename writer_target_t>
	inline void write(
		writer_t& writer_,
		writer_target_t const& target_,
		default_value_handler<writer_target_t> const* default_f_ = nullptr
	) {
		auto writer = create_writer<writer_t>(target_, default_f);
		key_writer.write(writer_);
	}

	inline void write(writer_t& writer_) const {
		if (default_write()) return;
		write(_target, writer_);
	}
};

template<typename target_t>
struct af_rjson_handler_integral_t : public af_rjson_handler_value_t<target_t> {

	using default_value_handler_t = default_value_handler<target_t>;

	af_rjson_handler_integral_t(target_t* target_, default_value_handler_t* default_ = nullptr) :
		af_rjson_handler_value_t<target_t>(target_, default_) {}
	
	af_rjson_handler_integral_t(target_t* target_, target_t const& default_value_) :
		af_rjson_handler_value_t<target_t>(target_, default_value_){}

	// reader part
	bool Bool(bool b)  override { return set(b); }
	bool Int(int i)  override { return set(i); }
	bool Uint(unsigned i)  override { return set(i); }
	bool Int64(int64_t i)  override { return set(i); }
	bool Uint64(uint64_t i) override { return set(i); }

};

CreateHandlers(af_rjson_handler_integral_t, std::is_integral<TargetT>::value )

template<typename target_t>
struct af_rjson_handler_floating_t : public af_rjson_handler_value_t<target_t>{

	using default_value_handler_t = default_value_handler<target_t>;

	af_rjson_handler_floating_t(target_t* target_, default_value_handler_t* default_ = nullptr) :
		af_rjson_handler_value_t<target_t>(target_, default_) {}

	af_rjson_handler_floating_t(target_t* target_, target_t const& default_value_) :
		af_rjson_handler_value_t<target_t>(target_, default_value_) {}

	// reader part
	bool Double(double d) override { return set(d); }
	bool Int(int i)  override { return set(i); }
	bool Uint(unsigned i)  override { return set(i); }
	bool Int64(int64_t i)  override { return set(i); }
	bool Uint64(uint64_t i) override { return set(i); }

};

CreateHandlers(af_rjson_handler_integral_t, std::is_floating_point<TargetT>::value)

template<typename target_t>
struct af_rjson_handler_string_t : public af_rjson_handler_value_t<target_t> {

	using default_value_handler_t = default_value_handler<target_t>;

	af_rjson_handler_string_t(target_t* target_, default_value_handler_t* default_ = nullptr) :
		af_rjson_handler_value_t<target_t>(target_, default_) {}

	af_rjson_handler_string_t(target_t* target_, target_t const& default_value_) :
		af_rjson_handler_value_t<target_t>(target_, default_value_) {}

	// reader part
	bool String(const Ch* str, SizeType length, bool copy) override {
		_target.assign(str);
		done();
		return true;
	}

};

CreateHandlers(af_rjson_handler_integral_t, is_string<TargetT>::value)

template<typename target_t>
struct af_rjson_handler_shared_ptr_t : public af_rjson_handler_value_t<target_t> {

	using default_value_handler_t = default_value_handler<target_t>;
	using contained_t = target_t::element_type;
	using value_handler_t = af_rjson_handler_value_t<contained_t>;

	value_handler_t* _value_handler;

	af_rjson_handler_shared_ptr_t(
			target_t* target_, 
			default_value_handler_t* default_ = nullptr) :
		af_rjson_handler_value_t<target_t>(target_, default_),
		_value_handler(nullptr){
	}
	af_rjson_handler_shared_ptr_t(
			target_t* target_,
			target_t const& default_value_) :
		af_rjson_handler_value_t<target_t>(target_, default_value_),
		_value_handler(nullptr){
	}

	bool is_done() const override { return _done || (_value_handler && _value_handler->is_done()); } // _done is set when null is read
	
	void reset(target_t* target_) override { 
		_target = target_; 
		_value_handler = create_handler(*(*_target));
		_done = false;
	}
	void reset_state() override {
		_value_handler = create_handler(*(*_target));
		_done = false;
	}

	inline void set_current_item() {
		if (!(*_target))
			*_target = target_t(new contained_t());
		if(!_value_handler)
			_value_handler = create_handler(*(*_target));
	}

	// reader part
	template<typename... ParamsT>
	bool delegate_f(bool (value_handler_t::* mf)(ParamsT ...), ParamsT... ps) {
		set_current_item();
		return (_value_handler->*mf)(ps...);
	}

	bool Null()  override {
		// null pointers can be read as null
		// it's terribly convenient
		if (_value_handler) 
			return _value_handler->Null(); 
		done(); // we are gonna handle this one way or another
		if (_default_h) {
			if (!(*_target))
				*_target = target_t(new contained_t());
			return _default_h->null(_target);
		}
		_target = nullptr;
		return true;
	}
	bool Bool(bool b) override { return delegate_f(&value_handler_t::Bool, b); }
	bool Int(int i) override { return delegate_f(&value_handler_t::Int, i); }
	bool Uint(unsigned i) override { return delegate_f(&value_handler_t::Uint, i); }
	bool Int64(int64_t i) override { return delegate_f(&value_handler_t::Int64, i); }
	bool Uint64(uint64_t i) override { return delegate_f(&value_handler_t::Uint64, i); }
	bool Double(double d) override { return delegate_f(&value_handler_t::Double, d); }
	bool RawNumber(const Ch* str, SizeType length, bool copy) override { return delegate_f(&value_handler_t::RawNumber, str, length, copy); }
	bool String(const Ch* str, SizeType length, bool copy) override { return delegate_f(&value_handler_t::String, str, length, copy); }
	bool StartObject()  override { return _value_handler->StartObject(); }
	bool Key(const Ch* str, SizeType length, bool copy) override { return _value_handler->Key(str, length, copy); }
	bool EndObject(SizeType memberCount)  override { return _value_handler->EndObject(memberCount); }
	bool StartArray() override { return _value_handler->StartArray(str, length, copy); }
	bool EndArray(SizeType elementCount)  override { return _value_handler->EndArray(elementCount); }

	void release() override {
		if (_value_handler) {
			_value_handler->release();
			_value_handler = nullptr;
		}
		af_rjson_handler_value_t<TargetT, WriterT>::release();
	}
};

template<typename target_t, typename writer_t>
struct af_rjson_writer_shared_ptr_t : public af_rjson_writer_value_t<target_t, writer_t> {

	using default_value_handler_t = default_value_handler<target_t>;

	af_rjson_writer_shared_ptr_t(
		target_t const& target_,
		default_value_handler_t const* default_f_
	) : af_rjson_writer_value_t<target_, writer_t>(target_, default_f_) {}

	void write(writer_t& writer_) const {
		if (!_target) // null pointers are always written as null
			writer.Null();
		else if (default_write(writer_t & writer_))
			return;
		else 
			write(writer_, *_target, nullptr);
	}
};

CreateHandlers(af_rjson_handler_shared_ptr_t, is_shared_ptr<TargetT>::value || std::is_pointer<TargetT>::value)

template<typename target_t>
struct af_rjson_handler_sequence_t : public af_rjson_handler_value_t<target_t> {

	using default_value_handler_t = default_value_handler<target_t>;
	using contained_t = target_t::value_type;
	using value_handler_t = af_rjson_handler_value_t<contained_t>;

	 
	value_handler_t* _value_handler;

	af_rjson_handler_sequence_t(
			target_t* target_, 
			default_value_handler_t* default_ = nullptr) :
		af_rjson_handler_value_t<target_t>(target_, default_),
		_value_handler(create_handler<contained_t, writer_t>(nullptr)){
	}
	af_rjson_handler_sequence_t(
			target_t* target_,
			target_t const& default_value_) :
		af_rjson_handler_value_t<target_t>(target_, default_value_),
		_value_handler(create_handler<contained_t, writer_t>(nullptr)){
	}

	inline void set_current_item() {
		if (!_value_handler->is_set() || _value_handler->is_done()) {
			_target->emplace_back({});
			contained_t& ret(_target->back());
			_value_handler->reset(&ret);
		}
	}

	 // reader part
	template<typename... ParamsT>
	inline bool delegate_f(bool (value_handler_t::* mf)(ParamsT ...), ParamsT... ps) {
		set_current_item();
		return (_value_handler->*mf)(ps...);
	}
	bool Null() override {
		if (!_value_handler->is_set())
			return af_rjson_handler_value_t<TargetT, WriterT>::Null();
		return delegate_f(&value_handler_t::Null);
	}
	bool Bool(bool b) override { return delegate_f(&value_handler_t::Bool, b); }
	bool Int(int i) override { return delegate_f(&value_handler_t::Int, i); }
	bool Uint(unsigned i) override { return delegate_f(&value_handler_t::Uint, i); }
	bool Int64(int64_t i) override { return delegate_f(&value_handler_t::Int64, i); }
	bool Uint64(uint64_t i) override { return delegate_f(&value_handler_t::Uint64, i); }
	bool Double(double d) override { return delegate_f(&value_handler_t::Double, d); }
	bool RawNumber(const Ch* str, SizeType length, bool copy) override { return delegate_f(&value_handler_t::RawNumber, str, length, copy); }
	bool String(const Ch* str, SizeType length, bool copy) override { return delegate_f(&value_handler_t::String, str, length, copy); }
	bool StartObject() override { return delegate_f(&value_handler_t::StartObject); }
	bool Key(const Ch* str, SizeType length, bool copy) { return delegate_f(&value_handler_t::String, str, length, copy); }
	bool EndObject(SizeType memberCount) override { return delegate_f(&value_handler_t::EndObject, memberCount); }
	bool StartArray() override { 
		if(_value_handler->is_set())
			return _value_handler->StartArray();
		return true;
	}
	bool EndArray(SizeType elementCount) override { 
		if(!_value_handler->is_set() || _value_handler->is_done()){
			done();
			return true;
		}
		return _value_handler->EndArray(elementCount);
	}

	 void release() override {
		 if (_value_handler) {
			 _value_handler->release();
			 _value_handler = nullptr;
		 }
		 af_rjson_handler_value_t<TargetT, WriterT>::release();
	 }
 };

template<typename target_t, typename writer_t>
struct af_rjson_writer_sequence_t : public af_rjson_writer_value_t<target_, writer_t> {

	using default_value_handler_t = default_value_handler<target_t>;
	
	af_rjson_writer_sequence_t(
		target_t const& target_,
		default_value_handler_t const* default_f_
	) : af_rjson_writer_value_t<target_, writer_t>(target_, default_f_){}

	void write(writer_t& writer_) const {
		if(default_write()) return;
		writer_->StartArray();
		for (auto const& t : _target)
			write(writer_, t, nullptr);
		writer_->EndArray();
	}
};

CreateHandlers(af_rjson_handler_sequence_t, is_sequence<TargetT>::value);

 template<typename target_t, typename writer_t>
 struct af_rjson_handler_setish_t : public af_rjson_handler_sequence_t<writer_t> {

	 using default_value_handler_t = default_value_handler<target_t, writer_t>;
	 using contained_t = target_t::value_type;
	 using value_handler_t = af_rjson_handler_value_t<contained_t, writer_t>;

	 contained_t _current_value;

	 af_rjson_handler_setish_t(target_t* target_, default_value_handler_t* default_ = nullptr) :
		 af_rjson_handler_sequence_t(target_, default_){

	 }
	 af_rjson_handler_setish_t(target_t* target_, target_t const& default_value_) :
		 af_rjson_handler_sequence_t(target_) {
	 }

	 inline void set_current_item() {
		 if (!_value_handler->is_set() || _value_handler->is_done()) {
			 _current_value = contained_t();
			 _value_handler->reset(&_current_value);
		 }
	 }
	 inline bool store(bool b) {
		 if (_value_handler->is_done()) 
			 _target->insert(_current_value);
		 return b;
	 }

	 // reader part
	 template<typename... ParamsT>
	 inline bool delegate_f(bool (value_handler_t::*mf)(ParamsT ...), ParamsT... ps) {
		 set_current_item();
		 return store((_value_handler->*mf)(ps...));
	 }
	 bool Null() override {
		 if (!_value_handler->is_set())
			 return af_rjson_handler_value_t<TargetT, WriterT>::Null();
		 return delegate_f(&value_handler_t::Null);
	 }
	 bool Bool(bool b) override { return delegate_f(&value_handler_t::Bool, b); }
	 bool Int(int i) override { return delegate_f(&value_handler_t::Int, i); }
	 bool Uint(unsigned i) override { return delegate_f(&value_handler_t::Uint, i); }
	 bool Int64(int64_t i) override { return delegate_f(&value_handler_t::Int64, i); }
	 bool Uint64(uint64_t i) override { return delegate_f(&value_handler_t::Uint64, i); }
	 bool Double(double d) override { return delegate_f(&value_handler_t::Double, d); }
	 bool RawNumber(const Ch* str, SizeType length, bool copy) override { return delegate_f(&value_handler_t::RawNumber, str, length, copy); }
	 bool String(const Ch* str, SizeType length, bool copy) override { return delegate_f(&value_handler_t::String, str, length, copy); }
	 bool StartObject() override { return delegate_f(&value_handler_t::StartObject); }
	 bool Key(const Ch* str, SizeType length, bool copy) { return delegate_f(&value_handler_t::String, str, length, copy); }
	 bool EndObject(SizeType memberCount) override { return delegate_f(&value_handler_t::EndObject, memberCount); }

 };

 CreateHandlers(af_rjson_handler_setish_t, is_setish<TargetT>::value);

// general maps are stored as arrays of pair objects: { "key" : key_value, "value" : value_value }
// there is an optimisation for string maps
// first we need an intermediate handler for these objects
namespace {
	static const char* const tag_key = "key";
	static const char* const tag_value = "value";
};
// reader and writer are different for these pairs
template< typename key_t, typename value_t>
struct af_rjson_handler_pair_t {
	using key_handler_t = af_rjson_handler_value_t<key_t>;
	using value_handler_t = af_rjson_handler_value_t<value_t>;
	using current_handler_t = af_rjson_handler_t;

	key_t _key;
	value_t _value;
	key_handler_t* _key_handler;
	value_handler_t* _value_handler;
	current_handler_t* _current_handler;

	af_rjson_handler_pair_t() :
		_key_handler(create_handler<key_t>(&_key)),
		_value_handler(create_handler<value_t>(&_value)),
		_current_handler(nullptr) {
	}

	key_t const& key() const { return _key; }
	value_t const& value() const { return _value; }

	bool is_done() const {
		return _key_handler->is_done() && _value_handler->is_done();
	}

	inline void reset() {
		_current_handler = nullptr;
	}

	template<typename... ParamsT>
	bool delegate_f(bool (current_handler_t::* mf)(ParamsT ...), ParamsT... ps) {
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
	bool Bool(bool b) {
		AF_ASSERT(_current_handler, "Unexpected value while reading a map.");
		return delegate_f(&current_handler_t::Bool, b);
	}
	bool Int(int i) {
		AF_ASSERT(_current_handler, "Unexpected value while reading a map.");
		return delegate_f(&current_handler_t::Int, i);
	}
	bool Uint(unsigned i) {
		AF_ASSERT(_current_handler, "Unexpected value while reading a map.");
		return delegate_f(&current_handler_t::Uint, i);
	}
	bool Int64(int64_t i) {
		AF_ASSERT(_current_handler, "Unexpected value while reading a map.");
		return delegate_f(&current_handler_t::Int64, i);
	}
	bool Uint64(uint64_t i) {
		AF_ASSERT(_current_handler, "Unexpected value while reading a map.");
		return delegate_f(&current_handler_t::Uint64, i);
	}
	bool Double(double d) {
		AF_ASSERT(_current_handler, "Unexpected value while reading a map.");
		return delegate_f(&current_handler_t::Double, i);
	}
	bool String(const Ch* str, SizeType length, bool copy) {
		AF_ASSERT(_current_handler, "Unexpected value while reading a map.");
		return delegate_f(&current_handler_t::String, str, length, copy);
	}

	bool StartObject() {
		if (_current_handler)
			return delegate_f(&current_handler_t::StartObject);
		return true;
	}
	bool Key(const Ch* str, SizeType length, bool copy) {
		if (_current_handler)
			return delegate_f(&current_handler_t::Key, str, length, copy);
		_key_handler->reset(&_key);
		_value_handler->reset(&_value);
		if (strncmp(str, tag_key) == 0)
			_current_handler = _key_handler;
		else if (strncmp(str, tag_value) == 0)
			_current_handler = _value_handler;
		else
			AF_ERROR("Unknown value: %", str);
		return true;
	}
	bool EndObject(SizeType memberCount) {
		if (_current_handler)
			return delegate_f(&current_handler_t::EndObject, memberCount);
		_current_handler = nullptr;
		return true;
	}

	bool StartArray() {
		AF_ASSERT(_current_handler, "Unexpected value while reading a map.");
		return delegate_f(&current_handler_t::StartArray);
	}
	bool EndArray(SizeType elementCount) {
		AF_ASSERT(_current_handler, "Unexpected value while reading a map.");
		return delegate_f(&current_handler_t::EndArray, elementCount);
	}
};

template< typename target_t, typename writer_t>
struct af_rjson_writer_pair_t : public af_rjson_writer_value_t<target_t, writer_t> {

	using default_value_handler_t = default_value_handler<target_t>;

	af_rjson_writer_pair_t(
		target_t const& target_,
		default_value_handler_t const* default_f_
	) : af_rjson_writer_value_t<target_, writer_t>(target_, default_f_) {}

	void write(WriterT& writer_) const {
		if (default_write()) return;
		writer_->StartObject();
		writer_->Key(tag_key);
		write(writer_, _target.first, nullptr);
		writer_->Key(tag_value);
		write(writer_, _target.second, nullptr);
		writer_->EndObject();
	}
};

template<typename target_t>
struct af_rjson_handler_mappish_t : public af_rjson_handler_value_t<target_t> {

	using default_value_handler_t = default_value_handler<target_t>;
	using key_t = target_t::key_type;
	using value_t = target_t::mapped_type;
	using pair_reader_t = af_json_pairs::af_rjson_handler_pair_t<key_t, value_t>;

	pair_reader_t _pair_reader;

	bool _this_array_started;

	af_rjson_handler_mappish_t(target_t* target_, default_value_handler_t* default_ = nullptr) :
		af_rjson_handler_value_t<target_t>(target_, default_),
		_this_array_started(false) {
	}
	af_rjson_handler_mappish_t(target_t* target_,target_t const& default_value_) :
		af_rjson_handler_value_t<target_t>(target_, default_value_),
		_this_array_started(false) {
	}

	template<typename... ParamsT>
	inline bool delegate_f(bool (af_rjson_handler_pair_t::*mf)(ParamsT ...), ParamsT... ps) {
		bool ret = (_pair_reader.*mf)(ps...);
		if (_pair_reader.is_done()) {
			_target[_pair_reader.key()] = _pair_reader.value();
			_pair_reader.reset();
		}
		return ret;
	}
	bool Null() {
		if (!_this_array_started)
			return af_rjson_handler_value_t<target_t>::Null();
		return delegate_f(&_pair_reader::Null);
	}
	bool Bool(bool b) override { return delegate_f(&_pair_reader::Bool, b); }
	bool Int(int i) override { return delegate_f(&_pair_reader::Int, i); }
	bool Uint(unsigned i) override { return delegate_f(&_pair_reader::Uint, i); }
	bool Int64(int64_t i) override { return delegate_f(&_pair_reader::Int64, i); }
	bool Uint64(uint64_t i) override { return delegate_f(&_pair_reader::Uint64, i); }
	bool Double(double d) override { return delegate_f(&_pair_reader::Double, d); }
	bool RawNumber(const Ch* str, SizeType length, bool copy) override { return delegate_f(&_pair_reader::RawNumber, str, length, copy); }
	bool String(const Ch* str, SizeType length, bool copy) override { return delegate_f(&_pair_reader::String, str, length, copy); }
	bool StartObject() override { return delegate_f(&_pair_reader::StartObject); }
	bool Key(const Ch* str, SizeType length, bool copy) { return delegate_f(&_pair_reader::Key, str, length, copy); }
	bool EndObject(SizeType memberCount) override {  }
	bool StartArray() override {
		if (!_this_array_started) {
			_this_array_started = true;
			return true;
		}
		return delegate_f(&_pair_reader::StartArray);
	}
	bool EndArray(SizeType elementCount) override {
		if (_pair_reader.is_done()) {
			done();
			return true;
		}
		else
			return delegate_f(&_pair_reader::EndArray, memberCount);
	}

};

template<typename target_t, typename writer_t>
using af_rjson_writer_mappish_t = af_rjson_writer_value_t<target_, writer_t>;

CreateHandlers(af_rjson_handler_mappish_t, (is_mappish<TargetT>::value && !(is_string<TargetT>::value)))

//opitimised format for maps with string keys
template<typename target_t>
struct af_rjson_handler_string_mappish_t : public af_rjson_handler_value_t<target_t> {

	using default_value_handler_t = default_value_handler<target_t>;
	using key_t = target_t::key_type; // this is some string type
	using value_t = target_t::mapped_type;
	using value_handler_t = af_rjson_handler_value_t<value_t>;

	key_t _current_key;
	value_t _current_value;

	value_handler_t* _value_handler;

	bool _this_object_started;

	af_rjson_handler_mappish_t(
			target_t* target_, 
			default_value_handler_t* default_ = nullptr) :
		af_rjson_handler_value_t<target_t>(target_, default_),
		_value_handler(create_handler<value_t>(nullptr)),
		_this_object_started(false) {
	}
	af_rjson_handler_mappish_t(
			target_t* target_,
			target_t const& default_value_) :
		af_rjson_handler_value_t<target_t>(target_, default_value_),
		_value_handler(create_handler<value_t>(nullptr)),
		_this_object_started(false) {
	}

	template<typename... ParamsT>
	inline bool delegate_f(bool (value_handler_t::* mf)(ParamsT ...), ParamsT... ps) {
		bool ret = (_value_handler.*mf)(ps...);
		if (_value_handler.is_done()) {
			_target[_current_key] = _current_value;
			_current_key = key_t();
			_current_value = value_t();
			_value_handler.reset(&current_value);
		}
		return ret;
	}
	bool Null() {
		if (_current_key.empty())
			return af_rjson_handler_value_t<target_t>::Null();
		return delegate_f(&_pair_reader::Null);
	}
	bool Bool(bool b) override { return delegate_f(&_pair_reader::Bool, b); }
	bool Int(int i) override { return delegate_f(&_pair_reader::Int, i); }
	bool Uint(unsigned i) override { return delegate_f(&_pair_reader::Uint, i); }
	bool Int64(int64_t i) override { return delegate_f(&_pair_reader::Int64, i); }
	bool Uint64(uint64_t i) override { return delegate_f(&_pair_reader::Uint64, i); }
	bool Double(double d) override { return delegate_f(&_pair_reader::Double, d); }
	bool RawNumber(const Ch* str, SizeType length, bool copy) override { return delegate_f(&_pair_reader::RawNumber, str, length, copy); }
	bool String(const Ch* str, SizeType length, bool copy) override { return delegate_f(&_pair_reader::String, str, length, copy); }
	bool StartObject() override { return delegate_f(&_pair_reader::StartObject); }
	bool Key(const Ch* str, SizeType length, bool copy) { return delegate_f(&_pair_reader::Key, str, length, copy); }
	bool EndObject(SizeType memberCount) override {  }
	bool StartArray() override {
		if (_current_key.empty())
			return true;
		return delegate_f(&_pair_reader::StartArray);
	}
	bool EndArray(SizeType elementCount) override {
		if (_pair_reader.is_done()) {
			done();
			return true;
		}
		else
			return delegate_f(&_pair_reader::EndArray, memberCount);
	}

	void release() override {
		if (_value_handler) {
			_value_handler->release();
			_value_handler = nullptr;
		}
		af_rjson_handler_value_t<TargetT, WriterT>::release();
	}
};

template<typename target_t, typename writer_t>
struct af_rjson_writer_string_mappish_t : public af_rjson_writer_value_t<target_, writer_t> {

	using default_value_handler_t = default_value_handler<target_t>;

	af_rjson_writer_string_mappish_t(
		target_t const& target_,
		default_value_handler_t const* default_f_
	) : af_rjson_writer_value_t<target_, writer_t>(target_, default_f_) {}

	void write(writer_t& writer_) const {
		if (default_write()) return;
		writer_->StartObject();
		for (auto const& t : _target) {
			writer_->Key(t.first);
			write(writer_, t.second, nullptr);
		}
		writer_->EndObject();
	}
};

template< typename TargetT, typename WriterT, TypeFilter((is_mappish<TargetT>::value && is_string<TargetT>::value)) >
af_rjson_handler_value_t<TargetT, WriterT>* create_handler(TargetT* target_, default_value_handler_t* default_ = nullptr) {
	if(af_json_configuration::optimised_string_maps())
		return new af_rjson_handler_string_mappish_t<TargetT, WriterT>(target_, default_);
	else
		return new af_rjson_handler_mappish_t<TargetT, WriterT>(target_, default_);
}

template< typename TargetT, typename WriterT, TypeFilter((is_mappish<TargetT>::value && is_string<TargetT>::value)) >
af_rjson_handler_value_t<TargetT, WriterT>* create_handler(TargetT* target_, TargetT const& default_value_) {
	if (af_json_configuration::optimised_string_maps())
		return new af_rjson_handler_string_mappish_t<TargetT, WriterT>(target_, default_value_);
	else
		return new af_rjson_handler_mappish_t<TargetT, WriterT>(target_, default_value_);
}


template<typename target_t>
struct af_rjson_handler_object_t : public af_rjson_handler_value_t<target_t> {

	using default_value_handler_t = default_value_handler<target_t>;
	using handler_p_t = af_rjson_handler*;
	using handlers_t = std::vector<std::pair<std::string, handler_p_t>>;
	// we use a vector here to preserve the order of elements
	handlers_t _handlers;
	std::string _current_key;
	handler_p_t _current_handler;

	af_rjson_handler_object_t(
			target_t* target_,
			default_value_handler_t* default_ = nullptr,
			handlers_t const& handlers_ = {}) :
		af_rjson_handler_value_t<target_t>(target_, default_),
		_handlers(handlers_),
		_current_handler(nullptr){
	}
	af_rjson_handler_object_t(
			target_t* target_,
			target_t const& default_value_,
			handlers_t const& handlers_ = {}) :
		af_rjson_handler_value_t<target_t>(target_, default_value_),
		_handlers(handlers_),
		_current_handler(nullptr) {
	}

	inline add_handler(std::string const& key_, handler_p_t handler_) {
		AF_ASSERT(!key_.empty(), "Empty key in object serialization.");
		AF_ASSERT(handler(key_) == nullptr, "Duplicate key in object serialization: %", key_);
		_handlers[key_] = handler_;
	}

	inline handler_p_t handler(const Ch* const key, SizeType length) {
		for (auto& h : _handlers) {
			if (strncmp(key, h.first, length) == 0)
				return h.second;
		}
		return nullptr;
	}

	bool validate_all_loaded() const {
		for (auto const& h : _handlers) {
			if (!h.second->is_done())
				if( !af_json_configuration::terse() || !h.second->has_default())
					return false;
		}
		return true;
	}

	void reset(target_t* target_) override { 
		_target = target_; 
		_done = false; 
		_current_key = "";
		_current_handler = nullptr;
		for (auto const& h : _handlers)
			h.second->reset_state();
	}
	void reset_state() override {
		_done = false;
		_current_key = "";
		_current_handler = nullptr;
		for (auto const& h : _handlers)
			h.second->reset_state();
	}

	inline void set_current_handler() {
		if (_current_handler && !_current_handler->is_done())
			return;
		AF_ASSERT(!_current_key.empty(), "Attempting to read object data when key is not set.");
		_current_handler = find_handler(_current_key);
		AF_ASSERT(_current_handler, "No handler specified for key %.", _current_key);
	}
	template<typename... ParamsT>
	inline bool delegate_f(bool (handler_p_t::* mf)(ParamsT ...), ParamsT... ps) {
		set_current_handler();
		return (_current_handler->*mf)(ps...);
	}
	bool Null() override {
		if (_current_handler)
			return _current_handler->Null();
		return af_rjson_handler_value_t<TargetT, WriterT>::Null();
	}
	bool Bool(bool b) override { return delegate_f(&handler_p_t::Bool, b); }
	bool Int(int i) override { return delegate_f(&handler_p_t::Int, i); }
	bool Uint(unsigned i) override { return delegate_f(&handler_p_t::Uint, i); }
	bool Int64(int64_t i) override { return delegate_f(&handler_p_t::Int64, i); }
	bool Uint64(uint64_t i) override { return delegate_f(&handler_p_t::Uint64, i); }
	bool Double(double d) override { return delegate_f(&handler_p_t::Double, d); }
	bool RawNumber(const Ch* str, SizeType length, bool copy) override { return delegate_f(&handler_p_t::RawNumber, str, length, copy); }
	bool String(const Ch* str, SizeType length, bool copy) override { return delegate_f(&handler_p_t::String, str, length, copy); }
	bool StartObject() override { 
		if (_current_handler && !_current_handler->is_done())
			return _current_handler->StartObject();
		return true;
	}
	bool Key(const Ch* str, SizeType length, bool copy) { 
		if (_current_handler && !_current_handler->is_done())
			return _current_handler->Key(std, length, copy);
		_current_key.assign(str);
	}
	bool EndObject(SizeType memberCount) override {
		if (_current_handler && !_current_handler->is_done())
			return _current_handler->EndObject(memberCount);
		AF_ASSERT(validate_all_loaded(), "Not all object members are loaded.");
		done();
		return true; 
	}
	bool StartArray() override { return delegate_f(&handler_p_t::StartArray); }
	bool EndArray(SizeType elementCount) override { return delegate_f(&handler_p_t::EndArray(elementCount); }

	void write(writer_t& writer_) const override {
		writer_->StartObject();
		for (auto const& h : _handlers) {
			writer_->Key(h.first);
			h.second->write(writer);
		}
		writer_->EndObject();
	}
	void release() override {
		for (auto const& h : _handlers)
			h.second->release();
		af_rjson_handler_value_t<TargetT, WriterT>::release();
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
		if (default_write()) return;

		writer_->StartObject();
		for (auto const& h : _target) {
			writer_->Key(h.first);
			write(writer_, t.second->get(), nullptr);
		}
		writer_->EndObject();
	}
};
