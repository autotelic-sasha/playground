#pragma once
#include "af_xloper_registration.h"

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


	}
}