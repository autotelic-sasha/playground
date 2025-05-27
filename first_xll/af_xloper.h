#pragma once
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>
#include "af_xloper/af_xloper_inner.h"
#include "af_xloper/af_xloper_data.h"
#include "af_xloper/af_xloper_object_caches.h"
#include "af_xloper/af_xloper_fast_array.h"
#include "af_xloper/af_xloper_util.h"
#include "af_xloper/af_xloper_errors.h"

#include <limits>

#include <type_traits>

#include <tuple>

#include <cstdarg>
#include <sstream>
#include <list>
#include <forward_list>
#include <map>
#include <unordered_map>
#include <iterator>
#include <ctype.h>
#include <math.h>
#include <iostream>
#include <algorithm>


#pragma warning ( disable : 26495)// known problem in visual studio, it doesn't like union constructors
namespace autotelica {
	namespace xloper {
		// much of this is based on https://www.wiley.com/en-gb/Financial+Applications+using+Excel+Add-in+Development+in+C+%2F+C%2B%2B%2C+2nd+Edition-p-9780470319048

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
			__AF_MEANING_OF_MISSING(data::xl_variant, data::xl_variant());
			__AF_MEANING_OF_MISSING(data::xl_nvp, data::xl_nvp());
			__AF_MEANING_OF_MISSING(data::xl_nvp_cs, data::xl_nvp_cs());
			__AF_MEANING_OF_MISSING(data::xl_table, data::xl_table());
			__AF_MEANING_OF_MISSING(data::xl_table_cs, data::xl_table_cs());

			inline bool is_xl_missing(const XLOPER12 * const in) {
				return inner::xl_type_ops::is_xl_type(*in, xltypeMissing | xltypeNil);
			}
			template<typename T>
			std::remove_const_t<std::remove_reference_t<T>> handle_missing(const XLOPER12 * const in) {
				if (!errors::error_policy::interpret_missing()) {
					throw std::runtime_error("Unexpected Nil or Missing value.");
				}
				return meaning_of_missing<std::remove_const_t<std::remove_reference_t<T>>>();
			}

#define CHECK_INPUT(XL12, OutV) util::check_input_xl(XL12);if(is_xl_missing(&XL12)){ OutV = handle_missing<decltype(OutV)>(&XL12); return;}

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
				inner::xl_strings::xlString(in, out);
			}
			static void from_xl(XLOPER12 const& in, std::wstring& out) {
				CHECK_INPUT(in, out);
				// based on Excel SDK, FRAMEWORK.C, XLOPER12 to XLOPER
				out = L"";
				inner::xl_type_ops::check_xl_type(in, xltypeStr);

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
				to_xl(inner::xl_strings::convert(in), out);
			}

			inline void from_xl(XLOPER12 const& in, std::string& out) {
				std::wstring wout;
				from_xl(in, wout);
				out = inner::xl_strings::convert(wout);
			}
			
			// integer types, doubles and bools are passed in as is, but returned as XLOPERS
			inline void int_to_xl(int const& in, XLOPER12& out) {
				inner::xl_type_ops::set_xl_type(out, xltypeInt);
				out.val.w = in;
			}

			inline void int_to_xl(int const& in, LPXLOPER12& out) {
				out = inner::xl_memory::new_xloper12();
				int_to_xl(in, *out);
			}
			inline void num_to_xl(double const& in, XLOPER12& out) {
				inner::xl_type_ops::set_xl_type(out, xltypeNum);
				out.val.num = in;
			}

			inline void num_to_xl(double const& in, LPXLOPER12& out) {
				out = inner::xl_memory::new_xloper12();
				num_to_xl(in, *out);
			}
			inline int xl_to_int(XLOPER12 const& in) {
				inner::xl_type_ops::check_xl_type(in, xltypeInt | xltypeNum);
				if (inner::xl_type_ops::is_xl_type(in, xltypeInt))
					return in.val.w;
				else
					return static_cast<int>(in.val.num);
			}
			inline double xl_to_num(XLOPER12 const& in) {
				inner::xl_type_ops::check_xl_type(in, xltypeInt | xltypeNum);
				if (inner::xl_type_ops::is_xl_type(in, xltypeInt))
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
				inner::xl_type_ops::set_xl_type(out, xltypeBool);
				out.val.xbool = in;
			}
			inline void from_xl(XLOPER12 const& in, bool& out) {
				CHECK_INPUT(in, out);
				inner::xl_type_ops::check_xl_type(in, xltypeBool);
				out = static_cast<bool>(in.val.xbool);
			}
			
