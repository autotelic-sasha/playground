#pragma once
#include <vector>
#include "asserts.h"
#include "testing_util.h"
#include "diagnostic_messages.h"

namespace autotelica {
    namespace examples {
        namespace asserts { 

            static int f() {
                // test function, to show that string interpolation only evaluates parameters when needed
                std::cout << "f() evaluated" << std::endl;
                return 3;
            }

            static void tracing_to_std_out() {
                using namespace autotelica::diagnostic_messages;
                
                auto l = [] { std::cout << "l() evaluated" << std::endl; return 3; };

                std::vector<double> v{ 1.23, 0.32435, 4.5656, 0.120981280932481 };

                print::title("Tracing to stdout");

                AF_MESSAGE("blah blah");
                AF_MESSAGE("Message with % parameters % some %", l(), "and", "more"); // evaluates l()
                AF_MESSAGE_EX("blah blah");
                AF_MESSAGE_EX("Warning with % parameters % some %", f(), "and", "more"); // evaluates f()

                AF_ASSERT_WARNING(true, "Error with % parameters % some %", f(), "and", "more"); // does not evaluate f()
                AF_ASSERT_WARNING(false, "Error with % parameters % some %", f(), "and", "more"); // evaluates f()
                AF_ASSERT_WARNING(false, "Error with % parameters % some %", l(), "and", "more"); // evaluates l()
                AF_ASSERT_WARNING_EX(false, "Error with % parameters % some %", l(), "and", "more"); // evaluates l()


                AF_ASSERT(true, "Error with % parameters % some %", f(), "and", "more"); // does not evaluate f()
                try {
                    AF_ASSERT(false, "Error with % parameters % some %", f(), "and", "more"); // evaluates f() and throws runtime_error
                }
                catch (std::runtime_error const& e) {
                    AF_MESSAGE("Exception: %", e.what());
                }

                AF_ASSERT_WARNING(v.size(), "The vector v size is %", v.size());
                AF_ASSERT_WARNING(v.size() == 0, "The vector v is not empty!");
                AF_ASSERT_WARNING_EX(v.size() == 0, "The vector v is not empty!");
                AF_ASSERT_WARNING_EX(v.size() == 0, 0);
                print::line();
            }

            static void tracing_to_std_out_lowercase() {
                using namespace autotelica::diagnostic_messages;

                auto l = [] { std::cout << "l() evaluated" << std::endl; return 3; };

                std::vector<double> v{ 1.23, 0.32435, 4.5656, 0.120981280932481 };

                print::title("Tracing to stdout (lowercase)");
                af_message("blah blah");
                af_message("Message with % parameters % some %", l(), "and", "more"); // evaluates l()
                af_message_ex("blah blah");
                af_message_ex("Warning with % parameters % some %", f(), "and", "more"); // evaluates f()

                af_assert_warning(true, "Error with % parameters % some %", f(), "and", "more"); // does not evaluate f()
                af_assert_warning(false, "Error with % parameters % some %", f(), "and", "more"); // evaluates f()
                af_assert_warning(false, "Error with % parameters % some %", l(), "and", "more"); // evaluates l()
                af_assert_warning_ex(false, "Error with % parameters % some %", l(), "and", "more"); // evaluates l()


                af_assert(true, "Error with % parameters % some %", f(), "and", "more"); // does not evaluate f()
                try {
                    af_assert(false, "Error with % parameters % some %", f(), "and", "more"); // evaluates f() and throws runtime_error
                }
                catch (std::runtime_error const& e) {
                    af_message("Exception: %", e.what());
                }

                af_assert_warning(v.size(), "The vector v size is %", v.size());
                af_assert_warning(v.size() == 0, "The vector v is not empty!");
                af_assert_warning_ex(v.size() == 0, "The vector v is not empty!");
                af_assert_warning_ex(v.size() == 0, 0);
                print::line();
            }

