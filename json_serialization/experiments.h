#pragma once


af_serializer serialise() {
	af_serialisation <<
		object() <<
		value(name, v) <<
		value(name_2, arr(vector)) <<
		value(name_3, arr(1, 2, 3, 4, 5, )) <<
		object(name_4) <<
		value(name_5,



			);

	return 
	af_stream <<
		pre_load_function_ << 
		use_default_defaults <<
		dont_use_default_defalts << 
		object <<
			name_1 << _v <<
			name_2 << _vector | {} | default_element_ <<    // (std::string, T*, T const&)
			name_3 <<
				object <<
					name_4 << _bool | false <<
					name_5 << _int  | 0 << 
					name_6 << _serialisable_object | nullptr <<
				end_object <<
			name_7 << af_rjson_handler_t*  << 
			name_8 << _map | {} | [default_element_] <<
		end_object <<
		post_load_function_;

}
class serialisation_description {
	std::map<std::string, std::pair<void*,std::type_index> > _key_to_handler;
};
enum class json_type {
	int_	= 1 << 1,	// int
	uint_	= 1 << 2,	// unsigned int, size_t
	int64_	= 1 << 3,	// long
	uint64_ = 1 << 4,   // unsigned long, size_t
	bool_	= 1 << 5,	// bool
	double_ = 1 << 6,	// double
	string_ = 1 << 7,	// std::string
	array_	= 1 << 8,	// array, vector, list 
	object_	= 1 << 9,	// af_json_serialisable	
	null_   = 1 << 10
};

template< typename TargetT >
using custom_get_set_f = std::function< TargetT const&(bool, TargetT const*)>; // TargetT must be default constructable

using null_setter_f = std::function<bool()>;

bool no_default_setter_f() {
	// TODO: make this a nicer errror
	throw std::runtime_error("Unexpected null read.");
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

#define VOID_CONVERSIONS( HANDLER_T, AF_TYPE_INDEX) \
	static constexpr af_handler_type_index _handler_type{ af_handler_type_index::AF_TYPE_INDEX }\
	void* to_void() { return static_cast<void*>(this); } \
	static HANDLER_T* from_void(void* p) { return static_cast<HANDLER_T*>(p); }

struct af_rjson_handler_base {
	virtual bool Null() { throw std::runtime_error("Unexpected"); }
	virtual bool Bool(bool b) { throw std::runtime_error("Unexpected"); }
	virtual bool Int(int i) { throw std::runtime_error("Unexpected"); }
	virtual bool Uint(unsigned i) { throw std::runtime_error("Unexpected"); }
	virtual bool Int64(int64_t i) { throw std::runtime_error("Unexpected"); }
	virtual bool Uint64(uint64_t i) { throw std::runtime_error("Unexpected"); }
	virtual bool Double(double d) { throw std::runtime_error("Unexpected"); }
	virtual bool RawNumber(const Ch* str, SizeType length, bool copy) { throw std::runtime_error("Unexpected"); }
	virtual bool String(const Ch* str, SizeType length, bool copy) { throw std::runtime_error("Unexpected"); }
	virtual bool StartObject() { throw std::runtime_error("Unexpected"); }
	virtual bool Key(const Ch* str, SizeType length, bool copy) { throw std::runtime_error("Unexpected"); }
	virtual bool EndObject(SizeType memberCount) { throw std::runtime_error("Unexpected"); }
	virtual bool StartArray() { throw std::runtime_error("Unexpected"); }
	virtual bool EndArray(SizeType elementCount) { throw std::runtime_error("Unexpected"); }
}


template< typename TargetT >
struct af_json_handler_plain : public af_rjson_handler_base {

	using target_t = TargetT;

	target_t& _target;


	af_json_handler_plain(target_t& target_) :
		_target(target_)
	}

	virtual ~af_json_handler_plain() {}

	inline bool null() { return no_default_handler_f(); }

	inline bool set(target_t const& value_) { _target = value_; return true; }
	
	template<typename ConvertibleT>
	inline bool set(ConvertibleT const& value_) { _target = target_t{ value_ }; return true; }

	inline target_type const& get() const { return _target; }
	inline target_type& get() { return _target; }

	// reader part
	virtual bool Null() { null(); }
	virtual bool Bool(bool b) { set(b); }
	virtual bool Int(int i) { set(i); }
	virtual bool Uint(unsigned i) { ; }
	virtual bool Int64(int64_t i) { throw std::runtime_error("Unexpected"); }
	virtual bool Uint64(uint64_t i) { throw std::runtime_error("Unexpected"); }
	virtual bool Double(double d) { throw std::runtime_error("Unexpected"); }
	virtual bool RawNumber(const Ch* str, SizeType length, bool copy) { throw std::runtime_error("Unexpected"); }
	virtual bool String(const Ch* str, SizeType length, bool copy) { throw std::runtime_error("Unexpected"); }
	virtual bool StartObject() { throw std::runtime_error("Unexpected"); }
	virtual bool Key(const Ch* str, SizeType length, bool copy) { throw std::runtime_error("Unexpected"); }
	virtual bool EndObject(SizeType memberCount) { throw std::runtime_error("Unexpected"); }
	virtual bool StartArray() { throw std::runtime_error("Unexpected"); }
	virtual bool EndArray(SizeType elementCount) { throw std::runtime_error("Unexpected"); }




	template<typename WriterT>
	inline void write(WriterT& writer_) const { write(get(), writer_); }

	inline void release() { delete this; }

	VOID_CONVERSIONS(af_json_handler_plain<target_t>);
};

