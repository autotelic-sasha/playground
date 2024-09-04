#pragma once
#include <string>
#include <exception>
#include <memory>
#include <iostream>
#include <sstream>
#include <ctime>
#include "string_util.h"
#include "diagnostic_messages.h"
namespace autotelica {
namespace asserts {
    // Simple but handy asserts and tracing utils
    // Implements simple string interpolation with lazy evaluation of tracing messages.
    // Can be externally configured to emit errors, warnings and messages in different environments. 
    // 


    //  Macros. I know, I know - it's bad form, rude, and bad for karma. 
    //  But ... stringifying, file, line, lazy evaluation of fucntion parameters ... all those nice things are just too tempting. 
    // 
    //  ASSERT is expected to throw an exception if the condition is false (default implementation throw runtime_error)
    //  ASSERT_WARNING shows the warning message if the condition is false 
    //  WARNING shows the warning message 
    //  MESSAGE shows the message
    // 
    // _EX versions add file and line information to error messages - these are not default for security purposes.
    // lowercase versions are to cater for variety in taste
    // _DBG_ versions only trace in DEBUG builds
    // _W versions work with wide characters (sorry, can't automate detection of those)
    // _T versions take string type as a parameter and do string conversions on the fly

#define __W_(str) L##str
#define __W(str) __W_(str)

#define AF_ASSERTS_LONG_FORM() { autotelica::diagnostic_messages::messages::messages::set_long_form(); }

#define AF_ASSERTS_SHORT_FORM() { autotelica::diagnostic_messages::messages::messages::set_long_form(false); }

#define AF_ASSERT(condition, ... ) {\
    if(!(condition)){\
        autotelica::diagnostic_messages::messages::error_assert(\
            #condition, __VA_ARGS__);\
    }}

#define AF_ASSERT_W(condition, ... ) {\
    if(!(condition)){\
        autotelica::diagnostic_messages::messages::error_assert(\
            __W(#condition), __VA_ARGS__);\
    }}

#define AF_ASSERT_T(string_t,condition,  ... ) {\
    if(!(condition)){\
        autotelica::diagnostic_messages::messages::error_assert(\
            autotelica::string_util::utf8_convert<string_t>(#condition), __VA_ARGS__);\
    }}

#define af_assert(condition, ...) AF_ASSERT(condition, __VA_ARGS__)
#define af_assert_w(condition, ...) AF_ASSERT_W(condition, __VA_ARGS__)
#define af_assert_t(string_t, condition,  ...) AF_ASSERT_T(string_t, condition, __VA_ARGS__)

#define AF_ASSERT_EX(condition, ... ) {\
    if(!(condition)){\
        autotelica::diagnostic_messages::messages::error_ex_assert(\
            #condition, __FILE__, __LINE__, __VA_ARGS__);\
    }}

#define AF_ASSERT_EX_W(condition, ... ) {\
    if(!(condition)){\
        autotelica::diagnostic_messages::messages::error_ex_assert(\
            __W(#condition), __W(__FILE__), __LINE__, __VA_ARGS__);\
    }}

#define AF_ASSERT_EX_T(string_t, condition, ... ) {\
    if(!(condition)){\
        autotelica::diagnostic_messages::messages::error_ex_assert(\
            autotelica::string_util::utf8_convert<string_t>(#condition), \
            autotelica::string_util::utf8_convert<string_t>(__FILE__), __LINE__, __VA_ARGS__);\
    }}


#define af_assert_ex(condition, ...) AF_ASSERT_EX(condition, __VA_ARGS__)
#define af_assert_ex_w(condition, ...) AF_ASSERT_EX_W(condition, __VA_ARGS__)
#define af_assert_ex_t(string_t, condition, ...) AF_ASSERT_EX_T(string_t, condition, __VA_ARGS__)

#define AF_ASSERT_WARNING(condition, ... ) {\
    if(!(condition)){\
         autotelica::diagnostic_messages::messages::warning_assert(\
            #condition, __VA_ARGS__);\
    }}

#define AF_ASSERT_WARNING_W(condition, ... ) {\
    if(!(condition)){\
         autotelica::diagnostic_messages::messages::warning_assert(\
            __W(#condition), __VA_ARGS__);\
    }}

#define AF_ASSERT_WARNING_T(string_t, condition, ... ) {\
    if(!(condition)){\
         autotelica::diagnostic_messages::messages::warning_assert(\
            autotelica::string_util::utf8_convert<string_t>(#condition), __VA_ARGS__);\
    }}

#define af_assert_warning(condition, ...) AF_ASSERT_WARNING(condition, __VA_ARGS__)
#define af_assert_warning_w(condition, ...) AF_ASSERT_WARNING_W(condition, __VA_ARGS__)
#define af_assert_warning_t(string_t, condition, ...) AF_ASSERT_WARNING_T(string_t, condition, __VA_ARGS__)

#define AF_ASSERT_WARNING_EX(condition, ... ) {\
    if(!(condition)){\
        autotelica::diagnostic_messages::messages::warning_ex_assert(\
            #condition, __FILE__, __LINE__, __VA_ARGS__ );\
    }}

#define AF_ASSERT_WARNING_EX_W(condition, ... ) {\
    if(!(condition)){\
        autotelica::diagnostic_messages::messages::warning_ex_assert(\
            __W(#condition), __W(__FILE__), __LINE__, __VA_ARGS__ );\
    }}

#define AF_ASSERT_WARNING_EX_T(string_t, condition, ... ) {\
    if(!(condition)){\
        autotelica::diagnostic_messages::messages::warning_ex_assert(\
            autotelica::string_util::utf8_convert<string_t>(#condition), \
            autotelica::string_util::utf8_convert<string_t>(__FILE__), __LINE__, __VA_ARGS__);\
    }}

#define af_assert_warning_ex(condition, ...) AF_ASSERT_WARNING_EX(condition, __VA_ARGS__)
#define af_assert_warning_ex_w(condition, ...) AF_ASSERT_WARNING_EX_W(condition, __VA_ARGS__)
#define af_assert_warning_ex_t(string_t, condition, ...) AF_ASSERT_WARNING_EX_T(string_t, condition, __VA_ARGS__)

#define AF_ERROR(...) \
    {autotelica::diagnostic_messages::messages::error(__VA_ARGS__);}
#define AF_ERROR_W(...) \
    {autotelica::diagnostic_messages::messages::error(__VA_ARGS__);}
#define AF_ERROR_T(string_t, ...) \
    {autotelica::diagnostic_messages::messages::error(__VA_ARGS__);}

#define af_error(...) AF_ERROR(__VA_ARGS__)
#define af_error_w(...) AF_ERROR_W(__VA_ARGS__)
#define af_error_t(string_t, ...) AF_ERROR_T(string_t, __VA_ARGS__)

#define AF_ERROR_EX(... ) {\
    autotelica::diagnostic_messages::messages::error_ex(\
        __FILE__, __LINE__, __VA_ARGS__);}

#define AF_ERROR_EX_W(... ) {\
    autotelica::diagnostic_messages::messages::error_ex(\
        __W(__FILE__), __LINE__, __VA_ARGS__);}

#define AF_ERROR_EX_T(string_t, ... ) {\
    autotelica::diagnostic_messages::messages::error_ex(\
        autotelica::string_util::utf8_convert<string_t>(__FILE__), __LINE__, __VA_ARGS__);}

#define af_error_ex(...) AF_ERROR_EX(__VA_ARGS__)
#define af_error_ex_w(...) AF_ERROR_EX_W(__VA_ARGS__)
#define af_error_ex_t(string_t, ...) AF_ERROR_EX_T(string_t, __VA_ARGS__)

#define AF_WARNING(...) \
    {autotelica::diagnostic_messages::messages::warning(__VA_ARGS__);}
#define AF_WARNING_W(...) \
    {autotelica::diagnostic_messages::messages::warning(__VA_ARGS__);}
#define AF_WARNING_T(string_t, ...) \
    {autotelica::diagnostic_messages::messages::warning(__VA_ARGS__);}

#define af_warning(...) AF_WARNING(__VA_ARGS__)
#define af_warning_w(...) AF_WARNING_W(__VA_ARGS__)
#define af_warning_t(string_t, ...) AF_WARNING_T(string_t, __VA_ARGS__)

#define AF_WARNING_EX(... ) {\
    autotelica::diagnostic_messages::messages::warning_ex(\
        __FILE__, __LINE__, __VA_ARGS__);}

#define AF_WARNING_EX_W(... ) {\
    autotelica::diagnostic_messages::messages::warning_ex(\
        __W(__FILE__), __LINE__, __VA_ARGS__);}

#define AF_WARNING_EX_T(string_t, ... ) {\
    autotelica::diagnostic_messages::messages::warning_ex(\
        autotelica::string_util::utf8_convert<string_t>(__FILE__), __LINE__, __VA_ARGS__);}

#define af_warning_ex(...) AF_WARNING_EX(__VA_ARGS__)
#define af_warning_ex_w(...) AF_WARNING_EX_W(__VA_ARGS__)
#define af_warning_ex_t(string_t, ...) AF_WARNING_EX_T(__VA_ARGS__)

#define AF_MESSAGE( ... ) \
    {autotelica::diagnostic_messages::messages::message(__VA_ARGS__);}
#define AF_MESSAGE_W( ... ) \
    {autotelica::diagnostic_messages::messages::message(__VA_ARGS__);}
#define AF_MESSAGE_T( string_t, ... ) \
    {autotelica::diagnostic_messages::messages::message(__VA_ARGS__);}

#define af_message(...) AF_MESSAGE(__VA_ARGS__)
#define af_message_w(...) AF_MESSAGE_W(__VA_ARGS__)
#define af_message_t(string_t, ...) AF_MESSAGE_W(string_t, __VA_ARGS__)

#define AF_MESSAGE_EX(... ) {\
    autotelica::diagnostic_messages::messages::message_ex(\
        __FILE__, __LINE__, __VA_ARGS__);}

#define AF_MESSAGE_EX_W(... ) {\
    autotelica::diagnostic_messages::messages::message_ex(\
        __W(__FILE__), __LINE__, __VA_ARGS__);}

