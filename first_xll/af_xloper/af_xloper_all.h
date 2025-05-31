
#pragma once
#define WIN32_LEAN_AND_MEAN   

#include <windows.h>
#include "XLCALL.H"
#include <time.h>
#include <stdexcept>
#include <type_traits>
#include <mutex>
#include <iostream>
#include <locale>
#include <codecvt>
#include <cwctype>
#include <sstream>
#include <functional>
#include <iterator>
#include <memory>
#include <string>
#include <list>
#include <forward_list>
#include <map>
#include <unordered_map>

namespace autotelica {
namespace xloper {

	namespace xl_inner {
		// implementation details of the af_xloper wrappers
		// much of the magic depends on this, but it is ugly stuff best hidden away

		namespace xl_type_ops {
			// helpers for basic xltype operations

			inline void set_xl_type(XLOPER12& xl, DWORD xltype) {
				xl.xltype |= xltype;
			}
			inline void overwrite_xl_type(XLOPER12& xl, DWORD xltype) {
				xl.xltype = xltype;
			}
			
			// checking functions 
			// is_ ... versions just return a boolean
			// check_ ... verstion throw if it is suitable to do so

			inline bool is_xl_error(LPXLOPER12 const& in) {
				return (!in || (in->xltype & xltypeErr));
			}

			inline void check_xl_error(LPXLOPER12 const& in) {
				if (is_xl_error(in))
					throw std::runtime_error("Error in input.");
			}
			inline bool is_xl_error(XLOPER12 const& in) {
				return (in.xltype & xltypeErr);
			}
			inline void check_xl_error(XLOPER12 const& in) {
				if (is_xl_error(in))
					throw std::runtime_error("Error in input.");
			}			
			inline bool is_xl_type(XLOPER12 const& xl, DWORD xltype) {
				return (xl.xltype & xltype);
			}
			inline void check_xl_type(XLOPER12 const& xl, DWORD xltype) {
				if (!is_xl_type(xl, xltype))
					throw std::runtime_error("Unexpected XLOPER type.");
			}
		}
		
		namespace xl_constants {
			// xloper12 constants
			auto const MAX_XL12_ROWS = 1048576;
			auto const MAX_XL12_COLS = 16384;
			auto const MAX_XL12_UDF_ARGS = 255;
			auto const MAX_XL12_STR_LEN = 32767u;

			// often used xloper12 values
			static XLOPER12 const& xlMissing() {
				static XLOPER12 missing{ 0, xltypeMissing };
				return missing;
			}
			static XLOPER12 const& xlEmptyString() {
				static wchar_t p[2]{ 0 };
				static XLOPER12 empty{ 0, xltypeStr };
				static std::once_flag _once;
				std::call_once(_once, [&]() {empty.val.str = p; }); // because prior to C++ 20 we can't initialise unions properly
				return empty;
			}
			static XLOPER12 const& xlNull() {
				static XLOPER12 _err{ 0, xltypeErr };
				static std::once_flag _once;
				std::call_once(_once, [&]() {_err.val.err = xlerrNull; }); // because prior to C++ 20 we can't initialise unions properly
				return _err;
			}
			static XLOPER12 const& xlNA() {
				static XLOPER12 _err{ 0, xltypeErr };
				static std::once_flag _once;
				std::call_once(_once, [&]() {_err.val.err = xlerrNA; }); // because prior to C++ 20 we can't initialise unions properly
				return _err;
			}
		}

		namespace xl_memory {
			// dealing with memory for xlopers is done the old fashioned way
			// because Excel API is C based
			// 

			// Create counted Unicode wchar string from null-terminated Unicode input
			static wchar_t* new_xl12string(const wchar_t* text)
			{
				using namespace xl_constants;
				size_t len;
				if (!text)
					throw std::runtime_error("Attempted to allocate null string");
				len = wcslen(text);

				if (len > MAX_XL12_STR_LEN)
					len = MAX_XL12_STR_LEN; // truncate
				wchar_t* p = (wchar_t*)malloc((len + 1) * sizeof(wchar_t));
				if (!p) throw std::runtime_error("Could not allocate memory for XLOPER12 string");
				memcpy(p + 1, text, (len) * sizeof(wchar_t));
				p[0] = (wchar_t)len;
				return p;
			}

			// create new xloper12s
			static LPXLOPER12 new_xloper12() {
				LPXLOPER12 out = (LPXLOPER12)malloc(sizeof(XLOPER12));
				if (!out)
					throw std::runtime_error("Could not allocate memory for XLOPER12");
				out->xltype = xlbitDLLFree;
				return out;
			}
			static LPXLOPER12 new_xloper12_array(size_t sz) {
				LPXLOPER12 out = (LPXLOPER12)malloc(sz * sizeof(XLOPER12));
				if (!out)
					throw std::runtime_error("Could not allocate memory for XLOPER12 array");
				return out;
			}

			// deep copy of xloper12s
			static void clone_value(XLOPER12 const& in, XLOPER12& out) {
				switch (out.xltype) {
				case xltypeNum:
				case xltypeBool:
				case xltypeErr:
				case xltypeInt:
				case xltypeMissing:
				case xltypeNil:
					out.val = in.val;
					break;
				case xltypeStr:
					out.val.str = new_xl12string(in.val.str);
					break;
				case xltypeMulti: {
					size_t sz = out.val.array.rows * out.val.array.columns;
					out.val.array.lparray = new_xloper12_array(sz);
					for (size_t i = 0; i < sz; ++i)
						clone_value(in.val.array.lparray[i], out.val.array.lparray[i]);
				}
								break;
				case xltypeRef:
				case xltypeSRef:
				case xltypeFlow:
					throw std::runtime_error("Unhandled Excel type.");
				default:
					throw std::runtime_error("Unknown Excel type.");
				}
			}
			static LPXLOPER12 clone(LPXLOPER12 in) {
				LPXLOPER12 out = new_xloper12();
				out->xltype = in->xltype;
				clone_value(*in, *out);
				return out;
			}

			// this is called from xlAutoFree
			static void freeXL(LPXLOPER12 pXL) {
				try {
					if (pXL->xltype & xlbitXLFree)
						Excel12(xlFree, 0, 1, pXL);
					else if (pXL->xltype & xlbitDLLFree) {
						if (pXL->xltype & xltypeStr) {
							free(pXL->val.str);
							pXL->val.str = 0;
						}
						else if (pXL->xltype & xltypeMulti) {
							int size = pXL->val.array.rows * pXL->val.array.columns;
							if (size == 0) return;
							LPXLOPER12 lparray = pXL->val.array.lparray;
							for (size_t i = 0; i < size; ++i) { // check elements for strings
								XLOPER12& XLi = lparray[i];
								if (XLi.xltype & (xltypeStr | xlbitDLLFree)) {
									if (XLi.val.str == 0) continue;
									free(XLi.val.str);
									XLi.val.str = 0;
								}
								else if (XLi.xltype & xlbitXLFree)
									Excel12(xlFree, 0, 1, &XLi);
							}
							free(pXL->val.array.lparray);
							pXL->val.array.lparray = 0;
						}
						free(pXL);
					}
				}
				catch (...) {
					// THIS IS BAD. LIKELY MEMORY LEAK, but nothing much can be done to handle it.
					std::cout << "Error releasing Excel memory";
				}
			}


		};

		namespace xl_strings {
			// strings are annoying, it's lots of work to detect and convert between different types of them
			inline std::wstring convert(std::string const& s) {
				std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> _converter;
				return _converter.from_bytes(s.c_str());
			}
			inline std::string convert(std::wstring const& w) {
				std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> _converter;
				return _converter.to_bytes(w.c_str());
			}


			static void xlString(std::wstring const& in, XLOPER12& out) {
				xl_type_ops::overwrite_xl_type(out, xltypeStr | xlbitDLLFree);
				out.val.str = xl_memory::new_xl12string(in.c_str());
				if (!out.val.str)
					xl_type_ops::overwrite_xl_type(out, xltypeNil);
			}
			inline void xlString(std::string const& in, XLOPER12& out) {
				xlString(convert(in), out);
			}
			static XLOPER12 xlString(std::wstring const& in) {
				XLOPER12 out;
				xlString(in, out);
				return out;
			}
			inline XLOPER12 xlString(std::string const& in) {
				return xlString(convert(in));
			}
			static void xlpString(std::wstring const& in, LPXLOPER12& out) {
				out = xl_memory::new_xloper12();
				xlString(in, *out);
			}
			inline void xlpString(std::string const& in, LPXLOPER12& out) {
				xlpString(convert(in), out);
			}
			static LPXLOPER12 xlpString(std::wstring const& in) {
				LPXLOPER12 out;
				xlpString(in, out);
				return out;
			}
			inline LPXLOPER12 xlpString(std::string const& in) {
				return xlpString(convert(in));
			}
		};

	}
	
	namespace xl_data {
		// use this for locally created xlopers, works like auto_ptr (kinda)
		struct auto_pxl {
			LPXLOPER12 _pxl;
			auto_pxl(LPXLOPER12 pxl) :_pxl(pxl) {}
			operator LPXLOPER12() { return _pxl; }
			LPXLOPER12 operator->() { return _pxl; }
			XLOPER12 operator*() { return *_pxl; }
			~auto_pxl() { xl_inner::xl_memory::freeXL(_pxl); }
		};

		// dealing with Excel time
		namespace xl_date {
			inline time_t xl_date_to_tm(double xl_date) {
				return  static_cast<time_t>((xl_date - 25569) * 86400); // the magic numbers are all over the interenet, e.g. https://stackoverflow.com/questions/75863926/convert-excels-now-time-format-but-in-c
			}
			static std::string format_xl_date(double xl_date, std::string const& format = "%FT%TZ") {
				time_t time_ = xl_date_to_tm(xl_date);
				tm tm_;
				localtime_s(&tm_, &time_);
				const size_t buffer_sz = 128;
				char out[buffer_sz];
				if (format.size() > buffer_sz)
					throw std::runtime_error("Time format string is too long.");
				strftime(out, buffer_sz, format.c_str(), &tm_);
				return out;
			}
			inline std::string xl_date_to_iso8601_date(double xl_date) {
				return format_xl_date(xl_date, "%F");
			}
			inline std::string xl_date_to_iso8601_date_time(double xl_date) {
				return format_xl_date(xl_date);
			}
			static std::string xl_date_to_iso8601_date_time_ms(double xl_date) {
				time_t t = xl_date_to_tm(xl_date);
				time_t t_ms = static_cast<time_t>((xl_date - 25569.0) * 86400000);
				int ms = static_cast<int>(t_ms - (t * 1000));
				std::string ts = format_xl_date(xl_date, "%FT%T.");
				std::stringstream os;
				os << ts << ms << "Z";
				return os.str();
			}
		}
		// A variant containing supported XLOPER12 types
		// handy for dealing with ranges of mixed values
		class xl_variant {
		public:
			enum xl_type {
				xl_typeNum = xltypeNum,
				xl_typeStr = xltypeStr,
				xl_typeBool = xltypeBool,
				xl_typeErr = xltypeErr,
				xl_typeInt = xltypeInt,
				xl_typeMulti = xltypeMulti,
				xl_typeMissing = xltypeMissing,
				xl_typeNil = xltypeNil,
				xl_typeNone = 0
			};
		private:
			xl_type _type;

			union xl_value {
				double _num;
				std::wstring _str;
				bool _xbool;
				int _err;
				int _w;
				std::vector<std::vector<xl_variant>> _array;
				xl_value() {}
				xl_value(xl_value const&) {}
#pragma warning ( default : 26495) // known problem in visual studio, it doesn't like union constructors
				~xl_value() {}
				xl_value& operator=(xl_value const&) { return *this; }
			};
			xl_value _value;

			inline void check_type(xl_type const& t) const {
				if (t != _type)
					throw std::runtime_error("Incorrect type cast for xl_variant.");
			}
			bool equals(xl_variant const& in) const {
				if (_type != in._type)
					return false;
				switch (_type) {
				case xl_type::xl_typeNum:
					return (fabs(_value._num - in._value._num) < std::numeric_limits<double>::epsilon());
					break;
				case xl_type::xl_typeStr:
					return(_value._str == in._value._str);
					break;
				case xl_type::xl_typeBool:
					return (_value._xbool == in._value._xbool);
					break;
				case xl_type::xl_typeErr:
					return (_value._err == in._value._err);
					break;
				case xl_type::xl_typeInt:
					return (_value._w == in._value._w);
					break;
				case xl_type::xl_typeMulti:
					return (_value._array == in._value._array);
					break;
				case xl_type::xl_typeMissing:
				case xl_type::xl_typeNil:
					return true;
				default:
					throw std::runtime_error("Incorrect type for xl_variant comparisson.");
					break;
				}
			}
			void copy_from(xl_variant const& in) {
				_type = in._type;
				switch (_type) {
				case xl_type::xl_typeNum:
					_value._num = in._value._num;
					break;
				case xl_type::xl_typeStr:
					_value._str = in._value._str;
					break;
				case xl_type::xl_typeBool:
					_value._xbool = in._value._xbool;
					break;
				case xl_type::xl_typeErr:
					_value._err = in._value._err;
					break;
				case xl_type::xl_typeInt:
					_value._w = in._value._w;
					break;
				case xl_type::xl_typeMulti:
					_value._array = in._value._array;
					break;
				case xl_type::xl_typeMissing:
				case xl_type::xl_typeNil:
					break;
				default:
					throw std::runtime_error("Unknown xltype when copying xl_variant.");
					break;
				}
			}
			void from_xl(XLOPER12 const& in) {
				if (in.xltype & xltypeNum) {
					_value._num = in.val.num;
					_type = xl_type::xl_typeNum;
				}
				else if (in.xltype & xltypeStr) {
					_value._str = in.val.str;
					_type = xl_type::xl_typeStr;
				}
				else if (in.xltype & xltypeBool) {
					_value._xbool = in.val.xbool;
					_type = xl_type::xl_typeBool;
				}
				else if (in.xltype & xltypeErr) {
					_value._err = in.val.err;
					_type = xl_type::xl_typeErr;
				}
				else if (in.xltype & xltypeInt) {
					_value._w = in.val.w;
					_type = xl_type::xl_typeInt;
				}
				else if (in.xltype & xltypeMulti) {
					_type = xl_type::xl_typeMulti;
					size_t rows = in.val.array.rows;
					size_t cols = in.val.array.columns;
					_value._array.resize(rows);
					size_t i = 0;
					for (size_t r = 0; r < rows; ++r) {
						auto& row = _value._array[r];
						if (row.size() != cols)
							row.resize(cols);
						for (size_t c = 0; c < cols; ++c)
							row[c].from_xl(in.val.array.lparray[i++]);
					}
				}
				else if (in.xltype & xltypeMissing) {
					_type = xl_type::xl_typeMissing;
				}
				else if (in.xltype & xltypeNil) {
					_type = xl_type::xl_typeNil;
				}
				else {
					throw std::runtime_error("Unsupported xltype type when creating xl_variant.");
				}
			}
		public:
			xl_variant() : _type(xl_type::xl_typeMissing) {
			}
			xl_variant(xl_variant const& in) {
				copy_from(in);
			}
			xl_variant(double num_) : _type(xl_type::xl_typeNum) {
				_value._num = num_;
			}
			xl_variant(std::wstring const& str_) : _type(xl_type::xl_typeStr) {
				_value._str = str_;
			}
			xl_variant(std::string const& str_) : _type(xl_type::xl_typeStr) {
				_value._str = xl_inner::xl_strings::convert(str_);
			}
			xl_variant(bool xbool_) : _type(xl_type::xl_typeBool) {
				_value._xbool = xbool_;
			}
			xl_variant(int w_) : _type(xl_type::xl_typeInt) {
				_value._w = w_;
			}
			xl_variant(std::vector<std::vector<xl_variant>> const& array_) : _type(xl_type::xl_typeMulti) {
				_value._array = array_;
			}
			xl_variant(XLOPER12 const& in) {
				from_xl(in);
			}
			xl_variant(LPXLOPER12 const& in) {
				from_xl(*in);
			}

			~xl_variant() {
				switch (_type) {
				case xl_type::xl_typeStr:
					(&_value._str)->std::wstring::~wstring();
					break;
				case xl_type::xl_typeMulti:
					(&_value._array)->std::vector<std::vector<xl_variant>>::~vector<std::vector<xl_variant>>();
					break;
				default:
					break;
				}
			}

			xl_variant& operator=(xl_variant const& in) {
				copy_from(in);
				return *this;
			}

			bool operator==(xl_variant const& in) const {
				return equals(in);
			}

			void set_type(xl_type const& type_) { _type = type_; }
			xl_type type() const { return _type; }

			void set(double num_) {
				_type = xl_type::xl_typeNum;
				_value._num = num_;
			}

			double get_double() const { check_type(xl_type::xl_typeNum);  return _value._num; }
			double& get_double() { check_type(xl_type::xl_typeNum);  return _value._num; }
			operator double() const { return get_double(); }
			operator double& () { return get_double(); }

			void set_error(int err_) {
				_type = xl_type::xl_typeErr;
				_value._err = err_;
			}
			int get_error() const { check_type(xl_type::xl_typeErr); return _value._err; }

			void set(std::wstring const& str_) {
				_type = xl_type::xl_typeStr;
				_value._str = str_;
			}
			void set(std::string const& str_) {
				_type = xl_type::xl_typeStr;
				_value._str = xl_inner::xl_strings::convert(str_);
			}
			std::wstring const& get_wstring() const { check_type(xl_type::xl_typeStr);  return _value._str; }
			std::wstring& get_wstring() { check_type(xl_type::xl_typeStr); return _value._str; }
			std::string get_string() const { check_type(xl_type::xl_typeStr);  return xl_inner::xl_strings::convert(_value._str); }
			operator std::wstring const& () const { return get_wstring(); }
			operator std::wstring& () { return get_wstring(); }
			operator std::string() const { return get_string(); }

			void set(bool xbool_) {
				_type = xl_type::xl_typeBool;
				_value._xbool = xbool_;
			}
			bool get_bool() const { check_type(xl_type::xl_typeBool);  return _value._xbool; }
			bool& get_bool() { check_type(xl_type::xl_typeBool); return _value._xbool; }
			operator bool() const { return get_bool(); }
			operator bool& () { return get_bool(); }

			void set(int w_) {
				_type = xl_type::xl_typeInt;
				_value._w = w_;
			}
			int get_int() const { check_type(xl_type::xl_typeInt);  return _value._w; }
			int& get_int() { check_type(xl_type::xl_typeInt); return _value._w; }
			operator int() const { return get_int(); }
			operator int& () { return get_int(); }

			void set(std::vector<std::vector<xl_variant>> const& array_) {
				_type = xl_type::xl_typeMulti;
				_value._array = array_;
			}
			std::vector<std::vector<xl_variant>> const& get_multi() const { check_type(xl_type::xl_typeMulti);  return _value._array; }
			std::vector<std::vector<xl_variant>>& get_multi() { check_type(xl_type::xl_typeMulti); return _value._array; }
			operator std::vector<std::vector<xl_variant>> const& () const { return get_multi(); }
			operator std::vector<std::vector<xl_variant>>& () { return get_multi(); }

			void set(XLOPER12 const& in) {
				from_xl(in);
			}

			double to_double() const {
				switch (type()) {
				case xl_type::xl_typeNum:
					return get_double();
				case xl_type::xl_typeInt:
					return get_int();
				case xl_type::xl_typeStr:
					return std::stod(get_wstring());
				case xl_type::xl_typeMulti: {
					auto const& m = get_multi();
					if (m.size() == 1 && m[0].size() == 1)
						return m[0][0].to_double();
				}
				default:
					throw std::runtime_error("Invalid xl_variant conversion.");
				}
				return 0;
			}
			int to_int() const {
				switch (type()) {
				case xl_type::xl_typeInt:
					return get_int();
				case xl_type::xl_typeBool:
					return get_bool();
				case xl_type::xl_typeStr:
					return std::stoi(get_wstring());
				case xl_type::xl_typeMulti: {
					auto const& m = get_multi();
					if (m.size() == 1 && m[0].size() == 1)
						return m[0][0].to_int();
				}
				default:
					throw std::runtime_error("Invalid xl_variant conversion.");
				}
				return 0;
			}
			bool to_bool() const {
				switch (type()) {
				case xl_type::xl_typeInt:
					return get_int();
				case xl_type::xl_typeBool:
					return get_bool();
				case xl_type::xl_typeStr:
					return std::stoi(get_wstring());
				case xl_type::xl_typeMulti: {
					auto const& m = get_multi();
					if (m.size() == 1 && m[0].size() == 1)
						return m[0][0].to_bool();
				}
				default:
					throw std::runtime_error("Invalid xl_variant conversion.");
				}
				return false;
			}
			std::string to_string() const {
				switch (type()) {
				case xl_type::xl_typeNum:
					return std::to_string(get_double());
				case xl_type::xl_typeInt:
					return std::to_string(get_int());
				case xl_type::xl_typeBool:
					return std::to_string(get_bool());
				case xl_type::xl_typeStr:
					return get_string();
				case xl_type::xl_typeMulti: {
					auto const& m = get_multi();
					if (m.size() == 1 && m[0].size() == 1)
						return m[0][0].to_string();
				}
				default:
					throw std::runtime_error("Invalid xl_variant conversion.");
				}
				return "";
			}

		};

		// xl_nvp is a kind of thing used all the time in Excel to pass parameters into functions
		// it is a two-column array where first column contains names of things
		// and the second one values
		// it may be convenient to lay it out horizontally in Excel though, so that 
		// first row contains names and the second one values
		// so when reading them in from Excel, we will try to deduce this automatically as follows:
		//		 if it has two columns, then it's vertical
		//		 else if it has two rows, then it is horizontal
		//		 but, if it is 2x2:
		//			if first column contains only strings and the second one doesn't, then it's vertival
		//			if first row contains only strings and the second one doesn't, then it is horizontal
		//			if it is all strings, then it is vertical (sorry)
		// There are two version of nvps implements:
		// xl_nvp strips all whitespace from keys and treats them as non-case sensitive (most of the time this is useful)
		// xl_nvp_cs is a cases sensitive version, it doesn't do any transformations, just uses strings as they are
		class xl_keyed_values {
			static void skip_white(std::wstring const& s, size_t& i) {
				while (i < s.size() && std::iswspace(s[i]))
					++i;
			}
		protected:
			static bool compare_keys(std::wstring const& lhs, std::wstring const& rhs) {
				size_t i_lhs = 0;
				size_t i_rhs = 0;
				while (true) {
					skip_white(lhs, i_lhs);
					skip_white(rhs, i_rhs);
					if (i_lhs == lhs.size())// if we hit the end of lhs ... 
						return i_rhs == rhs.size();// strings are the same only if we also hit the end of rhs
					if (i_rhs == rhs.size()) { // if we are here, we are not at the end of lhs yet
						return false;
					}
					if (std::towupper(lhs[i_lhs] != std::towupper(rhs[i_rhs]))) return false;
					++i_lhs;
					++i_rhs;
				}
				return false;// never happens, just to stop compiler from moaning
			}
		};
		class xl_nvp : protected xl_keyed_values {
			std::vector<std::vector<xl_variant>> _values; // the strange data structure is to preserve the order and help displaying values
			// the framework is capable of displaying vectors of vectors of xl_variants 

		protected:
			virtual size_t find_key(std::wstring const& key) const {
				for (size_t i = 0; i < _values.size(); ++i)
					if (compare_keys(_values[i][0].get_wstring(), key))
						return i;
				return -1;
			}
			inline bool check_key(std::wstring const& key) {
				return find_key(key) != -1;
			}
		public:
			void add(xl_variant const& key, xl_variant const& value) {
				if (check_key(key.get_wstring()))
					throw std::runtime_error(std::string("Key ") + key.get_string() + " already exists.");
				_values.push_back({ key, value });

			}
			xl_variant const& get(std::wstring const& key) const {
				size_t i = find_key(key);
				if (i == -1)
					throw std::runtime_error(std::string("Key ") + xl_inner::xl_strings::convert(key) + " is not present.");
				return _values[i][1];
			}
			xl_variant& get(std::wstring const& key) {
				size_t i = find_key(key);
				if (i == -1)
					throw std::runtime_error(std::string("Key ") + xl_inner::xl_strings::convert(key) + " is not present.");
				return _values[i][1];
			}
			inline std::vector<std::vector<xl_variant>> const& values() const { return _values; }
			inline std::vector<std::vector<xl_variant>>& values() { return _values; }
			inline xl_variant const& operator[](std::wstring const& key) const { return get(key); }
			inline xl_variant const& operator[](std::string const& key)  const { return get(xl_inner::xl_strings::convert(key)); }
			inline size_t size() const { return _values.size(); }

			static bool transpose_input(XLOPER12 const& in) {
				// assume in has already been checked for being empty and for type
				// for the logic, see comment above the class declaration
				if (in.val.array.columns == 2) {
					if (in.val.array.rows == 2) {
						if (xl_inner::xl_type_ops::is_xl_type(in.val.array.lparray[0], xltypeStr) &&
							xl_inner::xl_type_ops::is_xl_type(in.val.array.lparray[2], xltypeStr)) {
							return true;
						}
						return false;
					}
					return true;
				}
				return false;
			}
		};
		class xl_nvp_cs : public xl_nvp {
		protected:
			virtual size_t find_key(std::wstring const& key) const {
				for (size_t i = 0; i < values().size(); ++i)
					if (values()[i][0] == key)
						return i;
				return -1;
			}
		};

		// xl_table is a commonly used abstraction in Excel sheets: a table of values with column names
		// it is assumed to be horizontal, i.e. with titles across the top
		// when passing it in from Excel, we will to a simple check and adapt if we can:
		//		if the first row is all strings, all is good, we treat it as horizontal
		//		otherwise, we check if the first column is all strings; and if it is, we interpret it as being vertical
		//			meaning, the first column contains titles and values are horizontally to the right of it
		//		otherwise, we still load it, but fucntions using headings will fail
		// There are two version of tables implements:
		// xl_table strips all whitespace from keys and treats them as non-case sensitive (most of the time this is useful)
		// xl_table_cs is a cases sensitive version, it doesn't do any transformations, just uses strings as they are
		class xl_table : protected xl_keyed_values {
			std::vector<std::vector<xl_variant>> _table;   // the strange data structure is to preserve the order and help displaying values
			// the framework is capable of displaying vectors of vectors of xl_variants 
			// first rows contains titles
			// titles do not have to be unique


		protected:
			virtual std::vector<size_t> find_key(std::wstring const& key) const {
				std::vector<size_t> ret;
				if (_table.empty()) return ret;
				auto const& headings_ = _table[0];
				for (size_t i = 0; i < columns(); ++i)
					if (compare_keys(headings_[i].get_wstring(), key))
						ret.push_back(i);
				return ret;
			}

		public:
			std::vector<xl_variant> const& headings() const { return _table.at(0); }
			std::vector<xl_variant>& headings() { return _table.at(0); }
			std::vector<xl_variant> const& row(size_t i) const { return _table.at(i + 1); }
			std::vector<xl_variant>& row(size_t i) { return _table.at(i + 1); }

			size_t rows() const { return _table.size() - 1; }
			size_t columns() const { return _table.size() == 0 ? 0 : headings().size(); }

			std::vector<std::vector<xl_variant>> get_column(std::string const& key) const {
				return get_column(xl_inner::xl_strings::convert(key));
			}

			std::vector<std::vector<xl_variant>> get_column(std::wstring const& key) const {
				auto const column_idxs = find_key(key);
				std::vector<std::vector<xl_variant>> ret;
				if (column_idxs.empty()) return ret;
				ret.resize(column_idxs.size());
				for (size_t r = 0; r < rows(); ++r) {
					auto const& row_ = row(r);
					for (size_t i = 0; i < column_idxs.size(); ++i)
						ret[i].push_back(row_[column_idxs[i]]);
				}
				return ret;
			}
			void add_row(std::vector<xl_variant> const& row_) {
				if (columns() != 0 && row_.size() != columns())
					throw std::runtime_error("Row doesn't have the correct number of columns.");
				_table.push_back(row_);
			}
			void add_column(std::vector<xl_variant> const& column_) {
				if (rows() != 0 && column_.size() != rows())
					throw std::runtime_error("Column doesn't have the correct number of rows.");
				for (size_t i = 0; i < rows(); ++i) {
					_table[i].push_back(column_[i]);
				}
			}


			inline std::vector<std::vector<xl_variant>> const& table() const { return _table; }
			inline std::vector<std::vector<xl_variant>>& table() { return _table; }