template< typename UnderlyingHandlerT >
struct af_json_handler_null_default {

	using underlying_handler_t = UnderlyingHandlerT; 
	using target_t = underlying_handler_t::target_type;
	
	underlying_handler_t* _handler;
	target_t const _default_value;
	null_setter_f _null_setter;


	af_json_handler_null_default(
			underlying_handler_t* handler_, 
			target_t const& default_value_) :
		_handler(handler_),
		_default_value(default_value_),
		_null_setter(create_default_handler(handler_->get(), default_value_)) {
			//TODO: assert _handler not null
	}

	af_json_handler_null_default(
			underlying_handler_t* handler_,
			target_t const& default_value_, 
			null_setter_f null_setter_) :
		_target(target_),
		_default_value(default_value_),
		_null_setter(null_setter_) {

		//TODO: assert _handler not null
	}

	virtual ~af_json_handler_null_default() {}

	inline bool null() { return _null_handler(); }

	inline bool set(target_t const& value_) { _handler->set(value_); return true;  }
	
	template<typename ConvertibleT>
	inline bool set(ConvertibleT const& value_) { _handler->set<ConvertibleT>(value_); return true; }

	inline target_type const& get() const { return _handler->get(); }
	inline target_type& get() { return _handler->get(); }

	template<typename WriterT>
	inline void write(WriterT& writer_) const {
		if (get() != _default_value)
			_handler->write(writer_);
		else
			write_null(writer_);
	}

	inline void release() { 
		_handler->release();
		delete this; 
	}

	VOID_CONVERSIONS(af_json_handler_null_default<target_t>);
};


template< typename UnderlyingHandlerT >
struct af_json_handler_terse_default : public af_json_handler_null_default<UnderlyingHandlerT>{

	using underlying_handler_t = UnderlyingHandlerT;
	using target_t = underlying_handler_t::target_type;

	af_json_handler_terse_default(
		underlying_handler_t* handler_,
		target_t const& default_value_) :
		af_json_handler_null_default<UnderlyingHandlerT>(
			handler_, default_value_){

		null(); // here we initialise the target, in case he value never arrives

	}

	af_json_handler_terse_default(
		underlying_handler_t* handler_,
		target_t const& default_value_,
		null_setter_f null_setter_) :
			af_json_handler_null_default<UnderlyingHandlerT>(
				handler_, default_value_, null_setter_){

		//TODO: assert _handler not null
		null(); // here we initialise the target, in case he value never arrives
	}

	template<typename WriterT>
	inline void write(WriterT& writer_) { 
		if (get() != _default_value)
			_handler->write(writer_);
	}

	VOID_CONVERSIONS(af_json_handler_terse_default<TargetT>);
};


template< typename TargetT >
struct af_json_handler_with_custom_get_set { 
	
	using target_t = TargetT;

	target_t& _target;
	custom_get_set_f<target_t> _get_set_f;
	
	af_json_handler_with_conversion(
		target_t& target_,
		custom_get_set_f<target_t> get_set_f_) :
			_target(target_),
			_get_set_f(get_set_f_){
	}

	inline void null() { _get_set_f(true, nullptr); }
	
	inline bool set(target_t const& value_) { _get_set_f(true, &value_); return true; }

	template<typename ConvertibleT>
	inline bool set(ConvertibleT const& value_) { 
		target_t v{ value_ };
		_get_set_f(true, &v);
		return true;
	}

	inline target_type const& get() const { return _target; }
	inline target_type& get() { return _target; }

	template<typename WriterT>
	inline void write(WriterT& writer_) { write(get(), writer_); }


	VOID_CONVERSIONS(af_json_handler_with_conversion<target_type>);

};