			inline void to_xl(data::xl_variant const& in, XLOPER12& out);
			void from_xl(XLOPER12 const& in, data::xl_variant& out);

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
					inner::xl_type_ops::set_xl_type(out, xltypeNil);
					return;
				}

				inner::xl_type_ops::set_xl_type(out, xltypeMulti);
				out.val.array.columns = transposed?((RW)sz):1;
				out.val.array.rows = transposed?1:(RW)sz;
				out.val.array.lparray = inner::xl_memory::new_xloper12_array(sz);

				size_t i = 0;
				for (auto const& v : in){ 
					out.val.array.lparray[i].xltype = 0;
					to_xl(v, out.val.array.lparray[i++]);
				}
			}
			template<typename TLinear>
			void linear_from_xl(XLOPER12 const& in, TLinear& out) {
				CHECK_INPUT(in, out);
				inner::xl_type_ops::check_xl_type(in, xltypeMulti);
				if ((in.val.array.columns * in.val.array.rows) == 0) {
					out.clear();
					return;
				}
				if(in.val.array.columns != 1 && in.val.array.rows != 1)
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
			void to_xl(util::xl_transposed<std::vector<T>> const& in, XLOPER12& out) {
				linear_to_xl(in.value(), out, true);
			}
			template<typename T>
			void to_xl(util::xl_transposed<std::list<T>> const& in, XLOPER12& out) {
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
					inner::xl_type_ops::set_xl_type(out, xltypeNil);
					return;
				}
				if (transposed)
					std::swap(rows, cols);

				inner::xl_type_ops::set_xl_type(out, xltypeMulti);
				out.val.array.rows = (RW)rows;
				out.val.array.columns = (RW)cols;
				out.val.array.lparray = inner::xl_memory::new_xloper12_array(rows*cols);

				size_t i = 0;
				typename std::remove_reference<TSimpleGrid>::type::value_type::value_type default_value;
				for (auto const& row : in) {
					size_t c = 0;
					for(auto const& v : row){
						++c;
						out.val.array.lparray[i].xltype = 0;
						to_xl(v, out.val.array.lparray[i++]);
					}
					
					for (; c < (transposed?rows:cols); ++c) {
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
				inner::xl_type_ops::check_xl_type(in, xltypeMulti);

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
			void to_xl(util::xl_transposed <std::vector<std::vector<T>>> const& in, XLOPER12& out) {
				transposed_simple_grid_to_xl(in, out);
			}
			template<typename T>
			void to_xl(util::xl_transposed<std::vector<std::list<T>>> const& in, XLOPER12& out) {
				transposed_simple_grid_to_xl(in, out);
			}
			template<typename T>
			void to_xl(util::xl_transposed<std::list<std::list<T>>> const& in, XLOPER12& out) {
				transposed_simple_grid_to_xl(in, out);
			}
			template<typename T>
			void to_xl(util::xl_transposed<std::list<std::vector<T>>> const& in, XLOPER12& out) {
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
					inner::xl_type_ops::set_xl_type(out, xltypeNil);
					return;
				}

				inner::xl_type_ops::set_xl_type(out, xltypeMulti);
				out.val.array.rows = (RW)rows;
				out.val.array.columns = (RW)cols;
				if (transposed) 
					std::swap(out.val.array.rows, out.val.array.columns);
				
				out.val.array.lparray = inner::xl_memory::new_xloper12_array(rows * cols);

				size_t i = 0;
				if(transposed){
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
				inner::xl_type_ops::check_xl_type(in, xltypeMulti);

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
						XLOPER12 const& value_arg = in.val.array.lparray[i+stride];
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
			void to_xl(util::xl_transposed<std::map<TKey, TValue>> const& in, XLOPER12& out) {
				linear_map_to_xl(in.value(), out, true);
			}
			template<typename TKey, typename TValue>
			void to_xl(util::xl_transposed<std::unordered_map<TKey, TValue>> const& in, XLOPER12& out) {
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
					inner::xl_type_ops::set_xl_type(out, xltypeNil);
					return;
				}

				inner::xl_type_ops::set_xl_type(out, xltypeMulti);
				out.val.array.rows = (RW)rows+1;//keys are titles, so another row
				out.val.array.columns = (RW)cols;
				if (transposed)
					std::swap(out.val.array.rows, out.val.array.columns);
				size_t sz = (rows + 1) * cols;
				out.val.array.lparray = inner::xl_memory::new_xloper12_array(sz);
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
				inner::xl_type_ops::check_xl_type(in, xltypeMulti);

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
			void to_xl(util::xl_transposed<std::map<TKey, std::vector<TValue>>> const& in, XLOPER12& out) {
				map_grid_to_xl(in.value(), out, true);
			}
			template<typename TKey, typename TValue>
			void to_xl(util::xl_transposed<std::map<TKey, std::list<TValue>>> const& in, XLOPER12& out) {
				map_grid_to_xl(in.value(), out, true);
			}
			template<typename TKey, typename TValue>
			void to_xl(util::xl_transposed<std::unordered_map<TKey, std::vector<TValue>>> const& in, XLOPER12& out) {
				map_grid_to_xl(in.value(), out, true);
			}
			template<typename TKey, typename TValue>
			void to_xl(util::xl_transposed<std::unordered_map<TKey, std::list<TValue>>> const& in, XLOPER12& out) {
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
			static void to_xl(data::xl_variant const& in, XLOPER12& out) {
				using namespace data;
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
			inline void from_xl(XLOPER12 const& in, data::xl_variant& out) {
				CHECK_INPUT(in, out);
				out.set(in);
			}

			inline void to_xl(data::xl_nvp const& in, XLOPER12& out) {
				simple_grid_to_xl(in.values(), out);
			}
			inline void from_xl(XLOPER12 const& in, data::xl_nvp& out) {
				CHECK_INPUT(in, out);
				inner::xl_type_ops::check_xl_type(in, xltypeMulti);
				simple_grid_from_xl(in, out.values(), data::xl_nvp::transpose_input(in));
			}
			inline void to_xl(data::xl_nvp_cs const& in, XLOPER12& out) {
				simple_grid_to_xl(in.values(), out);
			}
			inline void from_xl(XLOPER12 const& in, data::xl_nvp_cs & out){
				CHECK_INPUT(in, out);
				inner::xl_type_ops::check_xl_type(in, xltypeMulti);
				simple_grid_from_xl(in, out.values(), data::xl_nvp::transpose_input(in));
			}
			inline void to_xl(data::xl_table const& in, XLOPER12 & out) {
				simple_grid_to_xl(in.table(), out);
			}
			inline void from_xl(XLOPER12 const& in, data::xl_table & out){
				CHECK_INPUT(in, out);
				inner::xl_type_ops::check_xl_type(in, xltypeMulti);
				simple_grid_from_xl(in, out.table(), data::xl_table::transpose_input(in));
			}
			inline void to_xl(data::xl_table_cs const& in, XLOPER12 & out) {
				simple_grid_to_xl(in.table(), out);
			}
			inline void from_xl(XLOPER12 const& in, data::xl_table_cs& out) {
				CHECK_INPUT(in, out);
				inner::xl_type_ops::check_xl_type(in, xltypeMulti);
				simple_grid_from_xl(in, out.table(), data::xl_table::transpose_input(in));
			}

			// FP12
			inline FP12* to_xl(fast_array::xl_fast_array const& in) {
				return in.fp12();
			}
			inline FP12* to_xlp(fast_array::xl_fast_array const& in) {
				return in.fp12();
			}
			inline fast_array::xl_fast_array from_xl(FP12* in) {
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
				static fast_array::xl_fast_array fp12(1, 1, 0.0);
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
				out = inner::xl_memory::new_xloper12();
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
				util::check_null_xlp(in);
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
			template< typename T> struct actual_xl_target_type{
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
			__AF_DECLARE_XL_TYPE_TRAIT(fast_array::xl_fast_array, FP12*, "K%");
			__AF_DECLARE_XL_TYPE_TRAIT(const fast_array::xl_fast_array, FP12*, "K%");
			__AF_DECLARE_XL_TYPE_TRAIT(const fast_array::xl_fast_array&, FP12*, "K%");

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
			template<> inline void append_xl_ret_type_string<fast_array::xl_fast_array>(std::string& out) {
				out.append("K%"); // everything other than FP12 is returned as XLOPER12
			}
			template<> inline void append_xl_ret_type_string<const fast_array::xl_fast_array>(std::string& out) {
				out.append("K%"); // everything other than FP12 is returned as XLOPER12
			}
			template<> inline void append_xl_ret_type_string<const fast_array::xl_fast_array&>(std::string& out) {
				out.append("K%"); // everything other than FP12 is returned as XLOPER12
			}
		}

		namespace xl_signature {
			// deducing the signature of functions that we are exposing
			// and building Excel type strings to register these functions is a bit of template magic

			template<typename RetT>
			void append_to_signature_string(std::string& out){
				xl_traits::append_xl_type_string<RetT>(out);
			}
			template<typename RetT, typename Arg1T, typename ... ArgTs>
			void append_to_signature_string(std::string& out){
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
			std::string get_signature_string(RetT(*)(ArgTs...)){
				std::string ret;
				xl_traits::append_xl_ret_type_string <RetT>(ret);
				append_to_signature_string<ArgTs...>(ret);
				return ret;
			}
		
			// quick and dirty functions and lambdas only ever deal with XLOPER12s, in and out
			template<typename RetT>
			void append_to_signature_string_qd(std::string& out) {
				out+="Q";
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

		// xl_registration of functions depends on the function signature
		// the monster switch statement is ugly, but does it efficiently
		namespace xl_registration {
			struct xl_function_data {
				std::string const _function_xl_name;
				std::string const _function_name;
				std::string const _signature;
				std::string const _function_help;
				std::string const _argument_names;
				std::vector<std::string> const _arguments_help;

				void xlfRegister_impl(XLOPER12 const& xDLL, std::string const& function_category_) const {
					using namespace xl_conversions;
					using namespace inner::xl_constants;
					using namespace data;

					auto_pxl function_category{to_xlp(function_category_)};
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
				static std::string const& get_function_category(std::string const& function_category) {
					return instance()->_function_category;
				}

				static bool register_function(
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
					std::string f_help(function_help.empty()? function_xl_name:function_help);
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
						f_xl_name,
						function_name,
						is_volatile?(signature+"!"):signature,
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
					for (auto const& func : inst->_functions)
						func->xlfRegister_impl(xDLL, f_category);
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

#define __AF_REGISTER_FUNCTION(__F, __F_HELP, __ARG_NAMES, __ARGS_HELP ) namespace{\
	static const auto registering_##__F = autotelica::xloper::xl_registration::xl_f_registry::register_function(\
			#__F, __AF_XL_IMPL_F_N_STR(__F),\
			__af_xl_signature( __F ), __af_xl_arg_count(__F),\
			false,\
			__F_HELP, __ARG_NAMES, __ARGS_HELP);\
	}

#define __AF_REGISTER_VOLATILE_FUNCTION(__F, __F_HELP, __ARG_NAMES, __ARGS_HELP )  namespace{\
	static const auto registering_##__F = autotelica::xloper::xl_registration::xl_f_registry::register_function(\
			#__F, __AF_XL_IMPL_F_N_STR(__F),\
			__af_xl_signature( __F ), __af_xl_arg_count(__F),\
			true,\
			__F_HELP, __ARG_NAMES, __ARGS_HELP);\
	}

#define __AF_REGISTER_NAMED_FUNCTION(__NAME, __F, __F_HELP, __ARG_NAMES, __ARGS_HELP ) namespace{\
	static const auto registering_##__NAME = autotelica::xloper::xl_registration::xl_f_registry::register_function(\
			#__NAME, __AF_XL_IMPL_F_N_STR(__NAME),\
			__af_xl_signature( __F ), __af_xl_arg_count(__F),\
			false,\
			__F_HELP, __ARG_NAMES, __ARGS_HELP);\
	}

#define __AF_REGISTER_VOLATILE_NAMED_FUNCTION(__NAME, __F, __F_HELP, __ARG_NAMES, __ARGS_HELP )  namespace{\
	static const auto registering_##__NAME = autotelica::xloper::xl_registration::xl_f_registry::register_function(\
			#__NAME, __AF_XL_IMPL_F_N_STR(__NAME),\
			__af_xl_signature( __F ), __af_xl_arg_count(__F),\
			true,\
			__F_HELP, __ARG_NAMES, __ARGS_HELP);\
	}
#define __AF_REGISTER_QD_FUNCTION(__F )  namespace{\
	static const auto registering_##__F = autotelica::xloper::xl_registration::xl_f_registry::register_function(\
			#__F, __AF_XL_IMPL_QD_F_N_STR(__F),\
			__af_xl_qd_signature( __F ), __af_xl_arg_count(__F),\
			false);\
	}

#define __AF_REGISTER_QD_VOLATILE_FUNCTION(__F )  namespace{\
	static const auto registering_##__F = autotelica::xloper::xl_registration::xl_f_registry::register_function(\
			#__F, __AF_XL_IMPL_QD_F_N_STR(__F),\
			__af_xl_qd_signature( __F ), __af_xl_arg_count(__F),\
			true);\
	}

#define __AF_REGISTER_LAMBDA_FUNCTION(__NAME, __F, __F_HELP, __ARG_NAMES, __ARGS_HELP ) namespace{\
	static const auto registering_##__NAME = autotelica::xloper::xl_registration::xl_f_registry::register_function(\
			#__NAME, __AF_XL_IMPL_LAMBDA_F_N_STR( __NAME ),\
			__af_xl_lambda_signature( __AF_XL_DECL_LAMBDA_F_N(__NAME) ), __af_xl_lambda_arg_count(__AF_XL_DECL_LAMBDA_F_N(__NAME)),\
			false,\
			__F_HELP, __ARG_NAMES, __ARGS_HELP);\
	}

