#pragma once
#include <string>
#include <exception>
#include <memory>
#include <iostream>
#include <sstream>
#include <fstream>
#include <ctime>
#include <time.h>
#include <cstring>
#include "string_util.h"

namespace autotelica {
	namespace diagnostic_messages {
        // Base class for emitting messages. 
        // Implement a new version to emit messages in a new environment.
        struct message_handler {
            virtual ~message_handler() {}
            // override this 
            virtual void message(std::string const& m) = 0;
            virtual void message(std::wstring const& m) = 0;
        };

        template<typename string_t>
        struct messages_traits;

        template<>
        struct messages_traits<std::string> {
            using char_t = char;
            using stringstream_t = std::basic_stringstream<char>;
            using fstream_t = std::basic_ofstream<char>;

            static constexpr auto timestamp_f = "[%F %T] ";
            static constexpr auto in_file = "(in file: ";
            static constexpr auto at_line = "; at line ";
            static constexpr auto rbracket = ")";
            static constexpr auto assertion = "Assertion ";
            static constexpr auto failed = "failed.";
            static constexpr auto failed_to_open_file = "ERROR: failed to open file: ";
            static constexpr auto tracing_to_file = "ERROR: failed to open file: ";
            
            static constexpr auto empty = "";
            static constexpr auto blank = " ";
            static constexpr auto newline = "\n";
            static constexpr auto decoration = '*';
            static constexpr auto underline = '=';

            static constexpr auto message_prefix = "";
            static constexpr auto warning_prefix = "WARNING: ";
            static constexpr auto error_prefix = "ERROR: ";

            static inline std::basic_ostream<char>& cout() {
                return std::cout;
            }

            static inline size_t ftime(
                char* strDest,
                size_t maxsize,
                const char* format,
                const struct tm* timeptr
            ) {
                return strftime(strDest,maxsize,format,timeptr);
            }
            inline bool is_supported_type(std::string const& s) { return true; }
            inline bool is_supported_type(std::wstring const& s) { return false; }
            
        };
        template<>
        struct messages_traits<const char*> : public messages_traits<std::string> {};
        
        template<>
        struct messages_traits<std::wstring> {
            using char_t = wchar_t;
            using stringstream_t = std::basic_stringstream<wchar_t>;
            using fstream_t = std::basic_ofstream<wchar_t>;

            static constexpr auto timestamp_f = L"[%F %T] ";
            static constexpr auto in_file = L"(in file: ";
            static constexpr auto at_line = L"; at line ";
            static constexpr auto rbracket = L")";
            static constexpr auto assertion = L"Assertion ";
            static constexpr auto failed = L"failed.";
            static constexpr auto failed_to_open_file = L"ERROR: failed to open file: ";
            static constexpr auto tracing_to_file = L"Tracing to file: ";

            static constexpr auto empty = L"";
            static constexpr auto blank = L" ";
            static constexpr auto newline = L"\n";
            static constexpr auto decoration = L'*';
            static constexpr auto underline = L'=';

            static constexpr auto message_prefix = L"";
            static constexpr auto warning_prefix = L"WARNING: ";
            static constexpr auto error_prefix = L"ERROR: ";

            static std::basic_ostream<wchar_t>& cout() {
                return std::wcout;
            }

            static size_t ftime(
                wchar_t* strDest,
                size_t maxsize,
                const wchar_t* format,
                const struct tm* timeptr
            ) {
                return wcsftime(strDest, maxsize, format, timeptr);
            }
            inline bool is_supported_type(std::string const& s) { return false; }
            inline bool is_supported_type(std::wstring const& s) { return true; }
        };
        template<>
        struct messages_traits<const wchar_t*> : public messages_traits<std::wstring> {};


        // Default message handler
        // This is used when nothing else is configured.
        // Traces to stdout, throws runtime_error exceptions on error.
        struct default_message_handler : public message_handler {
            template<typename string_t>
            inline void message_(string_t const& m){
                using traits_t = messages_traits<string_t>;
                traits_t::cout() << std::boolalpha << m << std::endl;
            }

