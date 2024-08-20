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

            virtual std::string const& message_prefix() {
                static std::string p;
                return p;
            }
            virtual std::string const& warning_prefix() {
                static std::string p("WARNING: ");
                return p;
            }
            virtual std::string const& error_prefix() {
                static std::string p("ERROR: ");
                return p;
            }
        };

        // Default message handler
        // This is used when nothing else is configured.
        // Traces to stdout, throws runtime_error exceptions on error.
        struct default_message_handler : public message_handler {
            void message(std::string const& m) override {
                std::cout << std::boolalpha << m << std::endl;
            }
        };
        // Implementation details
        class messages_policy {
            bool _emit_messages;
            bool _emit_warnings;
            bool _emit_errors;
            bool _throw_errors;
            bool _long_form;
        public:
            messages_policy(
                bool emit_messages_ = true,
                bool emit_warnings_ = true,
                bool emit_errors_ = true,
                bool throw_errors_ = true,
                bool long_form_ = true
            ) : _emit_messages(emit_messages_),
                _emit_warnings(emit_warnings_),
                _emit_errors(emit_errors_),
                _throw_errors(throw_errors_),
                _long_form(long_form_){
            }

            inline void configure(
                bool emit_messages_ = true,
                bool emit_warnings_ = true,
                bool emit_errors_ = true,
                bool throw_errors_ = true,
                bool long_form_ = true
            ) {
                _emit_messages = emit_messages_;
                _emit_warnings = emit_warnings_;
                _emit_errors = emit_errors_;
                _throw_errors = throw_errors_;
                _long_form = long_form_;
            }
            inline bool emit_messages() const { return _emit_messages; }
            inline bool emit_warnings() const { return _emit_warnings; }
            inline bool emit_errors() const { return _emit_errors; }
            inline bool throw_errors() const { return _throw_errors; }
            inline bool long_form() const { return _long_form; }

            inline void set_emit_messages(bool v = true) { _emit_messages = v; }
            inline void set_emit_warnings(bool v = true) { _emit_warnings = v; }
            inline void set_emit_errors(bool v = true) { _emit_errors = v; }
            inline void set_throw_errors(bool v = true) { _throw_errors = v; }
            inline void set_long_form(bool v ) { _long_form = v; }

        };

        namespace diagnostic_messages_impl {
            // Singleton manager for handling messaging. 
            class messages_impl {
                std::shared_ptr<message_handler> _handler; // current message handler
                std::string _timestamp_format;
                messages_policy _policy;
                // Constructor creates a default handler if not given one.
                messages_impl(
                    std::shared_ptr<message_handler> handler_ = nullptr
                ) : _handler(handler_),
                    _timestamp_format("[%F %T] "){
                    if (!_handler)
                        _handler = std::shared_ptr<message_handler>{ new default_message_handler() };
                }

                // Adds information about file and line to the message.
                static void append_file_line(std::stringstream& sout, const char* const file, int line) {
                    if (!file) return;
                    sout << "(in file: " << file << "; at line " << line << ")";
                }
                // Add information about condition that failed to the message. 
                static void prepend_failed_condition_message(std::stringstream& sout, std::string const& condition) {
                    if (condition.empty()) return;
                    sout << std::boolalpha << "Assertion " << condition << " failed. ";
                }
                // Prefixes the message with the timestamp
                static void prepend_timestamp(std::stringstream& sout) {
                    if (get()->_timestamp_format.empty()) return;
                    std::time_t const time = std::time({});
                    static char timeString[100];//we'll sacrifice 100 bytes for this
                    static tm tm_buff;
#ifdef __linux__
                    gmtime_r(&time, &tm_buff);
#else
                    gmtime_s(&tm_buff, &time);
#endif
                    std::strftime(timeString, sizeof(timeString)/sizeof(char),
                        get()->_timestamp_format.c_str(), &tm_buff);
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
                // A little type trickery to make templates work more nicely.
                typedef void (messages_impl::* handler_func_t)(std::string const&) const;
                typedef std::string const& (messages_impl::* handler_prefix_func_t)() const;

                // dispatching to the current actual handler
                void message_impl(std::string const& m) const { 
                    if (_policy.emit_messages() && _handler) _handler->message(m); 
                }
                void warning_impl(std::string const& m) const { 
                    if (_policy.emit_warnings() && _handler) _handler->message(m);
                }
                void error_text_impl(std::string const& m) const {
                    if (_policy.emit_errors() && _handler) _handler->message(m);
                }
                void error_impl(std::string const& m) const {
                    error_text_impl(m);
                    if(_policy.throw_errors())
                        throw std::runtime_error(m);
                }

                static std::string const& empty_s() { static const std::string s; return s; }
                std::string const& message_prefix_impl() const { return (_handler) ? _handler->message_prefix() : empty_s(); }
                std::string const& warning_prefix_impl() const { return (_handler) ? _handler->warning_prefix() : empty_s(); }
                std::string const& error_prefix_impl() const { return (_handler) ? _handler->error_prefix() : empty_s(); }

                static bool is_whitespace(const char* const m) {
                    const char* s = m;
                    while (*s++) {
                        if (!string_util::fast_is_space(*s)) return false;
                    }
                    return true;
                }
                // Main entry point for formatting and emitting messages.
                template<typename... Targs>
                static void emit(
                    handler_func_t message_f, // which function to call (it's a pointer to member of this class, kewl)
                    handler_prefix_func_t prefix_f, // function providing prefix for the message (it's a pointer to member of this class, kewl)
                    const char* const condition, // which condition failed (nullptr if you don't care)
                    const char* const file, // which file this happened in (nullptr if you don't care)
                    int line, // which line this happened at (ignored if file is nullptr)
                    const char* m = nullptr, // main body of the message
                    Targs... Fargs) // parameters for interpolation of m
                {
                    if (!m) return;
                    bool wht = is_whitespace(m);
                    std::stringstream sout;
                    if(!wht){
						prepend_timestamp(sout);
						sout << std::boolalpha << (get().get()->*prefix_f)();
						if (get()->_policy.long_form() && condition)
							prepend_failed_condition_message(sout, condition);
					}
                    string_util::var_printf(sout, m, Fargs...);
                    if(!wht){
						if (get()->_policy.long_form())
							append_file_line(sout, file, line);
					}
                    (get().get()->*message_f)(sout.str());
                }

                // Pointers to members that dispatch messages to appropriate handler.
                // Should make it easier to invoke emit
                static constexpr handler_func_t message_function() { return &messages_impl::message_impl; }
                static constexpr handler_func_t warning_function() { return &messages_impl::warning_impl; }
                static constexpr handler_func_t error_function() { return  &messages_impl::error_impl; }
                static constexpr handler_func_t error_text_function() { return  &messages_impl::error_text_impl; }

                static constexpr handler_prefix_func_t message_prefix() { return &messages_impl::message_prefix_impl; }
                static constexpr handler_prefix_func_t warning_prefix() { return &messages_impl::warning_prefix_impl; }
                static constexpr handler_prefix_func_t error_prefix() { return &messages_impl::error_prefix_impl; }
            public:

                // Change the handler. 
                // If this is the first thing you call, it will never create the default handler, makes it a little faster.
                static void set_handler(std::shared_ptr<message_handler> handler_) {
                    get(handler_);
                }

                // get the current handler, useful for stacking up handler overrides
                static std::shared_ptr<message_handler> current_handler() {
                    return get()->_handler;
                }

                // set timestamp format to empty string to disable timestamps
                static void set_timestamp_format(std::string const& f) {
                    get()->_timestamp_format = f;
                }

                // get the current timestamp format, useful for stacking up handler overrides
                static std::string current_timestamp_format() {
                    return get()->_timestamp_format;
                }
                
                // configure the behaviour of messages emission
                static void configure(
                    bool emit_messages_ = true,
                    bool emit_warnings_ = true,
                    bool emit_errors_ = true,
                    bool throw_errors_ = true, 
                    bool long_form_ = true) {
                    get()->_policy.configure(emit_messages_, emit_warnings_, emit_errors_, throw_errors_, long_form_);
                }
                static void set_long_form(bool v = true) {
                    get()->_policy.set_long_form(v);
                }
                static messages_policy const& current_policy() {
                    return get()->_policy;
                }
                static void set_policy(messages_policy const& policy_) {
                    get()->_policy = policy_;
                }

                // other code using this may want to deal with exceptions in a special way
                static bool throw_errors() {
                    return get()->_policy.throw_errors();
                }

                // Resets to default message handler. 
                static void reset_handler() {
                    get(std::shared_ptr<message_handler>{ new default_message_handler() });
                }

                // Disables logging
                static void disable() {
                    get()->_handler = nullptr;
                }

                template<typename... Targs>
                static void message(
                    const char* const condition, // which condition failed (nullptr if you don't care)
                    const char* const file, // which file this happened in (nullptr if you don't care)
                    int line, // which line this happened at (ignored if file is nullptr)
                    const char* m = nullptr, // main body of the message
                    Targs... Fargs) // parameters for interpolation of m
                {
                    emit(
                        message_function(), message_prefix(),
                        condition, file, line, m,
                        Fargs ...);
                }

                template<typename... Targs>
                static void warning(
                    const char* const condition, // which condition failed (nullptr if you don't care)
                    const char* const file, // which file this happened in (nullptr if you don't care)
                    int line, // which line this happened at (ignored if file is nullptr)
                    const char* m = nullptr, // main body of the message
                    Targs... Fargs) // parameters for interpolation of m
                {
                    emit(
                        warning_function(), warning_prefix(),
                        condition, file, line, m,
                        Fargs ...);
                }

                template<typename... Targs>
                static void error(
                    const char* const condition, // which condition failed (nullptr if you don't care)
                    const char* const file, // which file this happened in (nullptr if you don't care)
                    int line, // which line this happened at (ignored if file is nullptr)
                    const char* m = nullptr, // main body of the message
                    Targs... Fargs) // parameters for interpolation of m
                {
                    emit(
                        error_function(), error_prefix(),
                        condition, file, line, m,
                        Fargs ...);
                }

                template<typename... Targs>
                static void error_text(
                    const char* const condition, // which condition failed (nullptr if you don't care)
                    const char* const file, // which file this happened in (nullptr if you don't care)
                    int line, // which line this happened at (ignored if file is nullptr)
                    const char* m = nullptr, // main body of the message
                    Targs... Fargs) // parameters for interpolation of m
                {
                    emit(
                        error_text_function(), error_prefix(),
                        condition, file, line, m,
                        Fargs ...);
                }
            };
        }

        // Singleton manager for handling messaging. 
        struct messages {
            // Change the handler. 
            // If this is the first thing you call, it will never create the default handler, makes it a little faster.
            static void set_handler(std::shared_ptr<message_handler> handler_) {
                diagnostic_messages_impl::messages_impl::set_handler(handler_);
            }
            
            // get the current handler, useful for stacking up handler overrides
            static std::shared_ptr<message_handler> current_handler() {
                return diagnostic_messages_impl::messages_impl::current_handler();
            }
            // configure the behaviour of messages emission
            static void configure(
                bool emit_messages_ = true,
                bool emit_warnings_ = true,
                bool emit_errors_ = true,
                bool throw_errors_ = true, 
                bool long_form_ = true) {
                diagnostic_messages_impl::messages_impl::configure(
                    emit_messages_, emit_warnings_, emit_errors_, throw_errors_, long_form_);
            }
            static void set_long_form(bool v) {
                diagnostic_messages_impl::messages_impl::set_long_form(v);
            }
            static messages_policy const& current_policy() {
                return diagnostic_messages_impl::messages_impl::current_policy();
            }
            static void set_policy(messages_policy const& policy_) {
                diagnostic_messages_impl::messages_impl::set_policy(policy_);
            }

            // other code using this may want to deal with exceptions in a special way
            static bool throw_errors() {
                return diagnostic_messages_impl::messages_impl::throw_errors();
            }

            // set timestamp format to empty string to disable timestamps
            static void set_timestamp_format(std::string const& f) {
                diagnostic_messages_impl::messages_impl::set_timestamp_format(f);
            }

            // get the current timestamp format, useful for stacking up handler overrides
            static std::string current_timestamp_format() {
                return diagnostic_messages_impl::messages_impl::current_timestamp_format();
            }

            // Resets to default message handler. 
            static void reset_handler() {
                diagnostic_messages_impl::messages_impl::reset_handler();
            }
            
            // Disables logging
            static void disable() {
                diagnostic_messages_impl::messages_impl::disable();
            }

            template<typename... Targs>
            static void message_ex(
                const char* const condition, // which condition failed (nullptr if you don't care)
                const char* const file, // which file this happened in (nullptr if you don't care)
                int line, // which line this happened at (ignored if file is nullptr)
                const char* m = nullptr, // main body of the message
                Targs... Fargs) // parameters for interpolation of m
            {
                diagnostic_messages_impl::messages_impl::message(
                    condition, file, line, m,
                    Fargs ...);
            }

            template<typename... Targs>
            static void message(
                const char* m, // main body of the message
                Targs... Fargs) // parameters for interpolation of m
            {
                diagnostic_messages_impl::messages_impl::message(
                    nullptr, nullptr, 0,
                    m,Fargs ...);
            }


            template<typename... Targs>
            static void message(
                std::string const& m, // main body of the message
                Targs... Fargs) // parameters for interpolation of m
            {
                message(m.c_str(), Fargs...);
            }

            template<typename... Targs>
            static void warning_ex(
                const char* const condition, // which condition failed (nullptr if you don't care)
                const char* const file, // which file this happened in (nullptr if you don't care)
                int line, // which line this happened at (ignored if file is nullptr)
                const char* m = nullptr, // main body of the message
                Targs... Fargs) // parameters for interpolation of m
            {
                diagnostic_messages_impl::messages_impl::warning(
                    condition, file, line, m,
                    Fargs ...);
            }

            template<typename... Targs>
            static void warning(
                const char* m, // main body of the message
                Targs... Fargs) // parameters for interpolation of m
            {
                diagnostic_messages_impl::messages_impl::warning(
                    nullptr, nullptr, 0,
                    m, Fargs ...);
            }

            template<typename... Targs>
            static void warning(
                std::string const& m, // main body of the message
                Targs... Fargs) // parameters for interpolation of m
            {
                warning(m.c_str(), Fargs...);
            }

            template<typename... Targs>
            static void error_ex(
                const char* const condition, // which condition failed (nullptr if you don't care)
                const char* const file, // which file this happened in (nullptr if you don't care)
                int line, // which line this happened at (ignored if file is nullptr)
                const char* m = nullptr, // main body of the message
                Targs... Fargs) // parameters for interpolation of m
            {
                diagnostic_messages_impl::messages_impl::error(
                    condition, file, line, m,
                    Fargs ...);
            }

            template<typename... Targs>
            static void error(
                const char* m , // main body of the message
                Targs... Fargs) // parameters for interpolation of m
            {
                diagnostic_messages_impl::messages_impl::error(
                    nullptr, nullptr, 0,
                    m, Fargs ...);
            }

            template<typename... Targs>
            static void error(
                std::string const& m, // main body of the message
                Targs... Fargs) // parameters for interpolation of m
            {
                error(m.c_str(), Fargs...);
            }
                
            template<typename... Targs>
            static void error_text_ex(
                const char* const condition, // which condition failed (nullptr if you don't care)
                const char* const file, // which file this happened in (nullptr if you don't care)
                int line, // which line this happened at (ignored if file is nullptr)
                const char* m = nullptr, // main body of the message
                Targs... Fargs) // parameters for interpolation of m
            {
                diagnostic_messages_impl::messages_impl::error_text(
                    condition, file, line, m,
                    Fargs ...);
            }

            template<typename... Targs>
            static void error_text(
                const char* m, // main body of the message
                Targs... Fargs) // parameters for interpolation of m
            {
                diagnostic_messages_impl::messages_impl::error_text(
                    nullptr, nullptr, 0,
                    m, Fargs ...);
            }

            template<typename... Targs>
            static void error_text(
                std::string const& m, // main body of the message
                Targs... Fargs) // parameters for interpolation of m
            {
                error_text(m.c_str(), Fargs...);
            }
        };

        // helpers to print formatted stuff to currently configured handler
        struct print {
            static constexpr char decoration_char() { return '*'; }
            static constexpr char underline_char() { return '='; }
            
            // print an empty line
            static void newline() {
                messages::message("");
            }

            // print a title
            static void title(std::string const& title) {
                constexpr char decoration = decoration_char();
                size_t l = 6 + title.size();
                std::string line(l, decoration);
                std::stringstream out;
                out << "\n" << line << "\n" << decoration << "  " << title << "  " << decoration << "\n" << line << "\n" << std::endl;
                messages::message(out.str());
            }
            
            // print a title
            static void title(std::string const& prefix, std::string const& title) {
                print::title(prefix + " " + title);
            }

            // print a line of a given length
            static void line(size_t length = 120) {
                constexpr char decoration = decoration_char();
                std::string line(length, decoration);
                std::stringstream out;
                out << "\n" << line << "\n" << std::endl;
                messages::message(out.str());
            }
            
            // print an underlined string
            static void underline(std::string const& title, bool newline = false) {
                constexpr char underline = underline_char();
                size_t l = 2 + title.size();
                std::string line(l, underline);
                std::stringstream out;
                out << (newline ? "\n" : "") << " " << title << " " << "\n" << line << std::endl;
                messages::message(out.str());
            }

        };


        // scoped message handler swaps a given message handler 
        // for the current one (and then back) within a scope
        struct scoped_message_handler {
            messages_policy _previous_policy;
            std::shared_ptr<message_handler> _previous_handler;
            std::shared_ptr<message_handler> _handler;


            scoped_message_handler(std::shared_ptr<message_handler> handler_) :
                _previous_policy(messages::current_policy()),
                _previous_handler(messages::current_handler()),
                _handler(handler_) {
                messages::configure();
                messages::set_handler(_handler);
            }

            ~scoped_message_handler() {
                messages::set_handler(_previous_handler);
                messages::set_policy(_previous_policy);
            }
        };

        template<typename HandlerT>
        std::shared_ptr<scoped_message_handler> make_scoped_handler(std::shared_ptr<HandlerT> handler_) {
            std::shared_ptr<message_handler> mhandler(std::dynamic_pointer_cast<message_handler>(handler_));
            return std::shared_ptr <scoped_message_handler>(new scoped_message_handler(mhandler));
        }
        

        // string message handler, logs to a string
        class string_message_handler : public message_handler {
            std::stringstream _out;
            std::string& _out_string;
            void to_string() { _out_string = _out.str(); }
        public:
            string_message_handler(
                std::string& out_string) : 
                    _out_string(out_string) {}

            ~string_message_handler() {
                 to_string();
            }
            void message(std::string const& m) override {
                _out << m << std::endl;
            }
            void update() {
                to_string();
            }
            std::string str() { 
                to_string();
                return _out_string; 
            }
            void clear() { _out.str(""); }

            static std::shared_ptr<string_message_handler> create(
                std::string& out_string) {
                std::shared_ptr<string_message_handler> handler(new string_message_handler(out_string));
                return handler;
            }

            static std::shared_ptr<string_message_handler> make_active(
                    std::string& out_string) {
                auto handler = create(out_string);
                messages::set_handler(std::dynamic_pointer_cast<message_handler>(handler));
                return handler;
            }

            static std::shared_ptr<scoped_message_handler> make_scoped(
                    std::string& out_string) {
                auto handler = create(out_string);
                return make_scoped_handler(handler);
            }
        };

        // file message handler, logs to a file
        class file_message_handler : public message_handler {
            std::ofstream _out;
            std::string const _file_name;
        public:
            file_message_handler(
                std::string const& file_name) : _file_name(file_name) {
                try {
                    _out.open(_file_name.c_str());
                }
                catch (...) {
                    print::underline("ERROR: failed to open file: " + _file_name);
                    throw;
                }
                if (!_out.is_open()) {
                    std::string error = "ERROR: failed to open file: " + _file_name;
                    print::underline(error);
                    throw std::runtime_error(error);
                }
                print::underline("Tracing to file: " + _file_name, true);
            }

            void message(std::string const& m) override {
                _out << m << std::endl;
            }
            void close() {
                _out.close();
            }

            static std::shared_ptr<file_message_handler> create(
                    std::string const& file_name) {
                std::shared_ptr<file_message_handler> handler(new file_message_handler(file_name));
                return handler;
            }

            static std::shared_ptr<file_message_handler> make_active(
                    std::string const& file_name) {
                auto handler = create(file_name);
                messages::set_handler(std::dynamic_pointer_cast<message_handler>(handler));
                return handler;
            }
            static std::shared_ptr<scoped_message_handler> make_scoped(
                    std::string const& file_name) {
                auto handler = create(file_name);
                return make_scoped_handler(handler);
            }
        };

        // error_collector, collects errors in one place, ignores everything else
        // useful for running test strings
        // WARNING: it disables all other messages as soon as it comes into scope
        // on exit, it prints all collected errors to whatever handler was in place befor it was created
        class error_collector : public message_handler {
            std::vector<std::string> _errors;
            std::shared_ptr<message_handler> _previous_handler;
        public:
            error_collector() :
                _previous_handler(messages::current_handler()) {
                messages::configure(false, false, true, false);
            }
            ~error_collector() {
                if (_previous_handler) {
                    deactivate();
                }
            }
            void dump() const {
                messages::configure();
                for (auto const& error : _errors)
                    _previous_handler->message(error);
                messages::configure(false, false, true, false);
            }
            void deactivate() {
                messages::configure();
                messages::set_handler(_previous_handler);
                for (auto const& error : _errors)
                    messages::message(error);
                _previous_handler = nullptr;
            }
            void clear() {
                _errors.clear();
            }
            std::vector<std::string> const& errors() const {
                return _errors;
            }
            void message(std::string const& m) override {
                _errors.push_back(m);
            }
            static std::shared_ptr<error_collector> create() {
                std::shared_ptr<error_collector> handler(new error_collector());
                return handler;
            }

            static std::shared_ptr<error_collector> make_active() {
                auto handler = create();
                messages::set_handler(std::dynamic_pointer_cast<message_handler>(handler));
                return handler;
            }

            static std::shared_ptr<scoped_message_handler> make_scoped() {
                auto handler = create();
                return make_scoped_handler(handler);
            }
        };


        // scoped disabling of timestamps in logs, used for recording
        class timestamp_disabler {
            std::string const _current_timestamp_format;
        public:
            timestamp_disabler() : 
                    _current_timestamp_format(
                        messages::current_timestamp_format()) {
                messages::set_timestamp_format("");
            }
            ~timestamp_disabler() {
                messages::set_timestamp_format(
                    _current_timestamp_format);
            }
        };
	}
}