#define AF_MESSAGE_EX_T(string_t, ... ) {\
    autotelica::diagnostic_messages::messages::message_ex(\
        autotelica::string_util::utf8_convert<string_t>(__FILE__), __LINE__, __VA_ARGS__);}

#define af_message_ex(...) AF_MESSAGE_EX(__VA_ARGS__)
#define af_message_ex_w(...) AF_MESSAGE_EX_W(__VA_ARGS__)
#define af_message_ex_t(string_t, ...) AF_MESSAGE_EX_T(string_t, __VA_ARGS__)

#ifndef NDEBUG
#define AF_DBG_ASSERT(condition, ... ) AF_ASSERT(condition, __VA_ARGS__)
#define af_dbg_assert(condition, ...) af_assert(condition, __VA_ARGS__)
#define AF_DBG_ASSERT_EX(condition, ... ) AF_ASSERT_EX(condition, __VA_ARGS__ )
#define af_dbg_assert_ex(condition, ...) af_assert_ex(condition, __VA_ARGS__)
#define AF_DBG_ASSERT_WARNING(condition, ... ) AF_ASSERT_WARNING(condition, __VA_ARGS__ )
#define af_dbg_assert_warning(condition, ...) af_assert_warning(condition, __VA_ARGS__)
#define AF_DBG_ASSERT_WARNING_EX(condition, ... ) AF_ASSERT_WARNING_EX(condition, __VA_ARGS__)
#define af_dbg_assert_warning_ex(condition, ...) af_assert_warning_ex(condition, __VA_ARGS__)
#define AF_DBG_WARNING(... ) AF_WARNING( __VA_ARGS__ ) 
#define af_dbg_warning(...) af_warning(__VA_ARGS__)
#define AF_DBG_WARNING_EX(... ) AF_WARNING_EX(__VA_ARGS__)
#define af_dbg_warning_ex(...) af_warning_ex(__VA_ARGS__)
#define AF_DBG_MESSAGE( ... ) AF_MESSAGE(__VA_ARGS__)
#define af_dbg_message(...) af_message(__VA_ARGS__)
#define AF_DBG_MESSAGE_EX(... ) AF_MESSAGE_EX(__VA_ARGS__)
#define af_dbg_message_ex(...) af_message_ex(__VA_ARGS__)