#define __AF_REGISTER_LAMBDA_VOLATILE_FUNCTION(__NAME, __F, __F_HELP, __ARG_NAMES, __ARGS_HELP )  namespace{\
	static const auto registering_##__F = autotelica::xloper::xl_registration::xl_f_registry::register_function(\
			#__NAME, __AF_XL_IMPL_LAMBDA_F_N_STR( __NAME ),\
			__af_xl_lambda_signature( __AF_XL_DECL_LAMBDA_F_N(__NAME) ), __af_xl_lambda_arg_count(__AF_XL_DECL_LAMBDA_F_N(__NAME)),\
			true,\
			__F_HELP, __ARG_NAMES, __ARGS_HELP);\
	}

#define __AF_REGISTER_QD_LAMBDA_FUNCTION(__NAME )  namespace{\
	static const auto registering_##__NAME = autotelica::xloper::xl_registration::xl_f_registry::register_function(\
			#__NAME, __AF_XL_IMPL_LAMBDA_F_N_STR( __NAME ),\
			__af_xl_lambda_signature( __AF_XL_DECL_LAMBDA_F_N(__NAME) ), __af_xl_lambda_arg_count(__AF_XL_DECL_LAMBDA_F_N(__NAME)),\
			false);\
	}

#define __AF_REGISTER_QD_LAMBDA_VOLATILE_FUNCTION(__NAME)  namespace{\
	static const auto registering_##__NAME = autotelica::xloper::xl_registration::xl_f_registry::register_function(\
			#__NAME, __AF_XL_IMPL_LAMBDA_F_N_STR(__NAME),\
			__af_xl_lambda_signature( __AF_XL_DECL_LAMBDA_F_N(__NAME) ), __af_xl_lambda_arg_count(__AF_XL_DECL_LAMBDA_F_N(__NAME)),\
			true);\
	}
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