			static bool transpose_input(XLOPER12 const& in) {
				// assume in has already been checked for being empty and for type
				// for the logic, see comment above the class declaration
				bool all_strings = true;
				for (size_t i = 0; i < in.val.array.columns; ++i) {
					if (!xl_inner::xl_type_ops::is_xl_type(in.val.array.lparray[0], xltypeStr)) {
						all_strings = false;
						break;
					}
				}
				if (all_strings)
					return false;

				all_strings = true;
				size_t sz = in.val.array.columns * in.val.array.rows;
				for (size_t i = 0; i < sz; i += in.val.array.columns) {
					if (!xl_inner::xl_type_ops::is_xl_type(in.val.array.lparray[0], xltypeStr)) {
						all_strings = false;
						break;
					}
				}
				if (all_strings)
					return true;

				return false;
			}
		};
		class xl_table_cs : public xl_table {
		protected:
			std::vector<size_t> find_key(std::wstring const& key) const override {
				std::vector<size_t> ret;
				if (table().empty()) return ret;
				auto const& headings_ = headings();
				for (size_t i = 0; i < headings_.size(); ++i)
					if (headings_[i].get_wstring() == key)
						ret.push_back(i);
				return ret;
			}
		};

	}
	
	namespace xl_util {

		// called_from_wizard detects if a function was invoked from Excel function wizard
		struct called_from_wizard {
		private:
			// nasty but standard way to stop functions evaluating in the wizard dialog
			struct wizard_detection_impl {
				// detecting function wizard calls, taken from: https://learn.microsoft.com/en-us/office/client-developer/excel/how-to-call-xll-functions-from-the-function-wizard-or-replace-dialog-boxes
				// Data structure used as input to xldlg_enum_proc(), called by
				// called_from_paste_fn_dlg(), called_from_replace_dlg(), and
				// called_from_Excel_dlg(). These functions tell the caller whether
				// the current worksheet function was called from one or either of
				// these dialog boxes.
				typedef struct
				{
					bool is_dlg;
					short low_hwnd;
					char const* const window_title_text; // set to NULL if don't care
				}
				xldlg_enum_struct;

				// The callback function called by Windows for every top-level window.
				static BOOL CALLBACK xldlg_enum_proc(HWND hwnd, xldlg_enum_struct* p_enum)
				{
					auto constexpr CLASS_NAME_BUFFSIZE = 50;
					auto constexpr WINDOW_TEXT_BUFFSIZE = 50;
					// Check if the parent window is Excel.
					// Note: Because of the change from MDI (Excel 2010)
					// to SDI (Excel 2013), comment out this step in Excel 2013.
					//if (LOWORD((DWORD)GetParent(hwnd)) != p_enum->low_hwnd)
					//	return TRUE; // keep iterating
					char class_name[CLASS_NAME_BUFFSIZE + 1];
					//  Ensure that class_name is always null terminated for safety.
					class_name[CLASS_NAME_BUFFSIZE] = 0;
					GetClassNameA(hwnd, class_name, CLASS_NAME_BUFFSIZE);
					//  Do a case-insensitve comparison for the Excel dialog window
					//  class name with the Excel version number truncated.
					size_t len; // The length of the window's title text
					if (_strnicmp(class_name, "bosa_sdm_xl", 11) == 0)
					{
						// Check if a searching for a specific title string
						if (p_enum->window_title_text)
						{
							// Get the window's title and see if it matches the given text.
							char buffer[WINDOW_TEXT_BUFFSIZE + 1];
							buffer[WINDOW_TEXT_BUFFSIZE] = 0;
							len = GetWindowTextA(hwnd, buffer, WINDOW_TEXT_BUFFSIZE);
							if (len == 0) // No title
							{
								if (p_enum->window_title_text[0] != 0)
									return TRUE; // No match, so keep iterating
							}
							// Window has a title so do a case-insensitive comparison of the
							// title and the search text, if provided.
							else if (p_enum->window_title_text[0] != 0
								&& _stricmp(buffer, p_enum->window_title_text) != 0)
								return TRUE; // Keep iterating
						}
						p_enum->is_dlg = true;
						return FALSE; // Tells Windows to stop iterating.
					}
					return TRUE; // Tells Windows to continue iterating.
				}
				static bool called_from_paste_fn_dlg(void)
				{
					// commented out parts are not actually used in versions of excel higher than 10
					// leaving them in because this piece of code is very often cited in documentation, books, forums ... 
					//XLOPER xHwnd;
					// Calls Excel12, which only returns the low part of the Excel
					// main window handle. This is OK for the search however.

					//if (Excel12(xlGetHwnd, &xHwnd, 0))
					//	return false; // Couldn't get it, so assume not
					// Search for bosa_sdm_xl* dialog box with title Function Arguments.
					//xldlg_enum_struct es = { FALSE, xHwnd.val.w, "Function Arguments" };

					wizard_detection_impl::xldlg_enum_struct es = { FALSE, 0, "Function Arguments" };
					EnumWindows((WNDENUMPROC)wizard_detection_impl::xldlg_enum_proc, (LPARAM)&es);
					return es.is_dlg;
				}
			};
		public:
			static bool check() {
				return wizard_detection_impl::called_from_paste_fn_dlg();
			}
			operator bool() const {
				return check();
			}
		};

		// display a message box
		inline void alert(std::string const& text, std::string const& title = "Warning") {
			MessageBoxA(NULL, text.c_str(), title.c_str(), MB_ICONEXCLAMATION | MB_OK | MB_SETFOREGROUND);
		}

		// transposing linear and simple grids
		template< typename TValue>
		struct xl_transposed {
			using value_type = TValue;
			using stripped_value_type = typename std::remove_reference<typename TValue>::type;

			stripped_value_type const _value;
			xl_transposed(stripped_value_type const& value_ = stripped_value_type()) :_value(value_) {}
			stripped_value_type const& value() const { return _value; }
			operator stripped_value_type const& () const { return _value; }
		};

		template<typename TValue>
		inline xl_transposed<TValue> xl_transpose(TValue const& value) { return xl_transposed<TValue>{value}; }

		// sometimes you may want to transpose an XLOPER12 itself
		static LPXLOPER12 xl_transpose(LPXLOPER12 in) {
			if (!xl_inner::xl_type_ops::is_xl_type(*in, xltypeMulti))
				return in;
			if (((in->val.array.rows) * (in->val.array.columns)) == 0)
				return in;

			LPXLOPER12 out = xl_inner::xl_memory::new_xloper12();
			xl_inner::xl_type_ops::set_xl_type(*out, xltypeMulti);
			out->val.array.columns = in->val.array.rows;
			out->val.array.rows = in->val.array.columns;
			for (size_t r = 0; r < out->val.array.rows; ++r) {
				for (size_t c = 0; c < out->val.array.columns; ++c) {
					size_t i_out = c + r * out->val.array.columns;
					size_t i_in = r + c * in->val.array.columns;
					xl_inner::xl_memory::clone_value(in->val.array.lparray[i_in], out->val.array.lparray[i_out]);
				}
			}
			return in;
		}
	}
	
	namespace xl_errors {
		// there a few options on what to do with errors and empty values
		// all wrapped up in this namespace
		static LPXLOPER12 xlpError(DWORD errCode) {
			LPXLOPER12 out = xl_inner::xl_memory::new_xloper12();
			xl_inner::xl_type_ops::set_xl_type(*out, xltypeErr);
			out->val.err = errCode;
			return out;
		}

		class error_policy {
			// by default, all exceptions get transformed to #N/A
			// this helps spreadsheets work, because ISERROR function will detect these properly
			// but for debugging it may be helpful to display more information
			// then we can set the policy to show "rich" information
			// by returning strings prefixed by #ERR and containing exception text
			bool _rich_error_text; // when an exception is caught, return "ERR:" + exception.what()

			bool _propagate_errors; // when there is an error in the input XLOPER12, return it as function output
			bool _interpret_missing; // missing values are coerced to default values for types

			// disabling wizard calls is not strictly speaking about error handling
			// we have it here because it is really not worth adding complexity with a separate configuration class
			// but we will pretend that it's because slow execution is a kind of an error
			bool _disable_wizard_calls; // do not evaluate functions if invoked from the function wizard
		private:
			error_policy(
				bool rich_error_text_,
				bool propagate_errors_,
				bool interpret_missing_,
				bool disable_wizard_calls_) :
				_rich_error_text(rich_error_text_),
				_propagate_errors(propagate_errors_),
				_interpret_missing(interpret_missing_),
				_disable_wizard_calls(disable_wizard_calls_) {
			}
			static error_policy& instance() {
				static error_policy _instance{ false, true, true, true };
				return _instance;
			}
		public:
			static bool rich_error_text() {
				return instance()._rich_error_text;
			}
			static void set_rich_error_text(bool rich_error_text_) {
				instance()._rich_error_text = rich_error_text_;
			}
			static bool propagate_errors() {
				return instance()._propagate_errors;
			}
			static void set_propagate_errors(bool propagate_errors_) {
				instance()._propagate_errors = propagate_errors_;
			}
			static bool interpret_missing() {
				return instance()._interpret_missing;
			}
			static void set_interpret_missing(bool interpret_missing_) {
				instance()._interpret_missing = interpret_missing_;
			}
			static bool disable_wizard_calls() {
				return instance()._disable_wizard_calls;
			}
			static void set_disable_wizard_calls(bool disable_wizard_calls_) {
				instance()._disable_wizard_calls = disable_wizard_calls_;
			}
			static void set(
				bool rich_error_text_,
				bool propagate_errors_,
				bool interpret_missing_,
				bool disable_wizard_calls_) {
				set_rich_error_text(rich_error_text_);
				set_propagate_errors(propagate_errors_);
				set_interpret_missing(interpret_missing_);
				set_disable_wizard_calls(disable_wizard_calls_);
			}

			// to make these work more easily with excel, we tag the configuration parameters
			// helps a little in case we want to add something else later
			//										tag				getter							setter
			using tags_map = std::unordered_map < std::string, std::pair<std::function<bool()>, std::function<void(bool)>>>;

			static tags_map const& tags() {
				static tags_map _tags = {
					{"RichErrorText", {rich_error_text, set_rich_error_text}},
					{"PropagateErrors", {propagate_errors, set_propagate_errors}},
					{"InterpretMissing", {interpret_missing, set_interpret_missing}},
					{"DisableWizardCalls", {disable_wizard_calls, set_disable_wizard_calls}} };

				return _tags;
			}

		};
		// these are exposed by the XLL by default
		static bool af_xl_configure_error_policy(
			bool rich_error_text_,
			bool propagate_errors_,
			bool interpret_missing_,
			bool disable_wizard_calls_) {
			error_policy::set(rich_error_text_, propagate_errors_, interpret_missing_, disable_wizard_calls_);
			return true;
		}

		static std::unordered_map<std::string, bool> af_xl_display_error_policy() {
			std::unordered_map<std::string, bool>  out;
			for (auto const& tag : error_policy::tags()) {
				out[tag.first] = (tag.second.first)();
			}
			return out;
		}

		static std::unordered_map<std::string, bool>
			af_xl_configure_error_policy_ex(std::unordered_map<std::string, bool> const& policy_details) {
			for (auto const& in : policy_details) {
				auto it = error_policy::tags().find(in.first);
				if (it == error_policy::tags().end())
					throw std::runtime_error(std::string("Unknown configuration tag: ") + in.first);
				(it->second.second)(in.second);
			}
			return af_xl_display_error_policy();
		}


		// depending on the policy, we translate error text in various ways
		// that is all embedded in here
		static inline LPXLOPER12 translate_error(const char* const error_text) {
			static constexpr auto error_prefix = "#ERR: ";
			if (!error_policy::rich_error_text())
				return const_cast<LPXLOPER12>(&xl_inner::xl_constants::xlNA());

			try {
				std::string err(error_text);
				err = error_prefix + err;
				return xl_inner::xl_strings::xlpString(err);
			}
			catch (...) {
				return const_cast<LPXLOPER12>(&xl_inner::xl_constants::xlNull());
			}
		}
		static LPXLOPER12 translate_error(std::exception const& error_exception) {
			return translate_error(error_exception.what());
		}
		static LPXLOPER12 translate_error() {
			return translate_error("Unknown error.");
		}

		// we handle errors by using exceptions later in the code
		// this is to wrap XLOPER12 errors, so we can propagate them
		class xloper_exception : public std::exception {
			LPXLOPER12 _errorXlp;
		public:
			xloper_exception(DWORD errCode) :_errorXlp(xlpError(errCode)) {}
			xloper_exception(const XLOPER12* const xlp) :_errorXlp(xlpError(xlp->val.err)) {}
			const char* what() const {
				switch (_errorXlp->val.err) {
				case xlerrNull: return "Excel Error: Null";
				case xlerrDiv0: return "Excel Error: Division by 0";
				case xlerrValue: return "Excel Error: Value error";
				case xlerrRef: return "Excel Error: Invalid Reference";
				case xlerrName: return "Excel Error: Invalid Name";
				case xlerrNum: return "Excel Error: Invalid Number";
				case xlerrNA: return "Excel Error: NA";
				case xlerrGettingData: return "Excel Error: Error Getting Data";
				default: return "Excel Error: Unknown Excel Error";
				}
			}
			LPXLOPER12 errorXlp() const {
				if (error_policy::propagate_errors())
					return _errorXlp;
				else
					return translate_error(*this);
			}
		};



	}
		
	namespace xl_util {
		// finally, some common checks
		static void check_null_xlp(const XLOPER12* const xlp) {
			using namespace xl_errors;
			if (!xlp)
				if (error_policy::rich_error_text())
					throw std::runtime_error("NULL input.");
				else
					throw xloper_exception(&xl_inner::xl_constants::xlNull());
		}
		static void check_input_xl(XLOPER12 const& xl) {
			using namespace xl_errors;
			if (xl.xltype & xltypeErr) {
				if (error_policy::propagate_errors())
					throw xloper_exception(&xl);
				else if (error_policy::rich_error_text())
					throw std::runtime_error("Error in input.");
				else
					throw xloper_exception(&xl_inner::xl_constants::xlNA());
			}
		}
		static void check_wizard() {
			using namespace xl_errors;
			if (error_policy::disable_wizard_calls() &&
				called_from_wizard())
				throw xloper_exception(&xl_inner::xl_constants::xlNA());
		}
	}
	
	namespace xl_object_caches {

		// A cache for storing objects and accessing them by a versioned tag
		// A tag is the user given name for the object followed by ":" and a version number. 
		// The version number starts at 0 when you first instert the object into cache, then gets incremented when the object is updated.
		struct xl_objects_cache_base {
			virtual size_t size() const = 0;
			virtual void clear_cache() = 0;
			virtual std::list<std::string> list_cache() const = 0;
		};

		// Caches are typed
		template<typename ObjectT>
		class xl_objects_cache : public xl_objects_cache_base {
			struct tagged_object {
				std::shared_ptr<ObjectT> _object;
				std::string _name;
				size_t _version;
				const char* const _version_separator = ":";
				std::string tag() const {
					char t[10];
					itoa(_version, t, 10);
					return _name + _version_separator + t;
				}
				static std::string name_from_tag(std::string tag_) {
					std::size_t found = tag_.find_last_of(_version_separator);
					if (found == std::string::npos)
						return tag_;
					return tag_.substr(0, found);

				}
			};
			template<typename ObjecT>
			struct xl_objects_cache_impl {
				std::unordered_map<std::string, tagged_object> _cache;
			};
			static xl_objects_cache_impl<ObjectT>& instance() {
				static xl_objects_cache_impl<ObjectT> _inst;
				return _inst;
			}
		public:
			std::shared_ptr<ObjectT> get(std::string const& name_or_tag) {
				std::string name = tagged_object::name_from_tag(name_or_tag);
				auto it = instance()._cache.find(name);
				if (it == instance()._cache.end())
					return nullptr;
				return it->second._object;
			}

			std::string add(std::string const& name_or_tag, std::shared_ptr<ObjectT> object) {
				std::string name = tagged_object::name_from_tag(name_or_tag);
				auto it = instance()._cache.find(name);
				if (it == instance()._cache.end()) {
					tagged_object entry{ object, name, 0 };
					instance()._cache[name] = entry;
				}
				else {
					tagged_object entry(it->second);
					entry._version++;
					entry._object = object;
					instance()._cache[name] = entry;
				}
			}

			std::string update(std::string const& name_or_tag, std::shared_ptr<ObjectT> object) {
				std::string name = tagged_object::name_from_tag(name_or_tag);
				auto it = instance()._cache.find(name);
				if (it == instance()._cache.end()) {
					throw std::runtime_error(std::string("Object ") + name + " doesn't exist when trying to update it.");
				}
				else {
					tagged_object entry(it->second);
					entry._version++;
					entry._object = object;
					instance()._cache[name] = entry;
				}
			}
			void clear() {
				instance()._cache.clear();
			}

			size_t size() const override { return instance()->_cache.size(); }
			void clear_cache() override { clear(); }
			std::list<std::string> list_cache() const override {
				std::list<std::string> ret;
				for (auto const& c : instance()->_cache)
					ret.push_back(c.first);
				return ret;
			}
		};

		// to provide simple ways to list and clear caches
		// we have cache of caches
		class xl_objects_caches {
			using cache_cache_t = std::unordered_map<std::string, std::shared_ptr<xl_objects_cache_base>>;
			static cache_cache_t& cache_cache() {
				static cache_cache_t inst;
				return inst;
			}
		public:
			static bool exists(std::string const& cache_name) {
				auto it = cache_cache().find(cache_name);
				return it != cache_cache().end();
			}
			template<typename ObjectT>
			static xl_objects_cache<ObjectT>& get(std::string const& cache_name) {
				auto it = cache_cache().find(cache_name);
				if (it == cache_cache().end()) {
					std::shared_ptr<xl_objects_cache<ObjectT>> ret(new xl_objects_cache<ObjectT>());
					cache_cache()[cache_name] = ret;
					return ret;
				}
				else {
					std::shared_ptr<xl_objects_cache<ObjectT>> ret(std::dynamic_pointer_cast<xl_objects_cache<ObjectT>>(it->second));
					if (!ret)
						throw std::runtime_error(std::string("Cache ") + cache_name + (" is of wrong type."));
					return ret;
				}
			}
			template<typename ObjectT>
			static std::shared_ptr<ObjectT> get(std::string const& cache_name, std::string const& object_name_or_tag) {
				return get<ObjectT>(cache_name).get(object_name_or_tag);
			}

			template<typename ObjectT>
			static std::string add(std::string const& cache_name, std::string const& object_name_or_tag, std::shared_ptr<ObjectT> object) {
				return get<ObjectT>(cache_name).add(object_name_or_tag, object);
			}
			template<typename ObjectT>
			static std::string update(std::string const& cache_name, std::string const& object_name_or_tag, std::shared_ptr<ObjectT> object) {
				return get<ObjectT>(cache_name).update(object_name_or_tag, object);
			}

			static size_t cache_size(std::string const& cache_name) {
				auto it = cache_cache().find(cache_name);
				if (it == cache_cache().end())
					return 0;
				else
					return it->second->size();
			}
			static void clear_cache(std::string const& cache_name) {
				auto it = cache_cache().find(cache_name);
				if (it != cache_cache().end())
					it->second->clear_cache();
			}
			static std::list<std::string> list_cache(std::string const& cache_name) {
				auto it = cache_cache().find(cache_name);
				if (it != cache_cache().end())
					return it->second->list_cache();
				return {};
			}
			static std::unordered_map<std::string, size_t> cache_sizes() {
				std::unordered_map<std::string, size_t> ret;
				for (auto const& c : cache_cache())
					ret[c.first] = c.second->size();
				return ret;
			}
			static void clear_all_caches() {
				for (auto const& c : cache_cache())
					c.second->clear_cache();
			}
			static std::unordered_map<std::string, std::list<std::string>> list_all_caches() {
				std::unordered_map<std::string, std::list<std::string>> ret;
				for (auto const& c : cache_cache())
					ret[c.first] = c.second->list_cache();
				return ret;
			}
		};
		// these are exposed to XLLs by default
		inline size_t af_xl_object_cache_size(std::string const& cache_name) { return xl_objects_caches::cache_size(cache_name); }

		inline std::unordered_map<std::string, size_t> af_xl_object_cache_sizes() { return xl_objects_caches::cache_sizes(); }

		inline bool af_xl_clear_object_cache(std::string const& cache_name) { xl_objects_caches::clear_cache(cache_name); return true; }

		inline bool af_xl_clear_all_object_caches() { xl_objects_caches::clear_all_caches(); return true; }

		inline std::list<std::string> af_xl_list_objects_cache(std::string const& cache_name) { return xl_objects_caches::list_cache(cache_name); }

		inline std::unordered_map<std::string, std::list<std::string>> af_xl_list_all_objects_caches() { return xl_objects_caches::list_all_caches(); }

	
	}

	namespace xl_fast_array {
		// xl_fast_array is a wrapper for FP12 arrays
		// they are a pain to use, but apparently the fastest way to pass lots of number to and from excel
		class xl_fast_array {
			_FP12* _array;
			// auto free means delete the pointer on exit. if you need to use this semantics, do it, but it's a pain.
			bool _auto_free;
			// for fast resizing we may change the values inside _array, but not realocate space
			int32_t _actual_rows;
			int32_t _actual_columns;
			// a trick to access array values by double indexing fast
			// store a pointer to the start of each row in an array, they use those
			std::vector<double*> _rows;

			// dealing with memory for FP12 arrays is a real pain
			// xlAutoFree doesn't work for these
			// once you create one, you own it, but you can't delete it before you return it
			// it's so easy to loose track of them
			// instead, create them in a static cache, then once in a while clear that cache up
			static std::list<std::shared_ptr<xl_fast_array>>& cache() {
				static std::list<std::shared_ptr<xl_fast_array>> _cache;
				return _cache;
			}

			void copy(xl_fast_array& in) {
				_array = in._array;
				_auto_free = in._auto_free;
				_actual_rows = in._actual_rows;
				_actual_columns = in._actual_columns;
				_rows = in._rows;
				in._array = nullptr;
				in._auto_free = false;
				in._actual_rows = 0;
				in._actual_columns = 0;
				in._rows.clear();
			}
			void release_impl() {
				if (_array) {
					free(_array);
					_array = nullptr;
					_actual_rows = 0;
					_actual_columns = 0;
				}
			}
			void make_new(int32_t rows, int32_t columns) {
				free(_array);
				int64_t size = rows * columns;
				// checkout https://www.wiley.com/en-gb/Financial+Applications+using+Excel+Add-in+Development+in+C+%2F+C%2B%2B%2C+2nd+Edition-p-9780470319048
				// section 6.2.2 for the calcualtion below
				int64_t mem_size = sizeof(_FP12) + (size - 1) * sizeof(double);
				_array = (_FP12*)malloc(mem_size);
				if (!_array)
					throw std::runtime_error("Could not allocate _FP12 array large enough.");
				_actual_rows = rows;
				_actual_columns = columns;
				_array->rows = rows;
				_array->columns = columns;
				_rows.clear();
			}
			void init_impl(double v = 0) {
				if (_array)
					std::fill_n(_array->array, _array->rows * _array->columns, v);
			}

		public:
			xl_fast_array(_FP12* array_ = nullptr, bool auto_free_ = false) :
				_array(array_),
				_auto_free(auto_free_),
				_actual_rows(0),
				_actual_columns(0) {
				if (_array) {
					_actual_rows = _array->rows;
					_actual_columns = _array->columns;
				}
				_rows.clear();
			}
			xl_fast_array(int32_t rows, int32_t columns, bool auto_free_ = false) :
				_array(nullptr),
				_auto_free(auto_free_),
				_actual_rows(0),
				_actual_columns(0) {
				make_new(rows, columns);
			}
			xl_fast_array(int32_t rows, int32_t columns, double init_value, bool auto_free_ = false) :
				_array(nullptr),
				_auto_free(auto_free_),
				_actual_rows(0),
				_actual_columns(0) {
				make_new(rows, columns);
				init_impl(init_value);
			}
			~xl_fast_array() {
				if (_auto_free)
					release_impl();
			}
			xl_fast_array(xl_fast_array const& in) {
				copy(const_cast<xl_fast_array&>(in));
			}
			xl_fast_array& operator=(xl_fast_array const& in) {
				copy(const_cast<xl_fast_array&>(in));
			}

			// allocate tries to reuse existing memory, if your stuff can fit into it
			// very handy for reusing in paramters, but tricky to keep track of
			// if it returns true, you got to remember to delete it yourself
			bool allocate(int32_t rows, int32_t columns) {
				if (rows < _actual_rows && columns < _actual_columns) {
					_rows.clear();
					_array->rows = rows;
					_array->columns = columns;
					return false;
				}
				else {
					make_new(rows, columns);
					return true;
				}
			}

			inline FP12* fp12() const { return _array; }
			inline void release() { release_impl(); }
			inline void init(double v) { init_impl(v); }

			inline int32_t rows() const { return _array ? _array->rows : 0; }
			inline int32_t unsafe_rows() const { return _array->rows; }
			inline int32_t columns() const { return _array ? _array->columns : 0; }
			inline int32_t unsafe_columns() const { return _array->columns; }
			inline int32_t actual_rows() const { return _actual_rows; }
			inline int32_t actual_columns() const { return _actual_columns; }

			inline int64_t size() const { return _array ? (unsafe_rows() * unsafe_columns()) : 0; }
			inline int64_t actual_size() const { return _actual_rows * _actual_columns; }
			inline bool empty() const { return !_array || (size() == 0); }

			inline double const* values() const { return _array ? _array->array : nullptr; }
			inline double const* unsafe_values() const { return _array->array; }
			inline double* values() { return _array ? _array->array : nullptr; }
			inline double* unsafe_values() { return _array->array; }

			inline double get(int64_t i) const {
				if (!_array || i >= size())
					throw std::runtime_error("Out of bounds or non existent array access.");
				return _array->array[i];
			}
			inline double unsafe_get(int64_t i) const {
				return _array->array[i];
			}
			inline double& get(int64_t i) {
				if (!_array || i >= size())
					throw std::runtime_error("Out of bounds or non existent array access.");
				return _array->array[i];
			}
			inline double& unsafe_get(int64_t i) {
				return _array->array[i];
			}
			inline void set(int64_t i, double const d) { get(i) = d; }
			inline void unsafe_set(int64_t i, double const d) { unsafe_get(i) = d; }

			inline double get(int32_t row, int32_t column) const {
				if (!_array || row >= rows() || column >= columns())
					throw std::runtime_error("Out of bounds or non existent array access.");
				int64_t i = column + row * columns();
				return _array->array[i];
			}
			inline double unsafe_get(int32_t row, int32_t column) const {
				int64_t i = column + row * columns();
				return _array->array[i];
			}
			inline double& get(int32_t row, int32_t column) {
				if (!_array || row >= rows() || column >= columns())
					throw std::runtime_error("Out of bounds or non existent array access.");
				int64_t i = column + row * columns();
				return _array->array[i];
			}
			inline double& unsafe_get(int32_t row, int32_t column) {
				int64_t i = column + row * columns();
				return _array->array[i];
			}
			inline void set(int32_t row, int32_t column, double const d) { get(row, column) = d; }
			inline void unsafe_set(int32_t row, int32_t column, double const d) { unsafe_get(row, column) = d; }

			// to use clever indexing, you have to prepare the container
			// it may take a while for large arrays, so only worth it if you are going to index a lot
			void prepare_indexing() {
				if (empty()) {
					_rows.clear();
					return;
				}
				_rows.reserve(rows());
				for (size_t i = 0; i < rows(); ++i) {
					_rows.push_back(unsafe_values() + i * columns());
				}
			}
			double* operator[](int r) { return _rows[r]; }
			double const* operator[] (int r) const { return _rows[r]; }

			double* row(int r) { if (r > _rows.size()) throw std::runtime_error("Invalid row access."); return _rows[r]; }
			double const* row(int r) const { if (r > _rows.size()) throw std::runtime_error("Invalid row access."); return _rows[r]; }

			// handling the memory of these FP12 structures is a real pain
			// common suggestions is to create static instances, eek!
			// we'll provide a cache to store instances ... you do have to remember to clear it once in a while though
			static size_t cache_size() { return cache().size(); }
			static void clear_cache() { cache().clear(); }
			static std::shared_ptr<xl_fast_array> make_cached(int32_t rows, int32_t columns) {
				std::shared_ptr<xl_fast_array> ret(new xl_fast_array(rows, columns));
				cache().push_back(ret);
				return ret;
			}
			static std::shared_ptr<xl_fast_array> make_cached(int32_t rows, int32_t columns, double v) {
				std::shared_ptr<xl_fast_array> ret(new xl_fast_array(rows, columns, v));
				cache().push_back(ret);
				return ret;
			}

		};

	}

	namespace xl_object_caches {
		// these are exposed by the XLL by default
		inline size_t af_xl_fast_array_cache_size() { return xl_fast_array::xl_fast_array::cache_size(); }

		inline bool af_xl_clear_fast_array_cache() { xl_fast_array::xl_fast_array::clear_cache(); return true; }

		inline bool af_xl_clear_all_caches() { return af_xl_clear_fast_array_cache() && af_xl_clear_all_object_caches(); }
	
	}
	
