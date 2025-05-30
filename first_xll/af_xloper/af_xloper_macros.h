#pragma once
#include "af_xloper/af_xloper_registration.h"

namespace autotelica {
	namespace xloper {
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

	}
}