            void message(std::string const& m) override { message_(m); }
            void message(std::wstring const& m) override { message_(m); }
        };
        // Implementation details
        class messages_policy {
            bool _emit_messages;
            bool _emit_warnings;
            bool _emit_errors;
            bool _throw_errors;
            bool _long_form;
            bool _include_timestamp;
        public:
            messages_policy(
                bool emit_messages_ = true,
                bool emit_warnings_ = true,
                bool emit_errors_ = true,
                bool throw_errors_ = true,
                bool long_form_ = true,
                bool include_timestamp_ = true
            ) : _emit_messages(emit_messages_),
                _emit_warnings(emit_warnings_),
                _emit_errors(emit_errors_),
                _throw_errors(throw_errors_),
                _long_form(long_form_),
                _include_timestamp(include_timestamp_){
            }

            inline void configure(
                bool emit_messages_ = true,
                bool emit_warnings_ = true,
                bool emit_errors_ = true,
                bool throw_errors_ = true,
                bool long_form_ = true,
                bool include_timestamp_ = true
            ) {
                _emit_messages = emit_messages_;
                _emit_warnings = emit_warnings_;
                _emit_errors = emit_errors_;
                _throw_errors = throw_errors_;
                _long_form = long_form_;
                _include_timestamp = include_timestamp_;
            }
            inline bool emit_messages() const { return _emit_messages; }
            inline bool emit_warnings() const { return _emit_warnings; }
            inline bool emit_errors() const { return _emit_errors; }
            inline bool throw_errors() const { return _throw_errors; }
            inline bool long_form() const { return _long_form; }
            inline bool include_timestamp() const { return _include_timestamp; }

            inline void set_emit_messages(bool v = true) { _emit_messages = v; }
            inline void set_emit_warnings(bool v = true) { _emit_warnings = v; }
            inline void set_emit_errors(bool v = true) { _emit_errors = v; }
            inline void set_throw_errors(bool v = true) { _throw_errors = v; }
            inline void set_long_form(bool v = true) { _long_form = v; }
            inline void set_include_timestamp(bool v = true) { _include_timestamp = v; }
        };

        namespace diagnostic_messages_impl {
            // helpers to delegete stuff to the right string
            inline bool is_empty(const char* const s) {return s && strlen(s) == 0;}
            inline bool is_empty(const wchar_t* const s) {return s && wcslen(s) == 0;}
            inline bool is_empty(std::string const& s) {return s.empty();}
            inline bool is_empty(std::wstring const& s) {return s.empty();}

            // Singleton manager for handling messaging. 
            class messages_impl {
            protected:
                std::shared_ptr<message_handler> _handler; // current message handler
                messages_policy _policy;
                // Constructor creates a default handler if not given one.
                messages_impl(
                    std::shared_ptr<message_handler> handler_ = nullptr
                ) : _handler(handler_){
                    if (!_handler)
                        _handler = std::shared_ptr<message_handler>{ new default_message_handler() };
                }

                // Adds information about file and line to the message.
                template<typename stringstream_t, typename char_t>
                static inline void append_file_line(
                        stringstream_t& sout, 
                        const char_t* const file, 
                        int line) {
                    using string_t = typename std::basic_string<char_t>;
                    using traits_t = messages_traits<string_t>;

                    if (!file) return;
                    sout << traits_t::in_file << file << traits_t::at_line << line << traits_t::rbracket;
                }
                // Add information about condition that failed to the message. 
                template<typename stringstream_t, typename string_t>
                static inline void prepend_failed_condition_message(stringstream_t& sout, const string_t& condition) {
                    using traits_t = messages_traits<string_t>;
                    if (is_empty(condition)) return;
                    sout << std::boolalpha << traits_t::assertion << condition << traits_t::failed;
                }
                // Prefixes the message with the timestamp
                template<typename stringstream_t>
                static void prepend_timestamp(stringstream_t& sout) {
                    using char_t = typename stringstream_t::char_type;
                    using string_t = typename std::basic_string<char_t>;
                    using traits_t = messages_traits<string_t>;

                    if (!get()->_policy.include_timestamp()) return;
                    std::time_t const time = std::time({});
                    char_t timeString[100];//we'll sacrifice 100 bytes for this
                    tm tm_buff;
#ifdef __linux__
                    gmtime_r(&time, &tm_buff);
#else
                    gmtime_s(&tm_buff, &time);
#endif
                    traits_t::ftime(timeString, sizeof(timeString)/sizeof(char_t),
                        traits_t::timestamp_f, &tm_buff);
                    sout << timeString;
                }

                // Get the instance of this object, configure the handler while at it.
                static std::shared_ptr<messages_impl> get(std::shared_ptr<message_handler> handler_ = nullptr) {
                    static std::shared_ptr<messages_impl> _instance;
                    if (!_instance)
                        _instance = std::shared_ptr<messages_impl>(new messages_impl(handler_));
                    if (handler_ && _instance->_handler != handler_)
                        _instance->_handler = handler_;
                    return _instance;
                }