	namespace xl_conversions {
		// this is the beating heart of the magic part where we expose functions automatically
		// conversions between supported types and XLOPER12s (and FP12s)

		// for some types we can use sensible defaults for missing values
		// meaning_of_missing is a trait to encode the sensible defaults
		template<typename T>
		T meaning_of_missing() {
			throw std::runtime_error("Unexpected Nil or Missing value.");
		}

#define __AF_MEANING_OF_MISSING(T, V) template<> inline T meaning_of_missing<T>() { return V;}

		__AF_MEANING_OF_MISSING(bool, false);
		__AF_MEANING_OF_MISSING(float, 0);
		__AF_MEANING_OF_MISSING(double, 0);
		__AF_MEANING_OF_MISSING(unsigned short, 0);
		__AF_MEANING_OF_MISSING(short, 0);
		__AF_MEANING_OF_MISSING(int, 0);
		__AF_MEANING_OF_MISSING(size_t, 0);
		__AF_MEANING_OF_MISSING(std::string, "");
		__AF_MEANING_OF_MISSING(std::wstring, L"");
		__AF_MEANING_OF_MISSING(xl_data::xl_variant, xl_data::xl_variant());
		__AF_MEANING_OF_MISSING(xl_data::xl_nvp, xl_data::xl_nvp());
		__AF_MEANING_OF_MISSING(xl_data::xl_nvp_cs, xl_data::xl_nvp_cs());
		__AF_MEANING_OF_MISSING(xl_data::xl_table, xl_data::xl_table());
		__AF_MEANING_OF_MISSING(xl_data::xl_table_cs, xl_data::xl_table_cs());

		inline bool is_xl_missing(const XLOPER12* const in) {
			return xl_inner::xl_type_ops::is_xl_type(*in, xltypeMissing | xltypeNil);
		}
		template<typename T>
		std::remove_const_t<std::remove_reference_t<T>> handle_missing(const XLOPER12* const in) {
			if (!xl_errors::error_policy::interpret_missing()) {
				throw std::runtime_error("Unexpected Nil or Missing value.");
			}
			return meaning_of_missing<std::remove_const_t<std::remove_reference_t<T>>>();
		}

#define CHECK_INPUT(XL12, OutV) xl_util::check_input_xl(XL12);if(is_xl_missing(&XL12)){ OutV = handle_missing<decltype(OutV)>(&XL12); return;}

		// supported types:
		// supported contained types: int, unsigned short, short, double, float, string, wstring
		//
		// linear types: list, vector
		//		1D types linear types with any of the supported contained types
		//		2D types, any linear type containing any supperted 1D type
		//
		// associative: map, unordered_map
		//		keys are any of the supported contained types
		//		values are:
		//			any of the supported linear types
		//          tables: another associative container but only up to two levels deep (e.g. map<string, map<string, vector<double>>)

		// conversion functions
		// for every supported type we need to implement
		// 
		// void to_xl(TYPE const& in, XLOPER12& out)
		// void from_xl(XLOPER12 const& in, TYPE& out) 
		//
		// then the rest is automatic



		// xl_conversions for supported types
		// wstring, string 
		inline void to_xl(std::wstring const& in, XLOPER12& out) {
			xl_inner::xl_strings::xlString(in, out);
		}
		static void from_xl(XLOPER12 const& in, std::wstring& out) {
			CHECK_INPUT(in, out);
			// based on Excel SDK, FRAMEWORK.C, XLOPER12 to XLOPER
			out = L"";
			xl_inner::xl_type_ops::check_xl_type(in, xltypeStr);

			XCHAR* st;
			int cch;
			st = in.val.str;
			cch = st[0];
			out.resize(cch);
			for (size_t i = 0; i < cch; ++i) {
				out[i] = st[i + 1];
			}
		}

		inline void to_xl(std::string const& in, XLOPER12& out) {
			to_xl(xl_inner::xl_strings::convert(in), out);
		}

		inline void from_xl(XLOPER12 const& in, std::string& out) {
			std::wstring wout;
			from_xl(in, wout);
			out = xl_inner::xl_strings::convert(wout);
		}

		// integer types, doubles and bools are passed in as is, but returned as XLOPERS
		inline void int_to_xl(int const& in, XLOPER12& out) {
			xl_inner::xl_type_ops::set_xl_type(out, xltypeInt);
			out.val.w = in;
		}

		inline void int_to_xl(int const& in, LPXLOPER12& out) {
			out = xl_inner::xl_memory::new_xloper12();
			int_to_xl(in, *out);
		}
		inline void num_to_xl(double const& in, XLOPER12& out) {
			xl_inner::xl_type_ops::set_xl_type(out, xltypeNum);
			out.val.num = in;
		}

		inline void num_to_xl(double const& in, LPXLOPER12& out) {
			out = xl_inner::xl_memory::new_xloper12();
			num_to_xl(in, *out);
		}
		inline int xl_to_int(XLOPER12 const& in) {
			xl_inner::xl_type_ops::check_xl_type(in, xltypeInt | xltypeNum);
			if (xl_inner::xl_type_ops::is_xl_type(in, xltypeInt))
				return in.val.w;
			else
				return static_cast<int>(in.val.num);
		}
		inline double xl_to_num(XLOPER12 const& in) {
			xl_inner::xl_type_ops::check_xl_type(in, xltypeInt | xltypeNum);
			if (xl_inner::xl_type_ops::is_xl_type(in, xltypeInt))
				return static_cast<double>(in.val.w);
			else
				return in.val.num;
		}

		inline void to_xl(int const& in, XLOPER12& out) {
			int_to_xl(in, out);
		}
		inline void from_xl(XLOPER12 const& in, int& out) {
			CHECK_INPUT(in, out);
			out = xl_to_int(in);
		}
		inline void to_xl(unsigned short const& in, XLOPER12& out) {
			int_to_xl(in, out);
		}
		inline void from_xl(XLOPER12 const& in, unsigned short& out) {
			CHECK_INPUT(in, out);
			out = static_cast<unsigned short>(xl_to_int(in));
		}
		inline void to_xl(size_t const& in, XLOPER12& out) {
			int_to_xl(static_cast<int>(in), out);
		}
		inline void from_xl(XLOPER12 const& in, size_t& out) {
			CHECK_INPUT(in, out);
			out = static_cast<size_t>(xl_to_int(in));
		}
		inline void to_xl(short const& in, XLOPER12& out) {
			int_to_xl(in, out);
		}
		inline void from_xl(XLOPER12 const& in, short& out) {
			CHECK_INPUT(in, out);
			out = static_cast<short>(xl_to_int(in));
		}
		inline void to_xl(double const& in, XLOPER12& out) {
			num_to_xl(in, out);
		}
		inline void from_xl(XLOPER12 const& in, double& out) {
			CHECK_INPUT(in, out);
			out = xl_to_num(in);
		}
		inline void to_xl(float const& in, XLOPER12& out) {
			num_to_xl(in, out);
		}
		inline void from_xl(XLOPER12 const& in, float& out) {
			CHECK_INPUT(in, out);
			out = static_cast<float>(xl_to_num(in));
		}
		inline void to_xl(bool const& in, XLOPER12& out) {
			xl_inner::xl_type_ops::set_xl_type(out, xltypeBool);
			out.val.xbool = in;
		}
		inline void from_xl(XLOPER12 const& in, bool& out) {
			CHECK_INPUT(in, out);
			xl_inner::xl_type_ops::check_xl_type(in, xltypeBool);
			out = static_cast<bool>(in.val.xbool);
		}

		inline void to_xl(xl_data::xl_variant const& in, XLOPER12& out);
		void from_xl(XLOPER12 const& in, xl_data::xl_variant& out);

		// linear types are lists and vectors of strings, wstrings, int, short, unsigned, double, float, bool
		// reserve is an optimisation - helps a lot to pre-set vector sizes, but you can't do it with lists
		template<typename ContainerT>
		inline void reserve(ContainerT& c, size_t sz) {
		}

		template<typename ValueT>
		inline void reserve(std::vector<ValueT>& v, size_t sz) {
			v.reserve(sz);
		}


		template<typename TLinear>
		void linear_to_xl(TLinear const& in, XLOPER12& out, bool transposed = false) {
			const size_t sz = in.size();
			if (sz == 0) {
				xl_inner::xl_type_ops::set_xl_type(out, xltypeNil);
				return;
			}

			xl_inner::xl_type_ops::set_xl_type(out, xltypeMulti);
			out.val.array.columns = transposed ? ((RW)sz) : 1;
			out.val.array.rows = transposed ? 1 : (RW)sz;
			out.val.array.lparray = xl_inner::xl_memory::new_xloper12_array(sz);

			size_t i = 0;
			for (auto const& v : in) {
				out.val.array.lparray[i].xltype = 0;
				to_xl(v, out.val.array.lparray[i++]);
			}
		}
		template<typename TLinear>
		void linear_from_xl(XLOPER12 const& in, TLinear& out) {
			CHECK_INPUT(in, out);
			xl_inner::xl_type_ops::check_xl_type(in, xltypeMulti);
			if ((in.val.array.columns * in.val.array.rows) == 0) {
				out.clear();
				return;
			}
			if (in.val.array.columns != 1 && in.val.array.rows != 1)
				throw std::runtime_error("Invalid type in input, expected a one-dimensional array.");

			const size_t sz = in.val.array.columns * in.val.array.rows;
			reserve(out, sz);
			out.clear();
			for (size_t i = 0; i < sz; ++i) {
				XLOPER12 const& arg = in.val.array.lparray[i];
				typename TLinear::value_type arg_out;
				from_xl(arg, arg_out);
				out.push_back(arg_out);
			}
		}

		template<typename T>
		void to_xl(std::vector<T> const& in, XLOPER12& out) {
			linear_to_xl(in, out);
		}
		template<typename T>
		void to_xl(std::list<T> const& in, XLOPER12& out) {
			linear_to_xl(in, out);
		}
		template<typename T>
		void to_xl(xl_util::xl_transposed<std::vector<T>> const& in, XLOPER12& out) {
			linear_to_xl(in.value(), out, true);
		}
		template<typename T>
		void to_xl(xl_util::xl_transposed<std::list<T>> const& in, XLOPER12& out) {
			linear_to_xl(in.value(), out, true);
		}
		template<typename TLinearTransposed>
		void transposed_linear_to_xl(TLinearTransposed const& in, XLOPER12& out) {
			linear_to_xl(in, out, true);
		}
		template<typename T>
		void from_xl(XLOPER12 const& in, std::vector<T>& out) {
			linear_from_xl(in, out);
		}
		template<typename T>
		void from_xl(XLOPER12 const& in, std::list<T>& out) {
			linear_from_xl(in, out);
		}

		// simple grids - list of lists, vectors of vectors, lists of vectors, vectors of lists
		template<typename TSimpleGrid>
		void simple_grid_to_xl(TSimpleGrid const& in, XLOPER12& out, bool transposed = false) {
			size_t rows = in.size();
			size_t cols = 0;
			for (auto const r : in) {
				if (r.size() > cols) cols = r.size();
			}
			if (rows * cols == 0) {
				xl_inner::xl_type_ops::set_xl_type(out, xltypeNil);
				return;
			}
			if (transposed)
				std::swap(rows, cols);

			xl_inner::xl_type_ops::set_xl_type(out, xltypeMulti);
			out.val.array.rows = (RW)rows;
			out.val.array.columns = (RW)cols;
			out.val.array.lparray = xl_inner::xl_memory::new_xloper12_array(rows * cols);

			size_t i = 0;
			typename std::remove_reference<TSimpleGrid>::type::value_type::value_type default_value;
			for (auto const& row : in) {
				size_t c = 0;
				for (auto const& v : row) {
					++c;
					out.val.array.lparray[i].xltype = 0;
					to_xl(v, out.val.array.lparray[i++]);
				}

				for (; c < (transposed ? rows : cols); ++c) {
					out.val.array.lparray[i].xltype = 0;
					to_xl(default_value, out.val.array.lparray[i++]);
				}
			}
		}
		template<typename TSimpleGridTransposed>
		void transposed_simple_grid_to_xl(TSimpleGridTransposed const& in, XLOPER12& out) {
			simple_grid_to_xl(in, out, true);
		}
		template<typename TSimpleGrid>
		void simple_grid_from_xl(XLOPER12 const& in, TSimpleGrid& out, bool transposed = false) {
			CHECK_INPUT(in, out);
			xl_inner::xl_type_ops::check_xl_type(in, xltypeMulti);

			const size_t rows = in.val.array.rows;
			const size_t cols = in.val.array.columns;
			if ((rows * cols) == 0) {
				out.clear();
				return;
			}
			if (transposed) {
				using column_type = typename TSimpleGrid::value_type;
				using value_type = typename column_type::value_type;
				reserve(out, cols);
				for (size_t c = 0; c < cols; ++c) {
					out.push_back(column_type());
				}
				size_t i = 0;
				for (size_t c = 0; c < cols; ++c) {
					for (auto& column : out) {
						XLOPER12 const& arg = in.val.array.lparray[i++];
						value_type arg_out;
						from_xl(arg, arg_out);
						column.push_back(arg_out);
					}
				}
			}
			else {
				reserve(out, rows);
				size_t i = 0;
				for (size_t r = 0; r < rows; ++r) {
					using row_type = typename TSimpleGrid::value_type;
					out.push_back(row_type());
					row_type& row = out.back();
					reserve(row, cols);
					using value_type = typename row_type::value_type;
					for (size_t c = 0; c < cols; ++c, ++i) {
						XLOPER12 const& arg = in.val.array.lparray[i];
						value_type arg_out;
						from_xl(arg, arg_out);
						row.push_back(arg_out);
					}

				}
			}
		}

		template<typename T>
		void to_xl(std::vector<std::vector<T>> const& in, XLOPER12& out) {
			simple_grid_to_xl(in, out);
		}
		template<typename T>
		void to_xl(std::vector<std::list<T>> const& in, XLOPER12& out) {
			simple_grid_to_xl(in, out);
		}
		template<typename T>
		void to_xl(std::list<std::list<T>> const& in, XLOPER12& out) {
			simple_grid_to_xl(in, out);
		}
		template<typename T>
		void to_xl(std::list<std::vector<T>> const& in, XLOPER12& out) {
			simple_grid_to_xl(in, out);
		}
		template<typename T>
		void to_xl(xl_util::xl_transposed <std::vector<std::vector<T>>> const& in, XLOPER12& out) {
			transposed_simple_grid_to_xl(in, out);
		}
		template<typename T>
		void to_xl(xl_util::xl_transposed<std::vector<std::list<T>>> const& in, XLOPER12& out) {
			transposed_simple_grid_to_xl(in, out);
		}
		template<typename T>
		void to_xl(xl_util::xl_transposed<std::list<std::list<T>>> const& in, XLOPER12& out) {
			transposed_simple_grid_to_xl(in, out);
		}
		template<typename T>
		void to_xl(xl_util::xl_transposed<std::list<std::vector<T>>> const& in, XLOPER12& out) {
			transposed_simple_grid_to_xl(in, out);
		}

		template<typename T>
		void from_xl(XLOPER12 const& in, std::vector<std::vector<T>>& out) {
			simple_grid_from_xl(in, out);
		}
		template<typename T>
		void from_xl(XLOPER12 const& in, std::vector<std::list<T>>& out) {
			simple_grid_from_xl(in, out);
		}
		template<typename T>
		void from_xl(XLOPER12 const& in, std::list<std::vector<T>>& out) {
			simple_grid_from_xl(in, out);
		}
		template<typename T>
		void from_xl(XLOPER12 const& in, std::list<std::list<T>>& out) {
			simple_grid_from_xl(in, out);
		}

		// linear maps - maps where both key and value are PODs or strings
		template<typename TLinearMap>
		void linear_map_to_xl(TLinearMap const& in, XLOPER12& out, bool transposed = false) {
			const size_t rows = 2;
			size_t cols = in.size();
			if (rows == 0) {
				xl_inner::xl_type_ops::set_xl_type(out, xltypeNil);
				return;
			}

			xl_inner::xl_type_ops::set_xl_type(out, xltypeMulti);
			out.val.array.rows = (RW)rows;
			out.val.array.columns = (RW)cols;
			if (transposed)
				std::swap(out.val.array.rows, out.val.array.columns);

			out.val.array.lparray = xl_inner::xl_memory::new_xloper12_array(rows * cols);

			size_t i = 0;
			if (transposed) {
				for (auto const& row : in) {
					out.val.array.lparray[i].xltype = 0;
					to_xl(row.first, out.val.array.lparray[i++]);
					out.val.array.lparray[i].xltype = 0;
					to_xl(row.second, out.val.array.lparray[i++]);
				}
			}
			else {
				for (auto const& row : in) {
					out.val.array.lparray[i].xltype = 0;
					to_xl(row.first, out.val.array.lparray[i++]);
				}
				for (auto const& row : in) {
					out.val.array.lparray[i].xltype = 0;
					to_xl(row.second, out.val.array.lparray[i++]);
				}
			}
		}


		template<typename TLinearMap>
		void linear_map_from_xl(XLOPER12 const& in, TLinearMap& out) {
			CHECK_INPUT(in, out);
			xl_inner::xl_type_ops::check_xl_type(in, xltypeMulti);

			const size_t rows = in.val.array.rows;
			const size_t cols = in.val.array.columns;
			if ((rows * cols) == 0) {
				out.clear();
				return;
			}
			if (in.val.array.columns != 2 && in.val.array.rows != 2)
				throw std::runtime_error("Invalid type in input, expected two rows or two columns.");

			using key_type = typename TLinearMap::key_type;
			using value_type = typename TLinearMap::mapped_type;

			if (in.val.array.columns == 2) {
				// vertical range
				for (size_t i = 0; i < rows * cols; ) {
					XLOPER12 const& key_arg = in.val.array.lparray[i++];
					XLOPER12 const& value_arg = in.val.array.lparray[i++];
					key_type key;
					from_xl(key_arg, key);
					value_type value;
					from_xl(value_arg, value);
					out[key] = value;
				}
			}
			else {
				// horizontal range
				const size_t stride = cols;
				for (size_t i = 0; i < stride; ++i) {
					XLOPER12 const& key_arg = in.val.array.lparray[i];
					XLOPER12 const& value_arg = in.val.array.lparray[i + stride];
					key_type key;
					from_xl(key_arg, key);
					value_type value;
					from_xl(value_arg, value);
					out[key] = value;
				}
			}
		}
		template<typename TKey, typename TValue>
		void to_xl(std::map<TKey, TValue> const& in, XLOPER12& out) {
			linear_map_to_xl(in, out);
		}
		template<typename TKey, typename TValue>
		void to_xl(std::unordered_map<TKey, TValue> const& in, XLOPER12& out) {
			linear_map_to_xl(in, out);
		}
		template<typename TKey, typename TValue>
		void to_xl(xl_util::xl_transposed<std::map<TKey, TValue>> const& in, XLOPER12& out) {
			linear_map_to_xl(in.value(), out, true);
		}
		template<typename TKey, typename TValue>
		void to_xl(xl_util::xl_transposed<std::unordered_map<TKey, TValue>> const& in, XLOPER12& out) {
			linear_map_to_xl(in.value(), out, true);
		}
		template<typename TKey, typename TValue>
		void from_xl(XLOPER12 const& in, std::map<TKey, TValue>& out) {
			linear_map_from_xl(in, out);
		}
		template<typename TKey, typename TValue>
		void from_xl(XLOPER12 const& in, std::unordered_map<TKey, TValue>& out) {
			linear_map_from_xl(in, out);
		}

		// grid maps - maps where keys are PODs or string, but values are vectors or lists
		// we will always assume vertical orientation - keys in the first row
		template<typename TMapGrid>
		void map_grid_to_xl(TMapGrid const& in, XLOPER12& out, bool transposed = false) {
			const size_t cols = in.size();
			size_t rows = 0;
			for (auto const c : in)
				if (c.second.size() > rows) rows = c.second.size();

			if (rows * cols == 0) {
				xl_inner::xl_type_ops::set_xl_type(out, xltypeNil);
				return;
			}

			xl_inner::xl_type_ops::set_xl_type(out, xltypeMulti);
			out.val.array.rows = (RW)rows + 1;//keys are titles, so another row
			out.val.array.columns = (RW)cols;
			if (transposed)
				std::swap(out.val.array.rows, out.val.array.columns);
			size_t sz = (rows + 1) * cols;
			out.val.array.lparray = xl_inner::xl_memory::new_xloper12_array(sz);
			size_t i = 0;
			if (transposed) {
				for (auto const& in_ : in) {
					out.val.array.lparray[i].xltype = 0;
					to_xl(in_.first, out.val.array.lparray[i++]);
					for (auto const& v_ : in_.second) {
						out.val.array.lparray[i].xltype = 0;
						to_xl(v_, out.val.array.lparray[i++]);
					}
				}
			}
			else
			{
				// first we do titles as keys
				for (auto const& in_ : in) {
					auto const& key_value = in_.first;
					out.val.array.lparray[i].xltype = 0;
					to_xl(key_value, out.val.array.lparray[i++]);
				}
				// then we to painful many traversals row by row
				typename TMapGrid::mapped_type::value_type default_value;
				for (size_t row = 0; row < rows; ++row) {
					for (auto const& in_ : in) {
						auto& col = in_.second;
						if (col.size() < rows) {
							out.val.array.lparray[i].xltype = 0;
							to_xl(default_value, out.val.array.lparray[i++]);
						}
						else {
							typename TMapGrid::mapped_type::const_iterator it = col.begin();
							std::advance(it, row);
							out.val.array.lparray[i].xltype = 0;
							to_xl(*it, out.val.array.lparray[i++]);
						}
					}
				}
			}
		}

		template<typename TMapGrid>
		void map_grid_from_xl(XLOPER12 const& in, TMapGrid& out) {
			CHECK_INPUT(in, out);
			xl_inner::xl_type_ops::check_xl_type(in, xltypeMulti);

			const size_t rows = in.val.array.rows;
			const size_t cols = in.val.array.columns;
			if ((rows * cols) == 0) {
				out.clear();
				return;
			}
			size_t i = 0;
			// first row contains the keys
			using key_type = typename TMapGrid::key_type;
			using value_type = typename TMapGrid::mapped_type::value_type;
			// we have to keep track of the keys in order in which they appear in the first row
			std::vector<key_type> keys;
			keys.reserve(cols);
			for (size_t c = 0; c < cols; ++c) {
				XLOPER12 const& arg = in.val.array.lparray[i++];
				key_type key;
				from_xl(arg, key);
				keys.push_back(key);
				reserve(out[key], cols);
			}
			for (size_t r = 1; r < rows; ++r) {
				for (size_t c = 0; c < cols; ++c) {
					XLOPER12 const& arg = in.val.array.lparray[i++];
					value_type value;
					from_xl(arg, value);
					out[keys[c]].push_back(value);
				}
			}
		}

		template<typename TKey, typename TValue>
		void to_xl(std::map<TKey, std::vector<TValue>> const& in, XLOPER12& out) {
			map_grid_to_xl(in, out);
		}
		template<typename TKey, typename TValue>
		void to_xl(std::map<TKey, std::list<TValue>> const& in, XLOPER12& out) {
			map_grid_to_xl(in, out);
		}
		template<typename TKey, typename TValue>
		void to_xl(std::unordered_map<TKey, std::vector<TValue>> const& in, XLOPER12& out) {
			map_grid_to_xl(in, out);
		}
		template<typename TKey, typename TValue>
		void to_xl(std::unordered_map<TKey, std::list<TValue>> const& in, XLOPER12& out) {
			map_grid_to_xl(in, out);
		}
		template<typename TKey, typename TValue>
		void to_xl(xl_util::xl_transposed<std::map<TKey, std::vector<TValue>>> const& in, XLOPER12& out) {
			map_grid_to_xl(in.value(), out, true);
		}
		template<typename TKey, typename TValue>
		void to_xl(xl_util::xl_transposed<std::map<TKey, std::list<TValue>>> const& in, XLOPER12& out) {
			map_grid_to_xl(in.value(), out, true);
		}
		template<typename TKey, typename TValue>
		void to_xl(xl_util::xl_transposed<std::unordered_map<TKey, std::vector<TValue>>> const& in, XLOPER12& out) {
			map_grid_to_xl(in.value(), out, true);
		}
		template<typename TKey, typename TValue>
		void to_xl(xl_util::xl_transposed<std::unordered_map<TKey, std::list<TValue>>> const& in, XLOPER12& out) {
			map_grid_to_xl(in.value(), out, true);
		}
		template<typename TKey, typename TValue>
		void from_xl(XLOPER12 const& in, std::map<TKey, std::vector<TValue>>& out) {
			map_grid_from_xl(in, out);
		}
		template<typename TKey, typename TValue>
		void from_xl(XLOPER12 const& in, std::map<TKey, std::list<TValue>>& out) {
			map_grid_from_xl(in, out);
		}
		template<typename TKey, typename TValue>
		void from_xl(XLOPER12 const& in, std::unordered_map<TKey, std::vector<TValue>>& out) {
			map_grid_from_xl(in, out);
		}
		template<typename TKey, typename TValue>
		void from_xl(XLOPER12 const& in, std::unordered_map<TKey, std::list<TValue>>& out) {
			map_grid_from_xl(in, out);
		}
		// xl_variant, nvp, table
		static void to_xl(xl_data::xl_variant const& in, XLOPER12& out) {
			using namespace xl_data;
			switch (in.type()) {
			case xl_variant::xl_type::xl_typeNum:
				to_xl(in.get_double(), out);
				break;
			case xl_variant::xl_type::xl_typeStr:
				to_xl(in.get_wstring(), out);
				break;
			case xl_variant::xl_type::xl_typeBool:
				to_xl(in.get_bool(), out);
				break;
			case xl_variant::xl_type::xl_typeErr:
				out.xltype = xltypeErr;
				out.val.err = in.get_error();
				break;
			case xl_variant::xl_type::xl_typeInt:
				to_xl(in.get_int(), out);
				break;
			case xl_variant::xl_type::xl_typeMulti:
				simple_grid_to_xl(in.get_multi(), out);
				break;
			case xl_variant::xl_type::xl_typeMissing:
				out.xltype = xltypeMissing;
				break;
			case xl_variant::xl_type::xl_typeNil:
				out.xltype = xltypeNil;
				break;
			default:
				break;
			}
		}
		inline void from_xl(XLOPER12 const& in, xl_data::xl_variant& out) {
			CHECK_INPUT(in, out);
			out.set(in);
		}

		inline void to_xl(xl_data::xl_nvp const& in, XLOPER12& out) {
			simple_grid_to_xl(in.values(), out);
		}
		inline void from_xl(XLOPER12 const& in, xl_data::xl_nvp& out) {
			CHECK_INPUT(in, out);
			xl_inner::xl_type_ops::check_xl_type(in, xltypeMulti);
			simple_grid_from_xl(in, out.values(), xl_data::xl_nvp::transpose_input(in));
		}
		inline void to_xl(xl_data::xl_nvp_cs const& in, XLOPER12& out) {
			simple_grid_to_xl(in.values(), out);
		}
		inline void from_xl(XLOPER12 const& in, xl_data::xl_nvp_cs& out) {
			CHECK_INPUT(in, out);
			xl_inner::xl_type_ops::check_xl_type(in, xltypeMulti);
			simple_grid_from_xl(in, out.values(), xl_data::xl_nvp::transpose_input(in));
		}
		inline void to_xl(xl_data::xl_table const& in, XLOPER12& out) {
			simple_grid_to_xl(in.table(), out);
		}
		inline void from_xl(XLOPER12 const& in, xl_data::xl_table& out) {
			CHECK_INPUT(in, out);
			xl_inner::xl_type_ops::check_xl_type(in, xltypeMulti);
			simple_grid_from_xl(in, out.table(), xl_data::xl_table::transpose_input(in));
		}
		inline void to_xl(xl_data::xl_table_cs const& in, XLOPER12& out) {
			simple_grid_to_xl(in.table(), out);
		}
		inline void from_xl(XLOPER12 const& in, xl_data::xl_table_cs& out) {
			CHECK_INPUT(in, out);
			xl_inner::xl_type_ops::check_xl_type(in, xltypeMulti);
			simple_grid_from_xl(in, out.table(), xl_data::xl_table::transpose_input(in));
		}

		// FP12
		inline FP12* to_xl(xl_fast_array::xl_fast_array const& in) {
			return in.fp12();
		}
		inline FP12* to_xlp(xl_fast_array::xl_fast_array const& in) {
			return in.fp12();
		}
		inline xl_fast_array::xl_fast_array from_xl(FP12* in) {
			return in;
		}

		template<typename ReturnType>
		ReturnType to_return_type(LPXLOPER12 in);

		template<>
		inline LPXLOPER12 to_return_type<LPXLOPER12>(LPXLOPER12 in) {
			return in;
		}
		template<>
		inline FP12* to_return_type<FP12*>(LPXLOPER12 in) {
			static xl_fast_array::xl_fast_array fp12(1, 1, 0.0);
			return fp12.fp12();
		}



