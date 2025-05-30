#pragma once
#include "af_xloper_macros_deductions.h"

namespace autotelica {
	namespace xloper {
		// LOOK AWAY, LOOK AWAY ... MACROS GOING CRAZY HERE
		// this is the implementation details of the magic part


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


	}
}