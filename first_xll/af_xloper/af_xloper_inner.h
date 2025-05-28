#pragma once
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>
#include "XLCALL.H"
#include <string>
#include <memory>
#include <stdexcept>
#include <mutex>
#include <iostream>
#include <locale>
#include <codecvt>

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
	}
}