		// templated versions of conversions
		template<typename T>
		XLOPER12 to_xl(T const& in) {
			XLOPER12 ret;
			to_xl(in, ret);
			return ret;
		}

		template<typename T>
		void to_xlp(T const& in, LPXLOPER12& out) {
			out = xl_inner::xl_memory::new_xloper12();
			to_xl(in, *out);
		}

		template<typename T>
		LPXLOPER12 to_xlp(T const& in) {
			LPXLOPER12 ret;
			to_xlp(in, ret);
			return ret;
		}

		template<typename T>
		void from_xl(XLOPER12 const& in, T& out) {
			throw std::runtime_error(std::string("Transformation to XLOPER12 not implemented for type: ") + typeid(T).name());
		}

		template<typename T>
		T from_xl(XLOPER12 const& in) {
			T out;
			from_xl(in, out);
			return out;
		}
		template<typename T>
		typename std::remove_reference<T>::type from_xl(T const& in) {
			return in;
		}
		template<typename T>
		typename std::remove_const_t<std::remove_reference_t<T>> from_xl(LPXLOPER12 const& in) {
			xl_util::check_null_xlp(in);
			return from_xl<typename std::remove_const_t<std::remove_reference_t<T>>>(*in);
		}
		template<typename T>
		typename std::remove_reference<T>::type from_xlp(LPXLOPER12 const& in) {
			return from_xl<T>(in);
		}
		template<typename T>
		void from_xl(LPXLOPER12 const& in, T& out) {
			out = from_xl<T>(in);
		}

		// no_conversion functions are placeholders for where converison doesn't happen
		template< typename T >
		void trivial(T const& in, T& out) {
			out = in;
		}
		template< typename T >
		T trivial(T const& in) {
			return in;
		}
	}
	
	namespace xl_traits {
		// which conversions need to be applied is computed on the fly using traits

		// traits for types that can be passed to and from XLOPER12 interface
		template<typename T> struct xl_type_trait {
			using trait_type = T;
			using xl_target_type = LPXLOPER12;
			using xl_return_type = LPXLOPER12;
			static constexpr const char* const xl_type_string = "Q";
		};

		// actual target type is a workaround for type deduction limitation
		// LPXLOPER12 is a pointer, can be implicitly converted to all sorts
		// so when we need the target type for deductions, much better to use XLOPER12
		// but we still need to automatically translate this to an actual type
		template< typename T> struct actual_xl_target_type {
			using type = T;
		};
		template< > struct actual_xl_target_type<XLOPER12> {
			using type = LPXLOPER12;
		};
		// everything returns LPXLOPER12s (so that we have consistent error handling)
		// except functions that return FP12s
		template< typename T> struct actual_xl_return_type {
			using type = LPXLOPER12;
		};
		template< > struct actual_xl_return_type<FP12*> {
			using type = FP12*;
		};

		// xl_type traits specify how we deal with each individual type as is moves between Excel and std library
		// it becomes complicated because there isn't a single simple treatment to be applied to all types
		// XLOPER12 interface is highly optimised, we gotta respect that
		// so ... 
		// 1. whenever we can, we keep types that Excel treats as native, as native (PODs and strings)
		// 2. we do want consistent error handling, so we always return LPXLOPER12s from our functions, 
		//		it costs when we want to return pods, but we choose to pay that price for the sake of error handling
		// 3. we do have to do a bit of filthy translation - LPXLOPER12 is just a pointer, easily convertable to POD type
		//		so to have our template type deductions working, we pretend that return type of functions that return LPXLOPER12 is just XLOPER12
		// 4. we have even more special handling for std containers 
		// It wall becomes messy, and the xl traits are where we hide this this mess
#define __AF_DECLARE_XL_TYPE_TRAIT(TRAIT_TYPE, XL_TYPE, XL_TYPE_STRING) \
		template<> struct xl_type_trait<TRAIT_TYPE> {\
			using trait_type = TRAIT_TYPE;\
			using xl_target_type = actual_xl_target_type<XL_TYPE>::type;\
			using xl_return_type = actual_xl_return_type<XL_TYPE>::type;\
			static constexpr const char* const xl_type_string = XL_TYPE_STRING;\
		};

#define __AF_DECLARE_XL_POD_TRAIT(POD, XL_TYPE_STRING) \
__AF_DECLARE_XL_TYPE_TRAIT(POD,				POD,	XL_TYPE_STRING);\
__AF_DECLARE_XL_TYPE_TRAIT(const POD,const	POD,	XL_TYPE_STRING);\
__AF_DECLARE_XL_TYPE_TRAIT(const POD&,const POD&,	XL_TYPE_STRING);

		__AF_DECLARE_XL_POD_TRAIT(bool, "A");
		__AF_DECLARE_XL_POD_TRAIT(float, "B");
		__AF_DECLARE_XL_POD_TRAIT(double, "B");
		__AF_DECLARE_XL_POD_TRAIT(unsigned short, "H");
		__AF_DECLARE_XL_POD_TRAIT(short, "I");
		__AF_DECLARE_XL_POD_TRAIT(int, "J");

		// strings
#define __AF_DECLARE_XL_STRING_TRAIT(STRING_TYPE) \
__AF_DECLARE_XL_TYPE_TRAIT(STRING_TYPE,			XLOPER12, "C%");\
__AF_DECLARE_XL_TYPE_TRAIT(const STRING_TYPE,	XLOPER12, "C%");\
__AF_DECLARE_XL_TYPE_TRAIT(const STRING_TYPE&,	XLOPER12, "C%");

		__AF_DECLARE_XL_STRING_TRAIT(std::string);
		__AF_DECLARE_XL_STRING_TRAIT(std::wstring);

		// FP12
		__AF_DECLARE_XL_TYPE_TRAIT(xl_fast_array::xl_fast_array, FP12*, "K%");
		__AF_DECLARE_XL_TYPE_TRAIT(const xl_fast_array::xl_fast_array, FP12*, "K%");
		__AF_DECLARE_XL_TYPE_TRAIT(const xl_fast_array::xl_fast_array&, FP12*, "K%");

		// translate a type to Excel type string
		template< typename T >
		inline void append_xl_type_string(std::string& out) {
			using the_trait = xl_type_trait<T>;
			if (the_trait::xl_type_string)
				out.append(the_trait::xl_type_string);
		}
		template< typename T >
		inline void append_xl_ret_type_string(std::string& out) {
			out.append("Q"); // everything other than FP12 is returned as XLOPER12
		}
		template<> inline void append_xl_ret_type_string<xl_fast_array::xl_fast_array>(std::string& out) {
			out.append("K%"); // everything other than FP12 is returned as XLOPER12
		}
		template<> inline void append_xl_ret_type_string<const xl_fast_array::xl_fast_array>(std::string& out) {
			out.append("K%"); // everything other than FP12 is returned as XLOPER12
		}
		template<> inline void append_xl_ret_type_string<const xl_fast_array::xl_fast_array&>(std::string& out) {
			out.append("K%"); // everything other than FP12 is returned as XLOPER12
		}
	}
	
	namespace xl_signature {
		// deducing the signature of functions that we are exposing
		// and building Excel type strings to register these functions is a bit of template magic

		template<typename RetT>
		void append_to_signature_string(std::string& out) {
			xl_traits::append_xl_type_string<RetT>(out);
		}
		template<typename RetT, typename Arg1T, typename ... ArgTs>
		void append_to_signature_string(std::string& out) {
			xl_traits::append_xl_type_string<RetT>(out);
			append_to_signature_string<Arg1T, ArgTs...>(out);
		}
		template<typename RetT>
		std::string get_signature_string(RetT(*)()) {
			std::string ret;
			xl_traits::append_xl_ret_type_string <RetT>(ret);
			return ret;
		}
		template<typename RetT, typename ... ArgTs>
		std::string get_signature_string(RetT(*)(ArgTs...)) {
			std::string ret;
			xl_traits::append_xl_ret_type_string <RetT>(ret);
			append_to_signature_string<ArgTs...>(ret);
			return ret;
		}

		// quick and dirty functions and lambdas only ever deal with XLOPER12s, in and out
		template<typename RetT>
		void append_to_signature_string_qd(std::string& out) {
			out += "Q";
		}
		template<typename RetT, typename Arg1T, typename ... ArgTs>
		void append_to_signature_string_qd(std::string& out) {
			out += "Q";
			append_to_signature_string_qd<Arg1T, ArgTs...>(out);
		}
		template<typename RetT>
		std::string get_signature_string_qd(RetT(*)()) {
			std::string ret("Q");
			return ret;
		}
		template<typename RetT, typename ... ArgTs>
		std::string get_signature_string_qd(RetT(*)(ArgTs...)) {
			std::string ret("Q");
			append_to_signature_string_qd<ArgTs...>(ret);
			return ret;
		}
		template<typename RetT, typename Class>
		std::string get_lambda_signature_string_impl(RetT(Class::*)() const) {
			std::string ret("Q");
			return ret;
		}
		template<typename RetT, typename Class, typename ... ArgTs>
		std::string get_lambda_signature_string_impl(RetT(Class::*)(ArgTs...) const) {
			std::string ret("Q");
			append_to_signature_string_qd<ArgTs...>(ret);
			return ret;
		}
		template<typename LambdaT>
		std::string get_lambda_signature_string() {
			return get_lambda_signature_string_impl(&LambdaT::operator());
		}

		// magical template magic to deduce return and parameter types
		template<typename T> struct func_types;

		template<typename R>
		struct func_types<R(*)(void)> {
			using ReturnType = R;
			static constexpr size_t number_of_arguments = 0;
		};

		template<typename R, typename ... ArgTs>
		struct func_types<R(*)(ArgTs...)> {
			using ReturnType = R;
			static constexpr size_t number_of_arguments = sizeof...(ArgTs);

			using TypesTuple = std::tuple<ArgTs...>;
			template <int N>
			using ArgType = typename std::tuple_element<N, TypesTuple>::type;
		};

		template<typename T> struct mem_func_types;

		template<typename R, typename Class>
		struct mem_func_types<R(Class::*)(void) const> {
			using ReturnType = R;
			const size_t number_of_arguments = 0;
		};

		template<typename R, typename Class, typename ... ArgTs>
		struct mem_func_types<R(Class::*)(ArgTs...) const> {
			using ReturnType = R;
			static constexpr size_t number_of_arguments = sizeof...(ArgTs);

			using TypesTuple = std::tuple<ArgTs...>;
			template <int N>
			using ArgType = typename std::tuple_element<N, TypesTuple>::type;
		};

		template<typename Lambda>
		using lambda_func_types = mem_func_types<decltype(&Lambda::operator())>;
	}
	
	namespace xl_registration {
		// xl_registration of functions depends on the function signature
		// the monster switch statement is ugly, but does it efficiently
		struct xl_function_data {
			std::string const _function_xl_category;
			std::string const _function_xl_name;
			std::string const _function_name;
			std::string const _signature;
			std::string const _function_help;
			std::string const _argument_names;
			std::vector<std::string> const _arguments_help;

			void xlfRegister_impl(XLOPER12 const& xDLL, std::string const& function_category_) const {
				using namespace xl_conversions;
				using namespace xl_inner::xl_constants;
				using namespace xl_data;

				auto_pxl function_category{ to_xlp(function_category_) };
				auto_pxl function_xl_name{ to_xlp(_function_xl_name) };
				auto_pxl function_name{ to_xlp(_function_name) };
				auto_pxl signature{ to_xlp(_signature) };
				auto_pxl argument_names{ to_xlp(_argument_names) };
				auto_pxl function_help{ to_xlp(_function_help) };
				auto_pxl function_type{ to_xlp(1) };
				int total_params = 10 + (int)_arguments_help.size() + 1;//1 extra to fix excel trimming help strings
				XLOPER12 res;
				switch (_arguments_help.size()) {
				case 0:
					Excel12(xlfRegister, &res, total_params, (LPXLOPER12)&xDLL,
						function_name, signature, function_xl_name, argument_names, function_type, function_category,
						&xlMissing(), &xlMissing(), function_help, &xlEmptyString());
					break;
				case 1:
					Excel12(xlfRegister, &res, total_params, (LPXLOPER12)&xDLL,
						function_name, signature, function_xl_name, argument_names, function_type, function_category,
						&xlMissing(), &xlMissing(), function_help,
						auto_pxl(to_xlp(_arguments_help[0])), &xlEmptyString());
					break;
				case 2:
					Excel12(xlfRegister, &res, total_params, (LPXLOPER12)&xDLL,
						function_name, signature, function_xl_name, argument_names, function_type, function_category,
						&xlMissing(), &xlMissing(), function_help,
						auto_pxl(to_xlp(_arguments_help[0])), auto_pxl(to_xlp(_arguments_help[1])), &xlEmptyString());
					break;
				case 3:
					Excel12(xlfRegister, &res, total_params, (LPXLOPER12)&xDLL,
						function_name, signature, function_xl_name, argument_names, function_type, function_category,
						&xlMissing(), &xlMissing(), function_help,
						auto_pxl(to_xlp(_arguments_help[0])), auto_pxl(to_xlp(_arguments_help[1])),
						auto_pxl(to_xlp(_arguments_help[2])), &xlEmptyString());
					break;
				case 4:
					Excel12(xlfRegister, &res, total_params, (LPXLOPER12)&xDLL,
						function_name, signature, function_xl_name, argument_names, function_type, function_category,
						&xlMissing(), &xlMissing(), function_help,
						auto_pxl(to_xlp(_arguments_help[0])), auto_pxl(to_xlp(_arguments_help[1])),
						auto_pxl(to_xlp(_arguments_help[2])), auto_pxl(to_xlp(_arguments_help[3])), &xlEmptyString());
					break;
				case 5:
					Excel12(xlfRegister, &res, total_params, (LPXLOPER12)&xDLL,
						function_name, signature, function_xl_name, argument_names, function_type, function_category,
						&xlMissing(), &xlMissing(), function_help,
						auto_pxl(to_xlp(_arguments_help[0])), auto_pxl(to_xlp(_arguments_help[1])),
						auto_pxl(to_xlp(_arguments_help[2])), auto_pxl(to_xlp(_arguments_help[3])),
						auto_pxl(to_xlp(_arguments_help[4])), &xlEmptyString());
					break;
				case 6:
					Excel12(xlfRegister, &res, total_params, (LPXLOPER12)&xDLL,
						function_name, signature, function_xl_name, argument_names, function_type, function_category,
						&xlMissing(), &xlMissing(), function_help,
						auto_pxl(to_xlp(_arguments_help[0])), auto_pxl(to_xlp(_arguments_help[1])),
						auto_pxl(to_xlp(_arguments_help[2])), auto_pxl(to_xlp(_arguments_help[3])),
						auto_pxl(to_xlp(_arguments_help[4])), auto_pxl(to_xlp(_arguments_help[5])), &xlEmptyString());
					break;
				case 7:
					Excel12(xlfRegister, &res, total_params, (LPXLOPER12)&xDLL,
						function_name, signature, function_xl_name, argument_names, function_type, function_category,
						&xlMissing(), &xlMissing(), function_help,
						auto_pxl(to_xlp(_arguments_help[0])), auto_pxl(to_xlp(_arguments_help[1])),
						auto_pxl(to_xlp(_arguments_help[2])), auto_pxl(to_xlp(_arguments_help[3])),
						auto_pxl(to_xlp(_arguments_help[4])), auto_pxl(to_xlp(_arguments_help[5])),
						auto_pxl(to_xlp(_arguments_help[6])), &xlEmptyString());
					break;
				case 8:
					Excel12(xlfRegister, &res, total_params, (LPXLOPER12)&xDLL,
						function_name, signature, function_xl_name, argument_names, function_type, function_category,
						&xlMissing(), &xlMissing(), function_help,
						auto_pxl(to_xlp(_arguments_help[0])), auto_pxl(to_xlp(_arguments_help[1])),
						auto_pxl(to_xlp(_arguments_help[2])), auto_pxl(to_xlp(_arguments_help[3])),
						auto_pxl(to_xlp(_arguments_help[4])), auto_pxl(to_xlp(_arguments_help[5])),
						auto_pxl(to_xlp(_arguments_help[6])), auto_pxl(to_xlp(_arguments_help[7])), &xlEmptyString());
					break;
				case 9:
					Excel12(xlfRegister, &res, total_params, (LPXLOPER12)&xDLL,
						function_name, signature, function_xl_name, argument_names, function_type, function_category,
						&xlMissing(), &xlMissing(), function_help,
						auto_pxl(to_xlp(_arguments_help[0])), auto_pxl(to_xlp(_arguments_help[1])),
						auto_pxl(to_xlp(_arguments_help[2])), auto_pxl(to_xlp(_arguments_help[3])),
						auto_pxl(to_xlp(_arguments_help[4])), auto_pxl(to_xlp(_arguments_help[5])),
						auto_pxl(to_xlp(_arguments_help[6])), auto_pxl(to_xlp(_arguments_help[7])),
						auto_pxl(to_xlp(_arguments_help[8])), &xlEmptyString());
					break;
				case 10:
					Excel12(xlfRegister, &res, total_params, (LPXLOPER12)&xDLL,
						function_name, signature, function_xl_name, argument_names, function_type, function_category,
						&xlMissing(), &xlMissing(), function_help,
						auto_pxl(to_xlp(_arguments_help[0])), auto_pxl(to_xlp(_arguments_help[1])),
						auto_pxl(to_xlp(_arguments_help[2])), auto_pxl(to_xlp(_arguments_help[3])),
						auto_pxl(to_xlp(_arguments_help[4])), auto_pxl(to_xlp(_arguments_help[5])),
						auto_pxl(to_xlp(_arguments_help[6])), auto_pxl(to_xlp(_arguments_help[7])),
						auto_pxl(to_xlp(_arguments_help[8])), auto_pxl(to_xlp(_arguments_help[9])), &xlEmptyString());
					break;
				case 11:
					Excel12(xlfRegister, &res, total_params, (LPXLOPER12)&xDLL,
						function_name, signature, function_xl_name, argument_names, function_type, function_category,
						&xlMissing(), &xlMissing(), function_help,
						auto_pxl(to_xlp(_arguments_help[0])), auto_pxl(to_xlp(_arguments_help[1])),
						auto_pxl(to_xlp(_arguments_help[2])), auto_pxl(to_xlp(_arguments_help[3])),
						auto_pxl(to_xlp(_arguments_help[4])), auto_pxl(to_xlp(_arguments_help[5])),
						auto_pxl(to_xlp(_arguments_help[6])), auto_pxl(to_xlp(_arguments_help[7])),
						auto_pxl(to_xlp(_arguments_help[8])), auto_pxl(to_xlp(_arguments_help[9])),
						auto_pxl(to_xlp(_arguments_help[10])), &xlEmptyString());
					break;
				case 12:
					Excel12(xlfRegister, &res, total_params, (LPXLOPER12)&xDLL,
						function_name, signature, function_xl_name, argument_names, function_type, function_category,
						&xlMissing(), &xlMissing(), function_help,
						auto_pxl(to_xlp(_arguments_help[0])), auto_pxl(to_xlp(_arguments_help[1])),
						auto_pxl(to_xlp(_arguments_help[2])), auto_pxl(to_xlp(_arguments_help[3])),
						auto_pxl(to_xlp(_arguments_help[4])), auto_pxl(to_xlp(_arguments_help[5])),
						auto_pxl(to_xlp(_arguments_help[6])), auto_pxl(to_xlp(_arguments_help[7])),
						auto_pxl(to_xlp(_arguments_help[8])), auto_pxl(to_xlp(_arguments_help[9])),
						auto_pxl(to_xlp(_arguments_help[10])), auto_pxl(to_xlp(_arguments_help[11])), &xlEmptyString());
					break;
				default:
					break;

				}

			}
		};
		// registry of functions, this is how we make the single line macros work
		// all the parameters for xlfRegister for each function are stored here
		class xl_f_registry {
			std::vector<std::shared_ptr< xl_function_data > > _functions;
			std::string _function_category;
			xl_f_registry() {}
			static std::shared_ptr<xl_f_registry> instance() {
				static std::shared_ptr<xl_f_registry> _instance(new xl_f_registry());
				return _instance;
			}
		public:
			static bool set_function_category(std::string const& function_category) {
				instance()->_function_category = function_category;
				return true;
			}
			static std::string const& get_function_category() {
				return instance()->_function_category;
			}

			static bool register_function(
				std::string const& function_xl_category,
				std::string const& function_xl_name,
				std::string const& function_name,
				std::string const& signature,
				size_t const number_of_arguments,
				bool is_volatile = false,
				std::string const& function_help = "",
				std::string const& argument_names = "",
				std::vector<std::string> const& arguments_help = {}
			) {
				// excel hates fucntions that end in numbers
				std::string f_xl_name(function_xl_name);
				if (isdigit(f_xl_name.back()))
					f_xl_name += "_";
				// function help, argument names, and help strings arrive 
				// packaged in the variadic parameters: function_help, argument0_name, argument0_help, argument1_name, argument1_help ...
				// if not supplied, we can construct sensible defaults
				std::string f_help(function_help.empty() ? function_xl_name : function_help);
				std::string arg_names(argument_names);
				if (arg_names.empty()) {
					std::stringstream sout;
					for (size_t i = 0; i < number_of_arguments; ++i) {
						sout << "arg_" << i;
						if (i != number_of_arguments - 1)
							sout << ",";
					}
					arg_names = sout.str();
				}
				std::vector<std::string> args_help(arguments_help);
				if (args_help.empty()) {
					for (size_t i = 0; i < number_of_arguments; ++i) {
						std::stringstream sout;
						sout << "argument " << i;
						args_help.push_back(sout.str());
					}
				}

				std::shared_ptr<xl_function_data> fd(new xl_function_data{
					function_xl_category,
					f_xl_name,
					function_name,
					is_volatile ? (signature + "!") : signature,
					f_help,
					arg_names,
					args_help
					});
				instance()->_functions.push_back(fd);
				return true;
			}

