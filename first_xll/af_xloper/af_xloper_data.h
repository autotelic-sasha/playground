#pragma once
#include "af_xloper_inner.h"

namespace autotelica {
	namespace xloper {
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

			union {
				double _num;
				std::wstring _str;
				bool _xbool;
				int _err;
				int _w;
				std::vector<std::vector<xl_variant>> _array;
			};

			inline void check_type(xl_type const& t) const {
				if (t != _type)
					throw std::runtime_error("Incorrect type cast for xl_variant.");
			}
			bool equals(xl_variant const& in) const {
				if (_type != in._type)
					return false;
				switch (_type) {
				case xl_type::xl_typeNum:
					return (fabs(_num - in._num) < std::numeric_limits<double>::epsilon());
					break;
				case xl_type::xl_typeStr:
					return(_str == in._str);
					break;
				case xl_type::xl_typeBool:
					return (_xbool == in._xbool);
					break;
				case xl_type::xl_typeErr:
					return (_err == in._err);
					break;
				case xl_type::xl_typeInt:
					return (_w == in._w);
					break;
				case xl_type::xl_typeMulti:
					return (_array == in._array);
					break;
				case xl_type::xl_typeMissing:
				case xl_type::xl_typeNil:
					return true;
				default:
					throw std::runtime_error("Incorrect type for xl_variant comparisson.");
					break;
				}
			}
			inline void copy_from(xl_variant const& in) {
				_type = in._type;
				switch (_type) {
				case xl_type::xl_typeNum:
					_num = in._num;
					break;
				case xl_type::xl_typeStr:
					new(&_str) std::wstring(in._str);
					break;
				case xl_type::xl_typeBool:
					_xbool = in._xbool;
					break;
				case xl_type::xl_typeErr:
					_err = in._err;
					break;
				case xl_type::xl_typeInt:
					_w = in._w;
					break;
				case xl_type::xl_typeMulti:
					new(&_array) std::vector<std::vector<xl_variant>>(in._array);
					break;
				case xl_type::xl_typeMissing:
				case xl_type::xl_typeNil:
					break;
				default:
					throw std::runtime_error("Unknown xltype when copying xl_variant.");
					break;
				}
			}
			inline void free() {
				switch (_type) {
				case xl_type::xl_typeStr:
					(&_str)->std::wstring::~wstring();
					break;
				case xl_type::xl_typeMulti:
					(&_array)->std::vector<std::vector<xl_variant>>::~vector<std::vector<xl_variant>>();
					break;
				default:
					break;
				}
			}
			inline void move_from(xl_variant& in) {
				_type = in._type;
				switch (_type) {
				case xl_type::xl_typeNum:
					_num = in._num;
					break;
				case xl_type::xl_typeStr:
					new(&_str) std::wstring(std::move(in._str));
					break;
				case xl_type::xl_typeBool:
					_xbool = in._xbool;
					break;
				case xl_type::xl_typeErr:
					_err = in._err;
					break;
				case xl_type::xl_typeInt:
					_w = in._w;
					break;
				case xl_type::xl_typeMulti:
					new(&_array) std::vector<std::vector<xl_variant>>(std::move(in._array));
					break;
				case xl_type::xl_typeMissing:
				case xl_type::xl_typeNil:
					break;
				default:
					// moving should never throw
					break;
				}
			}

