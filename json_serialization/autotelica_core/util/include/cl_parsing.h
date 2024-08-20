#pragma once
#include <functional>
#include <vector>
#include <string>
#include <sstream>
#include "asserts.h"
#include "string_util.h"
#include "std_pretty_printing.h"

namespace autotelica {
	namespace cl_parsing {

        class cl_command {
        public:
            using command_function_t = std::function<void(std::vector<std::string> const&)>;
        private:

            // an individual command is specified by a some strings that identify it
            // and a function to execute it
            // arguments to the command get collected from the command line and are built of 
            // all the comman line arguments between the command string and 
            // the next command string or the end of arguments list
            const std::string _name;
            const std::string _help;
            const std::vector<std::string> _command_strings;
            const size_t _no_of_arguments;
            const command_function_t _command_function;


            friend class cl_command_executor;
        public:
            static constexpr const char command_prefix() { return '-'; }
            static void flag(std::vector<std::string> const&) {}
            // cl_command_executor is an instance of the cl_command
            // created once the arguments for it are parsed
            class cl_command_executor : public std::function<void()> {
                cl_command const* const _cl_command;
                const std::vector<std::string> _arguments;

                cl_command_executor(
                    cl_command const* const cl_command_,
                    std::vector<std::string> const arguments_) :
                    _cl_command(cl_command_),
                    _arguments(arguments_) {}
                friend class cl_command;
                static std::shared_ptr<cl_command_executor>
                    create(cl_command const* const cl_command_,
                        std::vector<std::string> const arguments_) {

                    AF_ASSERT(
                        (cl_command_->_no_of_arguments == -1) ||
                        (cl_command_->_no_of_arguments == arguments_.size()),
                        "Wrong number of arguments for command %, expected % and received %",
                        cl_command_->_name, cl_command_->_no_of_arguments, arguments_.size());

                    return std::shared_ptr<cl_command_executor>(new cl_command_executor(cl_command_, arguments_));
                }
            public:
                std::string const& name() const { return _cl_command->_name; }
                std::vector<std::string> const& arguments() const { return _arguments; }
                bool is_flag() const {
                    return cl_command::flag == _cl_command->_command_function.target<decltype(cl_command::flag)>();
                };
                void execute() const { _cl_command->_command_function(_arguments); }
                bool has_command_string(std::string const& s) const {
                    using namespace string_util;
                    return std::find_if(
                        _cl_command->_command_strings.begin(), 
                        _cl_command->_command_strings.end(),
                        [&](std::string const& cs) { return equal_nc(s, cs); }
                    ) != _cl_command->_command_strings.end();
                }

            };


            cl_command(
                std::string const& name_,
                std::string const& help_,
                std::vector<std::string> const& command_strings_,
                size_t no_of_arguments_,
                const command_function_t command_function_ = flag
            ) : _name(name_),
                _help(help_),
                _command_strings(command_strings_),
                _no_of_arguments(no_of_arguments_),
                _command_function(command_function_) {

            }

            std::string const& name() const { return _name; }
            std::string const& help() const { return _help; }
            size_t no_of_arguments() const { return _no_of_arguments; }
            std::vector<std::string> const& command_strings() const { return _command_strings; }

            const bool recognise(std::string const& command_string) {
                for (auto const& cs : _command_strings)
                    if (autotelica::string_util::equal_nc(command_string, cs))
                        return true;
                return false;
            }

            std::string formatted_help() const {
                using namespace autotelica::string_util;
                using namespace autotelica::std_pretty_printing;

                std::stringstream sout;
                std::vector<std::string> args;
                args.reserve(_command_strings.size());
                for (auto const& cs : _command_strings)
                    args.push_back(af_format_string("%%", cl_command::command_prefix(), cs));
                var_printf(sout, "% (%)\n\t%\n", _name, pretty_s(args), _help);
                return sout.str();
            }

            std::shared_ptr<cl_command_executor> create_executor(std::vector<std::string> const& arguments_) {
                return cl_command_executor::create(this, arguments_);
            }
        };

        class cl_commands {
        public:
            using command_function_t = cl_command::command_function_t;
            using cl_command_executor = cl_command::cl_command_executor;
        private:
            std::string _app_help;
            std::string _app_name;
            std::vector<std::string> _loose_arguments;
            std::vector<std::shared_ptr<cl_command>> _commands;
            std::vector<std::shared_ptr<cl_command_executor>> _executors;

            std::vector<std::shared_ptr<cl_command>>::const_iterator find_command(std::string const& name) const {
                using namespace autotelica::string_util;
                return std::find_if(_commands.begin(), _commands.end(),
                    [&](std::shared_ptr<cl_command> const& c) {
                        return equal_nc(name, c->name());
                    });
            }
            std::vector<std::shared_ptr<cl_command_executor>>::const_iterator find_executor(std::string const& name) const {
                using namespace autotelica::string_util;
                auto it = std::find_if(_executors.begin(), _executors.end(), 
                    [&](std::shared_ptr<cl_command_executor> const& c) {
                        return equal_nc(name, c->name());
                    });
                if (it == _executors.end()) {
                    it = std::find_if(_executors.begin(), _executors.end(),
                        [&](std::shared_ptr<cl_command_executor> const& c) {
                            return c->has_command_string(name);
                        });
                }
                return it;
            }