			// implementation of xlAutoOpen
			static int xlAutoOpen_impl(void) {
				using namespace autotelica::xloper::xl_conversions;
				auto inst = instance();
				std::string f_category(inst->_function_category.empty() ? "af test functions" : inst->_function_category);
				XLOPER12 xDLL;
				Excel12(xlGetName, &xDLL, 0);
				for (auto const& func : inst->_functions) {
					auto const& category = (func->_function_xl_category == "__GLOBAL_CATEGORY__") ?
						f_category : func->_function_xl_category;
					func->xlfRegister_impl(xDLL, category);
				}
				Excel12(xlFree, 0, 1, (LPXLOPER12)&xDLL);
				return 1;
			}
		};

	}
	

// LOOK AWAY, LOOK AWAY ... MACROS GOING CRAZY HERE
// this is the implementation details of the magic part

// the function actually invoked by Excel is a wrapper arround the user's actual function
// __AF_XL_IMPL_F_N builds the name of the wrapper function
#define __AF_XL_IMPL_F_N( __F ) __F##__af_impl
#define __AF_XL_IMPL_F_N_STR( __F ) #__F##"__af_impl"

#define __AF_XL_IMPL_QD_F_N( __F ) __F##__af_qd_impl
#define __AF_XL_IMPL_QD_F_N_STR( __F ) #__F##"__af_qd_impl"

#define __AF_XL_IMPL_LAMBDA_F_N( __NAME ) __NAME##__af_lambda_impl
#define __AF_XL_DECL_LAMBDA_F_N( __NAME ) __##__NAME##__af_lambda_impl
#define __AF_XL_IMPL_LAMBDA_F_N_STR( __NAME ) #__NAME##"__af_lambda_impl"

#define __af_xl_ret_tr( T )		autotelica::xloper::xl_traits::xl_type_trait<autotelica::xloper::xl_signature::func_types<decltype(&T)>::ReturnType>
#define __af_xl_arg_tr(T, N)	autotelica::xloper::xl_traits::xl_type_trait<autotelica::xloper::xl_signature::func_types<decltype(&T)>::ArgType<N>>
#define __af_xl_arg_count(__F)	autotelica::xloper::xl_signature::func_types<decltype(&__F)>::number_of_arguments
#define __af_xl_ret_t( T )		__af_xl_ret_tr(T)::xl_return_type
#define __af_xl_arg_t( T, N )	__af_xl_arg_tr(T, N)::trait_type
#define __af_xl_arg_xl_t( T, N )	__af_xl_arg_tr(T, N)::xl_target_type

#define __af_xl_lambda_ret_tr( __NAME )		autotelica::xloper::xl_traits::xl_type_trait<typename autotelica::xloper::xl_signature::lambda_func_types< typename decltype(__NAME) >::ReturnType>
#define __af_xl_lambda_arg_tr(__NAME, N)	autotelica::xloper::xl_traits::xl_type_trait<typename autotelica::xloper::xl_signature::lambda_func_types< typename decltype(__NAME) >::ArgType<N>>
#define __af_xl_lambda_arg_count(__NAME)	autotelica::xloper::xl_signature::lambda_func_types< typename decltype(__NAME) >::number_of_arguments
#define __af_xl_lambda_ret_t( __NAME )		__af_xl_lambda_ret_tr( __NAME )::xl_target_type
#define __af_xl_lambda_arg_t(__NAME, N)		__af_xl_lambda_arg_tr(__NAME, N)::trait_type
#define __af_xl_lambda_arg_xl_t(__NAME, N)	__af_xl_lambda_arg_tr(__NAME, N)::xl_target_type

#define __af_xl_signature( __F )	autotelica::xloper::xl_signature::get_signature_string(__F)
#define __af_xl_qd_signature( __F )	autotelica::xloper::xl_signature::get_signature_string_qd(__F)
#define __af_xl_lambda_signature( __F )	autotelica::xloper::xl_signature::get_lambda_signature_string<decltype(__F)>()


// Helpers to extract argument names and help strings
#define __AF_XL_ARG_NAMES_0(__F, __FH, ...) ""
#define __AF_XL_ARGS_HELP_0(__F, __FH, ...) {}
#define __AF_XL_ARG_NAMES_1(__F, __FH, __A0, __A0H) #__A0
#define __AF_XL_ARGS_HELP_1(__F, __FH, __A0, __A0H) {__A0H}
#define __AF_XL_ARG_NAMES_2(__F, __FH, __A0, __A0H, __A1, __A1H) #__A0 ## "," ## #__A1
#define __AF_XL_ARGS_HELP_2(__F, __FH, __A0, __A0H, __A1, __A1H) {__A0H, __A1H}
#define __AF_XL_ARG_NAMES_3(__F, __FH, __A0, __A0H, __A1, __A1H, __A2, __A2H) #__A0 ## "," ## #__A1 ## "," ## #__A2
#define __AF_XL_ARGS_HELP_3(__F, __FH, __A0, __A0H, __A1, __A1H, __A2, __A2H) {__A0H, __A1H, __A2H}
#define __AF_XL_ARG_NAMES_4(__F, __FH, __A0, __A0H, __A1, __A1H, __A2, __A2H, __A3, __A3H) #__A0 ## "," ## #__A1 ## "," ## #__A2 ## "," ## #__A3
#define __AF_XL_ARGS_HELP_4(__F, __FH, __A0, __A0H, __A1, __A1H, __A2, __A2H, __A3, __A3H) {__A0H, __A1H, __A2H, __A3H}
#define __AF_XL_ARG_NAMES_5(__F, __FH, __A0, __A0H, __A1, __A1H, __A2, __A2H, __A3, __A3H, __A4, __A4H) #__A0 ## "," ## #__A1 ## "," ## #__A2 ## "," ## #__A3 ## "," ## #__A4
#define __AF_XL_ARGS_HELP_5(__F, __FH, __A0, __A0H, __A1, __A1H, __A2, __A2H, __A3, __A3H, __A4, __A4H) {__A0H, __A1H, __A2H, __A3H, __A4H}
#define __AF_XL_ARG_NAMES_6(__F, __FH, __A0, __A0H, __A1, __A1H, __A2, __A2H, __A3, __A3H, __A4, __A4H, __A5, __A5H) #__A0 ## "," ## #__A1 ## "," ## #__A2 ## "," ## #__A3 ## "," ## #__A4 ## "," ## #__A5
#define __AF_XL_ARGS_HELP_6(__F, __FH, __A0, __A0H, __A1, __A1H, __A2, __A2H, __A3, __A3H, __A4, __A4H, __A5, __A5H) {__A0H, __A1H, __A2H, __A3H, __A4H, __A5H}
#define __AF_XL_ARG_NAMES_7(__F, __FH, __A0, __A0H, __A1, __A1H, __A2, __A2H, __A3, __A3H, __A4, __A4H, __A5, __A5H, __A6, __A6H) #__A0 ## "," ## #__A1 ## "," ## #__A2 ## "," ## #__A3 ## "," ## #__A4 ## "," ## #__A5 ## "," ## #__A6
#define __AF_XL_ARGS_HELP_7(__F, __FH, __A0, __A0H, __A1, __A1H, __A2, __A2H, __A3, __A3H, __A4, __A4H, __A5, __A5H, __A6, __A6H) {__A0H, __A1H, __A2H, __A3H, __A4H, __A5H, __A6H}
#define __AF_XL_ARG_NAMES_8(__F, __FH, __A0, __A0H, __A1, __A1H, __A2, __A2H, __A3, __A3H, __A4, __A4H, __A5, __A5H, __A6, __A6H, __A7, __A7H) #__A0 ## "," ## #__A1 ## "," ## #__A2 ## "," ## #__A3 ## "," ## #__A4 ## "," ## #__A5 ## "," ## #__A6 ## "," ## #__A7
#define __AF_XL_ARGS_HELP_8(__F, __FH, __A0, __A0H, __A1, __A1H, __A2, __A2H, __A3, __A3H, __A4, __A4H, __A5, __A5H, __A6, __A6H, __A7, __A7H) {__A0H, __A1H, __A2H, __A3H, __A4H, __A5H, __A6H, __A7H}
#define __AF_XL_ARG_NAMES_9(__F, __FH, __A0, __A0H, __A1, __A1H, __A2, __A2H, __A3, __A3H, __A4, __A4H, __A5, __A5H, __A6, __A6H, __A7, __A7H, __A8, __A8H) #__A0 ## "," ## #__A1 ## "," ## #__A2 ## "," ## #__A3 ## "," ## #__A4 ## "," ## #__A5 ## "," ## #__A6 ## "," ## #__A7 ## "," ## #__A8
#define __AF_XL_ARGS_HELP_9(__F, __FH, __A0, __A0H, __A1, __A1H, __A2, __A2H, __A3, __A3H, __A4, __A4H, __A5, __A5H, __A6, __A6H, __A7, __A7H, __A8, __A8H) {__A0H, __A1H, __A2H, __A3H, __A4H, __A5H, __A6H, __A7H, __A8H}
#define __AF_XL_ARG_NAMES_10(__F, __FH, __A0, __A0H, __A1, __A1H, __A2, __A2H, __A3, __A3H, __A4, __A4H, __A5, __A5H, __A6, __A6H, __A7, __A7H, __A8, __A8H, __A9, __A9H) #__A0 ## "," ## #__A1 ## "," ## #__A2 ## "," ## #__A3 ## "," ## #__A4 ## "," ## #__A5 ## "," ## #__A6 ## "," ## #__A7 ## "," ## #__A8 ## "," ## #__A9
#define __AF_XL_ARGS_HELP_10(__F, __FH, __A0, __A0H, __A1, __A1H, __A2, __A2H, __A3, __A3H, __A4, __A4H, __A5, __A5H, __A6, __A6H, __A7, __A7H, __A8, __A8H, __A9, __A9H) {__A0H, __A1H, __A2H, __A3H, __A4H, __A5H, __A6H, __A7H, __A8H, __A9H}
#define __AF_XL_ARG_NAMES_11(__F, __FH, __A0, __A0H, __A1, __A1H, __A2, __A2H, __A3, __A3H, __A4, __A4H, __A5, __A5H, __A6, __A6H, __A7, __A7H, __A8, __A8H, __A9, __A9H, __A10, __A10H) #__A0 ## "," ## #__A1 ## "," ## #__A2 ## "," ## #__A3 ## "," ## #__A4 ## "," ## #__A5 ## "," ## #__A6 ## "," ## #__A7 ## "," ## #__A8 ## "," ## #__A9 ## "," ## #__A10
#define __AF_XL_ARGS_HELP_11(__F, __FH, __A0, __A0H, __A1, __A1H, __A2, __A2H, __A3, __A3H, __A4, __A4H, __A5, __A5H, __A6, __A6H, __A7, __A7H, __A8, __A8H, __A9, __A9H, __A10, __A10H) {__A0H, __A1H, __A2H, __A3H, __A4H, __A5H, __A6H, __A7H, __A8H, __A9H, __A10H}
#define __AF_XL_ARG_NAMES_12(__F, __FH, __A0, __A0H, __A1, __A1H, __A2, __A2H, __A3, __A3H, __A4, __A4H, __A5, __A5H, __A6, __A6H, __A7, __A7H, __A8, __A8H, __A9, __A9H, __A10, __A10H, __A11, __A11H) #__A0 ## "," ## #__A1 ## "," ## #__A2 ## "," ## #__A3 ## "," ## #__A4 ## "," ## #__A5 ## "," ## #__A6 ## "," ## #__A7 ## "," ## #__A8 ## "," ## #__A9 ## "," ## #__A10 ## "," ## #__A11
#define __AF_XL_ARGS_HELP_12(__F, __FH, __A0, __A0H, __A1, __A1H, __A2, __A2H, __A3, __A3H, __A4, __A4H, __A5, __A5H, __A6, __A6H, __A7, __A7H, __A8, __A8H, __A9, __A9H, __A10, __A10H, __A11, __A11H) {__A0H, __A1H, __A2H, __A3H, __A4H, __A5H, __A6H, __A7H, __A8H, __A9H, __A10H, __A11H}


// not all argument sized lists are valid
#define __AF_XL_ARG_NAMES_ERROR( ... ) static_assert(false, "Declaration of Excel Function has a wrong number of arguments.")
#define __AF_XL_ARGS_HELP_ERROR( ... ) static_assert(false, "Declaration of Excel Function has a wrong number of arguments.")

#define __AF_XL_ARG_NAMES_DISPATCH( _26, _25, _24, _23, _22, _21, _20, _19, _18, _17, _16, _15, _14, _13, _12, _11, _10, _9, _8, _7, _6, _5, _4, _3, _2, _1, N, ...) __AF_XL_ARG_NAMES##N
#define __AF_XL_ARGS_HELP_DISPATCH( _26, _25, _24, _23, _22, _21, _20, _19, _18, _17, _16, _15, _14, _13, _12, _11, _10, _9, _8, _7, _6, _5, _4, _3, _2, _1, N, ...) __AF_XL_ARGS_HELP##N

// lil' bit o' trickery to make macro dispatching macro work
#define __AF_XL_EXPAND( x ) x

//													  26	   25	24      23   22      21  20      19  18      17  16      15  14      13  12      11  10       9   8       7   6       5   4       3       2       1
#define __AF_XL_ARG_NAMES(...) \
		__AF_XL_EXPAND(\
			 __AF_XL_ARG_NAMES_DISPATCH(__VA_ARGS__, _12, _ERROR, _11, _ERROR, _10, _ERROR, _9, _ERROR, _8, _ERROR, _7, _ERROR, _6, _ERROR, _5, _ERROR, _4, _ERROR, _3, _ERROR, _2, _ERROR, _1, _0, _ERROR, _ERROR, N, ...)(__VA_ARGS__)\
		)\

#define __AF_XL_ARGS_HELP(...) \
		__AF_XL_EXPAND(\
			 __AF_XL_ARGS_HELP_DISPATCH(__VA_ARGS__, _12, _ERROR, _11, _ERROR, _10, _ERROR, _9, _ERROR, _8, _ERROR, _7, _ERROR, _6, _ERROR, _5, _ERROR, _4, _ERROR, _3, _ERROR, _2, _ERROR, _1, _0, _ERROR, _ERROR, N, ...)(__VA_ARGS__)\
		)\


	
// AF_FUNCTION_CATEGORY can be used to change the category of a group of functions
#ifdef AF_FUNCTION_CATEGORY
#define __AF_CURRENT_CATEGORY AF_FUNCTION_CATEGORY
#else
#define __AF_CURRENT_CATEGORY "__GLOBAL_CATEGORY__"
#endif

// Registration of Functions
#define __AF_REGISTER_FUNCTION(__F, __F_HELP, __ARG_NAMES, __ARGS_HELP ) namespace{\
	static const auto registering_##__F = autotelica::xloper::xl_registration::xl_f_registry::register_function(\
			__AF_CURRENT_CATEGORY, #__F, __AF_XL_IMPL_F_N_STR(__F),\
			__af_xl_signature( __F ), __af_xl_arg_count(__F),\
			false,\
			__F_HELP, __ARG_NAMES, __ARGS_HELP);\
	}

#define __AF_REGISTER_VOLATILE_FUNCTION(__F, __F_HELP, __ARG_NAMES, __ARGS_HELP )  namespace{\
	static const auto registering_##__F = autotelica::xloper::xl_registration::xl_f_registry::register_function(\
			__AF_CURRENT_CATEGORY, #__F, __AF_XL_IMPL_F_N_STR(__F),\
			__af_xl_signature( __F ), __af_xl_arg_count(__F),\
			true,\
			__F_HELP, __ARG_NAMES, __ARGS_HELP);\
	}

#define __AF_REGISTER_NAMED_FUNCTION(__NAME, __F, __F_HELP, __ARG_NAMES, __ARGS_HELP ) namespace{\
	static const auto registering_##__NAME = autotelica::xloper::xl_registration::xl_f_registry::register_function(\
			__AF_CURRENT_CATEGORY, #__NAME, __AF_XL_IMPL_F_N_STR(__NAME),\
			__af_xl_signature( __F ), __af_xl_arg_count(__F),\
			false,\
			__F_HELP, __ARG_NAMES, __ARGS_HELP);\
	}

#define __AF_REGISTER_VOLATILE_NAMED_FUNCTION(__NAME, __F, __F_HELP, __ARG_NAMES, __ARGS_HELP )  namespace{\
	static const auto registering_##__NAME = autotelica::xloper::xl_registration::xl_f_registry::register_function(\
			__AF_CURRENT_CATEGORY, #__NAME, __AF_XL_IMPL_F_N_STR(__NAME),\
			__af_xl_signature( __F ), __af_xl_arg_count(__F),\
			true,\
			__F_HELP, __ARG_NAMES, __ARGS_HELP);\
	}
#define __AF_REGISTER_QD_FUNCTION(__F )  namespace{\
	static const auto registering_##__F = autotelica::xloper::xl_registration::xl_f_registry::register_function(\
			__AF_CURRENT_CATEGORY, #__F, __AF_XL_IMPL_QD_F_N_STR(__F),\
			__af_xl_qd_signature( __F ), __af_xl_arg_count(__F),\
			false);\
	}

#define __AF_REGISTER_QD_VOLATILE_FUNCTION(__F )  namespace{\
	static const auto registering_##__F = autotelica::xloper::xl_registration::xl_f_registry::register_function(\
			__AF_CURRENT_CATEGORY, #__F, __AF_XL_IMPL_QD_F_N_STR(__F),\
			__af_xl_qd_signature( __F ), __af_xl_arg_count(__F),\
			true);\
	}

#define __AF_REGISTER_LAMBDA_FUNCTION(__NAME, __F, __F_HELP, __ARG_NAMES, __ARGS_HELP ) namespace{\
	static const auto registering_##__NAME = autotelica::xloper::xl_registration::xl_f_registry::register_function(\
			__AF_CURRENT_CATEGORY, #__NAME, __AF_XL_IMPL_LAMBDA_F_N_STR( __NAME ),\
			__af_xl_lambda_signature( __AF_XL_DECL_LAMBDA_F_N(__NAME) ), __af_xl_lambda_arg_count(__AF_XL_DECL_LAMBDA_F_N(__NAME)),\
			false,\
			__F_HELP, __ARG_NAMES, __ARGS_HELP);\
	}

#define __AF_REGISTER_LAMBDA_VOLATILE_FUNCTION(__NAME, __F, __F_HELP, __ARG_NAMES, __ARGS_HELP )  namespace{\
	static const auto registering_##__F = autotelica::xloper::xl_registration::xl_f_registry::register_function(\
			__AF_CURRENT_CATEGORY, #__NAME, __AF_XL_IMPL_LAMBDA_F_N_STR( __NAME ),\
			__af_xl_lambda_signature( __AF_XL_DECL_LAMBDA_F_N(__NAME) ), __af_xl_lambda_arg_count(__AF_XL_DECL_LAMBDA_F_N(__NAME)),\
			true,\
			__F_HELP, __ARG_NAMES, __ARGS_HELP);\
	}

#define __AF_REGISTER_QD_LAMBDA_FUNCTION(__NAME )  namespace{\
	static const auto registering_##__NAME = autotelica::xloper::xl_registration::xl_f_registry::register_function(\
			__AF_CURRENT_CATEGORY, #__NAME, __AF_XL_IMPL_LAMBDA_F_N_STR( __NAME ),\
			__af_xl_lambda_signature( __AF_XL_DECL_LAMBDA_F_N(__NAME) ), __af_xl_lambda_arg_count(__AF_XL_DECL_LAMBDA_F_N(__NAME)),\
			false);\
	}

#define __AF_REGISTER_QD_LAMBDA_VOLATILE_FUNCTION(__NAME)  namespace{\
	static const auto registering_##__NAME = autotelica::xloper::xl_registration::xl_f_registry::register_function(\
			__AF_CURRENT_CATEGORY, #__NAME, __AF_XL_IMPL_LAMBDA_F_N_STR(__NAME),\
			__af_xl_lambda_signature( __AF_XL_DECL_LAMBDA_F_N(__NAME) ), __af_xl_lambda_arg_count(__AF_XL_DECL_LAMBDA_F_N(__NAME)),\
			true);\
	}

// Registration of functions with Category included
#define __AF_REGISTER_FUNCTION_CAT(__CAT, __F, __F_HELP, __ARG_NAMES, __ARGS_HELP ) namespace{\
	static const auto registering_##__F = autotelica::xloper::xl_registration::xl_f_registry::register_function(\
			__CAT, #__F, __AF_XL_IMPL_F_N_STR(__F),\
			__af_xl_signature( __F ), __af_xl_arg_count(__F),\
			false,\
			__F_HELP, __ARG_NAMES, __ARGS_HELP);\
	}

#define __AF_REGISTER_VOLATILE_FUNCTION_CAT(__CAT, __F, __F_HELP, __ARG_NAMES, __ARGS_HELP )  namespace{\
	static const auto registering_##__F = autotelica::xloper::xl_registration::xl_f_registry::register_function(\
			__CAT, #__F, __AF_XL_IMPL_F_N_STR(__F),\
			__af_xl_signature( __F ), __af_xl_arg_count(__F),\
			true,\
			__F_HELP, __ARG_NAMES, __ARGS_HELP);\
	}

#define __AF_REGISTER_NAMED_FUNCTION_CAT(__CAT, __NAME, __F, __F_HELP, __ARG_NAMES, __ARGS_HELP ) namespace{\
	static const auto registering_##__NAME = autotelica::xloper::xl_registration::xl_f_registry::register_function(\
			__CAT, #__NAME, __AF_XL_IMPL_F_N_STR(__NAME),\
			__af_xl_signature( __F ), __af_xl_arg_count(__F),\
			false,\
			__F_HELP, __ARG_NAMES, __ARGS_HELP);\
	}

#define __AF_REGISTER_VOLATILE_NAMED_FUNCTION_CAT(__CAT, __NAME, __F, __F_HELP, __ARG_NAMES, __ARGS_HELP )  namespace{\
	static const auto registering_##__NAME = autotelica::xloper::xl_registration::xl_f_registry::register_function(\
			__CAT, #__NAME, __AF_XL_IMPL_F_N_STR(__NAME),\
			__af_xl_signature( __F ), __af_xl_arg_count(__F),\
			true,\
			__F_HELP, __ARG_NAMES, __ARGS_HELP);\
	}
#define __AF_REGISTER_QD_FUNCTION_CAT(__CAT, __F )  namespace{\
	static const auto registering_##__F = autotelica::xloper::xl_registration::xl_f_registry::register_function(\
			__CAT, #__F, __AF_XL_IMPL_QD_F_N_STR(__F),\
			__af_xl_qd_signature( __F ), __af_xl_arg_count(__F),\
			false);\
	}

#define __AF_REGISTER_QD_VOLATILE_FUNCTION_CAT(__CAT, __F )  namespace{\
	static const auto registering_##__F = autotelica::xloper::xl_registration::xl_f_registry::register_function(\
			__CAT, #__F, __AF_XL_IMPL_QD_F_N_STR(__F),\
			__af_xl_qd_signature( __F ), __af_xl_arg_count(__F),\
			true);\
	}

#define __AF_REGISTER_LAMBDA_FUNCTION_CAT(__CAT, __NAME, __F, __F_HELP, __ARG_NAMES, __ARGS_HELP ) namespace{\
	static const auto registering_##__NAME = autotelica::xloper::xl_registration::xl_f_registry::register_function(\
			__CAT, #__NAME, __AF_XL_IMPL_LAMBDA_F_N_STR( __NAME ),\
			__af_xl_lambda_signature( __AF_XL_DECL_LAMBDA_F_N(__NAME) ), __af_xl_lambda_arg_count(__AF_XL_DECL_LAMBDA_F_N(__NAME)),\
			false,\
			__F_HELP, __ARG_NAMES, __ARGS_HELP);\
	}

#define __AF_REGISTER_LAMBDA_VOLATILE_FUNCTION_CAT(__CAT, __NAME, __F, __F_HELP, __ARG_NAMES, __ARGS_HELP )  namespace{\
	static const auto registering_##__F = autotelica::xloper::xl_registration::xl_f_registry::register_function(\
			__CAT, #__NAME, __AF_XL_IMPL_LAMBDA_F_N_STR( __NAME ),\
			__af_xl_lambda_signature( __AF_XL_DECL_LAMBDA_F_N(__NAME) ), __af_xl_lambda_arg_count(__AF_XL_DECL_LAMBDA_F_N(__NAME)),\
			true,\
			__F_HELP, __ARG_NAMES, __ARGS_HELP);\
	}

#define __AF_REGISTER_QD_LAMBDA_FUNCTION_CAT(__CAT, __NAME )  namespace{\
	static const auto registering_##__NAME = autotelica::xloper::xl_registration::xl_f_registry::register_function(\
			__CAT, #__NAME, __AF_XL_IMPL_LAMBDA_F_N_STR( __NAME ),\
			__af_xl_lambda_signature( __AF_XL_DECL_LAMBDA_F_N(__NAME) ), __af_xl_lambda_arg_count(__AF_XL_DECL_LAMBDA_F_N(__NAME)),\
			false);\
	}

#define __AF_REGISTER_QD_LAMBDA_VOLATILE_FUNCTION_CAT(__CAT, __NAME)  namespace{\
	static const auto registering_##__NAME = autotelica::xloper::xl_registration::xl_f_registry::register_function(\
			__CAT, #__NAME, __AF_XL_IMPL_LAMBDA_F_N_STR(__NAME),\
			__af_xl_lambda_signature( __AF_XL_DECL_LAMBDA_F_N(__NAME) ), __af_xl_lambda_arg_count(__AF_XL_DECL_LAMBDA_F_N(__NAME)),\
			true);\
	}



// Dispatchers for function registration, with names and help text included
#define __AF_REGISTER_FUNCTION_DISPATCH(__F, __F_HELP, ... )\
	__AF_XL_EXPAND(\
		__AF_REGISTER_FUNCTION(__F, __F_HELP,\
			__AF_XL_ARG_NAMES(__F, __F_HELP, __VA_ARGS__),\
			__AF_XL_ARGS_HELP(__F, __F_HELP, __VA_ARGS__))\
	)


#define __AF_REGISTER_VOLATILE_FUNCTION_DISPATCH(__F, __F_HELP, ... )\
	__AF_XL_EXPAND(\
		__AF_REGISTER_VOLATILE_FUNCTION(__F, __F_HELP,\
			__AF_XL_ARG_NAMES(__F, __F_HELP, __VA_ARGS__), \
			__AF_XL_ARGS_HELP(__F, __F_HELP, __VA_ARGS__))\
	)

#define __AF_REGISTER_NAMED_FUNCTION_DISPATCH(__NAME, __F, __F_HELP, ... )\
	__AF_XL_EXPAND(\
		__AF_REGISTER_NAMED_FUNCTION(__NAME, __F, __F_HELP,\
			__AF_XL_ARG_NAMES(__F, __F_HELP, __VA_ARGS__),\
			__AF_XL_ARGS_HELP(__F, __F_HELP, __VA_ARGS__))\
	)


#define __AF_REGISTER_VOLATILE_NAMED_FUNCTION_DISPATCH(__NAME, __F, __F_HELP, ... )\
	__AF_XL_EXPAND(\
		__AF_REGISTER_VOLATILE_NAMED_FUNCTION(__NAME, __F, __F_HELP,\
			__AF_XL_ARG_NAMES(__F, __F_HELP, __VA_ARGS__), \
			__AF_XL_ARGS_HELP(__F, __F_HELP, __VA_ARGS__))\
	)

#define __AF_REGISTER_LAMBDA_FUNCTION_DISPATCH(__NAME, __F, __F_HELP, ... )\
	__AF_XL_EXPAND(\
		__AF_REGISTER_LAMBDA_FUNCTION(__NAME, __F, __F_HELP,\
			__AF_XL_ARG_NAMES(__NAME, __F_HELP, __VA_ARGS__),\
			__AF_XL_ARGS_HELP(__NAME, __F_HELP, __VA_ARGS__))\
	)


#define __AF_REGISTER_LAMBDA_VOLATILE_FUNCTION_DISPATCH(__NAME, __F, __F_HELP, ... )\
	__AF_XL_EXPAND(\
		__AF_REGISTER_LAMBDA_VOLATILE_FUNCTION(__NAME, __F, __F_HELP,\
		__AF_XL_ARG_NAMES(__NAME, __F_HELP, __VA_ARGS__), \
		__AF_XL_ARGS_HELP(__NAME, __F_HELP, __VA_ARGS__))\
	)

// Dispatchers like above, but including function categories
#define __AF_REGISTER_FUNCTION_DISPATCH_CAT(__CAT, __F, __F_HELP, ... )\
	__AF_XL_EXPAND(\
		__AF_REGISTER_FUNCTION_CAT(__CAT, __F, __F_HELP,\
			__AF_XL_ARG_NAMES(__F, __F_HELP, __VA_ARGS__),\
			__AF_XL_ARGS_HELP(__F, __F_HELP, __VA_ARGS__))\
	)


#define __AF_REGISTER_VOLATILE_FUNCTION_DISPATCH_CAT(__CAT, __F, __F_HELP, ... )\
	__AF_XL_EXPAND(\
		__AF_REGISTER_VOLATILE_FUNCTION_CAT(__CAT, __F, __F_HELP,\
			__AF_XL_ARG_NAMES(__F, __F_HELP, __VA_ARGS__), \
			__AF_XL_ARGS_HELP(__F, __F_HELP, __VA_ARGS__))\
	)

#define __AF_REGISTER_NAMED_FUNCTION_DISPATCH_CAT(__CAT, __NAME, __F, __F_HELP, ... )\
	__AF_XL_EXPAND(\
		__AF_REGISTER_NAMED_FUNCTION_CAT(__CAT, __NAME, __F, __F_HELP,\
			__AF_XL_ARG_NAMES(__F, __F_HELP, __VA_ARGS__),\
			__AF_XL_ARGS_HELP(__F, __F_HELP, __VA_ARGS__))\
	)


#define __AF_REGISTER_VOLATILE_NAMED_FUNCTION_DISPATCH_CAT(__CAT, __NAME, __F, __F_HELP, ... )\
	__AF_XL_EXPAND(\
		__AF_REGISTER_VOLATILE_NAMED_FUNCTION_CAT(__CAT, __NAME, __F, __F_HELP,\
			__AF_XL_ARG_NAMES(__F, __F_HELP, __VA_ARGS__), \
			__AF_XL_ARGS_HELP(__F, __F_HELP, __VA_ARGS__))\
	)

#define __AF_REGISTER_LAMBDA_FUNCTION_DISPATCH_CAT(__CAT, __NAME, __F, __F_HELP, ... )\
	__AF_XL_EXPAND(\
		__AF_REGISTER_LAMBDA_FUNCTION_CAT(__CAT, __NAME, __F, __F_HELP,\
			__AF_XL_ARG_NAMES(__NAME, __F_HELP, __VA_ARGS__),\
			__AF_XL_ARGS_HELP(__NAME, __F_HELP, __VA_ARGS__))\
	)


#define __AF_REGISTER_LAMBDA_VOLATILE_FUNCTION_DISPATCH_CAT(__CAT, __NAME, __F, __F_HELP, ... )\
	__AF_XL_EXPAND(\
		__AF_REGISTER_LAMBDA_VOLATILE_FUNCTION_CAT(__CAT, __NAME, __F, __F_HELP,\
		__AF_XL_ARG_NAMES(__NAME, __F_HELP, __VA_ARGS__), \
		__AF_XL_ARGS_HELP(__NAME, __F_HELP, __VA_ARGS__))\
	)


	
// Error handling
#define __AF_XL_TRY try {\
	autotelica::xloper::xl_util::check_wizard();

#define __AF_XL_CATCH }\
		catch(autotelica::xloper::xl_errors::xloper_exception const& e) { return e.errorXlp(); }\
		catch(std::runtime_error const& e) { return autotelica::xloper::xl_errors::translate_error(e); }\
		catch(...) { return autotelica::xloper::xl_errors::translate_error(); }

#define __AF_XL_CATCH_2(__F) }\
		catch(autotelica::xloper::xl_errors::xloper_exception const& e) { return autotelica::xloper::xl_conversions::to_return_type<__af_xl_ret_t(__F)>(e.errorXlp()); }\
		catch(std::runtime_error const& e) { return autotelica::xloper::xl_conversions::to_return_type<__af_xl_ret_t(__F)>(autotelica::xloper::xl_errors::translate_error(e)); }\
		catch(...) { return autotelica::xloper::xl_conversions::to_return_type<__af_xl_ret_t(__F)>(autotelica::xloper::xl_errors::translate_error()); }



// Macros for defining exposed function implementations
// They declare and implement functions that are actually exposed from the dll and to Excel
// The functions must be declared with a fixed signature, so we need one of these for every 
// number of arguments we want to support.
// To keep lines shorter, we use ugly mnemoics:
// __NAME = function name as seen by Excel
// __F = function name
// __FH = function help text
// __AX = Xth argument name
// __AXH == Xth argument help text

#define __AF_XL_FUNC_ERROR(...) static_assert(false, "Declaration of Excel Function has a wrong number of arguments.");
#define __AF_XL_FUNC_0(__F, __FH)\
	extern "C" __declspec(dllexport) __af_xl_ret_t(__F) __stdcall\
	__AF_XL_IMPL_F_N(__F)(\
	){ \
	 __AF_XL_TRY;\
		using namespace autotelica::xloper::xl_conversions;\
		return to_xlp( __F(\
		)); \
	 __AF_XL_CATCH_2(__F);\
	}

#define __AF_XL_FUNC_1(__F, __FH, __A0, __A0H)\
	extern "C" __declspec(dllexport) __af_xl_ret_t(__F) __stdcall\
	__AF_XL_IMPL_F_N(__F)(\
		__af_xl_arg_xl_t(__F, 0) __A0\
	){ \
	 __AF_XL_TRY;\
		using namespace autotelica::xloper::xl_conversions;\
		return to_xlp( __F(\
			from_xl<__af_xl_arg_t(__F, 0)>(__A0)\
		)); \
	 __AF_XL_CATCH_2(__F);\
	}

#define __AF_XL_FUNC_2(__F, __FH, __A0, __A0H, __A1, __A1H)\
	extern "C" __declspec(dllexport) __af_xl_ret_t(__F) __stdcall \
	__AF_XL_IMPL_F_N(__F)(\
		__af_xl_arg_xl_t(__F, 0) __A0,\
		__af_xl_arg_xl_t(__F, 1) __A1\
	){ \
	 __AF_XL_TRY;\
		using namespace autotelica::xloper::xl_conversions;\
		return to_xlp( __F(\
			from_xl<__af_xl_arg_t(__F, 0)>(__A0),\
			from_xl<__af_xl_arg_t(__F, 1)>(__A1)\
		)); \
	 __AF_XL_CATCH_2(__F);\
	}

#define __AF_XL_FUNC_3(__F, __FH, __A0, __A0H, __A1, __A1H, __A2, __A2H)\
	extern "C" __declspec(dllexport) __af_xl_ret_t(__F) __stdcall \
	__AF_XL_IMPL_F_N(__F)(\
		__af_xl_arg_xl_t(__F, 0) __A0,\
		__af_xl_arg_xl_t(__F, 1) __A1,\
		__af_xl_arg_xl_t(__F, 2) __A2\
	){ \
	 __AF_XL_TRY;\
		using namespace autotelica::xloper::xl_conversions;\
		return to_xlp( __F(\
			from_xl<__af_xl_arg_t(__F, 0)>(__A0),\
			from_xl<__af_xl_arg_t(__F, 1)>(__A1),\
			from_xl<__af_xl_arg_t(__F, 2)>(__A2)\
		)); \
	 __AF_XL_CATCH_2(__F);\
	}

