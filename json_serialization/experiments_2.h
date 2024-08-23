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

template<typename TargetT, typename WriterT>
default_value_handler {
	TargetT const& _default_value;
	bool _target_initialised;
	inline TargetT& ref(void* target_) {
		return *(static_cast<TargetT*>(target_));
	}
	inline bool set_target(void* target_) {
		if (!_target_initialised) {
			 ref(target_) = _default_value;
			_target_initialised = true;
		}
		return true;
	}
public:
	default_value_handler(TargetT const& default_value_) :
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
	bool write(Target* target_, WriterT& writer_) const {
		// in terse mode, when _target is equal to default, we just don't write it
		return (af_json_configuration::terse() &&
			ref(target_) == _default_value));
	}
};


template<typename TargetT, typename WriterT>
void write(TargetT const& value, WriterT& writer);

template< typename WriterT >
inline void write(int const& value, WriterT& writer) { writer.Int(value); }

template< typename WriterT >
inline void write(size_t const& value, WriterT& writer) { writer.Uint64(static_cast<uint64_t>(value)); }

// TODO: more write functions

template<typename WriterT>
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

	// writer part 
	virtual void write(WriterT& writer_) const { AF_ERROR("Writer is not implemented."); }

	virtual void release() { delete this; }
};

template<typename TargetT, typename WriterT>
struct af_rjson_handler_value_t : public af_rjson_handler_t<WriterT> {

	using target_t = TargetT;
	using writer_t = WriterT;

	target_t* _target;
	default_value_handler* _default_h;
	bool _done;

	af_rjson_handler_value_t(target_t* target_) : 
		_target(target_), 
		_default_h(nullptr),
		_done(false){}