                template<typename char_t>
                static bool is_whitespace(const char_t* const m) {
                    const char_t* s = m;
                    while (*s++) {
                        if (!string_util::fast_is_space(*s)) return false;
                    }
                    return true;
                }
                // Main entry point for formatting and emitting messages.
                template<typename char_t, typename... Targs>
                static void emit(
                    bool throw_exception, // if true, also throw
                    const char_t* const prefix, // function providing prefix for the message (it's a pointer to member of this class, kewl)
                    const char_t* const condition, // which condition failed (nullptr if you don't care)
                    const char_t* const file, // which file this happened in (nullptr if you don't care)
                    int line, // which line this happened at (ignored if file is nullptr)
                    const char_t* m = nullptr, // main body of the message
                    Targs... Fargs) // parameters for interpolation of m
                {
                    using string_t = std::basic_string<char_t>;
                    using traits_t = messages_traits<string_t>;
                    using stringstream_t = typename traits_t::stringstream_t;

                    if (!m || !get()->_handler) return;
                    bool wht = is_whitespace(m);
                    stringstream_t sout;
                    if(!wht){
						prepend_timestamp(sout);
						sout << std::boolalpha << prefix;
						if (get()->_policy.long_form() && condition)
							prepend_failed_condition_message(sout, condition);
					}
                    string_util::var_printf(sout, m, Fargs...);
                    if(!wht){
						if (get()->_policy.long_form())
							append_file_line(sout, file, line);
					}
                    if (throw_exception) {
                        std::basic_string<char_t> const m(sout.str());
                        get()->_handler->message(m);
                        throw std::runtime_error(string_util::utf8::to_string(m));
                    }
                    else {
                        get()->_handler->message(sout.str());
                    }
                }


            public:

                // Change the handler. 
                // If this is the first thing you call, it will never create the default handler, makes it a little faster.
                static inline void set_handler(std::shared_ptr<message_handler> handler_) {
                    get(handler_);
                }

                // get the current handler, useful for stacking up handler overrides
                static inline std::shared_ptr<message_handler> current_handler() {
                    return get()->_handler;
                }

                // configure the behaviour of messages emission
                static inline void configure(
                    bool emit_messages_ = true,
                    bool emit_warnings_ = true,
                    bool emit_errors_ = true,
                    bool throw_errors_ = true, 
                    bool long_form_ = true,
                    bool include_timestamp_ = true) {
                    get()->_policy.configure(
                        emit_messages_, 
                        emit_warnings_, 
                        emit_errors_, 
                        throw_errors_, 
                        long_form_,
                        include_timestamp_);
                }
                static inline void set_emit_messages(bool v = true) { get()->_policy.set_emit_messages(v); }
                static inline void set_emit_warnings(bool v = true) { get()->_policy.set_emit_warnings(v); }
                static inline void set_emit_errors(bool v = true) { get()->_policy.set_emit_errors(v); }
                static inline void set_throw_errors(bool v = true) { get()->_policy.set_throw_errors(v); }
                static inline void set_long_form(bool v = true) { get()->_policy.set_long_form(v); }
                static inline void set_include_timestamp(bool v = true) { get()->_policy.set_include_timestamp(v);}

                static inline messages_policy const& current_policy() { return get()->_policy; }
                static inline void set_policy(messages_policy const& policy_) { get()->_policy = policy_;}

                static inline bool emit_messages() { return get()->_policy.emit_messages(); }
                static inline bool emit_warnings() { return get()->_policy.emit_warnings(); }
                static inline bool emit_errors() { return get()->_policy.emit_errors(); }
                static inline bool throw_errors() { return get()->_policy.throw_errors(); }
                static inline bool long_form() { return get()->_policy.long_form(); }
                static inline bool include_timestamp() { return get()->_policy.include_timestamp(); }

                // Resets to default message handler. 
                static inline void reset_handler() {
                    get(std::shared_ptr<message_handler>{ new default_message_handler() });
                }

                // Disables logging
                static inline void disable() {
                    get()->_handler = nullptr;
                }

