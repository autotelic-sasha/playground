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
#define AF_ASSERTS_LONG_FORM() autotelica::diagnostic_messages::messages::messages::set_long_form();

#define AF_ASSERTS_SHORT_FORM() autotelica::diagnostic_messages::messages::messages::set_long_form(false);

#define AF_ASSERT(condition, ... ) \
    if(!(condition)){\
        (autotelica::diagnostic_messages::messages::error_ex(\
            #condition, nullptr, 0, __VA_ARGS__));\
    }

#define af_assert(condition, ...) AF_ASSERT(condition, __VA_ARGS__)

#define AF_ASSERT_EX(condition, ... ) \
    if(!(condition)){\
        (autotelica::diagnostic_messages::messages::error_ex(\
            #condition, __FILE__, __LINE__, __VA_ARGS__));\
    }

#define af_assert_ex(condition, ...) AF_ASSERT_EX(condition, __VA_ARGS__)

#define AF_ASSERT_WARNING(condition, ... ) \
    if(!(condition)){\
         (autotelica::diagnostic_messages::messages::warning_ex(\
            #condition, nullptr, 0, __VA_ARGS__));\
    }

#define af_assert_warning(condition, ...) AF_ASSERT_WARNING(condition, __VA_ARGS__)

#define AF_ASSERT_WARNING_EX(condition, ... ) \
    if(!(condition)){\
        (autotelica::diagnostic_messages::messages::warning_ex(\
            #condition, __FILE__, __LINE__, __VA_ARGS__ ));\
    }

#define af_assert_warning_ex(condition, ...) AF_ASSERT_WARNING_EX(condition, __VA_ARGS__)

#define AF_ERROR(...) \
    (autotelica::diagnostic_messages::messages::error(__VA_ARGS__))

#define af_error(...) AF_ERROR(__VA_ARGS__)

#define AF_ERROR_EX(... ) \
    (autotelica::diagnostic_messages::messages::error_ex(\
        nullptr, __FILE__, __LINE__, __VA_ARGS__))

#define af_error_ex(...) AF_ERROR_EX(__VA_ARGS__)


#define AF_WARNING(...) \
    (autotelica::diagnostic_messages::messages::warning(__VA_ARGS__))

#define af_warning(...) AF_WARNING(__VA_ARGS__)

#define AF_WARNING_EX(... ) \
    (autotelica::diagnostic_messages::messages::warning_ex(\
        nullptr, __FILE__, __LINE__, __VA_ARGS__))

#define af_warning_ex(...) AF_WARNING_EX(__VA_ARGS__)

#define AF_MESSAGE( ... ) \
    (autotelica::diagnostic_messages::messages::message(__VA_ARGS__))

#define af_message(...) AF_MESSAGE(__VA_ARGS__)

#define AF_MESSAGE_EX(... ) \
    (autotelica::diagnostic_messages::messages::message_ex(\
        nullptr, __FILE__, __LINE__, __VA_ARGS__))

#define af_message_ex(...) AF_MESSAGE_EX(__VA_ARGS__)

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
#endif
}
}

