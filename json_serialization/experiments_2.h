#pragma once
#include "autotelica_core/util/include/std_disambiguation.h"
#include "autotelica_core/util/include/asserts.h"
using namespace autotelica::std_disambiguation;

template< typename TargetT >
using custom_get_set_f = std::function< TargetT const& (bool, TargetT const*)>; // TargetT must be default constructable

using null_setter_f = std::function<bool()>;
bool no_default_setter_f() {
	AF_ERROR("Null was not expected.");
	return false;
}
template< typename TargetT >
null_setter_f create_default_setter(TargetT& target_, TargetT const& default_value_) {
	null_setter_f func = [&]() { target_ = default_value_; return true; };
	return func;
}

template<typename TargetT, typename WriterT>
void write(TargetT const& value, WriterT& writer);

template< typename WriterT >
inline void write_null(WriterT& writer) { writer.Null(); }

template< typename WriterT >
inline void write(int const& value, WriterT& writer) { writer.Int(value); }

template< typename WriterT >
inline void write(size_t const& value, WriterT& writer) { writer.Uint64(static_cast<uint64_t>(value)); }

// TODO: more write functions

template<typename WriterT>
struct af_rjson_handler_t {
	
	virtual ~af_rjson_handler_t(){}
	virtual bool is_done() const = 0; // signal to the controller that we are done loading this value

	// reader part
	virtual bool Null() { AF_ERROR("Null was not expected."); }
	virtual bool Bool(bool b) { AF_ERROR("Boolean value was not expected.");}
	virtual bool Int(int i) { AF_ERROR("Int value was not expected."); }
	virtual bool Uint(unsigned i) { AF_ERROR("Uint value was not expected."); }
	virtual bool Int64(int64_t i) { AF_ERROR("Int64 value was not expected."); }
	virtual bool Uint64(uint64_t i) { AF_ERROR("Uint64 value was not expected."); }
	virtual bool Double(double d) { AF_ERROR("Double value was not expected."); }
	virtual bool RawNumber(const Ch* str, SizeType length, bool copy) { AF_ERROR("RawNumber value was not expected."); }
	virtual bool String(const Ch* str, SizeType length, bool copy) { AF_ERROR("String value was not expected."); }
	virtual bool StartObject() { AF_ERROR("Object start was not expected."); }
	virtual bool Key(const Ch* str, SizeType length, bool copy) { AF_ERROR("Object key was not expected."); }
	virtual bool EndObject(SizeType memberCount) { AF_ERROR("Object end was not expected."); }
	virtual bool StartArray() { AF_ERROR("Array start was not expected."); }
	virtual bool EndArray(SizeType elementCount) { AF_ERROR("Array end was not expected."); }

	// writer part 
	virtual void write(WriterT& writer_) const { AF_ERROR("Writer is not implemented."); }

	virtual void release() { delete this; }
};

template<typename TargetT, typename WriterT>
struct af_rjson_handler_value_t : public af_rjson_handler_t<WriterT> {

	using target_t = TargetT;
	using writer_t = WriterT;

	target_t* _target;
	null_setter_f _null_setter;
	bool _done;
	af_rjson_handler_value_t(target_t* target_, null_setter_f null_setter_ = no_default_setter_f) : 
		_target(target_), 
		_null_setter(null_setter_),
		_done(false){}

	inline void done() { _done = true; }
	bool is_done() const override { return _done; }
	
	inline void reset(target_t* target_) { _target = target_; _done = false; }

	inline bool null() { done(); return _null_setter(); }

	inline bool set(target_t const& value_) { *_target = value_; done(); return true; }

	template<typename ConvertibleT>
	inline bool set(ConvertibleT const& value_) { *_target = target_t{ value_ }; done(); return true; }

	inline target_type const& get() const { return *_target; }
	inline target_type& get() { return *_target; }

	virtual void release() { 
		delete _target;
		af_rjson_handler_t<WriterT>::release();
	}

};

template<typename TargetT, typename WriterT>
struct af_rjson_handler_integral_t : public af_rjson_handler_value_t<TargetT, WritterT> {

	using target_t = TargetT;
	using writer_t = WriterT;