                template<typename char_t, typename... Targs>
                static void message_(
                    const char_t* const condition, // which condition failed (nullptr if you don't care)
                    const char_t* const file, // which file this happened in (nullptr if you don't care)
                    int line, // which line this happened at (ignored if file is nullptr)
                    const char_t* m = nullptr, // main body of the message
                    Targs... Fargs) // parameters for interpolation of m
                {
                    using string_t = std::basic_string<char_t>;
                    using traits_t = messages_traits<string_t>;
                    
                    if (!get()->_policy.emit_messages()) return;
                    emit(
                        false,
                        traits_t::message_prefix,
                        condition, file, line, m,
                        Fargs ...);
                }

                template<typename char_t, typename... Targs>
                static void warning_(
                    const char_t* const condition, // which condition failed (nullptr if you don't care)
                    const char_t* const file, // which file this happened in (nullptr if you don't care)
                    int line, // which line this happened at (ignored if file is nullptr)
                    const char_t* m = nullptr, // main body of the message
                    Targs... Fargs) // parameters for interpolation of m
                {
                    using string_t = std::basic_string<char_t>;
                    using traits_t = messages_traits<string_t>;

                    if (!get()->_policy.emit_warnings()) return;
                    emit(
                        false,
                        traits_t::warning_prefix,
                        condition, file, line, m,
                        Fargs ...);
                }

                template<typename char_t, typename... Targs>
                static void error_(
                    const char_t* const condition, // which condition failed (nullptr if you don't care)
                    const char_t* const file, // which file this happened in (nullptr if you don't care)
                    int line, // which line this happened at (ignored if file is nullptr)
                    const char_t* m = nullptr, // main body of the message
                    Targs... Fargs) // parameters for interpolation of m
                {
                    using string_t = std::basic_string<char_t>;
                    using traits_t = messages_traits<string_t>;

                    if (!get()->_policy.emit_errors()) return;
                    emit(
                        get()->_policy.throw_errors(),
                        traits_t::error_prefix,
                        condition, file, line, m,
                        Fargs ...);
                }

                template<typename char_t, typename... Targs>
                static void error_text_(
                    const char_t* const condition, // which condition failed (nullptr if you don't care)
                    const char_t* const file, // which file this happened in (nullptr if you don't care)
                    int line, // which line this happened at (ignored if file is nullptr)
                    const char_t* m = nullptr, // main body of the message
                    Targs... Fargs) // parameters for interpolation of m
                {
                    using string_t = std::basic_string<char_t>;
                    using traits_t = messages_traits<string_t>;

                    if (!get()->_policy.emit_errors()) return;
                    emit(
                        false,
                        traits_t::error_prefix,
                        condition, file, line, m,
                        Fargs ...);
                }
            };
        }

        // Singleton manager for handling messaging. 
        struct messages : public diagnostic_messages_impl::messages_impl{
            
            template<typename char_t, typename... Targs>
            static inline void message_ex_assert(
                const char_t* const condition, // which condition failed (nullptr if you don't care)
                const char_t* const file, // which file this happened in (nullptr if you don't care)
                int line, // which line this happened at (ignored if file is nullptr)
                const char_t* m = nullptr, // main body of the message
                Targs... Fargs) // parameters for interpolation of m
            {
                message_<char_t, Targs...>(condition, file, line, m, Fargs ...);
            }

            template<typename char_t, typename... Targs>
            static inline void message_assert(
                const char_t* const condition, // which condition failed (nullptr if you don't care)
                const char_t* m = nullptr, // main body of the message
                Targs... Fargs) // parameters for interpolation of m
            {
                message_<char_t, Targs...>(condition, nullptr, 0, m, Fargs ...);
            }
            template<typename char_t, typename... Targs>
            static inline void message_ex(
                const char_t* const file, // which file this happened in (nullptr if you don't care)
                int line, // which line this happened at (ignored if file is nullptr)
                const char_t* m = nullptr, // main body of the message
                Targs... Fargs) // parameters for interpolation of m
            {
                message_<char_t, Targs...>(nullptr, file, line, m, Fargs ...);
            }

            template<typename char_t, typename... Targs>
            static inline void message(
                const char_t* m, // main body of the message
                Targs... Fargs) // parameters for interpolation of m
            {
                message_<char_t, Targs...>(nullptr, nullptr, 0,m,Fargs ...);
            }


            template<typename string_t, typename... Targs>
            static void message(
                string_t const& m, // main body of the message
                Targs... Fargs) // parameters for interpolation of m
            {
                message(m.c_str(), Fargs...);
            }