            void add_executor(std::shared_ptr<cl_command_executor> executor) {
                AF_ASSERT(find_executor(executor->name()) == _executors.end(),
                    "Command % already passed in as an argument.", executor->name());
                _executors.push_back(executor);
            }
            // parsing is simple:
            // 1. skip first argument (it's the app name)
            // 2. for each argument:
            //      if is starts with a command_prefix:
            //          start parsing command
            //      otherwise:
            //          start  parsing argument
            //          store in loose arguments
            // 3. parsing an argument:
            //      if is starts with a quote, trim quotes front and back
            // 
            // 4. parsing a command:
            //      skip all command_prefix characters (you can do - or -- or --- ... )
            //      parse till end, that's the command string
            //      find command in the registry (_commands)
            //      find out how many arguments the command can take
            //          if it is -1, keep collecting arguments till next command or the end
            //          otherwise, collect the needed number of arguments (if there is more or less, moan)
            //      create an executor
            bool is_command(const char* const arg) {
                return arg && strlen(arg) > 1 && arg[0] == cl_command::command_prefix();
            }
            std::string parse_argument(const char* const arg) {
                std::string out(arg);
                autotelica::string_util::trim_s(out, '"');
                return out;
            }
            std::vector<std::string> get_arguments(int count, int& current_arg, int argc, const char* argv[]) {
                std::vector<std::string> out;
                if (count == 0) return out;
                if (count != -1) {
                    for (int i = 0; i < count && current_arg < argc && !is_command(argv[current_arg]); ++current_arg, ++i)
                        out.push_back(parse_argument(argv[current_arg]));
                    AF_ASSERT(out.size() == count, "Not enough arguments supplied.");
                }
                else {
                    for (; current_arg < argc && !is_command(argv[current_arg]); ++current_arg)
                        out.push_back(parse_argument(argv[current_arg]));
                }
                return out;
            }
            std::shared_ptr<cl_command_executor> parse_command(int& current_arg, int argc, const char* argv[]) {
                using namespace autotelica::string_util;
                if (current_arg > argc) return nullptr;
                if (!is_command(argv[current_arg])) return nullptr;
                std::string command_s(argv[current_arg]);
                ltrim_s(command_s, cl_command::command_prefix());
                ltrim_s(command_s);
                std::shared_ptr<cl_command> command;
                for (auto const& c : _commands)
                    if (c->recognise(command_s)) {
                        command = c;
                        break;
                    }
                AF_ASSERT(command, "Unrecognised command %.", command_s);
                ++current_arg;
                std::vector<std::string> arguments = get_arguments(
                    static_cast<int>(command->no_of_arguments()),
                    current_arg, argc, argv);
                return command->create_executor(arguments);
            }

            void parse(int argc, const char* argv[]) {
                int current_arg = 1;// skip the app name
                while (current_arg < argc) {
                    auto command = parse_command(current_arg, argc, argv);
                    if (command)
                        add_executor(command);
                    else {
                        std::string loose_arg = parse_argument(argv[current_arg]);
                        ++current_arg;
                        _loose_arguments.push_back(loose_arg);
                    }
                }
            }

        public:
            cl_commands(std::string const& app_help_ = "") : _app_help(app_help_) {
                // help is always there
                register_command("help", "Helps.", { "help", "?" }, 0, [&](std::vector<std::string> const&) {this->help(); });
            }
            std::vector<std::shared_ptr<cl_command_executor>> const& executors() const {
                return _executors;
            }

            bool has(std::string const& name) const {
                return find_executor(name) != _executors.end();
            }

            std::vector<std::string> const& arguments(std::string const& name = "") {
                if (name.empty()) return _loose_arguments;
                auto const it = find_executor(name);
                AF_ASSERT(it != _executors.end(), "Command % was not passed on the command line.", name);
                return (*it)->arguments();

            }

            cl_commands& register_command(
                std::string const& name_,
                std::string const& help_,
                std::vector<std::string> const& command_strings_,
                size_t no_of_arguments_ = 0,
                const command_function_t command_function_ = cl_command::flag
            ) {
                AF_ASSERT(find_command(name_) == _commands.end(), "Command % is already registered.", name_);
                for (auto const& c : _commands) {
                    for (auto const& cs : command_strings_)
                        AF_ASSERT(!c->recognise(cs), "Command with command string % is already registered (%).", cs, c->name());

                }
                std::shared_ptr<cl_command> command(new cl_command{ name_, help_, command_strings_, no_of_arguments_, command_function_ });
                _commands.emplace_back(command);
                return *this;
            }
            void execute() const {
                for (auto executor : _executors)
                    executor->execute();
            }

            void clear() {
                _executors.clear();
                _loose_arguments.clear();
                _app_name = "";
            }
            void help() {
                using namespace autotelica::string_util;
                using namespace autotelica::diagnostic_messages;

                std::stringstream sout;
                var_printf(sout, "%\n\n", _app_name);
                if (!_app_help.empty())
                    var_printf(sout, "\n%\n", _app_help);
                for (auto const& command : _commands) {
                    var_printf(sout, "%\n", command->formatted_help());
                }
                timestamp_disabler t;
                messages::message(sout.str());
            }

            void parse_command_line(int argc, const char* argv[]) {
                _app_name = argv[0];
                parse(argc, argv);
            }
        };

	}
}