#define AF_DBG_ASSERT_W(condition, ... ) AF_ASSERT_W(condition, __VA_ARGS__)
#define af_dbg_assert_w(condition, ...) af_assert_w(condition, __VA_ARGS__)
#define AF_DBG_ASSERT_EX_W(condition, ... ) AF_ASSERT_EX_W(condition, __VA_ARGS__ )
#define af_dbg_assert_ex_w(condition, ...) af_assert_ex_w(condition, __VA_ARGS__)
#define AF_DBG_ASSERT_WARNING_W(condition, ... ) AF_ASSERT_WARNING_W(condition, __VA_ARGS__ )
#define af_dbg_assert_warning_w(condition, ...) af_assert_warning_w(condition, __VA_ARGS__)
#define AF_DBG_ASSERT_WARNING_EX_W(condition, ... ) AF_ASSERT_WARNING_EX_W(condition, __VA_ARGS__)
#define af_dbg_assert_warning_ex_w(condition, ...) af_assert_warning_ex_w(condition, __VA_ARGS__)
#define AF_DBG_WARNING_W(... ) AF_WARNING_W( __VA_ARGS__ ) 
#define af_dbg_warning_w(...) af_warning_w(__VA_ARGS__)
#define AF_DBG_WARNING_EX_W(... ) AF_WARNING_EX_W(__VA_ARGS__)
#define af_dbg_warning_ex_w(...) af_warning_ex_w(__VA_ARGS__)
#define AF_DBG_MESSAGE_W( ... ) AF_MESSAGE_W(__VA_ARGS__)
#define af_dbg_message_w(...) af_message_w(__VA_ARGS__)
#define AF_DBG_MESSAGE_EX_W(... ) AF_MESSAGE_EX_W(__VA_ARGS__)
#define af_dbg_message_ex_w(...) af_message_ex_w(__VA_ARGS__)