            template<typename char_t, typename... Targs>
            static inline void warning_ex_assert(
                const char_t* const condition, // which condition failed (nullptr if you don't care)
                const char_t* const file, // which file this happened in (nullptr if you don't care)
                int line, // which line this happened at (ignored if file is nullptr)
                const char_t* m = nullptr, // main body of the message
                Targs... Fargs) // parameters for interpolation of m
            {
                warning_<char_t, Targs...>(condition, file, line, m, Fargs ...);
            }
            template<typename char_t, typename... Targs>
            static inline void warning_assert(
                const char_t* const condition, // which condition failed (nullptr if you don't care)
                const char_t* m = nullptr, // main body of the message
                Targs... Fargs) // parameters for interpolation of m
            {
                warning_<char_t, Targs...>(condition, nullptr, 0, m, Fargs ...);
            }
            template<typename char_t, typename... Targs>
            static inline void warning_ex(
                const char_t* const file, // which file this happened in (nullptr if you don't care)
                int line, // which line this happened at (ignored if file is nullptr)
                const char_t* m = nullptr, // main body of the message
                Targs... Fargs) // parameters for interpolation of m
            {
                warning_<char_t, Targs...>(nullptr, file, line, m, Fargs ...);
            }
            template<typename char_t, typename... Targs>
            static inline void warning(
                const char_t* m, // main body of the message
                Targs... Fargs) // parameters for interpolation of m
            {
                warning_<char_t, Targs...>(nullptr, nullptr, 0,m, Fargs ...);
            }

            template<typename string_t, typename... Targs>
            static inline void warning(
                string_t const& m, // main body of the message
                Targs... Fargs) // parameters for interpolation of m
            {
                warning(m.c_str(), Fargs...);
            }

            template<typename char_t, typename... Targs>
            static inline void error_ex_assert(
                const char_t* const condition, // which condition failed (nullptr if you don't care)
                const char_t* const file, // which file this happened in (nullptr if you don't care)
                int line, // which line this happened at (ignored if file is nullptr)
                const char_t* m = nullptr, // main body of the message
                Targs... Fargs) // parameters for interpolation of m
            {
                error_<char_t, Targs...>(condition, file, line, m, Fargs ...);
            }
            template<typename char_t, typename... Targs>
            static inline void error_assert(
                const char_t* const condition, // which condition failed (nullptr if you don't care)
                const char_t* m = nullptr, // main body of the message
                Targs... Fargs) // parameters for interpolation of m
            {
                error_<char_t, Targs...>(condition, nullptr, 0, m, Fargs ...);
            }
            template<typename char_t, typename... Targs>
            static inline void error_ex(
                const char_t* const file, // which file this happened in (nullptr if you don't care)
                int line, // which line this happened at (ignored if file is nullptr)
                const char_t* m = nullptr, // main body of the message
                Targs... Fargs) // parameters for interpolation of m
            {
                error_<char_t, Targs...>(nullptr, file, line, m, Fargs ...);
            }

            template<typename char_t, typename... Targs>
            static inline void error(
                const char_t* m , // main body of the message
                Targs... Fargs) // parameters for interpolation of m
            {
                error_<char_t, Targs...>(nullptr, nullptr, 0, m, Fargs ...);
            }

            template<typename string_t, typename... Targs>
            static inline void error(
                string_t const& m, // main body of the message
                Targs... Fargs) // parameters for interpolation of m
            {
                error(m.c_str(), Fargs...);
            }
                
            template<typename char_t, typename... Targs>
            static inline void error_text_ex_assert(
                const char_t* const condition, // which condition failed (nullptr if you don't care)
                const char_t* const file, // which file this happened in (nullptr if you don't care)
                int line, // which line this happened at (ignored if file is nullptr)
                const char_t* m = nullptr, // main body of the message
                Targs... Fargs) // parameters for interpolation of m
            {
                error_text_<char_t, Targs...>(condition, file, line, m, Fargs ...);
            }
            template<typename char_t, typename... Targs>
            static inline void error_text_assert(
                const char_t* const condition, // which condition failed (nullptr if you don't care)
                const char_t* m = nullptr, // main body of the message
                Targs... Fargs) // parameters for interpolation of m
            {
                error_text_<char_t, Targs...>(condition, nullptr, 0, m, Fargs ...);
            }
            template<typename char_t, typename... Targs>
            static inline void error_text_ex(
                const char_t* const file, // which file this happened in (nullptr if you don't care)
                int line, // which line this happened at (ignored if file is nullptr)
                const char_t* m = nullptr, // main body of the message
                Targs... Fargs) // parameters for interpolation of m
            {
                error_text_<char_t, Targs...>(nullptr, file, line, m, Fargs ...);
            }