	af_rjson_handler_integral_t(target_t* target_, null_setter_f null_setter_ = no_default_setter_f) :
		af_rjson_handler_value_t(target_, null_setter_) {}
	
	// reader part
	bool Null()  override { return null(); }
	bool Bool(bool b)  override { return set(b); }
	bool Int(int i)  override { return set(i); }
	bool Uint(unsigned i)  override { return set(i); }
	bool Int64(int64_t i)  override { return set(i); }
	bool Uint64(uint64_t i) override { return set(i); }

	// writer part 
	void write(writer_t& writer_) const override { write(get(), writer_); }
};

// SFINAE based type filter
#define TypeFilter( condition ) std::enable_if_t<condition, bool> = true

template< typename TargetT, typename WriterT, TypeFilter( std::is_integral<TargetT>::value ) >
af_rjson_handler* create_handler(TargetT& target_, null_setter_f null_setter_ = no_default_setter_f) {
	return new af_rjson_handler_integral_t<TargetT, WriterT>(target_, null_setter_);
}

template<typename TargetT, typename WriterT>
struct af_rjson_handler_floating_t : public af_rjson_handler_value_t<TargetT, WritterT>{

	using target_t = TargetT;
	using writer_t = WriterT;

	af_rjson_handler_floating_t(target_t& target_, null_setter_f null_setter_ = no_default_setter_f) : 
		af_rjson_handler_value_t(target_, null_setter_) {}

	// reader part
	bool Null()  override { return null(); }
	bool Double(double d) override { return set(d); }

	// writer part 
	void write(writer_t& writer_) const override { write(get(), writer_); }
};

template< typename TargetT, typename WriterT, TypeFilter(std::is_floating_point<TargetT>::value) >
af_rjson_handler* create_handler(TargetT& target_, null_setter_f null_setter_ = no_default_setter_f) {
	return new af_rjson_handler_floating_t<TargetT, WriterT>(target_, null_setter_);
}

template<typename TargetT, typename WriterT>
struct af_rjson_handler_string_t : public af_rjson_handler_value_t<TargetT, WritterT> {

	using target_t = TargetT;
	using writer_t = WriterT;

	af_rjson_handler_string_t(target_t& target_, null_setter_f null_setter_ = no_default_setter_f) : 
		af_rjson_handler_value_t(target_, null_setter_) {}

	// reader part
	bool Null() override { return null(); }
	bool String(const Ch* str, SizeType length, bool copy) override {
		_target.assign(str, length);
		done();
		return true;
	}

	// writer part 
	void write(writer_t& writer_) const override { write(get(), writer_); }
};

template< typename TargetT, typename WriterT, TypeFilter(is_string<TargetT>::value) >
af_rjson_handler* create_handler(TargetT& target_, null_setter_f null_setter_ = no_default_setter_f) {
	return new af_rjson_handler_string_t<TargetT, WriterT>(target_, null_setter_);
}

template<typename TargetT, typename WriterT>
struct af_rjson_handler_shared_ptr_t : public af_rjson_handler_t<WriterT> {

	using target_t = TargetT;
	using writer_t = WriterT;
	using contained_t = target_t::element_type;
	using value_handler_t = af_rjson_handler_value_t<contained_t, writer_t>;


	target_t* _target;
	null_setter_f _null_setter;
	value_handler_t* _value_handler;

	af_rjson_handler_vector_t(
		target_t* target_, null_setter_f null_setter_ = no_default_setter_f) :
		_target(target_),
		_null_setter(null_setter_),
		_value_handler(nullptr){
		
		if (!(*_target))
			*_target = target_t(new contained_t());
		_value_handler = create_handler(*(*_target));
	}
	bool is_done() const override { return _value_handler->is_done(); }

	inline bool null() { return _null_setter(); }