#define __AF_XL_TRY try {\
	autotelica::xloper::util::check_wizard();

#define __AF_XL_CATCH }\
		catch(autotelica::xloper::errors::xloper_exception const& e) { return e.errorXlp(); }\
		catch(std::runtime_error const& e) { return autotelica::xloper::errors::translate_error(e); }\
		catch(...) { return autotelica::xloper::errors::translate_error(); }

#define __AF_XL_CATCH_2(__F) }\
		catch(autotelica::xloper::errors::xloper_exception const& e) { return autotelica::xloper::xl_conversions::to_return_type<__af_xl_ret_t(__F)>(e.errorXlp()); }\
		catch(std::runtime_error const& e) { return autotelica::xloper::xl_conversions::to_return_type<__af_xl_ret_t(__F)>(autotelica::xloper::errors::translate_error(e)); }\
		catch(...) { return autotelica::xloper::xl_conversions::to_return_type<__af_xl_ret_t(__F)>(autotelica::xloper::errors::translate_error()); }



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

#define AF_DECLARE_EXCEL_VOLATILE_FUNCTION(...) \
		__AF_XL_EXPAND(\
			 __AF_XL_FUNC_DISPATCH(__VA_ARGS__, _12, _ERROR, _11, _ERROR, _10, _ERROR, _9, _ERROR, _8, _ERROR, _7, _ERROR, _6, _ERROR, _5, _ERROR, _4, _ERROR, _3, _ERROR, _2, _ERROR, _1, _ERROR, _0, _ERROR, N, ...)(__VA_ARGS__)\
		);\
		__AF_XL_EXPAND(\
			__AF_REGISTER_VOLATILE_FUNCTION(__VA_ARGS__)\
		);