            template<typename char_t, typename... Targs>
            static inline void error_text(
                const char_t* m, // main body of the message
                Targs... Fargs) // parameters for interpolation of m
            {
                error_text_<char_t, Targs...>(nullptr, nullptr, 0, m, Fargs ...);
            }

            template<typename string_t, typename... Targs>
            static inline void error_text(
                string_t const& m, // main body of the message
                Targs... Fargs) // parameters for interpolation of m
            {
                error_text(m.c_str(), Fargs...);
            }
        };

        // helpers to print formatted stuff to currently configured handler
        struct print{
        private:
            // print a title
            template<typename string_t>
            static void title_(string_t const& title_) {
                using traits_t = messages_traits<string_t>;
                using stringstream_t = typename traits_t::stringstream_t;
                auto constexpr nl = traits_t::newline;
                auto constexpr bl = traits_t::blank;
                size_t l = 6 + title_.size();
                string_t line(l, traits_t::decoration);
                stringstream_t out;
                out << nl << line << nl << 
                    traits_t::decoration << bl << bl << title_ << bl << bl << traits_t::decoration <<
                    nl << line << nl << std::endl;
                messages::message(out.str());
            }
            // print a title with prefix, it's handy
            template<typename string_t>
            static void title_(string_t const& prefix, string_t const& title) {
                using traits_t = messages_traits<string_t>;
                title_(prefix + traits_t::blank + title);
            }

            // print a line of a given length
            template<typename string_t>
            static void line_(size_t length = 120) {
                using traits_t = messages_traits<string_t>;
                using stringstream_t = typename traits_t::stringstream_t;

                auto constexpr nl = traits_t::newline;
                string_t line(length, traits_t::decoration);
                stringstream_t out;
                out << nl << line << nl << std::endl;
                messages::message(out.str());
            }

            // print an empty line
            template<typename string_t>
            static void newline_() {
                using traits_t = messages_traits<string_t>;
                messages::message(traits_t::newline);
            }

            // print an underlined string
            template<typename string_t>
            static void underline_(string_t const& title_, bool newline = false) {
                using traits_t = messages_traits<string_t>;
                using stringstream_t = typename traits_t::stringstream_t;

                auto constexpr nl = traits_t::newline;
                auto constexpr ul = traits_t::underline;
                auto constexpr bl = traits_t::blank;
                auto constexpr emp = traits_t::empty;
                size_t l = 2 + title_.size();
                string_t line(l, ul);
                stringstream_t out;
                out << (newline ? nl : emp) << bl << title_ << bl << nl << line << std::endl;
                messages::message(out.str());
            }
        public:
            static inline void title(std::string const& t) {
                title_(t);
            }
            static inline void title(std::wstring const& t) {
                title_(t);
            }
            static inline void title(std::string const& prefix_, std::string const& t) {
                title_(prefix_, t);
            }
            static inline void title(std::wstring const& prefix_, std::wstring const& t) {
                title_(prefix_, t);
            }
            static inline void line(size_t length = 120) {
                line_<std::string>(length);
            }
            static inline void line(char unused, size_t length) {
                line_<std::string>(length);
            }
            static inline void line(wchar_t unused, size_t length) {
                line_<std::wstring>(length);
            }
            static inline void wline(size_t length = 120) {
                line_<std::wstring>(length);
            }
            static inline void newline() {
                newline_<std::string>();
            }
            static inline void newline(char unused) {
                newline_<std::string>();
            }
            static inline void newline(wchar_t unused) {
                newline_<std::wstring>();
            }
            static inline void wnewline() {
                newline_<std::wstring>();
            }

            static inline void underline(std::string const& t, bool newline = false) {
                underline_(t, newline);
            }
            static inline void underline(std::wstring const& t, bool newline = false) {
                underline_(t, newline);
            }

        };

        // scoped message handler swaps a given message handler 
        // for the current one (and then back) within a scope
        struct scoped_message_handler{
            messages_policy _previous_policy;
            std::shared_ptr<message_handler> _previous_handler;

            scoped_message_handler(std::shared_ptr<message_handler> handler_) :
                    _previous_policy(messages::current_policy()),
                    _previous_handler(messages::current_handler()){
                messages::configure();
                messages::set_handler(handler_);
            }