	// reader part
	bool Null() override { null(); }
	bool Bool(bool b) override { return _value_handler->Bool(b); }
	bool Int(int i) override { return _value_handler->Int(i); }
	bool Uint(unsigned i) override { return _value_handler->Uint(i); }
	bool Int64(int64_t i) override { return _value_handler->Int64(i); }
	bool Uint64(uint64_t i) override { return _value_handler->Uint64(i); }
	bool Double(double d) override { return _value_handler->Double(d); }
	bool RawNumber(const Ch* str, SizeType length, bool copy) override { return _value_handler->RawNumber(str, length, copy); }
	bool String(const Ch* str, SizeType length, bool copy) override { return _value_handler->String(str, length, copy); }
	bool StartObject()  override { return _value_handler->StartObject(); }
	bool Key(const Ch* str, SizeType length, bool copy) override { return _value_handler->Key(str, length, copy); }
	bool EndObject(SizeType memberCount)  override { return _value_handler->EndObject(memberCount); }
	bool StartArray() override { return _value_handler->StartArray(str, length, copy); }
	bool EndArray(SizeType elementCount)  override { return _value_handler->EndArray(elementCount); }

	// writer part 
	void write(writer_t& writer_) const override { _value_handler->write(writer_); }

	void release() override {
		_value_handler->release();
		delete this;
	}
};