	af_rjson_handler_value_t(target_t* target_, target_t const& default_value_) :
			_target(target_),
			_default_h(new default_value_handler<TargetT, WriterT>(default_value_),
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
	void write(writer_t& writer_) const override {
		if (_default_h && _default_h->write(_target))
			return;
		write(get(), writer_);
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

template<typename TargetT, typename WriterT>
struct af_rjson_handler_integral_t : public af_rjson_handler_value_t<TargetT, WritterT> {

	using target_t = TargetT;
	using writer_t = WriterT;

	af_rjson_handler_integral_t(target_t* target_) :
		af_rjson_handler_value_t<TargetT, WriterT>(target_) {}
	
	af_rjson_handler_integral_t(target_t* target_, target_t const& default_value_) :
		af_rjson_handler_value_t<TargetT, WriterT>(target_, default_value_){}

	// reader part
	bool Bool(bool b)  override { return set(b); }
	bool Int(int i)  override { return set(i); }
	bool Uint(unsigned i)  override { return set(i); }
	bool Int64(int64_t i)  override { return set(i); }
	bool Uint64(uint64_t i) override { return set(i); }

};

// SFINAE based type filter
#define TypeFilter( condition ) std::enable_if_t<condition, bool> = true
#define CreateHandlers( HandlerType, Condition ) \
	template< typename TargetT, typename WriterT, TypeFilter( Condition ) >\
	af_rjson_handler_value_t<TargetT, WriterT>* create_handler(TargetT* target_) {\
		return new HandlerType<TargetT, WriterT>(target_);\
	}\
	template< typename TargetT, typename WriterT, TypeFilter( Condition ) >\
	af_rjson_handler_value_t<TargetT, WriterT>* create_handler(TargetT* target_, TargetT const& default_value_) {\
		return new HandlerType<TargetT, WriterT>(target_, default_value_);\
	}

CreateHandlers(af_rjson_handler_integral_t, std::is_integral<TargetT>::value )
//template< typename TargetT, typename WriterT, TypeFilter( std::is_integral<TargetT>::value ) >
//af_rjson_handler* create_handler(TargetT* target_) {
//	return new af_rjson_handler_integral_t<TargetT, WriterT>(target_);
//}
//template< typename TargetT, typename WriterT, TypeFilter(std::is_integral<TargetT>::value) >
//af_rjson_handler* create_handler(TargetT* target_, target_t const& default_value_) {
//	return new af_rjson_handler_integral_t<TargetT, WriterT>(target_, default_value_);
//}

template<typename TargetT, typename WriterT>
struct af_rjson_handler_floating_t : public af_rjson_handler_value_t<TargetT, WritterT>{

	using target_t = TargetT;
	using writer_t = WriterT;

	af_rjson_handler_floating_t(target_t* target_) :
		af_rjson_handler_value_t<TargetT, WriterT>(target_) {}

	af_rjson_handler_floating_t(target_t* target_, target_t const& default_value_) :
		af_rjson_handler_value_t<TargetT, WriterT>(target_, default_value_) {}

	// reader part
	bool Double(double d) override { return set(d); }
	bool Int(int i)  override { return set(i); }
	bool Uint(unsigned i)  override { return set(i); }
	bool Int64(int64_t i)  override { return set(i); }
	bool Uint64(uint64_t i) override { return set(i); }

};

CreateHandlers(af_rjson_handler_integral_t, std::is_floating_point<TargetT>::value)

//template< typename TargetT, typename WriterT, TypeFilter(std::is_floating_point<TargetT>::value) >
//af_rjson_handler* create_handler(TargetT& target_, null_setter_f null_setter_ = no_default_setter_f) {
//	return new af_rjson_handler_floating_t<TargetT, WriterT>(target_, null_setter_);
//}

template<typename TargetT, typename WriterT>
struct af_rjson_handler_string_t : public af_rjson_handler_value_t<TargetT, WritterT> {

	using target_t = TargetT;
	using writer_t = WriterT;

	af_rjson_handler_string_t(target_t* target_) :
		af_rjson_handler_value_t<TargetT, WriterT>(target_) {}

	af_rjson_handler_string_t(target_t* target_, target_t const& default_value_) :
		af_rjson_handler_value_t<TargetT, WriterT>(target_, default_value_) {}

	// reader part
	bool String(const Ch* str, SizeType length, bool copy) override {
		_target.assign(str);
		done();
		return true;
	}

};

CreateHandlers(af_rjson_handler_integral_t, is_string<TargetT>::value)

//template< typename TargetT, typename WriterT, TypeFilter(is_string<TargetT>::value) >
//af_rjson_handler* create_handler(TargetT& target_, null_setter_f null_setter_ = no_default_setter_f) {
//	return new af_rjson_handler_string_t<TargetT, WriterT>(target_, null_setter_);
//}

template<typename TargetT, typename WriterT>
struct af_rjson_handler_shared_ptr_t : public af_rjson_handler_value_t<WriterT> {

	using target_t = TargetT;
	using writer_t = WriterT;
	using contained_t = target_t::element_type;
	using value_handler_t = af_rjson_handler_value_t<contained_t, writer_t>;


	value_handler_t* _value_handler;

	af_rjson_handler_shared_ptr_t(
			target_t* target_) :
		af_rjson_handler_value_t<TargetT, WriterT>(target_),
		_value_handler(nullptr){
	}
	af_rjson_handler_shared_ptr_t(
			target_t* target_,
			target_t const& default_value_) :
		af_rjson_handler_value_t<TargetT, WriterT>(target_, default_value_),
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

	// writer part 
	void write(writer_t& writer_) const override { 
		if (!(*_target)) // null pointers are always written as null
			writer.Null();
		else if (_default_h)
			_default_h->write(_target);
		else {
			if (!_value_handler)
				create_value_handler(_target);
			_value_handler->write(writer_);
		}
	}

	void release() override {
		if (_value_handler) {
			_value_handler->release();
			_value_handler = nullptr;
		}
		af_rjson_handler_value_t<TargetT, WriterT>::release();
	}
};

CreateHandlers(af_rjson_handler_shared_ptr_t, is_shared_ptr<TargetT>::value || std::is_pointer<TargetT>::value)

//template< typename TargetT, typename WriterT, TypeFilter(is_shared_ptr<TargetT>::value || std::is_pointer<TargetT>::value) >
//af_rjson_handler* create_handler(TargetT& target_, null_setter_f null_setter_ = no_default_setter_f) {
//	return new af_rjson_handler_shared_ptr_t<TargetT, WriterT>(target_, null_setter_);
//}

template<typename TargetT, typename WriterT>
struct af_rjson_handler_sequence_t : public af_rjson_handler_value_t<WriterT> {

	using target_t = TargetT;
	using writer_t = WriterT;
	using contained_t = target_t::value_type;
	using value_handler_t = af_rjson_handler_value_t<contained_t, writer_t>;

	 
	value_handler_t* _value_handler;
	contained_t const _empty_value;

	af_rjson_handler_sequence_t(
			target_t* target_, 
			contained_t const& empty_value_ = contained_t()) :
		af_rjson_handler_value_t<TargetT, WriterT>(target_),
		_value_handler(create_handler<contained_t, writer_t>(nullptr)),
		_empty_value(empty_value_){
	}
	af_rjson_handler_sequence_t(
			target_t* target_,
			target_t const& default_value_,
			contained_t const& empty_value_ = contained_t()) :
		af_rjson_handler_value_t<TargetT, WriterT>(target_, default_value_),
		_value_handler(create_handler<contained_t, writer_t>(nullptr)),
		_empty_value(empty_value_){
	}

	inline void set_current_item() {
		if (!_value_handler->is_set() || _value_handler->is_done()) {
			_target->push_back(_empty_value);
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

	 // writer part 
	 void write(writer_t& writer_) const override { 
		 writer_->StartArray();
		 for (auto const& t : *_target) {
			 _value_handler->reset(&t);
			 _value_handler->write(writer_);
		 }
		 writer_->EndArray();
	 }

	 void release() override {
		 if (_value_handler) {
			 _value_handler->release();
			 _value_handler = nullptr;
		 }
		 af_rjson_handler_value_t<TargetT, WriterT>::release();
	 }
 };

#define CreateArrayHandlers( HandlerType, Condition ) \
	template< typename TargetT, typename WriterT, TypeFilter( Condition ) >\
	af_rjson_handler_value_t<TargetT, WriterT>* create_handler(\
			TargetT* target_, \
			typename TargetT::value_type const& empty_value_ = contained_t()) {\
		return new HandlerType<TargetT, WriterT>(target_, empty_value_);\
	}\
	template< typename TargetT, typename WriterT, TypeFilter( Condition ) >\
	af_rjson_handler_value_t<TargetT, WriterT>* create_handler(\
			TargetT* target_, \
			TargetT const& default_value_,\
			typename TargetT::value_type const& empty_value_ = contained_t()) {\
		return new HandlerType<TargetT, WriterT>(target_, default_value_, empty_value_);\
	}

CreateArrayHandlers(af_rjson_handler_sequence_t, is_sequence<TargetT>::value);

 //template< typename TargetT, typename WriterT, TypeFilter(is_sequence<TargetT>::value) >
 //af_rjson_handler* create_handler(
	//	TargetT& target_, 
	//	null_setter_f null_setter_ = no_default_setter_f,
	//	contained_t const& empty_value_ = contained_t()	) {
	// return new af_rjson_handler_sequence_t<TargetT, WriterT>(target_, null_setter_, empty_value_);
 //}

 template<typename TargetT, typename WriterT>
 struct af_rjson_handler_setish_t : public af_rjson_handler_sequence_t<WriterT> {

	 using target_t = TargetT;
	 using writer_t = WriterT;
	 using contained_t = target_t::value_type;
	 using value_handler_t = af_rjson_handler_value_t<contained_t, writer_t>;

	 contained_t _current_value;

	 af_rjson_handler_setish_t(
			 target_t* target_,
			 contained_t const& empty_value_ = contained_t()) :
		 af_rjson_handler_sequence_t(target_, empty_value_),
		 _current_value(empty_value){

	 }
	 af_rjson_handler_setish_t(
			 target_t* target_,
			 target_t const& default_value_,
			 contained_t const& empty_value_ = contained_t()) :
		 af_rjson_handler_sequence_t(target_, empty_value_),
		 _current_value(empty_value) {

	 }

	 inline void set_current_item() {
		 if (!_value_handler->is_set() || _value_handler->is_done()) {
			 _current_value = _empty_value;
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

 CreateArrayHandlers(af_rjson_handler_setish_t, is_setish<TargetT>::value);

/*template< typename TargetT, typename WriterT, TypeFilter(is_setish<TargetT>::value) >
af_rjson_handler* create_handler(
	TargetT& target_,
	null_setter_f null_setter_ = no_default_setter_f,
	contained_t const& empty_value_ = contained_t()) {
	return new af_rjson_handler_setish_t<TargetT, WriterT>(target_, null_setter_, empty_value_);
}*/

// general maps are stored as arrays of pair objects: { "key" : key_value, "value" : value_value }
// there is an optimisation for string maps
// first we need an intermediate handler for these objects
 namespace af_json_pairs {
	 namespace {
		 static const char* const tag_key = "key";
		 static const char* const tag_value = "value";
	 };
	 // reader and writer are different for these pairs
	 template< typename KeyT, typename ValueT, typename WriterT>
	 struct af_rjson_handler_pair_t {
		 using key_t = KeyT;
		 using value_t = ValueT;
		 using writer_t = WriterT;
		 using key_handler_t = af_rjson_handler_value_t<key_t, writer_t>;
		 using value_handler_t = af_rjson_handler_value_t<value_t, writer_t>;
		 using current_handler_t = af_rjson_handler_t<writer_t>;

		 key_t _key;
		 value_t _value;
		 key_handler_t* _key_handler;
		 value_handler_t* _value_handler;
		 current_handler_t* _current_handler;

		 af_rjson_handler_pair_t() :
			 _key_handler(create_handler<key_t, writer_t>(&_key)),
			 _value_handler(create_handler<value_t, writer_t>(&_value)),
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

	 template< typename KeyT, typename ValueT, typename WriterT>
	 struct af_rjson_writer_pair_t {
		 using key_t = KeyT;
		 using value_t = ValueT;
		 using writer_t = WriterT;
		 using key_writer_t = af_rjson_handler_value_t<key_t, writer_t>;
		 using value_writer_t = af_rjson_handler_value_t<value_t, writer_t>;

		 key_writer_t* _key_writer;
		 value_writer_t* _value_writer;

		 af_rjson_writer_pair_t() :
			 _key_writer(create_handler<key_t, writer_t>(nullptr)),
			 _value_writer(create_handler<value_t, writer_t>(nullptr)) {
		 }
		 void reset(key_t& key_, value_t& value_) {
			 _key_writer->reset(&key_);
			 _value_writer->reset(&value_);
		 }

		 void write(WriterT& writer_) const {
			 writer_->StartObject();
			 writer_->Key(tag_key);
			 _key_writer->write(writer_);
			 writer_->Key(tag_value);
			 _value_writer->write(writer_);
			 writer_->EndObject();
		 }
	 };
 }

template<typename TargetT, typename WriterT>
struct af_rjson_handler_mappish_t : public af_rjson_handler_value_t<WriterT> {

	using target_t = TargetT;
	using writer_t = WriterT;
	using key_t = target_t::key_type;
	using value_t = target_t::mapped_type;
	using pair_reader_t = af_json_pairs::af_rjson_handler_pair_t<key_t, value_t, writer_t>;
	using pair_writer_t = af_json_pairs::af_rjson_writer_pair_t<key_t, value_t, writer_t>;

	pair_reader_t _pair_reader;
	pair_writer_t _pair_writer;

	bool _this_array_started;

	af_rjson_handler_mappish_t(target_t* target_) :
		af_rjson_handler_value_t<TargetT, WriterT>(target_),
		_this_array_started(false) {
	}
	af_rjson_handler_mappish_t(target_t* target_,target_t const& default_value_) :
		af_rjson_handler_value_t<TargetT, WriterT>(target_, default_value_),
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
			return af_rjson_handler_value_t<TargetT, WriterT>::Null();
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

	void write(writer_t& writer_) const override {
		writer_->StartArray();
		for (auto const& t : *_target) {
			_pair_writer.reset(t.first, t.second);
			_pair_writer.write(writer_);
		}
		writer_->EndArray();
	}
};

CreateHandlers(af_rjson_handler_mappish_t, (is_mappish<TargetT>::value && !(is_string<TargetT>::value)))

//opitimised format for maps with string keys
template<typename TargetT, typename WriterT>
struct af_rjson_handler_string_mappish_t : public af_rjson_handler_value_t<WriterT> {

	using target_t = TargetT;
	using writer_t = WriterT;
	using key_t = target_t::key_type; // this is some string type
	using value_t = target_t::mapped_type;
	using value_handler_t = af_rjson_handler_value_t<value_t, writer_t>;

	key_t _current_key;
	value_t _current_value;
	value_t const _empty_value;

	value_handler_t* _value_handler;

	bool _this_object_started;

	af_rjson_handler_mappish_t(
			target_t* target_, 
			value_t const& empty_value_ = value_t()) :
		af_rjson_handler_value_t<TargetT, WriterT>(target_),
		_current_value(empty_value_),
		_empty_value(empty_value_)
		_value_handler(create_handler<value_t, writer_t>(nullptr)),
		_this_object_started(false) {
	}
	af_rjson_handler_mappish_t(
			target_t* target_,
			target_t const& default_value_,
			value_t const& empty_value_ = value_t()) :
		af_rjson_handler_value_t<TargetT, WriterT>(target_, default_value_),
		_current_value(empty_value_),
		_empty_value(empty_value_)
		_value_handler(create_handler<value_t, writer_t>(nullptr)),
		_this_object_started(false) {
	}

	template<typename... ParamsT>
	inline bool delegate_f(bool (value_handler_t::* mf)(ParamsT ...), ParamsT... ps) {
		bool ret = (_value_handler.*mf)(ps...);
		if (_value_handler.is_done()) {
			_target[_current_key] = _current_value;
			_current_key = key_t();
			_current_value = _empty_value;
			_value_handler.reset(&current_value);
		}
		return ret;
	}
	bool Null() {
		if (_current_key.empty())
			return af_rjson_handler_value_t<TargetT, WriterT>::Null();
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

	void write(writer_t& writer_) const override {
		writer_->StartObject();
		for (auto const& t : *_target) {
			writer_->Key(t.first);
			_value_handler->reset(&t.second);
			_value_handler->write(writer_);
		}
		writer_->EndObject();
	}

	void release() override {
		if (_value_handler) {
			_value_handler->release();
			_value_handler = nullptr;
		}
		af_rjson_handler_value_t<TargetT, WriterT>::release();
	}
};

template< typename TargetT, typename WriterT, TypeFilter((is_mappish<TargetT>::value && is_string<TargetT>::value)) >
af_rjson_handler_value_t<TargetT, WriterT>* create_handler(TargetT* target_) {
	if(af_json_configuration::optimised_string_maps())
		return new af_rjson_handler_string_mappish_t<TargetT, WriterT>(target_);
	else
		return new af_rjson_handler_mappish_t<TargetT, WriterT>(target_);
}

template< typename TargetT, typename WriterT, TypeFilter((is_mappish<TargetT>::value && is_string<TargetT>::value)) >
af_rjson_handler_value_t<TargetT, WriterT>* create_handler(TargetT* target_, TargetT const& default_value_) {
	if (af_json_configuration::optimised_string_maps())
		return new af_rjson_handler_string_mappish_t<TargetT, WriterT>(target_, default_value_);
	else
		return new af_rjson_handler_mappish_t<TargetT, WriterT>(target_, default_value_);
}


template<typename TargetT, typename WriterT>
struct af_rjson_handler_object_t : public af_rjson_handler_value_t<TargetT, WriterT> {

	using target_t = TargetT;
	using writer_t = WriterT;
	using handler_p_t = af_rjson_handler*;
	using handlers_t = std::vector<std::pair<std::string, handler_p_t>>;
	// we use a vector here to preserve the order of elements
	handlers_t _handlers;
	std::string _current_key;
	handler_p_t _current_handler;

	af_rjson_handler_object_t(
			target_t* target_, 
			handlers_t const& handlers_ = {}) :
		af_rjson_handler_value_t<TargetT, WriterT>(target_),
		_handlers(handlers_),
		_current_handler(nullptr){
	}
	af_rjson_handler_object_t(
			target_t* target_,
			target_t const& default_value_,
			handlers_t const& handlers_ = {}) :
		af_rjson_handler_value_t<TargetT, WriterT>(target_, default_value_),
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