            ~scoped_message_handler() {
                messages::set_handler(_previous_handler);
                messages::set_policy(_previous_policy);
            }
        };

        template<typename HandlerT>
        scoped_message_handler make_scoped_handler(std::shared_ptr<HandlerT> handler_) {
            std::shared_ptr<message_handler> mhandler(std::dynamic_pointer_cast<message_handler>(handler_));
            return scoped_message_handler(mhandler);
        }
        

        // string message handler, logs to a string
        template<typename string_t>
        class string_message_handler_ : public message_handler {
            using traits_t = messages_traits<string_t>;
            using stringstream_t = typename traits_t::stringstream_t;

            stringstream_t _out;
            string_t& _out_string;
            void to_string() { _out_string = _out.str(); }
        public:
            string_message_handler_(string_t& out_string) : 
                    _out_string(out_string) {}

            ~string_message_handler_() {
                 to_string();
            }
            inline void message_(string_t const& m){
                _out << m << std::endl;
            }

            void update() {
                to_string();
            }
            string_t str() { 
                to_string();
                return _out_string; 
            }
            void clear() { _out.str(traits_t::blank); }

        };
        class string_message_handler : public string_message_handler_<std::string> {
        public:
            using base_t = string_message_handler_<std::string>;
            string_message_handler(std::string& out_string) :base_t(out_string) {}
            
            void message(std::string const& m) override {
                message_(m);
            }
            void message(std::wstring const& m) override {
                throw std::runtime_error("Writing wstrings with string_message_handler is not supported.");
            }
        };
        class wstring_message_handler : public string_message_handler_<std::wstring> {
        public:
            using base_t = string_message_handler_<std::wstring>;
            wstring_message_handler(std::wstring& out_string) :base_t(out_string) {}
            
            void message(std::string const& m) override {
                throw std::runtime_error("Writing strings with wstring_message_handler is not supported.");
            }
            void message(std::wstring const& m) override {
                message_(m);
            }
        };

        static inline std::shared_ptr<message_handler> make_string_message_handler(std::string& out_string) {
            std::shared_ptr<message_handler> handler(new string_message_handler(out_string));
            return handler;
        }
        static inline std::shared_ptr<message_handler> make_string_message_handler(std::wstring& out_string) {
            std::shared_ptr<message_handler> handler(new wstring_message_handler(out_string));
            return handler;
        }
        template<typename string_t>
        static inline std::shared_ptr<message_handler> make_string_message_handler_active(string_t& out_string) {
            auto handler = make_string_message_handler(out_string);
            messages::set_handler(handler);
            return handler;
        }
        template<typename string_t>
        static inline scoped_message_handler make_scoped_string_message_handler(string_t& out_string) {
            return make_scoped_handler(make_string_message_handler(out_string));
        }

        // file message handler, logs to a file
        template<typename string_t = std::string>
        class file_message_handler_ : public message_handler {
            using traits_t = messages_traits<string_t>;
            using stringstream_t = typename traits_t::stringstream_t;
            using fstream_t = typename traits_t::fstream_t;

            fstream_t _out;

            string_t const _file_name;
        public:
            file_message_handler_(string_t const& file_name) : _file_name(file_name) {
                using namespace string_util;
                try {
                    _out.open(utf8::to_string(_file_name));
                }
                catch (...) {
                    print::underline(traits_t::failed_to_open_file + _file_name);
                    throw;
                }
                if (!_out.is_open()) {
                    string_t error = traits_t::failed_to_open_file + _file_name;
                    print::underline(error);
                    throw std::runtime_error(string_util::utf8::to_string(error));
                }
                print::underline(traits_t::tracing_to_file + _file_name, true);
            }
            ~file_message_handler_() {
                close();
            }
            inline void message_(string_t const& m) {
                _out << m << std::endl;
            }
            void close() {
                _out.close();
            }
        };
        class file_message_handler : public file_message_handler_<std::string> {
        public:
            using base_t = file_message_handler_<std::string>;
            file_message_handler(std::string const& file_name) :base_t(file_name) {}

            void message(std::string const& m) override {
                message_(m);
            }
            void message(std::wstring const& m) override {
                throw std::runtime_error("Writing wstrings with file_message_handler is not supported.");
            }
        };
        class wfile_message_handler : public file_message_handler_<std::wstring> {
        public:
            using base_t = file_message_handler_<std::wstring>;
            wfile_message_handler(std::wstring const& file_name) :base_t(file_name) {}