template< typename TargetT, typename WriterT, TypeFilter(is_shared_ptr<TargetT>::value || std::is_pointer<TargetT>::value) >
af_rjson_handler* create_handler(TargetT& target_, null_setter_f null_setter_ = no_default_setter_f) {
	return new af_rjson_handler_shared_ptr_t<TargetT, WriterT>(target_, null_setter_);
}
 template<typename TargetT, typename WriterT>
 struct af_rjson_handler_sequence_t : public af_rjson_handler_value_t<WriterT> {

	 using target_t = TargetT;
	 using writer_t = WriterT;
	 using contained_t = target_t::value_type;
	 using value_handler_t = af_rjson_handler_value_t<ContainedT, WriterT>;

	 
	 value_handler_t* _value_handler;
	 contained_t const _empty_value;

	 bool _delegate_to_current;
	 bool _this_array_started;

	 
	 af_rjson_handler_sequence_t(
			target_t* target_, 
			null_setter_f null_setter_ = no_default_setter_f, 
			contained_t const& empty_value_ = contained_t()) :
		_target(target_),
		_null_setter(null_setter_),
		_value_handler(create_handler<contained_t, writer_t>(nullptr)),
		_empty_value(empty_value_),
		_delegate_to_current(false),
		_this_array_started(false),
		_done(false){
	 }

	 inline bool null() { done(); return _null_setter(); }

	 inline void set_current_item() {
		 if (!_delegate_to_current) {
			 _target->push_back(_default_value);
			 contained_t& ret(_target->back());
			 _value_handler->reset(&ret);
		 }
	 }

	 // reader part
	 template<typename... ParamsT>
	 bool delegate_f(bool (value_handler_t::* mf)(ParamsT ...), ParamsT... ps) {
		 set_current_item();
		 return (_value_handler->*mf)(ps...);
	 }
	 bool Bool(bool b) override { return delegate_f(&value_handler_t::Bool, b); }
	 bool Int(int i) override { return delegate_f(&value_handler_t::Int, i); }
	 bool Uint(unsigned i) override { return delegate_f(&value_handler_t::Uint, i); }
	 bool Int64(int64_t i) override { return delegate_f(&value_handler_t::Int64, i); }
	 bool Uint64(uint64_t i) override { return delegate_f(&value_handler_t::Uint64, i); }
	 bool Double(double d) override { return delegate_f(&value_handler_t::Double, d); }
	 bool RawNumber(const Ch* str, SizeType length, bool copy) override { return delegate_f(&value_handler_t::RawNumber, str, length, copy); }
	 bool String(const Ch* str, SizeType length, bool copy) override { return delegate_f(&value_handler_t::String, str, length, copy); }
	 bool StartObject() override {
		 set_current_item();
		 _delegate_to_current = true;
		 return _value_handler->StartObject();
	 }
	 bool Key(const Ch* str, SizeType length, bool copy) { 
		 AF_ASSERT(_delegate_to_current, "Keys are not expeted in arrays.");
		 return _value_handler->Key(str, length, copy);
	 }
	 bool EndObject(SizeType memberCount) override { 
		 set_current_item();
		 bool ret = _value_handler->EndObject(memberCount);
		 _delegate_to_current = _value_handler->is_done();
		 return ret;
	 }
	 bool StartArray() override { 
		 if (!_this_array_started) {
			 _this_array_started = true;
			 return true;
		 }
		 set_current_item();
		 _delegate_to_current = true;
		 _value_handler->StartArray();
	 }
	 bool EndArray(SizeType elementCount) override { 
		 if (!_delegate_to_current) {
			 done();
			 return true;
		 }
		 bool ret = _value_handler->EndArray();
		 _delegate_to_current = _value_handler->is_done();
		 return ret;
	 }

	 // writer part 
	 void write(writer_t& writer_) const override { 
		 writer_->StartArray();
		 for (auto const& t : *_target)
			 t->write(writer_);
		 writer_->EndArray();
	 }

	 void release() override {
		 _value_handler->release();
		 delete this;
	 }
 };

 template< typename TargetT, typename WriterT, TypeFilter(is_sequence<TargetT>::value) >
 af_rjson_handler* create_handler(
		TargetT& target_, 
		null_setter_f null_setter_ = no_default_setter_f,
		contained_t const& empty_value_ = contained_t()	) {
	 return new af_rjson_handler_sequence_t<TargetT, WriterT>(target_, null_setter_, empty_value_);
 }

 template<typename TargetT, typename WriterT>
 struct af_rjson_handler_setish_t : public af_rjson_handler_value_t<WriterT> {

	 using target_t = TargetT;
	 using writer_t = WriterT;
	 using contained_t = target_t::value_type;
	 using value_handler_t = af_rjson_handler_value_t<ContainedT, WriterT>;

	 value_handler_t* _value_handler;
	 contained_t const _empty_value;

	 contained_t _current_value;

	 bool _delegate_to_current;
	 bool _this_array_started;


	 af_rjson_handler_sequence_t(
		 target_t* target_,
		 null_setter_f null_setter_ = no_default_setter_f,
		 contained_t const& empty_value_ = contained_t()) :
		 _target(target_),
		 _null_setter(null_setter_),
		 _value_handler(create_handler<contained_t, writer_t>(nullptr)),
		 _empty_value(empty_value_),
		 _current_value(empty_value),
		 _delegate_to_current(false),
		 _this_array_started(false),
		 _done(false) {

		 _value_handler->reset(&_current_value);
	 }

	 inline bool null() { done(); return _null_setter(); }

	 inline void set_current_item() {
		 if (!_delegate_to_current) {
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
	 bool delegate_f(bool (value_handler_t::*mf)(ParamsT ...), ParamsT... ps) {
		 set_current_item();
		 return store((_value_handler->*mf)(ps...));
	 }
	 bool Null() override {
		 if (!_this_array_started)
			 return null();
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
	 bool StartObject() override {
		 set_current_item();
		 _delegate_to_current = true;
		 return _value_handler->StartObject();
	 }
	 bool Key(const Ch* str, SizeType length, bool copy) {
		 AF_ASSERT(_delegate_to_current, "Keys are not expeted in arrays.");
		 return _value_handler->Key(str, length, copy);
	 }
	 bool EndObject(SizeType memberCount) override {
		 set_current_item();
		 bool ret = _value_handler->EndObject(memberCount);
		 _delegate_to_current = _value_handler->is_done();
		 return ret;

	 }
	 bool StartArray() override {
		 if (!_this_array_started) {
			 _this_array_started = true;
			 return true;
		 }
		 set_current_item();
		 _delegate_to_current = true;
		 return _value_handler->StartArray();
	 }
	 bool EndArray(SizeType elementCount) override {
		 if (!_delegate_to_current) {
			 done();
			 return true;
		 }
		 bool ret = _value_handler->EndArray();
		 _delegate_to_current = _value_handler->is_done();
		 return ret;
	 }

	 // writer part 
	 void write(writer_t& writer_) const override {
		 writer_->StartArray();
		 for (auto const& t : *_target)
			 t->write(writer_);
		 writer_->EndArray();
	 }

	 void release() override {
		 _value_handler->release();
		 delete this;
	 }
 };

 template< typename TargetT, typename WriterT, TypeFilter(is_setish<TargetT>::value) >
 af_rjson_handler* create_handler(
	 TargetT& target_,
	 null_setter_f null_setter_ = no_default_setter_f,
	 contained_t const& empty_value_ = contained_t()) {
	 return new af_rjson_handler_setish_t<TargetT, WriterT>(target_, null_setter_, empty_value_);
 }

