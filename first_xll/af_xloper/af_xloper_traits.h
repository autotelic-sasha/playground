#pragma once
#include "af_xloper_conversions.h"

namespace autotelica {
	namespace xloper {
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
	}
}