#define AF_DECLARE_EXCEL_NAMED_FUNCTION(__NAME, ...) \
		__AF_XL_EXPAND(\
			 __AF_XL_NAMED_FUNC_DISPATCH(__VA_ARGS__, _12, _ERROR, _11, _ERROR, _10, _ERROR, _9, _ERROR, _8, _ERROR, _7, _ERROR, _6, _ERROR, _5, _ERROR, _4, _ERROR, _3, _ERROR, _2, _ERROR, _1, _ERROR, _0, _ERROR, N, ...)(__NAME, __VA_ARGS__)\
		);\
		__AF_XL_EXPAND(\
			__AF_REGISTER_NAMED_FUNCTION_DISPATCH(__NAME, __VA_ARGS__)\
		);

#define AF_DECLARE_EXCEL_VOLATILE_NAMED_FUNCTION(__NAME, ...) \
		__AF_XL_EXPAND(\
			 __AF_XL_NAMED_FUNC_DISPATCH(__VA_ARGS__, _12, _ERROR, _11, _ERROR, _10, _ERROR, _9, _ERROR, _8, _ERROR, _7, _ERROR, _6, _ERROR, _5, _ERROR, _4, _ERROR, _3, _ERROR, _2, _ERROR, _1, _ERROR, _0, _ERROR, N, ...)(__NAME, __VA_ARGS__)\
		);\
		__AF_XL_EXPAND(\
			__AF_REGISTER_VOLATILE_NAMED_FUNCTION(__NAME, __VA_ARGS__)\
		);