#define AF_DBG_ASSERT_T(string_t, condition, ... ) AF_ASSERT_T(string_t, condition, __VA_ARGS__)
#define af_dbg_assert_t(string_t, condition, ...) af_assert_t(string_t, condition, __VA_ARGS__)
#define AF_DBG_ASSERT_EX_T(string_t, condition, ... ) AF_ASSERT_EX_T(string_t, condition, __VA_ARGS__ )
#define af_dbg_assert_ex_t(string_t, condition, ...) af_assert_ex_t(string_t, condition, __VA_ARGS__)
#define AF_DBG_ASSERT_TARNING_T(string_t, condition, ... ) AF_ASSERT_TARNING_T(string_t, condition, __VA_ARGS__ )
#define af_dbg_assert_tarning_t(string_t, condition, ...) af_assert_tarning_t(string_t, condition, __VA_ARGS__)
#define AF_DBG_ASSERT_TARNING_EX_T(string_t, condition, ... ) AF_ASSERT_TARNING_EX_T(string_t, condition, __VA_ARGS__)
#define af_dbg_assert_tarning_ex_t(string_t, condition, ...) af_assert_tarning_ex_t(string_t, condition, __VA_ARGS__)
#define AF_DBG_TARNING_T(string_t, ... ) AF_TARNING_T(string_t, __VA_ARGS__ ) 
#define af_dbg_tarning_t(string_t, ...) af_tarning_t(string_t, __VA_ARGS__)
#define AF_DBG_TARNING_EX_T(string_t, ... ) AF_TARNING_EX_T(string_t, __VA_ARGS__)
#define af_dbg_tarning_ex_t(string_t, ...) af_tarning_ex_t(string_t, __VA_ARGS__)
#define AF_DBG_MESSAGE_T(string_t, ... ) AF_MESSAGE_T(string_t, __VA_ARGS__)
#define af_dbg_message_t(string_t, ...) af_message_t(string_t, __VA_ARGS__)
#define AF_DBG_MESSAGE_EX_T(string_t, ... ) AF_MESSAGE_EX_T(string_t, __VA_ARGS__)
#define af_dbg_message_ex_t(string_t, ...) af_message_ex_t(string_t, __VA_ARGS__)


