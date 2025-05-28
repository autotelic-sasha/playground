#pragma once

#include "af_xloper/af_xloper_macros.h"

namespace autotelica {
	namespace xloper {
		// bits of this are relying on https://www.wiley.com/en-gb/Financial+Applications+using+Excel+Add-in+Development+in+C+%2F+C%2B%2B%2C+2nd+Edition-p-9780470319048

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