#define __AF_XL_FUNC_4(__F, __FH, __A0, __A0H, __A1, __A1H, __A2, __A2H, __A3, __A3H)\
	extern "C" __declspec(dllexport) __af_xl_ret_t(__F) __stdcall \
	__AF_XL_IMPL_F_N(__F)(\
		__af_xl_arg_xl_t(__F, 0) __A0,\
		__af_xl_arg_xl_t(__F, 1) __A1,\
		__af_xl_arg_xl_t(__F, 2) __A2,\
		__af_xl_arg_xl_t(__F, 3) __A3\
	){ \
	 __AF_XL_TRY;\
		using namespace autotelica::xloper::xl_conversions;\
		return to_xlp( __F(\
			from_xl<__af_xl_arg_t(__F, 0)>(__A0),\
			from_xl<__af_xl_arg_t(__F, 1)>(__A1),\
			from_xl<__af_xl_arg_t(__F, 2)>(__A2),\
			from_xl<__af_xl_arg_t(__F, 3)>(__A3)\
		)); \
	 __AF_XL_CATCH_2(__F);\
	}

#define __AF_XL_FUNC_5(__F, __FH, __A0, __A0H, __A1, __A1H, __A2, __A2H, __A3, __A3H, __A4, __A4H)\
	extern "C" __declspec(dllexport) __af_xl_ret_t(__F) __stdcall \
	__AF_XL_IMPL_F_N(__F)(\
		__af_xl_arg_xl_t(__F, 0) __A0,\
		__af_xl_arg_xl_t(__F, 1) __A1,\
		__af_xl_arg_xl_t(__F, 2) __A2,\
		__af_xl_arg_xl_t(__F, 3) __A3,\
		__af_xl_arg_xl_t(__F, 4) __A4\
	){ \
	 __AF_XL_TRY;\
		using namespace autotelica::xloper::xl_conversions;\
		return to_xlp( __F(\
			from_xl<__af_xl_arg_t(__F, 0)>(__A0),\
			from_xl<__af_xl_arg_t(__F, 1)>(__A1),\
			from_xl<__af_xl_arg_t(__F, 2)>(__A2),\
			from_xl<__af_xl_arg_t(__F, 3)>(__A3),\
			from_xl<__af_xl_arg_t(__F, 4)>(__A4)\
		)); \
	 __AF_XL_CATCH_2(__F);\
	}

#define __AF_XL_FUNC_6(__F, __FH, __A0, __A0H, __A1, __A1H, __A2, __A2H, __A3, __A3H, __A4, __A4H, __A5, __A6H)\
	extern "C" __declspec(dllexport) __af_xl_ret_t(__F) __stdcall \
	__AF_XL_IMPL_F_N(__F)(\
		__af_xl_arg_xl_t(__F, 0) __A0,\
		__af_xl_arg_xl_t(__F, 1) __A1,\
		__af_xl_arg_xl_t(__F, 2) __A2,\
		__af_xl_arg_xl_t(__F, 3) __A3,\
		__af_xl_arg_xl_t(__F, 4) __A4,\
		__af_xl_arg_xl_t(__F, 5) __A5\
	){ \
	 __AF_XL_TRY;\
		using namespace autotelica::xloper::xl_conversions;\
		return to_xlp( __F(\
			from_xl<__af_xl_arg_t(__F, 0)>(__A0),\
			from_xl<__af_xl_arg_t(__F, 1)>(__A1),\
			from_xl<__af_xl_arg_t(__F, 2)>(__A2),\
			from_xl<__af_xl_arg_t(__F, 3)>(__A3),\
			from_xl<__af_xl_arg_t(__F, 4)>(__A4),\
			from_xl<__af_xl_arg_t(__F, 5)>(__A5)\
		)); \
	 __AF_XL_CATCH_2(__F);\
	}

#define __AF_XL_FUNC_7(__F, __FH, __A0, __A0H, __A1, __A1H, __A2, __A2H, __A3, __A3H, __A4, __A4H, __A5, __A5H, __A6, __A6H)\
	extern "C" __declspec(dllexport) __af_xl_ret_t(__F) __stdcall \
	__AF_XL_IMPL_F_N(__F)(\
		__af_xl_arg_xl_t(__F, 0) __A0,\
		__af_xl_arg_xl_t(__F, 1) __A1,\
		__af_xl_arg_xl_t(__F, 2) __A2,\
		__af_xl_arg_xl_t(__F, 3) __A3,\
		__af_xl_arg_xl_t(__F, 4) __A4,\
		__af_xl_arg_xl_t(__F, 5) __A5,\
		__af_xl_arg_xl_t(__F, 6) __A6\
	){ \
	 __AF_XL_TRY;\
		using namespace autotelica::xloper::xl_conversions;\
		return to_xlp( __F(\
			from_xl<__af_xl_arg_t(__F, 0)>(__A0),\
			from_xl<__af_xl_arg_t(__F, 1)>(__A1),\
			from_xl<__af_xl_arg_t(__F, 2)>(__A2),\
			from_xl<__af_xl_arg_t(__F, 3)>(__A3),\
			from_xl<__af_xl_arg_t(__F, 4)>(__A4),\
			from_xl<__af_xl_arg_t(__F, 5)>(__A5),\
			from_xl<__af_xl_arg_t(__F, 6)>(__A6)\
		)); \
	 __AF_XL_CATCH_2(__F);\
	}

#define __AF_XL_FUNC_8(__F, __FH, __A0, __A0H, __A1, __A1H, __A2, __A2H, __A3, __A3H, __A4, __A4H, __A5, __A5H, __A6, __A6H, __A7, __A7H)\
	extern "C" __declspec(dllexport) __af_xl_ret_t(__F) __stdcall \
	__AF_XL_IMPL_F_N(__F)(\
		__af_xl_arg_xl_t(__F, 0) __A0,\
		__af_xl_arg_xl_t(__F, 1) __A1,\
		__af_xl_arg_xl_t(__F, 2) __A2,\
		__af_xl_arg_xl_t(__F, 3) __A3,\
		__af_xl_arg_xl_t(__F, 4) __A4,\
		__af_xl_arg_xl_t(__F, 5) __A5,\
		__af_xl_arg_xl_t(__F, 6) __A6,\
		__af_xl_arg_xl_t(__F, 7) __A7\
	){ \
	 __AF_XL_TRY;\
		using namespace autotelica::xloper::xl_conversions;\
		return to_xlp( __F(\
			from_xl<__af_xl_arg_t(__F, 0)>(__A0),\
			from_xl<__af_xl_arg_t(__F, 1)>(__A1),\
			from_xl<__af_xl_arg_t(__F, 2)>(__A2),\
			from_xl<__af_xl_arg_t(__F, 3)>(__A3),\
			from_xl<__af_xl_arg_t(__F, 4)>(__A4),\
			from_xl<__af_xl_arg_t(__F, 5)>(__A5),\
			from_xl<__af_xl_arg_t(__F, 6)>(__A6),\
			from_xl<__af_xl_arg_t(__F, 7)>(__A7)\
		)); \
	 __AF_XL_CATCH_2(__F);\
	}

#define __AF_XL_FUNC_9(__F, __FH, __A0, __A0H, __A1, __A1H, __A2, __A2H, __A3, __A3H, __A4, __A4H, __A5, __A5H, __A6, __A6H, __A7, __A7H, __A8, __A8H)\
	extern "C" __declspec(dllexport) __af_xl_ret_t(__F) __stdcall \
	__AF_XL_IMPL_F_N(__F)(\
		__af_xl_arg_xl_t(__F, 0) __A0,\
		__af_xl_arg_xl_t(__F, 1) __A1,\
		__af_xl_arg_xl_t(__F, 2) __A2,\
		__af_xl_arg_xl_t(__F, 3) __A3,\
		__af_xl_arg_xl_t(__F, 4) __A4,\
		__af_xl_arg_xl_t(__F, 5) __A5,\
		__af_xl_arg_xl_t(__F, 6) __A6,\
		__af_xl_arg_xl_t(__F, 7) __A7,\
		__af_xl_arg_xl_t(__F, 8) __A8\
	){ \
	 __AF_XL_TRY;\
		using namespace autotelica::xloper::xl_conversions;\
		return to_xlp( __F(\
			from_xl<__af_xl_arg_t(__F, 0)>(__A0),\
			from_xl<__af_xl_arg_t(__F, 1)>(__A1),\
			from_xl<__af_xl_arg_t(__F, 2)>(__A2),\
			from_xl<__af_xl_arg_t(__F, 3)>(__A3),\
			from_xl<__af_xl_arg_t(__F, 4)>(__A4),\
			from_xl<__af_xl_arg_t(__F, 5)>(__A5),\
			from_xl<__af_xl_arg_t(__F, 6)>(__A6),\
			from_xl<__af_xl_arg_t(__F, 7)>(__A7),\
			from_xl<__af_xl_arg_t(__F, 8)>(__A8)\
		)); \
	 __AF_XL_CATCH_2(__F);\
	}

#define __AF_XL_FUNC_10(__F, __FH, __A0, __A0H, __A1, __A1H, __A2, __A2H, __A3, __A3H, __A4, __A4H, __A5, __A5H, __A6, __A6H, __A7, __A7H, __A8, __A8H, __A9, __A9H)\
	extern "C" __declspec(dllexport) __af_xl_ret_t(__F) __stdcall \
	__AF_XL_IMPL_F_N(__F)(\
		__af_xl_arg_xl_t(__F, 0) __A0,\
		__af_xl_arg_xl_t(__F, 1) __A1,\
		__af_xl_arg_xl_t(__F, 2) __A2,\
		__af_xl_arg_xl_t(__F, 3) __A3,\
		__af_xl_arg_xl_t(__F, 4) __A4,\
		__af_xl_arg_xl_t(__F, 5) __A5,\
		__af_xl_arg_xl_t(__F, 6) __A6,\
		__af_xl_arg_xl_t(__F, 7) __A7,\
		__af_xl_arg_xl_t(__F, 8) __A8,\
		__af_xl_arg_xl_t(__F, 9) __A9\
	){ \
	 __AF_XL_TRY;\
		using namespace autotelica::xloper::xl_conversions;\
		return to_xlp( __F(\
			from_xl<__af_xl_arg_t(__F, 0)>(__A0),\
			from_xl<__af_xl_arg_t(__F, 1)>(__A1),\
			from_xl<__af_xl_arg_t(__F, 2)>(__A2),\
			from_xl<__af_xl_arg_t(__F, 3)>(__A3),\
			from_xl<__af_xl_arg_t(__F, 4)>(__A4),\
			from_xl<__af_xl_arg_t(__F, 5)>(__A5),\
			from_xl<__af_xl_arg_t(__F, 6)>(__A6),\
			from_xl<__af_xl_arg_t(__F, 7)>(__A7),\
			from_xl<__af_xl_arg_t(__F, 8)>(__A8),\
			from_xl<__af_xl_arg_t(__F, 9)>(__A9)\
		)); \
	 __AF_XL_CATCH_2(__F);\
	}

#define __AF_XL_FUNC_11(__F, __FH, __A0, __A0H, __A1, __A1H, __A2, __A2H, __A3, __A3H, __A4, __A4H, __A5, __A5H, __A6, __A6H, __A7, __A7H, __A8, __A8H, __A9, __A9H, __A10, __A10H)\
	extern "C" __declspec(dllexport) __af_xl_ret_t(__F) __stdcall \
	__AF_XL_IMPL_F_N(__F)(\
		__af_xl_arg_xl_t(__F, 0) __A0,\
		__af_xl_arg_xl_t(__F, 1) __A1,\
		__af_xl_arg_xl_t(__F, 2) __A2,\
		__af_xl_arg_xl_t(__F, 3) __A3,\
		__af_xl_arg_xl_t(__F, 4) __A4,\
		__af_xl_arg_xl_t(__F, 5) __A5,\
		__af_xl_arg_xl_t(__F, 6) __A6,\
		__af_xl_arg_xl_t(__F, 7) __A7,\
		__af_xl_arg_xl_t(__F, 8) __A8,\
		__af_xl_arg_xl_t(__F, 9) __A9,\
		__af_xl_arg_xl_t(__F, 10) __A10\
	){ \
	 __AF_XL_TRY;\
		return to_xlp( __F(\
			from_xl<__af_xl_arg_t(__F, 0)>(__A0),\
			from_xl<__af_xl_arg_t(__F, 1)>(__A1),\
			from_xl<__af_xl_arg_t(__F, 2)>(__A2),\
			from_xl<__af_xl_arg_t(__F, 3)>(__A3),\
			from_xl<__af_xl_arg_t(__F, 4)>(__A4),\
			from_xl<__af_xl_arg_t(__F, 5)>(__A5),\
			from_xl<__af_xl_arg_t(__F, 6)>(__A6),\
			from_xl<__af_xl_arg_t(__F, 7)>(__A7),\
			from_xl<__af_xl_arg_t(__F, 8)>(__A8),\
			from_xl<__af_xl_arg_t(__F, 9)>(__A9),\
			from_xl<__af_xl_arg_t(__F, 10)>(__A10)\
		)); \
	 __AF_XL_CATCH_2(__F);\
	}

#define __AF_XL_FUNC_12(__F, __FH, __A0, __A0H, __A1, __A1H, __A2, __A2H, __A3, __A3H, __A4, __A4H, __A5, __A5H, __A6, __A6H, __A7, __A7H, __A8, __A8H, __A9, __A9H, __A10, __A10H, __A11, __A11H)\
	extern "C" __declspec(dllexport) __af_xl_ret_t(__F) __stdcall \
	__AF_XL_IMPL_F_N(__F)(\
		__af_xl_arg_xl_t(__F, 0) __A0,\
		__af_xl_arg_xl_t(__F, 1) __A1,\
		__af_xl_arg_xl_t(__F, 2) __A2,\
		__af_xl_arg_xl_t(__F, 3) __A3,\
		__af_xl_arg_xl_t(__F, 4) __A4,\
		__af_xl_arg_xl_t(__F, 5) __A5,\
		__af_xl_arg_xl_t(__F, 6) __A6,\
		__af_xl_arg_xl_t(__F, 7) __A7,\
		__af_xl_arg_xl_t(__F, 8) __A8,\
		__af_xl_arg_xl_t(__F, 9) __A9,\
		__af_xl_arg_xl_t(__F, 10) __A10,\
		__af_xl_arg_xl_t(__F, 11) __A11\
	){ \
	 __AF_XL_TRY;\
		return to_xlp( __F(\
			from_xl<__af_xl_arg_t(__F, 0)>(__A0),\
			from_xl<__af_xl_arg_t(__F, 1)>(__A1),\
			from_xl<__af_xl_arg_t(__F, 2)>(__A2),\
			from_xl<__af_xl_arg_t(__F, 3)>(__A3),\
			from_xl<__af_xl_arg_t(__F, 4)>(__A4),\
			from_xl<__af_xl_arg_t(__F, 5)>(__A5),\
			from_xl<__af_xl_arg_t(__F, 6)>(__A6),\
			from_xl<__af_xl_arg_t(__F, 7)>(__A7),\
			from_xl<__af_xl_arg_t(__F, 8)>(__A8),\
			from_xl<__af_xl_arg_t(__F, 9)>(__A9),\
			from_xl<__af_xl_arg_t(__F, 10)>(__A10),\
			from_xl<__af_xl_arg_t(__F, 11)>(__A11)\
		)); \
	 __AF_XL_CATCH_2(__F);\
	}

#define __AF_XL_NAMED_FUNC_ERROR(...) static_assert(false, "Declaration of Excel Function has a wrong number of arguments.");
#define __AF_XL_NAMED_FUNC_0(__NAME, __F, __FH)\
	extern "C" __declspec(dllexport) LPXLOPER12 __stdcall\
	__AF_XL_IMPL_F_N(__NAME)(\
	){ \
	 __AF_XL_TRY;\
		using namespace autotelica::xloper::xl_conversions;\
		return to_xlp( __F(\
			\
		)); \
	 __AF_XL_CATCH;\
	}

#define __AF_XL_NAMED_FUNC_1(__NAME, __F, __FH, __A0, __A0H)\
	extern "C" __declspec(dllexport) LPXLOPER12 __stdcall\
	__AF_XL_IMPL_F_N(__NAME)(\
		__af_xl_arg_xl_t(__F, 0) __A0\
	){ \
	 __AF_XL_TRY;\
		using namespace autotelica::xloper::xl_conversions;\
		return to_xlp( __F(\
			from_xl<__af_xl_arg_t(__F, 0)>(__A0)\
		)); \
	 __AF_XL_CATCH;\
	}

#define __AF_XL_NAMED_FUNC_2(__NAME, __F, __FH, __A0, __A0H, __A1, __A1H)\
	extern "C" __declspec(dllexport) LPXLOPER12 __stdcall \
	__AF_XL_IMPL_F_N(__NAME)(\
		__af_xl_arg_xl_t(__F, 0) __A0,\
		__af_xl_arg_xl_t(__F, 1) __A1\
	){ \
	 __AF_XL_TRY;\
		using namespace autotelica::xloper::xl_conversions;\
		return to_xlp( __F(\
			from_xl<__af_xl_arg_t(__F, 0)>(__A0),\
			from_xl<__af_xl_arg_t(__F, 1)>(__A1)\
		)); \
	 __AF_XL_CATCH;\
	}

#define __AF_XL_NAMED_FUNC_3(__NAME, __F, __FH, __A0, __A0H, __A1, __A1H, __A2, __A2H)\
	extern "C" __declspec(dllexport) LPXLOPER12 __stdcall \
	__AF_XL_IMPL_F_N(__NAME)(\
		__af_xl_arg_xl_t(__F, 0) __A0,\
		__af_xl_arg_xl_t(__F, 1) __A1,\
		__af_xl_arg_xl_t(__F, 2) __A2\
	){ \
	 __AF_XL_TRY;\
		using namespace autotelica::xloper::xl_conversions;\
		return to_xlp( __F(\
			from_xl<__af_xl_arg_t(__F, 0)>(__A0),\
			from_xl<__af_xl_arg_t(__F, 1)>(__A1),\
			from_xl<__af_xl_arg_t(__F, 2)>(__A2)\
		)); \
	 __AF_XL_CATCH;\
	}

#define __AF_XL_NAMED_FUNC_4(__NAME, __F, __FH, __A0, __A0H, __A1, __A1H, __A2, __A2H, __A3, __A3H)\
	extern "C" __declspec(dllexport) LPXLOPER12 __stdcall \
	__AF_XL_IMPL_F_N(__NAME)(\
		__af_xl_arg_xl_t(__F, 0) __A0,\
		__af_xl_arg_xl_t(__F, 1) __A1,\
		__af_xl_arg_xl_t(__F, 2) __A2,\
		__af_xl_arg_xl_t(__F, 3) __A3\
	){ \
	 __AF_XL_TRY;\
		using namespace autotelica::xloper::xl_conversions;\
		return to_xlp( __F(\
			from_xl<__af_xl_arg_t(__F, 0)>(__A0),\
			from_xl<__af_xl_arg_t(__F, 1)>(__A1),\
			from_xl<__af_xl_arg_t(__F, 2)>(__A2),\
			from_xl<__af_xl_arg_t(__F, 3)>(__A3)\
		)); \
	 __AF_XL_CATCH;\
	}

#define __AF_XL_NAMED_FUNC_5(__NAME, __F, __FH, __A0, __A0H, __A1, __A1H, __A2, __A2H, __A3, __A3H, __A4, __A4H)\
	extern "C" __declspec(dllexport) LPXLOPER12 __stdcall \
	__AF_XL_IMPL_F_N(__NAME)(\
		__af_xl_arg_xl_t(__F, 0) __A0,\
		__af_xl_arg_xl_t(__F, 1) __A1,\
		__af_xl_arg_xl_t(__F, 2) __A2,\
		__af_xl_arg_xl_t(__F, 3) __A3,\
		__af_xl_arg_xl_t(__F, 4) __A4\
	){ \
	 __AF_XL_TRY;\
		using namespace autotelica::xloper::xl_conversions;\
		return to_xlp( __F(\
			from_xl<__af_xl_arg_t(__F, 0)>(__A0),\
			from_xl<__af_xl_arg_t(__F, 1)>(__A1),\
			from_xl<__af_xl_arg_t(__F, 2)>(__A2),\
			from_xl<__af_xl_arg_t(__F, 3)>(__A3),\
			from_xl<__af_xl_arg_t(__F, 4)>(__A4)\
		)); \
	 __AF_XL_CATCH;\
	}

#define __AF_XL_NAMED_FUNC_6(__NAME, __F, __FH, __A0, __A0H, __A1, __A1H, __A2, __A2H, __A3, __A3H, __A4, __A4H, __A5, __A6H)\
	extern "C" __declspec(dllexport) LPXLOPER12 __stdcall \
	__AF_XL_IMPL_F_N(__NAME)(\
		__af_xl_arg_xl_t(__F, 0) __A0,\
		__af_xl_arg_xl_t(__F, 1) __A1,\
		__af_xl_arg_xl_t(__F, 2) __A2,\
		__af_xl_arg_xl_t(__F, 3) __A3,\
		__af_xl_arg_xl_t(__F, 4) __A4,\
		__af_xl_arg_xl_t(__F, 5) __A5\
	){ \
	 __AF_XL_TRY;\
		using namespace autotelica::xloper::xl_conversions;\
		return to_xlp( __F(\
			from_xl<__af_xl_arg_t(__F, 0)>(__A0),\
			from_xl<__af_xl_arg_t(__F, 1)>(__A1),\
			from_xl<__af_xl_arg_t(__F, 2)>(__A2),\
			from_xl<__af_xl_arg_t(__F, 3)>(__A3),\
			from_xl<__af_xl_arg_t(__F, 4)>(__A4),\
			from_xl<__af_xl_arg_t(__F, 5)>(__A5)\
		)); \
	 __AF_XL_CATCH;\
	}

#define __AF_XL_NAMED_FUNC_7(__NAME, __F, __FH, __A0, __A0H, __A1, __A1H, __A2, __A2H, __A3, __A3H, __A4, __A4H, __A5, __A5H, __A6, __A6H)\
	extern "C" __declspec(dllexport) LPXLOPER12 __stdcall \
	__AF_XL_IMPL_F_N(__NAME)(\
		__af_xl_arg_xl_t(__F, 0) __A0,\
		__af_xl_arg_xl_t(__F, 1) __A1,\
		__af_xl_arg_xl_t(__F, 2) __A2,\
		__af_xl_arg_xl_t(__F, 3) __A3,\
		__af_xl_arg_xl_t(__F, 4) __A4,\
		__af_xl_arg_xl_t(__F, 5) __A5,\
		__af_xl_arg_xl_t(__F, 6) __A6\
	){ \
	 __AF_XL_TRY;\
		using namespace autotelica::xloper::xl_conversions;\
		return to_xlp( __F(\
			from_xl<__af_xl_arg_t(__F, 0)>(__A0),\
			from_xl<__af_xl_arg_t(__F, 1)>(__A1),\
			from_xl<__af_xl_arg_t(__F, 2)>(__A2),\
			from_xl<__af_xl_arg_t(__F, 3)>(__A3),\
			from_xl<__af_xl_arg_t(__F, 4)>(__A4),\
			from_xl<__af_xl_arg_t(__F, 5)>(__A5),\
			from_xl<__af_xl_arg_t(__F, 6)>(__A6)\
		)); \
	 __AF_XL_CATCH;\
	}

#define __AF_XL_NAMED_FUNC_8(__NAME, __F, __FH, __A0, __A0H, __A1, __A1H, __A2, __A2H, __A3, __A3H, __A4, __A4H, __A5, __A5H, __A6, __A6H, __A7, __A7H)\
	extern "C" __declspec(dllexport) LPXLOPER12 __stdcall \
	__AF_XL_IMPL_F_N(__NAME)(\
		__af_xl_arg_xl_t(__F, 0) __A0,\
		__af_xl_arg_xl_t(__F, 1) __A1,\
		__af_xl_arg_xl_t(__F, 2) __A2,\
		__af_xl_arg_xl_t(__F, 3) __A3,\
		__af_xl_arg_xl_t(__F, 4) __A4,\
		__af_xl_arg_xl_t(__F, 5) __A5,\
		__af_xl_arg_xl_t(__F, 6) __A6,\
		__af_xl_arg_xl_t(__F, 7) __A7\
	){ \
	 __AF_XL_TRY;\
		using namespace autotelica::xloper::xl_conversions;\
		return to_xlp( __F(\
			from_xl<__af_xl_arg_t(__F, 0)>(__A0),\
			from_xl<__af_xl_arg_t(__F, 1)>(__A1),\
			from_xl<__af_xl_arg_t(__F, 2)>(__A2),\
			from_xl<__af_xl_arg_t(__F, 3)>(__A3),\
			from_xl<__af_xl_arg_t(__F, 4)>(__A4),\
			from_xl<__af_xl_arg_t(__F, 5)>(__A5),\
			from_xl<__af_xl_arg_t(__F, 6)>(__A6),\
			from_xl<__af_xl_arg_t(__F, 7)>(__A7)\
		)); \
	 __AF_XL_CATCH;\
	}

#define __AF_XL_NAMED_FUNC_9(__NAME, __F, __FH, __A0, __A0H, __A1, __A1H, __A2, __A2H, __A3, __A3H, __A4, __A4H, __A5, __A5H, __A6, __A6H, __A7, __A7H, __A8, __A8H)\
	extern "C" __declspec(dllexport) LPXLOPER12 __stdcall \
	__AF_XL_IMPL_F_N(__NAME)(\
		__af_xl_arg_xl_t(__F, 0) __A0,\
		__af_xl_arg_xl_t(__F, 1) __A1,\
		__af_xl_arg_xl_t(__F, 2) __A2,\
		__af_xl_arg_xl_t(__F, 3) __A3,\
		__af_xl_arg_xl_t(__F, 4) __A4,\
		__af_xl_arg_xl_t(__F, 5) __A5,\
		__af_xl_arg_xl_t(__F, 6) __A6,\
		__af_xl_arg_xl_t(__F, 7) __A7,\
		__af_xl_arg_xl_t(__F, 8) __A8\
	){ \
	 __AF_XL_TRY;\
		using namespace autotelica::xloper::xl_conversions;\
		return to_xlp( __F(\
			from_xl<__af_xl_arg_t(__F, 0)>(__A0),\
			from_xl<__af_xl_arg_t(__F, 1)>(__A1),\
			from_xl<__af_xl_arg_t(__F, 2)>(__A2),\
			from_xl<__af_xl_arg_t(__F, 3)>(__A3),\
			from_xl<__af_xl_arg_t(__F, 4)>(__A4),\
			from_xl<__af_xl_arg_t(__F, 5)>(__A5),\
			from_xl<__af_xl_arg_t(__F, 6)>(__A6),\
			from_xl<__af_xl_arg_t(__F, 7)>(__A7),\
			from_xl<__af_xl_arg_t(__F, 8)>(__A8)\
		)); \
	 __AF_XL_CATCH;\
	}

#define __AF_XL_NAMED_FUNC_10(__NAME, __F, __FH, __A0, __A0H, __A1, __A1H, __A2, __A2H, __A3, __A3H, __A4, __A4H, __A5, __A5H, __A6, __A6H, __A7, __A7H, __A8, __A8H, __A9, __A9H)\
	extern "C" __declspec(dllexport) LPXLOPER12 __stdcall \
	__AF_XL_IMPL_F_N(__NAME)(\
		__af_xl_arg_xl_t(__F, 0) __A0,\
		__af_xl_arg_xl_t(__F, 1) __A1,\
		__af_xl_arg_xl_t(__F, 2) __A2,\
		__af_xl_arg_xl_t(__F, 3) __A3,\
		__af_xl_arg_xl_t(__F, 4) __A4,\
		__af_xl_arg_xl_t(__F, 5) __A5,\
		__af_xl_arg_xl_t(__F, 6) __A6,\
		__af_xl_arg_xl_t(__F, 7) __A7,\
		__af_xl_arg_xl_t(__F, 8) __A8,\
		__af_xl_arg_xl_t(__F, 9) __A9\
	){ \
	 __AF_XL_TRY;\
		using namespace autotelica::xloper::xl_conversions;\
		return to_xlp( __F(\
			from_xl<__af_xl_arg_t(__F, 0)>(__A0),\
			from_xl<__af_xl_arg_t(__F, 1)>(__A1),\
			from_xl<__af_xl_arg_t(__F, 2)>(__A2),\
			from_xl<__af_xl_arg_t(__F, 3)>(__A3),\
			from_xl<__af_xl_arg_t(__F, 4)>(__A4),\
			from_xl<__af_xl_arg_t(__F, 5)>(__A5),\
			from_xl<__af_xl_arg_t(__F, 6)>(__A6),\
			from_xl<__af_xl_arg_t(__F, 7)>(__A7),\
			from_xl<__af_xl_arg_t(__F, 8)>(__A8),\
			from_xl<__af_xl_arg_t(__F, 9)>(__A9)\
		)); \
	 __AF_XL_CATCH;\
	}

