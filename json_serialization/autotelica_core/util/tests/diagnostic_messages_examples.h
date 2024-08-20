#pragma once
#pragma once
#include "testing_util.h"
#include "diagnostic_messages.h"
namespace autotelica {
    namespace examples {
        namespace diagnostic_messages {
            
            void trace_some_messages() {
                // main purpose of diagnostic messages is to trace various messages
                using namespace autotelica::diagnostic_messages;

                messages::message("Some % message, with some stuff inserted %, %, %",
                    "formatted", 1, 2.5234234, false);

                messages::warning("Some % message, with some stuff inserted %, %, %",
                    "formatted", 1, 2.5234234, false);

                try {
                    messages::error("Some % message, with some stuff inserted %, %, %",
                        "formatted", 1, 2.5234234, false);
                }
                catch (std::exception& e) {
                    messages::message("messages::error threw an exception with text: %",
                        e.what());
                }

                messages::error_text("Some % message, with some stuff inserted %, %, %",
                    "formatted", 1, 2.5234234, false);


            }

            template< bool = true> // declaring it as a template is a way to work aroud c++ limitations about declaring things in headers
            void examples() {
                using namespace autotelica::diagnostic_messages;

                print::title("Diagnostic Messages Examples");

                print::underline("Some  underlined text.");

                print::line(80);

                trace_some_messages();

                {
                    print::newline();
                    print::underline("Tracing with no timestamps.");
                    // here we use timestamp disabler to remove timestamps from tracing
                    timestamp_disabler _t;
                    trace_some_messages();
                }

                // we can also store all the traces into a string
                std::string string_storage;
                {
                    print::newline();
                    print::underline("Tracing into a string.");
                    
                    // by default string message handler doesn't throw error exceptions
                    // just traces error text
                    // but this can be configured by setting second constructor parameter to true
                    auto string_traces = string_message_handler::make_scoped(string_storage);
                    trace_some_messages();
                }

                print::newline();
                print::underline("What came out of the string:");
                std::cout << string_storage << std::endl;

                // and we can print to a file
                {
                    print::newline();
                    print::underline("Tracing errors into a file.");

                    // by default file message handler doesn't throw error exceptions
                    // just traces error text
                    // but this can be configured by setting second constructor parameter to true
                    auto string_traces = file_message_handler::make_scoped("trace_file.txt");
                    trace_some_messages();

                }

            }
            template< bool = true> // declaring it as a template is a way to work aroud c++ limitations about declaring things in headers
            void tests() {
                // disagnostic messages are not tested on purpose 
                // every other test tests them
            }
        }
    }
}

 AF_DECLARE_TEST_SET("DiagnosticMessages", diagnostic_messages,
    autotelica::examples::diagnostic_messages::examples<>() , autotelica::examples::diagnostic_messages::tests<>());