#else
#define AF_DBG_ASSERT(condition, ... ) 
#define af_dbg_assert(condition, ...) 
#define AF_DBG_ASSERT_EX(condition, ... ) 
#define af_dbg_assert_ex(condition, ...) 
#define AF_DBG_ASSERT_WARNING(condition, ... ) 
#define af_dbg_assert_warning(condition, ...) 
#define AF_DBG_ASSERT_WARNING_EX(condition, ... ) 
#define af_dbg_assert_warning_ex(condition, ...) 
#define AF_DBG_WARNING(... ) 
#define af_dbg_warning(...) 
#define AF_DBG_WARNING_EX(... ) 
#define af_dbg_warning_ex(...) 
#define AF_DBG_MESSAGE( ... ) 
#define af_dbg_message(...) 
#define AF_DBG_MESSAGE_EX(... ) 
#define af_dbg_message_ex(...) 

#define AF_DBG_ASSERT_W(condition, ... ) 
#define af_dbg_assert_w(condition, ...) 
#define AF_DBG_ASSERT_EX_W(condition, ... ) 
#define af_dbg_assert_ex_w(condition, ...) 
#define AF_DBG_ASSERT_WARNING_W(condition, ... ) 
#define af_dbg_assert_warning_w(condition, ...) 
#define AF_DBG_ASSERT_WARNING_EX_W(condition, ... ) 
#define af_dbg_assert_warning_ex_w(condition, ...) 
#define AF_DBG_WARNING_W(... ) 
#define af_dbg_warning_w(...) 
#define AF_DBG_WARNING_EX_W(... ) 
#define af_dbg_warning_ex_w(...) 
#define AF_DBG_MESSAGE_W( ... ) 
#define af_dbg_message_w(...) 
#define AF_DBG_MESSAGE_EX_W(... ) 
#define af_dbg_message_ex_w(...) 

#define AF_DBG_ASSERT_T(string_t, condition, ... ) 
#define af_dbg_assert_t(string_t, condition, ...) 
#define AF_DBG_ASSERT_EX_T(string_t, condition, ... ) 
#define af_dbg_assert_ex_t(string_t, condition, ...) 
#define AF_DBG_ASSERT_TARNING_T(string_t, condition, ... ) 
#define af_dbg_assert_tarning_t(string_t, condition, ...) 
#define AF_DBG_ASSERT_TARNING_EX_T(string_t, condition, ... ) 
#define af_dbg_assert_tarning_ex_t(string_t, condition, ...) 
#define AF_DBG_TARNING_T(string_t, ... ) 
#define af_dbg_tarning_t(string_t, ...) 
#define AF_DBG_TARNING_EX_T(string_t, ... ) 
#define af_dbg_tarning_ex_t(string_t, ...) 
#define AF_DBG_MESSAGE_T(string_t, ... ) 
#define af_dbg_message_t(string_t, ...) 
#define AF_DBG_MESSAGE_EX_T(string_t, ... ) 
#define af_dbg_message_ex_t(string_t, ...) 

#endif
}
}