#define __AF_XL_NAMED_FUNC_11(__NAME, __F, __FH, __A0, __A0H, __A1, __A1H, __A2, __A2H, __A3, __A3H, __A4, __A4H, __A5, __A5H, __A6, __A6H, __A7, __A7H, __A8, __A8H, __A9, __A9H, __A10, __A10H)\
	extern "C" __declspec(dllexport) LPXLOPER12 __stdcall \
	__AF_XL_IMPL_F_N(__NAME)(\
		__af_xl_arg_xl_t(__F, 0) __A0,\
		__af_xl_arg_xl_t(__F, 1) __A1,\
		__af_xl_arg_xl_t(__F, 2) __A2,\
		__af_xl_arg_xl_t(__F, 3) __A3,\
		__af_xl_arg_xl_t(__F, 4) __A4,\
		__af_xl_arg_xl_t(__F, 5) __A5,\
		__af_xl_arg_xl_t(__F, 6) __A6,\
		__af_xl_arg_xl_t(__F, 7) __A7,\
		__af_xl_arg_xl_t(__F, 8) __A8,\
		__af_xl_arg_xl_t(__F, 9) __A9,\
		__af_xl_arg_xl_t(__F, 10) __A10\
	){ \
	 __AF_XL_TRY;\
		return to_xlp( __F(\
			from_xl<__af_xl_arg_t(__F, 0)>(__A0),\
			from_xl<__af_xl_arg_t(__F, 1)>(__A1),\
			from_xl<__af_xl_arg_t(__F, 2)>(__A2),\
			from_xl<__af_xl_arg_t(__F, 3)>(__A3),\
			from_xl<__af_xl_arg_t(__F, 4)>(__A4),\
			from_xl<__af_xl_arg_t(__F, 5)>(__A5),\
			from_xl<__af_xl_arg_t(__F, 6)>(__A6),\
			from_xl<__af_xl_arg_t(__F, 7)>(__A7),\
			from_xl<__af_xl_arg_t(__F, 8)>(__A8),\
			from_xl<__af_xl_arg_t(__F, 9)>(__A9),\
			from_xl<__af_xl_arg_t(__F, 10)>(__A10)\
		)); \
	 __AF_XL_CATCH;\
	}

#define __AF_XL_NAMED_FUNC_12(__NAME, __F, __FH, __A0, __A0H, __A1, __A1H, __A2, __A2H, __A3, __A3H, __A4, __A4H, __A5, __A5H, __A6, __A6H, __A7, __A7H, __A8, __A8H, __A9, __A9H, __A10, __A10H, __A11, __A11H)\
	extern "C" __declspec(dllexport) LPXLOPER12 __stdcall \
	__AF_XL_IMPL_F_N(__NAME)(\
		__af_xl_arg_xl_t(__F, 0) __A0,\
		__af_xl_arg_xl_t(__F, 1) __A1,\
		__af_xl_arg_xl_t(__F, 2) __A2,\
		__af_xl_arg_xl_t(__F, 3) __A3,\
		__af_xl_arg_xl_t(__F, 4) __A4,\
		__af_xl_arg_xl_t(__F, 5) __A5,\
		__af_xl_arg_xl_t(__F, 6) __A6,\
		__af_xl_arg_xl_t(__F, 7) __A7,\
		__af_xl_arg_xl_t(__F, 8) __A8,\
		__af_xl_arg_xl_t(__F, 9) __A9,\
		__af_xl_arg_xl_t(__F, 10) __A10,\
		__af_xl_arg_xl_t(__F, 11) __A11\
	){ \
	 __AF_XL_TRY;\
		return to_xlp( __F(\
			from_xl<__af_xl_arg_t(__F, 0)>(__A0),\
			from_xl<__af_xl_arg_t(__F, 1)>(__A1),\
			from_xl<__af_xl_arg_t(__F, 2)>(__A2),\
			from_xl<__af_xl_arg_t(__F, 3)>(__A3),\
			from_xl<__af_xl_arg_t(__F, 4)>(__A4),\
			from_xl<__af_xl_arg_t(__F, 5)>(__A5),\
			from_xl<__af_xl_arg_t(__F, 6)>(__A6),\
			from_xl<__af_xl_arg_t(__F, 7)>(__A7),\
			from_xl<__af_xl_arg_t(__F, 8)>(__A8),\
			from_xl<__af_xl_arg_t(__F, 9)>(__A9),\
			from_xl<__af_xl_arg_t(__F, 10)>(__A10),\
			from_xl<__af_xl_arg_t(__F, 11)>(__A11)\
		)); \
	 __AF_XL_CATCH;\
	}

// Quick & Dirty functions
#define __AF_XL_QD_WRAPPER_FUNCTION_ERROR(...) static_assert(false, "Declaration of Excel Function has a wrong number of arguments.");
#define __AF_XL_QD_WRAPPER_FUNCTION_0(__F) extern "C" __declspec(dllexport) LPXLOPER12 __stdcall\
	__AF_XL_IMPL_QD_F_N( __F ) (){\
		using namespace autotelica::xloper::xl_conversions;\
		__AF_XL_TRY;\
			return to_xlp(__F(\
			));\
		__AF_XL_CATCH;\
		}

#define __AF_XL_QD_WRAPPER_FUNCTION_1(__F) extern "C" __declspec(dllexport) LPXLOPER12 __stdcall\
	__AF_XL_IMPL_QD_F_N( __F ) (LPXLOPER12 __a0){\
		using namespace autotelica::xloper::xl_conversions;\
		__AF_XL_TRY;\
			return to_xlp(__F(\
				from_xlp<__af_xl_arg_t(__F, 0)>(__a0)\
			));\
		__AF_XL_CATCH;\
		}

#define __AF_XL_QD_WRAPPER_FUNCTION_2(__F) extern "C" __declspec(dllexport) LPXLOPER12 __stdcall\
	__AF_XL_IMPL_QD_F_N( __F ) (LPXLOPER12 __a0, LPXLOPER12 __a1){\
		using namespace autotelica::xloper::xl_conversions;\
		__AF_XL_TRY;\
			return to_xlp(__F(\
				from_xlp<__af_xl_arg_t(__F, 0)>(__a0),\
				from_xlp<__af_xl_arg_t(__F, 1)>(__a1)\
			));\
		__AF_XL_CATCH;\
		}

#define __AF_XL_QD_WRAPPER_FUNCTION_3(__F) extern "C" __declspec(dllexport) LPXLOPER12 __stdcall\
	__AF_XL_IMPL_QD_F_N( __F ) (LPXLOPER12 __a0, LPXLOPER12 __a1, LPXLOPER12 __a2){\
		using namespace autotelica::xloper::xl_conversions;\
		__AF_XL_TRY;\
			return to_xlp(__F(\
				from_xlp<__af_xl_arg_t(__F, 0)>(__a0),\
				from_xlp<__af_xl_arg_t(__F, 1)>(__a1),\
				from_xlp<__af_xl_arg_t(__F, 2)>(__a2)\
			));\
		__AF_XL_CATCH;\
		}

#define __AF_XL_QD_WRAPPER_FUNCTION_4(__F) extern "C" __declspec(dllexport) LPXLOPER12 __stdcall\
	__AF_XL_IMPL_QD_F_N( __F ) (LPXLOPER12 __a0, LPXLOPER12 __a1, LPXLOPER12 __a2, LPXLOPER12 __a3){\
		using namespace autotelica::xloper::xl_conversions;\
		__AF_XL_TRY;\
			return to_xlp(__F(\
				from_xlp<__af_xl_arg_t(__F, 0)>(__a0),\
				from_xlp<__af_xl_arg_t(__F, 1)>(__a1),\
				from_xlp<__af_xl_arg_t(__F, 2)>(__a2),\
				from_xlp<__af_xl_arg_t(__F, 3)>(__a3)\
			));\
		__AF_XL_CATCH;\
		}

#define __AF_XL_QD_WRAPPER_FUNCTION_5(__F) extern "C" __declspec(dllexport) LPXLOPER12 __stdcall\
	__AF_XL_IMPL_QD_F_N( __F ) (LPXLOPER12 __a0, LPXLOPER12 __a1, LPXLOPER12 __a2, LPXLOPER12 __a3, LPXLOPER12 __a4){\
		using namespace autotelica::xloper::xl_conversions;\
		__AF_XL_TRY;\
			return to_xlp(__F(\
				from_xlp<__af_xl_arg_t(__F, 0)>(__a0),\
				from_xlp<__af_xl_arg_t(__F, 1)>(__a1),\
				from_xlp<__af_xl_arg_t(__F, 2)>(__a2),\
				from_xlp<__af_xl_arg_t(__F, 3)>(__a3),\
				from_xlp<__af_xl_arg_t(__F, 4)>(__a4)\
			));\
		__AF_XL_CATCH;\
		}

#define __AF_XL_QD_WRAPPER_FUNCTION_6(__F) extern "C" __declspec(dllexport) LPXLOPER12 __stdcall\
	__AF_XL_IMPL_QD_F_N( __F ) (LPXLOPER12 __a0, LPXLOPER12 __a1, LPXLOPER12 __a2, LPXLOPER12 __a3, LPXLOPER12 __a4, LPXLOPER12 __a5){\
		using namespace autotelica::xloper::xl_conversions;\
		__AF_XL_TRY;\
			return to_xlp(__F(\
				from_xlp<__af_xl_arg_t(__F, 0)>(__a0),\
				from_xlp<__af_xl_arg_t(__F, 1)>(__a1),\
				from_xlp<__af_xl_arg_t(__F, 2)>(__a2),\
				from_xlp<__af_xl_arg_t(__F, 3)>(__a3),\
				from_xlp<__af_xl_arg_t(__F, 4)>(__a4),\
				from_xlp<__af_xl_arg_t(__F, 5)>(__a5)\
			));\
		__AF_XL_CATCH;\
		}

#define __AF_XL_QD_WRAPPER_FUNCTION_7(__F) extern "C" __declspec(dllexport) LPXLOPER12 __stdcall\
	__AF_XL_IMPL_QD_F_N( __F ) (LPXLOPER12 __a0, LPXLOPER12 __a1, LPXLOPER12 __a2, LPXLOPER12 __a3, LPXLOPER12 __a4, LPXLOPER12 __a5, \
								LPXLOPER12 __a6){\
		using namespace autotelica::xloper::xl_conversions;\
		__AF_XL_TRY;\
			return to_xlp(__F(\
				from_xlp<__af_xl_arg_t(__F, 0)>(__a0),\
				from_xlp<__af_xl_arg_t(__F, 1)>(__a1),\
				from_xlp<__af_xl_arg_t(__F, 2)>(__a2),\
				from_xlp<__af_xl_arg_t(__F, 3)>(__a3),\
				from_xlp<__af_xl_arg_t(__F, 4)>(__a4),\
				from_xlp<__af_xl_arg_t(__F, 5)>(__a5),\
				from_xlp<__af_xl_arg_t(__F, 6)>(__a6)\
			));\
		__AF_XL_CATCH;\
		}

#define __AF_XL_QD_WRAPPER_FUNCTION_8(__F) extern "C" __declspec(dllexport) LPXLOPER12 __stdcall\
	__AF_XL_IMPL_QD_F_N( __F ) (LPXLOPER12 __a0, LPXLOPER12 __a1, LPXLOPER12 __a2, LPXLOPER12 __a3, LPXLOPER12 __a4, LPXLOPER12 __a5,\
								LPXLOPER12 __a6, LPXLOPER12 __a7){\
		using namespace autotelica::xloper::xl_conversions;\
		__AF_XL_TRY;\
			return to_xlp(__F(\
				from_xlp<__af_xl_arg_t(__F, 0)>(__a0),\
				from_xlp<__af_xl_arg_t(__F, 1)>(__a1),\
				from_xlp<__af_xl_arg_t(__F, 2)>(__a2),\
				from_xlp<__af_xl_arg_t(__F, 3)>(__a3),\
				from_xlp<__af_xl_arg_t(__F, 4)>(__a4),\
				from_xlp<__af_xl_arg_t(__F, 5)>(__a5),\
				from_xlp<__af_xl_arg_t(__F, 6)>(__a6),\
				from_xlp<__af_xl_arg_t(__F, 7)>(__a7)\
			));\
		__AF_XL_CATCH;\
		}

#define __AF_XL_QD_WRAPPER_FUNCTION_9(__F) extern "C" __declspec(dllexport) LPXLOPER12 __stdcall\
	__AF_XL_IMPL_QD_F_N( __F ) (LPXLOPER12 __a0, LPXLOPER12 __a1, LPXLOPER12 __a2, LPXLOPER12 __a3, LPXLOPER12 __a4, LPXLOPER12 __a5,\
								LPXLOPER12 __a6, LPXLOPER12 __a7, LPXLOPER12 __a8){\
		using namespace autotelica::xloper::xl_conversions;\
		__AF_XL_TRY;\
			return to_xlp(__F(\
				from_xlp<__af_xl_arg_t(__F, 0)>(__a0),\
				from_xlp<__af_xl_arg_t(__F, 1)>(__a1),\
				from_xlp<__af_xl_arg_t(__F, 2)>(__a2),\
				from_xlp<__af_xl_arg_t(__F, 3)>(__a3),\
				from_xlp<__af_xl_arg_t(__F, 4)>(__a4),\
				from_xlp<__af_xl_arg_t(__F, 5)>(__a5),\
				from_xlp<__af_xl_arg_t(__F, 6)>(__a6),\
				from_xlp<__af_xl_arg_t(__F, 7)>(__a7),\
				from_xlp<__af_xl_arg_t(__F, 8)>(__a8)\
			));\
		__AF_XL_CATCH;\
		}

#define __AF_XL_QD_WRAPPER_FUNCTION_10(__F) extern "C" __declspec(dllexport) LPXLOPER12 __stdcall\
	__AF_XL_IMPL_QD_F_N( __F ) (LPXLOPER12 __a0, LPXLOPER12 __a1, LPXLOPER12 __a2, LPXLOPER12 __a3, LPXLOPER12 __a4, LPXLOPER12 __a5,\
								LPXLOPER12 __a6, LPXLOPER12 __a7, LPXLOPER12 __a8, LPXLOPER12 __a9){\
		using namespace autotelica::xloper::xl_conversions;\
		__AF_XL_TRY;\
			return to_xlp(__F(\
				from_xlp<__af_xl_arg_t(__F, 0)>(__a0),\
				from_xlp<__af_xl_arg_t(__F, 1)>(__a1),\
				from_xlp<__af_xl_arg_t(__F, 2)>(__a2),\
				from_xlp<__af_xl_arg_t(__F, 3)>(__a3),\
				from_xlp<__af_xl_arg_t(__F, 4)>(__a4),\
				from_xlp<__af_xl_arg_t(__F, 5)>(__a5),\
				from_xlp<__af_xl_arg_t(__F, 6)>(__a6),\
				from_xlp<__af_xl_arg_t(__F, 7)>(__a7),\
				from_xlp<__af_xl_arg_t(__F, 8)>(__a8),\
				from_xlp<__af_xl_arg_t(__F, 9)>(__a9)\
			));\
		__AF_XL_CATCH;\
		}

#define __AF_XL_QD_WRAPPER_FUNCTION_11(__F) extern "C" __declspec(dllexport) LPXLOPER12 __stdcall\
	__AF_XL_IMPL_QD_F_N( __F ) (LPXLOPER12 __a0, LPXLOPER12 __a1, LPXLOPER12 __a2, LPXLOPER12 __a3, LPXLOPER12 __a4, LPXLOPER12 __a5,\
								LPXLOPER12 __a6, LPXLOPER12 __a7, LPXLOPER12 __a8, LPXLOPER12 __a9, LPXLOPER12 __a10){\
		using namespace autotelica::xloper::xl_conversions;\
		__AF_XL_TRY;\
			return to_xlp(__F(\
				from_xlp<__af_xl_arg_t(__F, 0)>(__a0),\
				from_xlp<__af_xl_arg_t(__F, 1)>(__a1),\
				from_xlp<__af_xl_arg_t(__F, 2)>(__a2),\
				from_xlp<__af_xl_arg_t(__F, 3)>(__a3),\
				from_xlp<__af_xl_arg_t(__F, 4)>(__a4),\
				from_xlp<__af_xl_arg_t(__F, 5)>(__a5),\
				from_xlp<__af_xl_arg_t(__F, 6)>(__a6),\
				from_xlp<__af_xl_arg_t(__F, 7)>(__a7),\
				from_xlp<__af_xl_arg_t(__F, 8)>(__a8),\
				from_xlp<__af_xl_arg_t(__F, 9)>(__a9),\
				from_xlp<__af_xl_arg_t(__F, 10)>(__a10)\
			));\
		__AF_XL_CATCH;\
		}

#define __AF_XL_QD_WRAPPER_FUNCTION_12(__F) extern "C" __declspec(dllexport) LPXLOPER12 __stdcall\
	__AF_XL_IMPL_QD_F_N( __F ) (LPXLOPER12 __a0, LPXLOPER12 __a1, LPXLOPER12 __a2, LPXLOPER12 __a3, LPXLOPER12 __a4,  LPXLOPER12 __a5,\
								LPXLOPER12 __a6, LPXLOPER12 __a7, LPXLOPER12 __a8, LPXLOPER12 __a9, LPXLOPER12 __a10, LPXLOPER12 __a11){\
		using namespace autotelica::xloper::xl_conversions;\
		__AF_XL_TRY;\
			return to_xlp(__F(\
				from_xlp<__af_xl_arg_t(__F, 0)>(__a0),\
				from_xlp<__af_xl_arg_t(__F, 1)>(__a1),\
				from_xlp<__af_xl_arg_t(__F, 2)>(__a2),\
				from_xlp<__af_xl_arg_t(__F, 3)>(__a3),\
				from_xlp<__af_xl_arg_t(__F, 4)>(__a4),\
				from_xlp<__af_xl_arg_t(__F, 5)>(__a5),\
				from_xlp<__af_xl_arg_t(__F, 6)>(__a6),\
				from_xlp<__af_xl_arg_t(__F, 7)>(__a7),\
				from_xlp<__af_xl_arg_t(__F, 8)>(__a8),\
				from_xlp<__af_xl_arg_t(__F, 9)>(__a9),\
				from_xlp<__af_xl_arg_t(__F, 10)>(__a10),\
				from_xlp<__af_xl_arg_t(__F, 11)>(__a11)\
			));\
		__AF_XL_CATCH;\
		}


// Lambda functions
#define __AF_XL_LAMBDA_WRAPPER_FUNCTION_ERROR(...) static_assert(false, "Declaration of Excel Function has a wrong number of arguments.");

#define __AF_XL_LAMBDA_WRAPPER_FUNCTION_0(__NAME) extern "C" __declspec(dllexport) LPXLOPER12 __stdcall\
	__AF_XL_IMPL_LAMBDA_F_N( __NAME ) (){\
	using namespace autotelica::xloper::xl_conversions;\
	__AF_XL_TRY;\
		return to_xlp(\
			__AF_XL_DECL_LAMBDA_F_N(__NAME)(\
			));\
	__AF_XL_CATCH;\
	}

#define __AF_XL_LAMBDA_WRAPPER_FUNCTION_1(__NAME) extern "C" __declspec(dllexport) LPXLOPER12 __stdcall\
	__AF_XL_IMPL_LAMBDA_F_N( __NAME ) (LPXLOPER12 __a0){\
	using namespace autotelica::xloper::xl_conversions;\
	__AF_XL_TRY;\
		return to_xlp(\
			__AF_XL_DECL_LAMBDA_F_N(__NAME)(\
				from_xlp<__af_xl_lambda_arg_t(__AF_XL_DECL_LAMBDA_F_N(__NAME), 0)>(__a0)\
			));\
	__AF_XL_CATCH;\
	}

#define __AF_XL_LAMBDA_WRAPPER_FUNCTION_2(__NAME) extern "C" __declspec(dllexport) LPXLOPER12 __stdcall\
	__AF_XL_IMPL_LAMBDA_F_N( __NAME ) (LPXLOPER12 __a0, LPXLOPER12 __a1){\
		using namespace autotelica::xloper::xl_conversions;\
		__AF_XL_TRY;\
		return to_xlp(\
			__AF_XL_DECL_LAMBDA_F_N(__NAME)(\
				from_xlp<__af_xl_lambda_arg_t(__AF_XL_DECL_LAMBDA_F_N(__NAME), 0)>(__a0),\
				from_xlp<__af_xl_lambda_arg_t(__AF_XL_DECL_LAMBDA_F_N(__NAME), 1)>(__a1)\
			));\
		__AF_XL_CATCH;\
		}

#define __AF_XL_LAMBDA_WRAPPER_FUNCTION_3(__NAME) extern "C" __declspec(dllexport) LPXLOPER12 __stdcall\
	__AF_XL_IMPL_LAMBDA_F_N(  __NAME ) (LPXLOPER12 __a0, LPXLOPER12 __a1, LPXLOPER12 __a2){\
		using namespace autotelica::xloper::xl_conversions;\
		__AF_XL_TRY;\
		return to_xlp(\
			__AF_XL_DECL_LAMBDA_F_N(__NAME)(\
				from_xlp<__af_xl_lambda_arg_t(__AF_XL_DECL_LAMBDA_F_N(__NAME), 0)>(__a0),\
				from_xlp<__af_xl_lambda_arg_t(__AF_XL_DECL_LAMBDA_F_N(__NAME), 1)>(__a1),\
				from_xlp<__af_xl_lambda_arg_t(__AF_XL_DECL_LAMBDA_F_N(__NAME), 2)>(__a2)\
			));\
		__AF_XL_CATCH;\
		}

#define __AF_XL_LAMBDA_WRAPPER_FUNCTION_4(__NAME) extern "C" __declspec(dllexport) LPXLOPER12 __stdcall\
	__AF_XL_IMPL_LAMBDA_F_N( __NAME ) (LPXLOPER12 __a0, LPXLOPER12 __a1, LPXLOPER12 __a2, LPXLOPER12 __a3){\
		using namespace autotelica::xloper::xl_conversions;\
		__AF_XL_TRY;\
		return to_xlp(\
			__AF_XL_DECL_LAMBDA_F_N(__NAME)(\
				from_xlp<__af_xl_lambda_arg_t(__AF_XL_DECL_LAMBDA_F_N(__NAME), 0)>(__a0),\
				from_xlp<__af_xl_lambda_arg_t(__AF_XL_DECL_LAMBDA_F_N(__NAME), 1)>(__a1),\
				from_xlp<__af_xl_lambda_arg_t(__AF_XL_DECL_LAMBDA_F_N(__NAME), 2)>(__a2),\
				from_xlp<__af_xl_lambda_arg_t(__AF_XL_DECL_LAMBDA_F_N(__NAME), 3)>(__a3)\
			));\
		__AF_XL_CATCH;\
		}

#define __AF_XL_LAMBDA_WRAPPER_FUNCTION_5(__NAME) extern "C" __declspec(dllexport) LPXLOPER12 __stdcall\
	__AF_XL_IMPL_LAMBDA_F_N( __NAME ) (LPXLOPER12 __a0, LPXLOPER12 __a1, LPXLOPER12 __a2, LPXLOPER12 __a3, LPXLOPER12 __a4){\
		using namespace autotelica::xloper::xl_conversions;\
		__AF_XL_TRY;\
		return to_xlp(\
			__AF_XL_DECL_LAMBDA_F_N(__NAME)(\
				from_xlp<__af_xl_lambda_arg_t(__AF_XL_DECL_LAMBDA_F_N(__NAME), 0)>(__a0),\
				from_xlp<__af_xl_lambda_arg_t(__AF_XL_DECL_LAMBDA_F_N(__NAME), 1)>(__a1),\
				from_xlp<__af_xl_lambda_arg_t(__AF_XL_DECL_LAMBDA_F_N(__NAME), 2)>(__a2),\
				from_xlp<__af_xl_lambda_arg_t(__AF_XL_DECL_LAMBDA_F_N(__NAME), 3)>(__a3),\
				from_xlp<__af_xl_lambda_arg_t(__AF_XL_DECL_LAMBDA_F_N(__NAME), 4)>(__a4)\
			));\
		__AF_XL_CATCH;\
		}

#define __AF_XL_LAMBDA_WRAPPER_FUNCTION_6(__NAME) extern "C" __declspec(dllexport) LPXLOPER12 __stdcall\
	__AF_XL_IMPL_LAMBDA_F_N( __NAME ) (LPXLOPER12 __a0, LPXLOPER12 __a1, LPXLOPER12 __a2, LPXLOPER12 __a3, LPXLOPER12 __a4, LPXLOPER12 __a5){\
		using namespace autotelica::xloper::xl_conversions;\
		__AF_XL_TRY;\
		return to_xlp(\
			__AF_XL_DECL_LAMBDA_F_N(__NAME)(\
				from_xlp<__af_xl_lambda_arg_t(__AF_XL_DECL_LAMBDA_F_N(__NAME), 0)>(__a0),\
				from_xlp<__af_xl_lambda_arg_t(__AF_XL_DECL_LAMBDA_F_N(__NAME), 1)>(__a1),\
				from_xlp<__af_xl_lambda_arg_t(__AF_XL_DECL_LAMBDA_F_N(__NAME), 2)>(__a2),\
				from_xlp<__af_xl_lambda_arg_t(__AF_XL_DECL_LAMBDA_F_N(__NAME), 3)>(__a3),\
				from_xlp<__af_xl_lambda_arg_t(__AF_XL_DECL_LAMBDA_F_N(__NAME), 4)>(__a4),\
				from_xlp<__af_xl_lambda_arg_t(__AF_XL_DECL_LAMBDA_F_N(__NAME), 5)>(__a5)\
			));\
		__AF_XL_CATCH;\
		}

#define __AF_XL_LAMBDA_WRAPPER_FUNCTION_7(__NAME) extern "C" __declspec(dllexport) LPXLOPER12 __stdcall\
	__AF_XL_IMPL_LAMBDA_F_N( __NAME ) (LPXLOPER12 __a0, LPXLOPER12 __a1, LPXLOPER12 __a2, LPXLOPER12 __a3, LPXLOPER12 __a4, LPXLOPER12 __a5, \
								LPXLOPER12 __a6){\
		using namespace autotelica::xloper::xl_conversions;\
		__AF_XL_TRY;\
		return to_xlp(\
			__AF_XL_DECL_LAMBDA_F_N(__NAME)(\
				from_xlp<__af_xl_lambda_arg_t(__AF_XL_DECL_LAMBDA_F_N(__NAME), 0)>(__a0),\
				from_xlp<__af_xl_lambda_arg_t(__AF_XL_DECL_LAMBDA_F_N(__NAME), 1)>(__a1),\
				from_xlp<__af_xl_lambda_arg_t(__AF_XL_DECL_LAMBDA_F_N(__NAME), 2)>(__a2),\
				from_xlp<__af_xl_lambda_arg_t(__AF_XL_DECL_LAMBDA_F_N(__NAME), 3)>(__a3),\
				from_xlp<__af_xl_lambda_arg_t(__AF_XL_DECL_LAMBDA_F_N(__NAME), 4)>(__a4),\
				from_xlp<__af_xl_lambda_arg_t(__AF_XL_DECL_LAMBDA_F_N(__NAME), 5)>(__a5),\
				from_xlp<__af_xl_lambda_arg_t(__AF_XL_DECL_LAMBDA_F_N(__NAME), 6)>(__a6)\
			)); \
		__AF_XL_CATCH;\
		}

#define __AF_XL_LAMBDA_WRAPPER_FUNCTION_8(__NAME) extern "C" __declspec(dllexport) LPXLOPER12 __stdcall\
	__AF_XL_IMPL_LAMBDA_F_N( __NAME ) (LPXLOPER12 __a0, LPXLOPER12 __a1, LPXLOPER12 __a2, LPXLOPER12 __a3, LPXLOPER12 __a4, LPXLOPER12 __a5,\
								LPXLOPER12 __a6, LPXLOPER12 __a7){\
		using namespace autotelica::xloper::xl_conversions;\
		__AF_XL_TRY;\
		return to_xlp(\
			__AF_XL_DECL_LAMBDA_F_N(__NAME)(\
				from_xlp<__af_xl_lambda_arg_t(__AF_XL_DECL_LAMBDA_F_N(__NAME), 0)>(__a0),\
				from_xlp<__af_xl_lambda_arg_t(__AF_XL_DECL_LAMBDA_F_N(__NAME), 1)>(__a1),\
				from_xlp<__af_xl_lambda_arg_t(__AF_XL_DECL_LAMBDA_F_N(__NAME), 2)>(__a2),\
				from_xlp<__af_xl_lambda_arg_t(__AF_XL_DECL_LAMBDA_F_N(__NAME), 3)>(__a3),\
				from_xlp<__af_xl_lambda_arg_t(__AF_XL_DECL_LAMBDA_F_N(__NAME), 4)>(__a4),\
				from_xlp<__af_xl_lambda_arg_t(__AF_XL_DECL_LAMBDA_F_N(__NAME), 5)>(__a5),\
				from_xlp<__af_xl_lambda_arg_t(__AF_XL_DECL_LAMBDA_F_N(__NAME), 6)>(__a6),\
				from_xlp<__af_xl_lambda_arg_t(__AF_XL_DECL_LAMBDA_F_N(__NAME), 7)>(__a7)\
			)); \
		__AF_XL_CATCH;\
		}

