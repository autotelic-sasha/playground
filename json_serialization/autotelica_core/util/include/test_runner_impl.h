#include "testing_util.h"
#include "cl_parsing.h"
#include "diagnostic_messages.h"

namespace autotelica {

// -l, -list_examples
// -re, -run_examples with no arguments runs them all, with arguments runs listed ones
// -rt, -run_tests with no arguments runs them all, with arguments runs listed ones
// -record with no arguments runs them all, with arguments runs listed ones
// -f filename outputs to file
// -csv outputs in csv format
// -excel outputs in excel csv format
// -no_timestamps
// 
// -messages    trace messages
// -warnings    trace warnings
// -error_text  trace errors as text only
// -errors      trace errors and throw exceptions (in csv modes no exceptions are thrown on test errors)
// -all         trace everything (default behaviour)
class test_runner_impl{
    static void list_examples(std::vector<std::string> const& v) {
        AF_LIST_TEST_SETS();
    }
    static void run_examples(std::vector<std::string> const& v) {
        if (v.empty()) {
            AF_RUN_ALL_EXAMPLES();
        }
        else {
            AF_RUN_EXAMPLES(v);
        }
    }
    static void run_tests(std::vector<std::string> const& v) {
        if (v.empty()) {
            AF_RUN_ALL_TESTS();
        }
        else {
            AF_RUN_TESTS(v);
        }
    }
    static void record(std::vector<std::string> const& v) {
        if (v.empty())
            AF_RECORD_ALL_TESTS();
        else
            AF_RECORD_TESTS(v);
    }

    static autotelica::cl_parsing::cl_commands register_commands(){
        using namespace autotelica::cl_parsing;
        cl_commands commands("Runs tests or examples, records them, and such.");
        commands
            .register_command(
                "List examples",
                "Lists all available examples.",
                { "l", "list_examples", "list-examples" },
                0,
                list_examples)
            .register_command(
                "Run examples",
                "Runs named examples (if none are named, all are run).",
                { "re", "run_examples", "run-examples" },
                -1,
                run_examples)
            .register_command(
                "Run tests",
                "Runs named tests (if none are named, all are run).",
                { "rt", "run_tests", "run-tests" },
                -1,
                run_tests)
            .register_command(
                "Record",
                "Records named tests (if none are named, all are recorded).",
                { "record" },
                -1,
                run_tests)
            .register_command(
                "Save to file",
                "Redirects test outputs to a named file.",
                { "f", "output_file" },
                1)
            .register_command(
                "CSV format",
                "Output in csv format.",
                { "csv" })
            .register_command(
                "Excel CSV format",
                "Output in excel csv format.",
                { "excel" })
            .register_command(
                "No timestamps",
                "Run without tracing timestamps.",
                { "no_timestamps" })
            .register_command(
                "Messages",
                "Trace messages.",
                { "messages" })
            .register_command(
                "Warnings",
                "Trace warnings.",
                { "warnings" })
            .register_command(
                "Error text",
                "Trace errors as text (don't throw exceptions).",
                { "error_text" })
            .register_command(
                "Errors",
                "Trace errors and throw exceptions.",
                { "errors" })
            .register_command(
                "ErrorsOnly",
                "Collect all errors, and only errors.",
                { "errors_only" })
            .register_command(
                "All",
                "Trace everything.",
                { "all" });
        return commands;
    }

public:
    static int main_impl(int argc, const char* argv[])
    {
        using namespace autotelica::diagnostic_messages;
        using namespace autotelica::testing;


        auto commands = register_commands();
        commands.parse_command_line(argc, argv);
        
        if(commands.executors().empty()){
            commands.help();
            return 0;
        }
        if (commands.has("no_timestamps")) {
            messages::set_timestamp_format("");
        }
        if (commands.has("csv") || commands.has("excel")) {
            if (commands.has("excel"))
                testing_config::set_run_mode_excel_csv();
            else
                testing_config::set_run_mode_plain_csv();

        }
        std::shared_ptr<file_message_handler> file_message_handler;
        if (commands.has("output_file")) {
            std::string file_name = commands.arguments("output_file")[0];
            file_message_handler = file_message_handler::make_active(file_name);
        }

        bool trace_messages = commands.has("messages");
        bool trace_warnings = commands.has("warnings");
        bool trace_error_text = commands.has("error_text");
        bool throw_errors = commands.has("errors");
        bool errors_only = commands.has("errors_only");
        
        bool trace_all = commands.has("all") || !(trace_messages || trace_warnings || trace_error_text || throw_errors);
        std::shared_ptr<error_collector> only_errors;

        if (errors_only) {
            only_errors = error_collector::make_active();
        }
        else if (!trace_all) {
            messages::configure(trace_messages, trace_warnings, trace_error_text || throw_errors, throw_errors);
        }

        commands.execute();

        if (errors_only) {
            if (!only_errors->errors().empty()) {
                auto count = only_errors->errors().size();
                only_errors->deactivate();//killing it will make it dump errors
                messages::warning("Errors during test run. Total errors found: %.", count);
                return 1;
            }
            only_errors->deactivate();//killing it will make it dump errors
            messages::message("All tests completed with no errors.");
            return 0;
        }
        return 0;
    }
};
}