template < typename UnderlyingHandlerT > 
struct rjson_handler_simple_types {
	typename handler_t = UnderlyingHandlerT;

	handler_t* _handler;

	inline bool Null() { return _handler->null(); }
	inline bool Bool(bool b) { return _handler->set(b); }
	inline bool Int(int i) { return _handler->set(i); }
	inline bool Uint(unsigned i) { return _handler->set(i); }
	inline bool Int64(int64_t i) { return _handler->set(i); }
	inline bool Uint64(uint64_t i) { return _handler->set(i); }
	inline bool Double(double d) { return _handler->set(d); }
	inline bool RawNumber(const Ch* str, SizeType length, bool copy) {
		throw std::runtime_error("Unsupported");
	}
	inline bool String(const Ch* str, SizeType length, bool copy) {
		return _handler->set(std::string(str, length));
	}
	inline bool StartObject() { throw std::runtime_error("Unexpected"); }
	inline bool Key(const Ch* str, SizeType length, bool copy) { throw std::runtime_error("Unexpected"); }
	inline bool EndObject(SizeType memberCount) { throw std::runtime_error("Unexpected"); }
	inline bool StartArray() { throw std::runtime_error("Unexpected"); }
	inline bool EndArray(SizeType elementCount) { throw std::runtime_error("Unexpected"); }

	template<typename WriterT>
	inline void write(WriterT& writer_) { _handler->write(writer_); }

	inline void release() {
		_handler->release();
		delete this;
	}



	VOID_CONVERSIONS(af_json_handler_with_conversion<target_type>);
};


template< typename TargetT >
struct af_json_handler {
	TargetT& _target;

	af_handler(TargetT& target_) :_target(target_) {}
	inline void set(TargetT const& value_) { _target = value_; }

	inline Target_t const& get() const { return _target; }
	
	static af_json_handler<Target_T>* from_void(void* v) { 
		return static_cast<af_json_handler<Target_T>*>(v);
	}
}; 


template<>
struct af_handler<int> {
	int& _target;
	af_handler(int& target_) :_target(target_) {}
	inline void set(int const& value_) { _target = value_; }
	inline void set(unsigned const& i) { _i = static_cast<int>(i); }

	inline int const& get() const { return _target; }
};
template<>
struct af_handler<int> {
	int& _target;
	af_handler(int& target_) :_target(target_) {}
	inline void set(int const& value_) { _target = value_; }
	inline void set(unsigned const& i) { _i = static_cast<int>(i); }

	inline int const& get() const { return _target; }
};


struct af_handler<int> {
	std::string& _s;
	inline void set(std::string const& s) { _s = s; }
	inline std::string const& get() const { return _s; }
};
struct string_handler {
	std::string& _s;
	inline void set(std::string const& s) { _s = s; }
	inline std::string const& get() const { return _s; }
};



enum class json_event {
	key_			= 1 << 1,    // char const * const or std::string const
	int_			= 1 << 2,	// int
	uint_			= 1 << 3,	// unsigned int, size_t
	int64_			= 1 << 4,	// long
	uint64_			= 1 << 5,   // unsigned long, size_t
	double_			= 1 << 6,	// double
	string_			= 1 << 7,	// std::string
	start_object_	= 1 << 8,	
	end_object_		= 1 << 9,
	start_array_	= 1 << 10,	// array, vector, list 
	end_array_,		= 1 << 11,
	raw_number_		= 1 << 12,
};


class af_sax_handler {

};




template< typename TargetT >
//struct af_json_handler_null_default {
//	TargetT& _target;
//	TargetT const _default_value;
//	null_handler_f _null_handler;
//
//	using target_type = TargetT;
//
//	af_json_handler_null_default(TargetT& target_, TargetT const& default_value_) :
//		_target(target_),
//		_default_value(default_value_),
//		_null_handler(create_default_handler(target_, default_value_)) {
//	}
//
//	af_json_handler_null_default(TargetT& target_, TargetT const& default_value_, null_handler_f null_handler_) :
//		_target(target_),
//		_default_value(default_value_),
//		_null_handler(null_handler_) {
//	}
//
//	virtual ~af_json_handler_null_default() {}
//
//	inline bool null() { return _null_handler(); }
//
//	inline void set(TargetT const& value_) { _target = value_; }
//
//	template<typename WriterT>
//	inline void write(WriterT& writer_) {
//		if (_target != _default_value)
//			write(_target, writer_);
//		else
//			write_null(writer_);
//	}
//
//	inline void release() { delete this; }
//
//	VOID_CONVERSIONS(af_json_handler_null_default<TargetT>);
//};
//