#define __AF_XL_LAMBDA_WRAPPER_FUNCTION_9(__NAME) extern "C" __declspec(dllexport) LPXLOPER12 __stdcall\
	__AF_XL_IMPL_LAMBDA_F_N( __NAME ) (LPXLOPER12 __a0, LPXLOPER12 __a1, LPXLOPER12 __a2, LPXLOPER12 __a3, LPXLOPER12 __a4, LPXLOPER12 __a5,\
								LPXLOPER12 __a6, LPXLOPER12 __a7, LPXLOPER12 __a8){\
		using namespace autotelica::xloper::xl_conversions;\
		__AF_XL_TRY;\
		return to_xlp(\
			__AF_XL_DECL_LAMBDA_F_N(__NAME)(\
				from_xlp<__af_xl_lambda_arg_t(__AF_XL_DECL_LAMBDA_F_N(__NAME), 0)>(__a0),\
				from_xlp<__af_xl_lambda_arg_t(__AF_XL_DECL_LAMBDA_F_N(__NAME), 1)>(__a1),\
				from_xlp<__af_xl_lambda_arg_t(__AF_XL_DECL_LAMBDA_F_N(__NAME), 2)>(__a2),\
				from_xlp<__af_xl_lambda_arg_t(__AF_XL_DECL_LAMBDA_F_N(__NAME), 3)>(__a3),\
				from_xlp<__af_xl_lambda_arg_t(__AF_XL_DECL_LAMBDA_F_N(__NAME), 4)>(__a4),\
				from_xlp<__af_xl_lambda_arg_t(__AF_XL_DECL_LAMBDA_F_N(__NAME), 5)>(__a5),\
				from_xlp<__af_xl_lambda_arg_t(__AF_XL_DECL_LAMBDA_F_N(__NAME), 6)>(__a6),\
				from_xlp<__af_xl_lambda_arg_t(__AF_XL_DECL_LAMBDA_F_N(__NAME), 7)>(__a7),\
				from_xlp<__af_xl_lambda_arg_t(__AF_XL_DECL_LAMBDA_F_N(__NAME), 8)>(__a8)\
			));\
		__AF_XL_CATCH;\
		}

#define __AF_XL_LAMBDA_WRAPPER_FUNCTION_10(__NAME) extern "C" __declspec(dllexport) LPXLOPER12 __stdcall\
	__AF_XL_IMPL_LAMBDA_F_N( __NAME ) (LPXLOPER12 __a0, LPXLOPER12 __a1, LPXLOPER12 __a2, LPXLOPER12 __a3, LPXLOPER12 __a4, LPXLOPER12 __a5,\
								LPXLOPER12 __a6, LPXLOPER12 __a7, LPXLOPER12 __a8, LPXLOPER12 __a9){\
		using namespace autotelica::xloper::xl_conversions;\
		__AF_XL_TRY;\
		return to_xlp(\
			__AF_XL_DECL_LAMBDA_F_N(__NAME)(\
				from_xlp<__af_xl_lambda_arg_t(__AF_XL_DECL_LAMBDA_F_N(__NAME), 0)>(__a0),\
				from_xlp<__af_xl_lambda_arg_t(__AF_XL_DECL_LAMBDA_F_N(__NAME), 1)>(__a1),\
				from_xlp<__af_xl_lambda_arg_t(__AF_XL_DECL_LAMBDA_F_N(__NAME), 2)>(__a2),\
				from_xlp<__af_xl_lambda_arg_t(__AF_XL_DECL_LAMBDA_F_N(__NAME), 3)>(__a3),\
				from_xlp<__af_xl_lambda_arg_t(__AF_XL_DECL_LAMBDA_F_N(__NAME), 4)>(__a4),\
				from_xlp<__af_xl_lambda_arg_t(__AF_XL_DECL_LAMBDA_F_N(__NAME), 5)>(__a5),\
				from_xlp<__af_xl_lambda_arg_t(__AF_XL_DECL_LAMBDA_F_N(__NAME), 6)>(__a6),\
				from_xlp<__af_xl_lambda_arg_t(__AF_XL_DECL_LAMBDA_F_N(__NAME), 7)>(__a7),\
				from_xlp<__af_xl_lambda_arg_t(__AF_XL_DECL_LAMBDA_F_N(__NAME), 8)>(__a8),\
				from_xlp<__af_xl_lambda_arg_t(__AF_XL_DECL_LAMBDA_F_N(__NAME), 9)>(__a9)\
			));\
		__AF_XL_CATCH;\
		}

#define __AF_XL_LAMBDA_WRAPPER_FUNCTION_11(__NAME) extern "C" __declspec(dllexport) LPXLOPER12 __stdcall\
	__AF_XL_IMPL_LAMBDA_F_N( __NAME ) (LPXLOPER12 __a0, LPXLOPER12 __a1, LPXLOPER12 __a2, LPXLOPER12 __a3, LPXLOPER12 __a4, LPXLOPER12 __a5,\
								LPXLOPER12 __a6, LPXLOPER12 __a7, LPXLOPER12 __a8, LPXLOPER12 __a9, LPXLOPER12 __a10){\
		using namespace autotelica::xloper::xl_conversions;\
		__AF_XL_TRY;\
		return to_xlp(\
			__AF_XL_DECL_LAMBDA_F_N(__NAME)(\
				from_xlp<__af_xl_lambda_arg_t(__AF_XL_DECL_LAMBDA_F_N(__NAME), 0)>(__a0),\
				from_xlp<__af_xl_lambda_arg_t(__AF_XL_DECL_LAMBDA_F_N(__NAME), 1)>(__a1),\
				from_xlp<__af_xl_lambda_arg_t(__AF_XL_DECL_LAMBDA_F_N(__NAME), 2)>(__a2),\
				from_xlp<__af_xl_lambda_arg_t(__AF_XL_DECL_LAMBDA_F_N(__NAME), 3)>(__a3),\
				from_xlp<__af_xl_lambda_arg_t(__AF_XL_DECL_LAMBDA_F_N(__NAME), 4)>(__a4),\
				from_xlp<__af_xl_lambda_arg_t(__AF_XL_DECL_LAMBDA_F_N(__NAME), 5)>(__a5),\
				from_xlp<__af_xl_lambda_arg_t(__AF_XL_DECL_LAMBDA_F_N(__NAME), 6)>(__a6),\
				from_xlp<__af_xl_lambda_arg_t(__AF_XL_DECL_LAMBDA_F_N(__NAME), 7)>(__a7),\
				from_xlp<__af_xl_lambda_arg_t(__AF_XL_DECL_LAMBDA_F_N(__NAME), 8)>(__a8),\
				from_xlp<__af_xl_lambda_arg_t(__AF_XL_DECL_LAMBDA_F_N(__NAME), 9)>(__a9),\
				from_xlp<__af_xl_lambda_arg_t(__AF_XL_DECL_LAMBDA_F_N(__NAME), 10)>(__a10)\
			));\
		__AF_XL_CATCH;\
		}

#define __AF_XL_LAMBDA_WRAPPER_FUNCTION_12(__NAME) extern "C" __declspec(dllexport) LPXLOPER12 __stdcall\
	__AF_XL_IMPL_LAMBDA_F_N( __NAME ) (LPXLOPER12 __a0, LPXLOPER12 __a1, LPXLOPER12 __a2, LPXLOPER12 __a3, LPXLOPER12 __a4,  LPXLOPER12 __a5,\
								LPXLOPER12 __a6, LPXLOPER12 __a7, LPXLOPER12 __a8, LPXLOPER12 __a9, LPXLOPER12 __a10, LPXLOPER12 __a11){\
		using namespace autotelica::xloper::xl_conversions;\
		__AF_XL_TRY;\
		return to_xlp(\
			__AF_XL_DECL_LAMBDA_F_N(__NAME)(\
				from_xlp<__af_xl_lambda_arg_t(__AF_XL_DECL_LAMBDA_F_N(__NAME), 0)>(__a0),\
				from_xlp<__af_xl_lambda_arg_t(__AF_XL_DECL_LAMBDA_F_N(__NAME), 1)>(__a1),\
				from_xlp<__af_xl_lambda_arg_t(__AF_XL_DECL_LAMBDA_F_N(__NAME), 2)>(__a2),\
				from_xlp<__af_xl_lambda_arg_t(__AF_XL_DECL_LAMBDA_F_N(__NAME), 3)>(__a3),\
				from_xlp<__af_xl_lambda_arg_t(__AF_XL_DECL_LAMBDA_F_N(__NAME), 4)>(__a4),\
				from_xlp<__af_xl_lambda_arg_t(__AF_XL_DECL_LAMBDA_F_N(__NAME), 5)>(__a5),\
				from_xlp<__af_xl_lambda_arg_t(__AF_XL_DECL_LAMBDA_F_N(__NAME), 6)>(__a6),\
				from_xlp<__af_xl_lambda_arg_t(__AF_XL_DECL_LAMBDA_F_N(__NAME), 7)>(__a7),\
				from_xlp<__af_xl_lambda_arg_t(__AF_XL_DECL_LAMBDA_F_N(__NAME), 8)>(__a8),\
				from_xlp<__af_xl_lambda_arg_t(__AF_XL_DECL_LAMBDA_F_N(__NAME), 9)>(__a9),\
				from_xlp<__af_xl_lambda_arg_t(__AF_XL_DECL_LAMBDA_F_N(__NAME), 10)>(__a10),\
				from_xlp<__af_xl_lambda_arg_t(__AF_XL_DECL_LAMBDA_F_N(__NAME), 11)>(__a11)\
			));\
		__AF_XL_CATCH;\
		}

// Dispatchers to macros creating wrapper functions
#define __AF_XL_FUNC_DISPATCH( _26, _25, _24, _23, _22, _21, _20, _19, _18, _17, _16, _15, _14, _13, _12, _11, _10, _9, _8, _7, _6, _5, _4, _3, _2, _1, N, ...) __AF_XL_FUNC##N
#define __AF_XL_NAMED_FUNC_DISPATCH( _26, _25, _24, _23, _22, _21, _20, _19, _18, _17, _16, _15, _14, _13, _12, _11, _10, _9, _8, _7, _6, _5, _4, _3, _2, _1, N, ...) __AF_XL_NAMED_FUNC##N
#define __AF_XL_LAMBDA_FUNC_DISPATCH( _26, _25, _24, _23, _22, _21, _20, _19, _18, _17, _16, _15, _14, _13, _12, _11, _10, _9, _8, _7, _6, _5, _4, _3, _2, _1, N, ...) __AF_XL_LAMBDA_WRAPPER_FUNCTION##N

	

// This is he user interface parts
// It is hard to read macro's code, can't be helped. 
// You are much better off reading the documentation
#define AF_DECLARE_EXCEL_FUNCTION(...) \
		__AF_XL_EXPAND(\
			 __AF_XL_FUNC_DISPATCH(__VA_ARGS__, _12, _ERROR, _11, _ERROR, _10, _ERROR, _9, _ERROR, _8, _ERROR, _7, _ERROR, _6, _ERROR, _5, _ERROR, _4, _ERROR, _3, _ERROR, _2, _ERROR, _1, _ERROR, _0, _ERROR, N, ...)(__VA_ARGS__)\
		);\
		__AF_XL_EXPAND(\
			__AF_REGISTER_FUNCTION_DISPATCH(__VA_ARGS__)\
		);

#define AF_DECLARE_EXCEL_FUNCTION_CAT(__CAT, ...) \
		__AF_XL_EXPAND(\
			 __AF_XL_FUNC_DISPATCH(__VA_ARGS__, _12, _ERROR, _11, _ERROR, _10, _ERROR, _9, _ERROR, _8, _ERROR, _7, _ERROR, _6, _ERROR, _5, _ERROR, _4, _ERROR, _3, _ERROR, _2, _ERROR, _1, _ERROR, _0, _ERROR, N, ...)(__VA_ARGS__)\
		);\
		__AF_XL_EXPAND(\
			__AF_REGISTER_FUNCTION_DISPATCH_CAT(__CAT, __VA_ARGS__)\
		);



#define AF_DECLARE_EXCEL_VOLATILE_FUNCTION(...) \
		__AF_XL_EXPAND(\
			 __AF_XL_FUNC_DISPATCH(__VA_ARGS__, _12, _ERROR, _11, _ERROR, _10, _ERROR, _9, _ERROR, _8, _ERROR, _7, _ERROR, _6, _ERROR, _5, _ERROR, _4, _ERROR, _3, _ERROR, _2, _ERROR, _1, _ERROR, _0, _ERROR, N, ...)(__VA_ARGS__)\
		);\
		__AF_XL_EXPAND(\
			__AF_REGISTER_VOLATILE_FUNCTION_DISPATCH(__VA_ARGS__)\
		);

#define AF_DECLARE_EXCEL_VOLATILE_FUNCTION_CAT(__CAT, ...) \
		__AF_XL_EXPAND(\
			 __AF_XL_FUNC_DISPATCH(__VA_ARGS__, _12, _ERROR, _11, _ERROR, _10, _ERROR, _9, _ERROR, _8, _ERROR, _7, _ERROR, _6, _ERROR, _5, _ERROR, _4, _ERROR, _3, _ERROR, _2, _ERROR, _1, _ERROR, _0, _ERROR, N, ...)(__VA_ARGS__)\
		);\
		__AF_XL_EXPAND(\
			__AF_REGISTER_VOLATILE_FUNCTION_DISPATCH_CAT(__CAT, __VA_ARGS__)\
		);



#define AF_DECLARE_EXCEL_NAMED_FUNCTION(__NAME, ...) \
		__AF_XL_EXPAND(\
			 __AF_XL_NAMED_FUNC_DISPATCH(__VA_ARGS__, _12, _ERROR, _11, _ERROR, _10, _ERROR, _9, _ERROR, _8, _ERROR, _7, _ERROR, _6, _ERROR, _5, _ERROR, _4, _ERROR, _3, _ERROR, _2, _ERROR, _1, _ERROR, _0, _ERROR, N, ...)(__NAME, __VA_ARGS__)\
		);\
		__AF_XL_EXPAND(\
			__AF_REGISTER_NAMED_FUNCTION_DISPATCH(__NAME, __VA_ARGS__)\
		);

#define AF_DECLARE_EXCEL_NAMED_FUNCTION_CAT(__CAT,__NAME, ...) \
		__AF_XL_EXPAND(\
			 __AF_XL_NAMED_FUNC_DISPATCH(__VA_ARGS__, _12, _ERROR, _11, _ERROR, _10, _ERROR, _9, _ERROR, _8, _ERROR, _7, _ERROR, _6, _ERROR, _5, _ERROR, _4, _ERROR, _3, _ERROR, _2, _ERROR, _1, _ERROR, _0, _ERROR, N, ...)(__NAME, __VA_ARGS__)\
		);\
		__AF_XL_EXPAND(\
			__AF_REGISTER_NAMED_FUNCTION_DISPATCH_CAT(__CAT, __NAME, __VA_ARGS__)\
		);




#define AF_DECLARE_EXCEL_VOLATILE_NAMED_FUNCTION(__NAME, ...) \
		__AF_XL_EXPAND(\
			 __AF_XL_NAMED_FUNC_DISPATCH(__VA_ARGS__, _12, _ERROR, _11, _ERROR, _10, _ERROR, _9, _ERROR, _8, _ERROR, _7, _ERROR, _6, _ERROR, _5, _ERROR, _4, _ERROR, _3, _ERROR, _2, _ERROR, _1, _ERROR, _0, _ERROR, N, ...)(__NAME, __VA_ARGS__)\
		);\
		__AF_XL_EXPAND(\
			__AF_REGISTER_VOLATILE_NAMED_FUNCTION_DISPATCH(__NAME, __VA_ARGS__)\
		);

#define AF_DECLARE_EXCEL_VOLATILE_NAMED_FUNCTION_CAT(__CAT, __NAME, ...) \
		__AF_XL_EXPAND(\
			 __AF_XL_NAMED_FUNC_DISPATCH(__VA_ARGS__, _12, _ERROR, _11, _ERROR, _10, _ERROR, _9, _ERROR, _8, _ERROR, _7, _ERROR, _6, _ERROR, _5, _ERROR, _4, _ERROR, _3, _ERROR, _2, _ERROR, _1, _ERROR, _0, _ERROR, N, ...)(__NAME, __VA_ARGS__)\
		);\
		__AF_XL_EXPAND(\
			__AF_REGISTER_VOLATILE_NAMED_FUNCTION_DISPATCH_CAT(__CAT,__NAME, __VA_ARGS__)\
		);




#define AF_DECLARE_QD_EXCEL_FUNCTION(__F, __NO_OF_ARGS) \
		__AF_XL_EXPAND(\
			 __AF_XL_QD_WRAPPER_FUNCTION_##__NO_OF_ARGS(__F)\
		);\
		__AF_XL_EXPAND(\
			__AF_REGISTER_QD_FUNCTION(__F )\
		);

#define AF_DECLARE_QD_EXCEL_FUNCTION_CAT(__CAT, __F, __NO_OF_ARGS) \
		__AF_XL_EXPAND(\
			 __AF_XL_QD_WRAPPER_FUNCTION_##__NO_OF_ARGS(__F)\
		);\
		__AF_XL_EXPAND(\
			__AF_REGISTER_QD_FUNCTION_CAT(__CAT, __F )\
		);


#define AF_DECLARE_QD_EXCEL_VOLATILE_FUNCTION(__F, __NO_OF_ARGS) \
		__AF_XL_EXPAND(\
			 __AF_XL_QD_WRAPPER_FUNCTION_##__NO_OF_ARGS(__F)\
		);\
		__AF_XL_EXPAND(\
			__AF_REGISTER_QD_VOLATILE_FUNCTION(__F )\
		);

#define AF_DECLARE_QD_EXCEL_VOLATILE_FUNCTION_CAT(__CAT, __F, __NO_OF_ARGS) \
		__AF_XL_EXPAND(\
			 __AF_XL_QD_WRAPPER_FUNCTION_##__NO_OF_ARGS(__F)\
		);\
		__AF_XL_EXPAND(\
			__AF_REGISTER_QD_VOLATILE_FUNCTION_CAT(__CAT, __F )\
		);



#define AF_DECLARE_LAMBDA_EXCEL_FUNCTION(__NAME, __F, ...) \
		namespace {\
			auto __AF_XL_DECL_LAMBDA_F_N(__NAME) = __F;\
			__AF_XL_EXPAND(\
				 __AF_XL_LAMBDA_FUNC_DISPATCH(__F, __VA_ARGS__, _12, _ERROR, _11, _ERROR, _10, _ERROR, _9, _ERROR, _8, _ERROR, _7, _ERROR, _6, _ERROR, _5, _ERROR, _4, _ERROR, _3, _ERROR, _2, _ERROR, _1, _ERROR, _0, _ERROR, N, ...)(__NAME)\
			);\
			__AF_XL_EXPAND(\
				__AF_REGISTER_LAMBDA_FUNCTION_DISPATCH(__NAME, __F, __VA_ARGS__)\
			);\
		}

#define AF_DECLARE_LAMBDA_EXCEL_FUNCTION_CAT(__CAT, __NAME, __F, ...) \
		namespace {\
			auto __AF_XL_DECL_LAMBDA_F_N(__NAME) = __F;\
			__AF_XL_EXPAND(\
				 __AF_XL_LAMBDA_FUNC_DISPATCH(__F, __VA_ARGS__, _12, _ERROR, _11, _ERROR, _10, _ERROR, _9, _ERROR, _8, _ERROR, _7, _ERROR, _6, _ERROR, _5, _ERROR, _4, _ERROR, _3, _ERROR, _2, _ERROR, _1, _ERROR, _0, _ERROR, N, ...)(__NAME)\
			);\
			__AF_XL_EXPAND(\
				__AF_REGISTER_LAMBDA_FUNCTION_DISPATCH_CAT(__CAT, __NAME, __F, __VA_ARGS__)\
			);\
		}


#define AF_DECLARE_LAMBDA_EXCEL_VOLATILE_FUNCTION(__NAME, __F, ...) \
		namespace {\
			auto __AF_XL_DECL_LAMBDA_F_N(__NAME) = __F;\
			__AF_XL_EXPAND(\
				 __AF_XL_LAMBDA_FUNC_DISPATCH(__F, __VA_ARGS__, _12, _ERROR, _11, _ERROR, _10, _ERROR, _9, _ERROR, _8, _ERROR, _7, _ERROR, _6, _ERROR, _5, _ERROR, _4, _ERROR, _3, _ERROR, _2, _ERROR, _1, _ERROR, _0, _ERROR, N, ...)(__NAME)\
			);\
			__AF_XL_EXPAND(\
				__AF_REGISTER_LAMBDA_VOLATILE_FUNCTION_DISPATCH(__NAME, __F, __VA_ARGS__)\
			);\
		}

#define AF_DECLARE_LAMBDA_EXCEL_VOLATILE_FUNCTION_CAT(__CAT, __NAME, __F, ...) \
		namespace {\
			auto __AF_XL_DECL_LAMBDA_F_N(__NAME) = __F;\
			__AF_XL_EXPAND(\
				 __AF_XL_LAMBDA_FUNC_DISPATCH(__F, __VA_ARGS__, _12, _ERROR, _11, _ERROR, _10, _ERROR, _9, _ERROR, _8, _ERROR, _7, _ERROR, _6, _ERROR, _5, _ERROR, _4, _ERROR, _3, _ERROR, _2, _ERROR, _1, _ERROR, _0, _ERROR, N, ...)(__NAME)\
			);\
			__AF_XL_EXPAND(\
				__AF_REGISTER_LAMBDA_VOLATILE_FUNCTION_DISPATCH_CAT(__CAT, __NAME, __F, __VA_ARGS__)\
			);\
		}


#define AF_DECLARE_QD_LAMBDA_EXCEL_FUNCTION(__NAME, __F, __NO_OF_ARGS) \
		namespace {\
			auto __AF_XL_DECL_LAMBDA_F_N(__NAME) = __F;\
			__AF_XL_EXPAND(\
				 __AF_XL_LAMBDA_WRAPPER_FUNCTION_##__NO_OF_ARGS( __NAME )\
			);\
			__AF_XL_EXPAND(\
				__AF_REGISTER_QD_LAMBDA_FUNCTION(__NAME )\
			);\
		}

#define AF_DECLARE_QD_LAMBDA_EXCEL_FUNCTION_CAT(__CAT, __NAME, __F, __NO_OF_ARGS) \
		namespace {\
			auto __AF_XL_DECL_LAMBDA_F_N(__NAME) = __F;\
			__AF_XL_EXPAND(\
				 __AF_XL_LAMBDA_WRAPPER_FUNCTION_##__NO_OF_ARGS( __NAME )\
			);\
			__AF_XL_EXPAND(\
				__AF_REGISTER_QD_LAMBDA_FUNCTION_CAT(__CAT, __NAME )\
			);\
		}



#define AF_DECLARE_QD_LAMBDA_EXCEL_VOLATILE_FUNCTION(__NAME, __F, __NO_OF_ARGS) \
		namespace {\
			auto __AF_XL_DECL_LAMBDA_F_N(__NAME) = __F;\
			__AF_XL_EXPAND(\
				 __AF_XL_NAME_WRAPPER_FUNCTION_##__NO_OF_ARGS(__NAME)\
			);\
			__AF_XL_EXPAND(\
				__AF_REGISTER_QD_LAMBDA_VOLATILE_FUNCTION(__NAME)\
			);\
		}

#define AF_DECLARE_QD_LAMBDA_EXCEL_VOLATILE_FUNCTION_CAT(__CAT, __NAME, __F, __NO_OF_ARGS) \
		namespace {\
			auto __AF_XL_DECL_LAMBDA_F_N(__NAME) = __F;\
			__AF_XL_EXPAND(\
				 __AF_XL_NAME_WRAPPER_FUNCTION_##__NO_OF_ARGS(__NAME)\
			);\
			__AF_XL_EXPAND(\
				__AF_REGISTER_QD_LAMBDA_VOLATILE_FUNCTION_CAT(__CAT, __NAME)\
			);\
		}


#ifdef AF_EXCLUDE_DEFAULT_FUNCTIONS 
#define AF_DECLARE_XLL(Category) \
		namespace {\
			static const auto af_xl_category_declaration = autotelica::xloper::xl_registration::xl_f_registry::set_function_category(Category);\
		}\
		extern "C" __declspec(dllexport) void __stdcall xlAutoFree12(LPXLOPER12 pXL) {autotelica::xloper::xl_inner::xl_memory::freeXL(pXL);}\
		extern "C" __declspec(dllexport) int __stdcall xlAutoOpen(void){return autotelica::xloper::xl_registration::xl_f_registry::xlAutoOpen_impl();}
#else
#define AF_DECLARE_XLL(Category) \
		namespace {\
			static const auto af_xl_category_declaration = autotelica::xloper::xl_registration::xl_f_registry::set_function_category(Category);\
		}\
		extern "C" __declspec(dllexport) void __stdcall xlAutoFree12(LPXLOPER12 pXL) {autotelica::xloper::xl_inner::xl_memory::freeXL(pXL);}\
		extern "C" __declspec(dllexport) int __stdcall xlAutoOpen(void){return autotelica::xloper::xl_registration::xl_f_registry::xlAutoOpen_impl();}\
		AF_DECLARE_EXCEL_NAMED_FUNCTION(af_xl_configure_error_policy, autotelica::xloper::xl_errors::af_xl_configure_error_policy, \
			"Configure handling of errors and invocation behaviour.", \
			RichErrorText, "Provide detailed error descriptions (instead of just #N\\A).",\
			PropagateErrors, "When there is an error in input, return the same error from function.",\
			InterpretMissing, "Replace missing values with defaults where sensible.",\
			DisableWizardCalls, "Disable function invocation within Excel function wizard.");\
		AF_DECLARE_EXCEL_NAMED_FUNCTION(af_xl_display_error_policy,autotelica::xloper::xl_errors::af_xl_display_error_policy,\
			"Display current configuration of error policy.");\
		AF_DECLARE_EXCEL_NAMED_FUNCTION(af_xl_configure_error_policy_ex, autotelica::xloper::xl_errors::af_xl_configure_error_policy_ex, \
			"Configure handling of errors and invocation behaviour.", \
			PolicyDetails, "An array containing inputs laid out like in the output of af_xl_display_error_policy function.");\
		AF_DECLARE_EXCEL_NAMED_FUNCTION(af_xl_fast_array_cache_size,autotelica::xloper::xl_object_caches::af_xl_fast_array_cache_size,\
			"Display the number of elements in the fast array cache.");\
		AF_DECLARE_EXCEL_NAMED_FUNCTION(af_xl_clear_fast_array_cache,autotelica::xloper::xl_object_caches::af_xl_clear_fast_array_cache,\
			"Clear fast array cache.");\
		AF_DECLARE_EXCEL_NAMED_FUNCTION(af_xl_object_cache_size,autotelica::xloper::xl_object_caches::af_xl_object_cache_size,\
			"Size of an objects cache.",\
			cache_name, "Name of an objects cache.");\
		AF_DECLARE_EXCEL_NAMED_FUNCTION(af_xl_object_cache_sizes,autotelica::xloper::xl_object_caches::af_xl_object_cache_sizes,\
			"Sizes of all objects caches.");\
		AF_DECLARE_EXCEL_NAMED_FUNCTION(af_xl_clear_object_cache,autotelica::xloper::xl_object_caches::af_xl_clear_object_cache,\
			"Clear an objects cache.",\
			cache_name, "Name of an objects cache.");\
		AF_DECLARE_EXCEL_NAMED_FUNCTION(af_xl_clear_all_object_caches,autotelica::xloper::xl_object_caches::af_xl_clear_all_object_caches,\
			"Clear all objects caches.");\
		AF_DECLARE_EXCEL_NAMED_FUNCTION(af_xl_list_objects_cache,autotelica::xloper::xl_object_caches::af_xl_list_objects_cache,\
			"List the content of an objects cache.",\
			cache_name, "Name of an objects cache.");\
		AF_DECLARE_EXCEL_NAMED_FUNCTION(af_xl_list_all_objects_caches,autotelica::xloper::xl_object_caches::af_xl_list_all_objects_caches,\
			"List the content of all objects caches.");\
		AF_DECLARE_EXCEL_NAMED_FUNCTION(af_xl_clear_all_caches,autotelica::xloper::xl_object_caches::af_xl_clear_all_caches,\
			"Clear all caches (objects and fast arrays).");
#endif

/*
To change function category for some functions only:

#define AF_FUNCTION_CATEGORY NewCategoryName
...
SOME FUNCTION DECLARATIONS
...
#undef AF_FUNCTION_CATEGORY

*/

	
}
}