			void from_xl(XLOPER12 const& in) {
				if (in.xltype & xltypeNum) {
					_num = in.val.num;
					_type = xl_type::xl_typeNum;
				}
				else if (in.xltype & xltypeStr) {
					_str = in.val.str;
					_type = xl_type::xl_typeStr;
				}
				else if (in.xltype & xltypeBool) {
					_xbool = in.val.xbool;
					_type = xl_type::xl_typeBool;
				}
				else if (in.xltype & xltypeErr) {
					_err = in.val.err;
					_type = xl_type::xl_typeErr;
				}
				else if (in.xltype & xltypeInt) {
					_w = in.val.w;
					_type = xl_type::xl_typeInt;
				}
				else if (in.xltype & xltypeMulti) {
					_type = xl_type::xl_typeMulti;
					size_t rows = in.val.array.rows;
					size_t cols = in.val.array.columns;
					_array.resize(rows);
					size_t i = 0;
					for (size_t r = 0; r < rows; ++r) {
						auto& row = _array[r];
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
			xl_variant() : _type(xl_type::xl_typeNil),_err(0) {
			}
			xl_variant(xl_variant const& in) {
				copy_from(in);
			}
			xl_variant(xl_variant&& in) noexcept {
				move_from(in);
			}
			xl_variant(double num_) : _type(xl_type::xl_typeNum), _num(num_) {
			}
			xl_variant(std::wstring const& str_) : _type(xl_type::xl_typeStr),_str(str_) {
			}
			xl_variant(std::string const& str_) : _type(xl_type::xl_typeStr), 
				_str(xl_inner::xl_strings::convert(str_)) {
			}
			xl_variant(bool xbool_) : _type(xl_type::xl_typeBool), _xbool(xbool_) {
			}
			xl_variant(int w_) : _type(xl_type::xl_typeInt), _w(w_) {
			}
			xl_variant(std::vector<std::vector<xl_variant>> const& array_) : _type(xl_type::xl_typeMulti), _array(array_) {
			}
			xl_variant(XLOPER12 const& in) {
				from_xl(in);
			}
			xl_variant(LPXLOPER12 const& in) {
				from_xl(*in);
			}

			~xl_variant() {
				free();
			}

			xl_variant& operator=(xl_variant const& in) {
				free();
				copy_from(in);
				return *this;
			}
			xl_variant& operator=(xl_variant&& in) noexcept {
				free();
				move_from(in);
				return *this;
			}

			bool operator==(xl_variant const& in) const {
				return equals(in);
			}

			inline void set_type(xl_type const& type_) { free(); _type = type_; }
			inline xl_type type() const { return _type; }

			inline void set(double num_) {
				free();
				_type = xl_type::xl_typeNum;
				_num = num_;
			}

			inline double get_double() const { check_type(xl_type::xl_typeNum);  return _num; }
			inline double& get_double() { check_type(xl_type::xl_typeNum);  return _num; }
			inline operator double() const { return get_double(); }
			inline operator double& () { return get_double(); }

			inline void set_error(int err_) {
				free();
				_type = xl_type::xl_typeErr;
				_err = err_;
			}
			inline int get_error() const { check_type(xl_type::xl_typeErr); return _err; }

			void set(std::wstring const& str_) {
				if (_type & xl_type::xl_typeStr)
					_str = str_;
				else {
					free();
					_type = xl_type::xl_typeStr;
					new(&_str) std::wstring(str_);
				}
			}
			void set(std::string const& str_) {
				if (_type & xl_type::xl_typeStr)
					_str = xl_inner::xl_strings::convert(str_);
				else {
					free();
					_type = xl_type::xl_typeStr;
					new(&_str) std::wstring(xl_inner::xl_strings::convert(str_));
				}
			}
			inline std::wstring const& get_wstring() const { check_type(xl_type::xl_typeStr);  return _str; }
			inline std::wstring& get_wstring() { check_type(xl_type::xl_typeStr); return _str; }
			inline std::string get_string() const { check_type(xl_type::xl_typeStr);  return xl_inner::xl_strings::convert(_str); }
			inline operator std::wstring const& () const { return get_wstring(); }
			inline operator std::wstring& () { return get_wstring(); }
			inline operator std::string() const { return get_string(); }

			inline void set(bool xbool_) {
				free();
				_type = xl_type::xl_typeBool;
				_xbool = xbool_;
			}
			inline bool get_bool() const { check_type(xl_type::xl_typeBool);  return _xbool; }
			inline bool& get_bool() { check_type(xl_type::xl_typeBool); return _xbool; }
			inline operator bool() const { return get_bool(); }
			inline operator bool& () { return get_bool(); }

			inline void set(int w_) {
				free();
				_type = xl_type::xl_typeInt;
				_w = w_;
			}
			inline int get_int() const { check_type(xl_type::xl_typeInt);  return _w; }
			inline int& get_int() { check_type(xl_type::xl_typeInt); return _w; }
			inline operator int() const { return get_int(); }
			inline operator int& () { return get_int(); }

			void set(std::vector<std::vector<xl_variant>> const& array_) {
				if (_type & xl_type::xl_typeMulti)
					_array = array_;
				else {
					free();
					_type = xl_type::xl_typeMulti;
					new(&_array) std::vector<std::vector<xl_variant>>(array_);
				}
			}
			inline std::vector<std::vector<xl_variant>> const& get_multi() const { check_type(xl_type::xl_typeMulti);  return _array; }
			inline std::vector<std::vector<xl_variant>>& get_multi() { check_type(xl_type::xl_typeMulti); return _array; }
			inline operator std::vector<std::vector<xl_variant>> const& () const { return get_multi(); }
			inline operator std::vector<std::vector<xl_variant>>& () { return get_multi(); }

			inline void set(XLOPER12 const& in) {
				free();
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
	}
}