            void message(std::string const& m) override {
                throw std::runtime_error("Writing strings with wfile_message_handler is not supported.");
            }
            void message(std::wstring const& m) override {
                message_(m);
            }
        };

        static inline std::shared_ptr<message_handler> make_file_message_handler(std::string const& file_name) {
            std::shared_ptr<message_handler> handler(new file_message_handler(file_name));
            return handler;
        }
        static inline std::shared_ptr<message_handler> make_file_message_handler(std::wstring const& file_name) {
            std::shared_ptr<message_handler> handler(new wfile_message_handler(file_name));
            return handler;
        }
        static inline std::shared_ptr<message_handler> make_file_message_handler_active(std::string const& file_name) {
            auto handler = make_file_message_handler(file_name);
            messages::set_handler(handler);
            return handler;
        }
        static inline std::shared_ptr<message_handler> make_file_message_handler_active(std::wstring const& file_name) {
            auto handler = make_file_message_handler(file_name);
            messages::set_handler(handler);
            return handler;
        }
        static inline scoped_message_handler make_scoped_file_message_handler(std::string const& file_name) {
            return make_scoped_handler(make_file_message_handler(file_name));
        }
        static scoped_message_handler make_scoped_file_message_handler(std::wstring const& file_name) {
            return make_scoped_handler(make_file_message_handler(file_name));
        }


        // error_collector, collects errors in one place, ignores everything else
        // useful for running test strings
        // WARNING: it disables all other messages as soon as it comes into scope
        // on exit, it prints all collected errors to whatever handler was in place befor it was created
        template<class string_t>
        class error_collector_ : public message_handler {
            using traits_t = messages_traits<string_t>;

            std::vector<string_t>& _errors;
            std::shared_ptr<message_handler> _previous_handler;
            messages_policy _previous_policy;
        public:
            error_collector_(std::vector<string_t>& errors_) :
                _errors(errors_),
                _previous_handler(messages::current_handler()),
                _previous_policy(messages::current_policy()) {
                messages::configure(false, false, true, false);
            }
            ~error_collector_() {
                deactivate();
                messages::set_policy(_previous_policy);
            }
            void dump() const {
                messages::set_policy(_previous_policy);
                for (auto const& error : _errors)
                    _previous_handler->message(error);
                messages::configure(false, false, true, false);
            }
            void deactivate() {
                messages::set_policy(_previous_policy);
                messages::set_handler(_previous_handler);
            }
            void clear() {
                _errors.clear();
            }
            std::vector<string_t> const& errors() const {
                return _errors;
            }
            void message_(string_t const& m){
                _errors.push_back(m);
            }
        };
        class error_collector : public error_collector_<std::string> {
        public:
            using base_t = error_collector_<std::string>;
            error_collector(std::vector<std::string>& errors_) :base_t(errors_) {}

            void message(std::string const& m) override {
                message_(m);
            }
            void message(std::wstring const& m) override {
                throw std::runtime_error("Writing wstrings with error_collector is not supported.");
            }
        };
        class werror_collector : public error_collector_<std::wstring> {
        public:
            using base_t = error_collector_<std::wstring>;
            werror_collector(std::vector<std::wstring>& errors_) :base_t(errors_) {}

            void message(std::string const& m) override {
                throw std::runtime_error("Writing strings with error_collector is not supported.");
            }
            void message(std::wstring const& m) override {
                message_(m);
            }
        };
        static inline std::shared_ptr<message_handler> make_error_collector(std::vector<std::string>& errors_) {
            std::shared_ptr<message_handler> handler(new error_collector(errors_));
            return handler;
        }
        static inline std::shared_ptr<message_handler> make_error_collector(std::vector<std::wstring>& errors_) {
            std::shared_ptr<message_handler> handler(new werror_collector(errors_));
            return handler;
        }
        template<typename string_t>
        static inline std::shared_ptr<message_handler> make_error_collector_active(std::vector<string_t>& errors_) {
            auto handler = make_error_collector(errors_);
            messages::set_handler(handler); 
            return handler;
        }
        template<typename string_t>
        static inline scoped_message_handler make_scoped_error_collector(std::vector<string_t>& errors_) {
            return make_scoped_handler(make_error_collector(errors_));
        }


        // scoped disabling of timestamps in logs, used for recording
        class timestamp_disabler {
        public:
            timestamp_disabler() {
                messages::set_include_timestamp(false);
            }
            ~timestamp_disabler() {
                messages::set_include_timestamp(true);
            }
        };
	}
}