            static void tracing_to_std_out_debug() {
                using namespace autotelica::diagnostic_messages;

                auto l = [] { std::cout << "l() evaluated" << std::endl; return 3; };

                std::vector<double> v{ 1.23, 0.32435, 4.5656, 0.120981280932481 };

#ifndef NDEBUG
                print::title("Debug tracing to stdout (we are in debug mode, there should be output)");
#else
                print::title("Debug tracing to stdout (we are not in debug mode, there should be no output)");
#endif
                af_dbg_message("blah blah");
                af_dbg_message("Message with % parameters % some %", l(), "and", "more"); // evaluates l()
                af_dbg_message_ex("blah blah");
                af_dbg_message_ex("Warning with % parameters % some %", f(), "and", "more"); // evaluates f()

                af_dbg_assert_warning(true, "Error with % parameters % some %", f(), "and", "more"); // does not evaluate f()
                af_dbg_assert_warning(false, "Error with % parameters % some %", f(), "and", "more"); // evaluates f()
                af_dbg_assert_warning(false, "Error with % parameters % some %", l(), "and", "more"); // evaluates l()
                af_dbg_assert_warning_ex(false, "Error with % parameters % some %", l(), "and", "more"); // evaluates l()


                af_dbg_assert(true, "Error with % parameters % some %", f(), "and", "more"); // does not evaluate f()
                try {
                    af_dbg_assert(false, "Error with % parameters % some %", f(), "and", "more"); // evaluates f() and throws runtime_error
                }
#ifndef NDEBUG
                catch (std::runtime_error const& e) {
                    af_dbg_message("Exception: %", e.what());
                }
#else
                catch (std::runtime_error const&) {
                    // doesn't happen in release builds
                }
#endif

                af_dbg_assert_warning(v.size(), "The vector v size is %", v.size());
                af_dbg_assert_warning(v.size() == 0, "The vector v is not empty!");
                af_dbg_assert_warning_ex(v.size() == 0, "The vector v is not empty!");
                af_dbg_assert_warning_ex(v.size() == 0, 0);
                print::line();
            }


            static void tracing_to_string() {
                using namespace autotelica::diagnostic_messages;

                std::string message_store;
                {
                    auto message_handler = string_message_handler::make_scoped(message_store);
                    // disable exceptions being throws
                    messages::configure(true, true, true, false);
                    
                    auto l = [] { std::cout << "l() evaluated" << std::endl; return 3; };

                    std::vector<double> v{ 1.23, 0.32435, 4.5656, 0.120981280932481 };

                    AF_MESSAGE("blah blah");
                    AF_MESSAGE("Message with % parameters % some %", l(), "and", "more"); // evaluates l()
                    AF_MESSAGE_EX("blah blah");
                    AF_MESSAGE_EX("Warning with % parameters % some %", f(), "and", "more"); // evaluates f()

                    AF_ASSERT_WARNING(true, "Error with % parameters % some %", f(), "and", "more"); // does not evaluate f()
                    AF_ASSERT_WARNING(false, "Error with % parameters % some %", f(), "and", "more"); // evaluates f()
                    AF_ASSERT_WARNING(false, "Error with % parameters % some %", l(), "and", "more"); // evaluates l()
                    AF_ASSERT_WARNING_EX(false, "Error with % parameters % some %", l(), "and", "more"); // evaluates l()

                    AF_ASSERT(true, "Error with % parameters % some %", f(), "and", "more"); // does not evaluate f()
                    AF_ASSERT(false, "Error with % parameters % some %", f(), "and", "more"); // evaluates f() but does not throw exceptions, just logs (we disabled exception in the string handler)

                    AF_ASSERT_WARNING(v.size(), "The vector v size is %", v.size());
                    AF_ASSERT_WARNING(v.size() == 0, "The vector v is not empty!");
                    AF_ASSERT_WARNING_EX(v.size() == 0, "The vector v is not empty!");
                    AF_ASSERT_WARNING_EX(v.size() == 0,0);
                    // re-enable exceptions being throws
                    messages::configure(true, true, true, true);

                }
                print::title("Logs recorded in a string");
                std::cout << message_store << "\n\n" << std::endl;
                print::line();
            }




            template< bool = true> // declaring it as a template is a way to work aroud c++ limitations about declaring things in headers
            void examples() {
                // code here will only be run in example runs
                tracing_to_std_out();
                tracing_to_std_out_lowercase();
                tracing_to_std_out_debug();
                tracing_to_string();
            }
            template< bool = true> // declaring it as a template is a way to work aroud c++ limitations about declaring things in headers
            void tests() {
                // code here will be run in test, examples and record mode
            }
        }
    }
}

AF_DECLARE_TEST_SET("Asserts", asserts, 
    autotelica::examples::asserts::examples<>() , autotelica::examples::asserts::tests<>());