#define AF_DECLARE_QD_EXCEL_FUNCTION(__F, __NO_OF_ARGS) \
		__AF_XL_EXPAND(\
			 __AF_XL_QD_WRAPPER_FUNCTION_##__NO_OF_ARGS(__F)\
		);\
		__AF_XL_EXPAND(\
			__AF_REGISTER_QD_FUNCTION(__F )\
		);

#define AF_DECLARE_QD_EXCEL_VOLATILE_FUNCTION(__F, __NO_OF_ARGS) \
		__AF_XL_EXPAND(\
			 __AF_XL_QD_WRAPPER_FUNCTION_##__NO_OF_ARGS(__F)\
		);\
		__AF_XL_EXPAND(\
			__AF_REGISTER_QD_VOLATILE_FUNCTION(__F )\
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

#ifdef AF_EXCLUDE_DEFAULT_FUNCTIONS 
#define AF_DECLARE_XLL(Category) \
		namespace {\
			static const auto af_xl_category_declaration = autotelica::xloper::xl_registration::xl_f_registry::set_function_category(Category);\
		}\
		extern "C" __declspec(dllexport) void __stdcall xlAutoFree12(LPXLOPER12 pXL) {autotelica::xloper::inner::xl_memory::freeXL(pXL);}\
		extern "C" __declspec(dllexport) int __stdcall xlAutoOpen(void){return autotelica::xloper::xl_registration::xl_f_registry::xlAutoOpen_impl();}
#else
#define AF_DECLARE_XLL(Category) \
		namespace {\
			static const auto af_xl_category_declaration = autotelica::xloper::xl_registration::xl_f_registry::set_function_category(Category);\
		}\
		extern "C" __declspec(dllexport) void __stdcall xlAutoFree12(LPXLOPER12 pXL) {autotelica::xloper::inner::xl_memory::freeXL(pXL);}\
		extern "C" __declspec(dllexport) int __stdcall xlAutoOpen(void){return autotelica::xloper::xl_registration::xl_f_registry::xlAutoOpen_impl();}\
		AF_DECLARE_EXCEL_NAMED_FUNCTION(af_xl_configure_error_policy, autotelica::xloper::errors::af_xl_configure_error_policy, \
			"Configure handling of errors and invocation behaviour.", \
			RichErrorText, "Provide detailed error descriptions (instead of just #N\\A).",\
			PropagateErrors, "When there is an error in input, return the same error from function.",\
			InterpretMissing, "Replace missing values with defaults where sensible.",\
			DisableWizardCalls, "Disable function invocation within Excel function wizard.");\
		AF_DECLARE_EXCEL_NAMED_FUNCTION(af_xl_display_error_policy,autotelica::xloper::errors::af_xl_display_error_policy,\
			"Display current configuration of error policy.");\
		AF_DECLARE_EXCEL_NAMED_FUNCTION(af_xl_configure_error_policy_ex, autotelica::xloper::errors::af_xl_configure_error_policy_ex, \
			"Configure handling of errors and invocation behaviour.", \
			PolicyDetails, "An array containing inputs laid out like in the output of af_xl_display_error_policy function.");\
		AF_DECLARE_EXCEL_NAMED_FUNCTION(af_xl_fast_array_cache_size,autotelica::xloper::object_caches::af_xl_fast_array_cache_size,\
			"Display the number of elements in the fast array cache.");\
		AF_DECLARE_EXCEL_NAMED_FUNCTION(af_xl_clear_fast_array_cache,autotelica::xloper::object_caches::af_xl_clear_fast_array_cache,\
			"Clear fast array cache.");\
		AF_DECLARE_EXCEL_NAMED_FUNCTION(af_xl_object_cache_size,autotelica::xloper::object_caches::af_xl_object_cache_size,\
			"Size of an objects cache.",\
			cache_name, "Name of an objects cache.");\
		AF_DECLARE_EXCEL_NAMED_FUNCTION(af_xl_object_cache_sizes,autotelica::xloper::object_caches::af_xl_object_cache_sizes,\
			"Sizes of all objects caches.");\
		AF_DECLARE_EXCEL_NAMED_FUNCTION(af_xl_clear_object_cache,autotelica::xloper::object_caches::af_xl_clear_object_cache,\
			"Clear an objects cache.",\
			cache_name, "Name of an objects cache.");\
		AF_DECLARE_EXCEL_NAMED_FUNCTION(af_xl_clear_all_object_caches,autotelica::xloper::object_caches::af_xl_clear_all_object_caches,\
			"Clear all objects caches.");\
		AF_DECLARE_EXCEL_NAMED_FUNCTION(af_xl_list_objects_cache,autotelica::xloper::object_caches::af_xl_list_objects_cache,\
			"List the content of an objects cache.",\
			cache_name, "Name of an objects cache.");\
		AF_DECLARE_EXCEL_NAMED_FUNCTION(af_xl_list_all_objects_caches,autotelica::xloper::object_caches::af_xl_list_all_objects_caches,\
			"List the content of all objects caches.");\
		AF_DECLARE_EXCEL_NAMED_FUNCTION(af_xl_clear_all_caches,autotelica::xloper::object_caches::af_xl_clear_all_caches,\
			"Clear all caches (objects and fast arrays).");
#endif
	}
}