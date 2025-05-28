#pragma once
#include "af_xloper/af_xloper_errors.h"
#include "af_xloper/af_xloper_fast_array.h"

#include <type_traits>
#include <list>
#include <forward_list>
#include <map>
#include <unordered_map>
#include <iterator>

namespace autotelica {
	namespace xloper {
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
	}
}