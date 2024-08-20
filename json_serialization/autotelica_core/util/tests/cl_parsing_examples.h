#pragma once
#include "testing_util.h"
#include "diagnostic_messages.h"
#include "cl_parsing.h"

namespace autotelica {
    namespace examples {
        namespace cl_parsing {
            void print(std::vector<std::string> const& arguments) {
                using namespace autotelica::diagnostic_messages;
                timestamp_disabler t;
                // a command that prints its arguments to stdout
                for (auto const& s : arguments)
                    messages::message(s);
            }
            
            void add(std::vector<std::string> const& arguments) {
                using namespace autotelica::diagnostic_messages;
                timestamp_disabler t;
                // a command that adds two numbers
                // it assumes it was given two arguments, the cl framework checks that
                messages::message("%+%=%", arguments[0], arguments[1], std::stod(arguments[0]) + std::stod(arguments[1]));
            }


            template< bool = true> // declaring it as a template is a way to work aroud c++ limitations about declaring things in headers
            void examples() {
                using namespace autotelica::cl_parsing;
                cl_commands commands;
                commands.register_command("a_simple_flag",
                    "A simple flag that get's turned on if mentioned.",
                    { "f", "simple_flag" });
                commands.register_command("print",
                    "Just prints its arguments to stdout.",
                    { "print", "p" },
                    -1, print);
                commands.register_command("add",
                    "Adds two numbers.",
                    { "add", "a" },
                    2, add);


                const char* args1[] = { "demo", "--help", "first", "second", "third" };
                commands.parse_command_line(sizeof(args1)/sizeof(char*), args1);
                commands.execute();
                commands.clear();
                const char* args2[] = {"demo", "-p", "first", "second", "third"};
                std::cout << "Flag is " << (commands.has("a_simple_flag") ? "set" : "not set") << std::endl;
                commands.parse_command_line(sizeof(args2) / sizeof(char*), args2);
                commands.execute();
                commands.clear();
                const char* args3[] = { "demo", "-p", "first", "second", "third", "-f"};
                commands.parse_command_line(sizeof(args3) / sizeof(char*), args3);
                std::cout << "Flag is " << (commands.has("a_simple_flag") ? "set" : "not set") << std::endl;
                commands.execute();
                commands.clear();
                const char* args4[] = { "demo", "first", "second", "third", "-f", "-a", "12", "0.5", "fourth"};
                commands.parse_command_line(sizeof(args4) / sizeof(char*), args4);
                std::cout << "Flag is " << (commands.has("a_simple_flag") ? "set" : "not set") << std::endl;
                std::cout << "Additional arguments: " << autotelica::std_pretty_printing::pretty_s(commands.arguments()) << std::endl;
                commands.execute();
                commands.clear();
                const char* args5[] = { "demo", "first", "-p", "second", "third", "-f", "-a", "12", "0.5", "fourth" };
                commands.parse_command_line(sizeof(args5) / sizeof(char*), args5);
                std::cout << "Flag is " << (commands.has("a_simple_flag") ? "set" : "not set") << std::endl;
                std::cout << "Additional arguments: " << autotelica::std_pretty_printing::pretty_s(commands.arguments()) << std::endl;
                commands.execute();
                commands.clear();
                const char* args6[] = { "demo", "first", "second", "third", "-f", "-a", "12", "0.5", "fourth", "-p" };
                commands.parse_command_line(sizeof(args6) / sizeof(char*), args6);
                std::cout << "Flag is " << (commands.has("a_simple_flag") ? "set" : "not set") << std::endl;
                std::cout << "Additional arguments: " << autotelica::std_pretty_printing::pretty_s(commands.arguments()) << std::endl;
                commands.execute();
                commands.clear();



            }
            template< bool = true> // declaring it as a template is a way to work aroud c++ limitations about declaring things in headers
            void tests() {
                using namespace autotelica::cl_parsing;
                using namespace autotelica::diagnostic_messages;
                using namespace autotelica::std_pretty_printing;
                bool simple_flag = false;
                std::vector<std::string> additional_arguments;



                cl_commands commands;
                commands.register_command("a_simple_flag",
                    "A simple flag that get's turned on if mentioned.",
                    { "f", "simple_flag" });
                commands.register_command("print",
                    "Just prints its arguments to stdout.",
                    { "print", "p" },
                    -1, print);
                commands.register_command("add",
                    "Adds two numbers.",
                    { "add", "a" },
                    2, add);

                std::string traces;

                AF_TEST_COMMENT("demo --help first second third");
                AF_START_STRING_TRACING(traces);
                const char* args1[] = { "demo", "--help", "first", "second", "third" };
                commands.parse_command_line(sizeof(args1) / sizeof(char*), args1);
                commands.execute();
                commands.clear();
                AF_END_STRING_TRACING();
                AF_TEST_RESULT(
                    "demo\n\n"
                    "help ([\"-help\",\"-?\"])\n"
                    "\tHelps.\n\n"
                    "a_simple_flag ([\"-f\",\"-simple_flag\"])\n"
                    "\tA simple flag that get's turned on if mentioned.\n\n"
                    "print ([\"-print\",\"-p\"])\n"
                    "\tJust prints its arguments to stdout.\n\n"
                    "add ([\"-add\",\"-a\"])\n"
                    "\tAdds two numbers.\n\n\n", traces);

                AF_TEST_COMMENT("demo -p first second third");
                AF_START_STRING_TRACING(traces);
                const char* args2[] = { "demo", "-p", "first", "second", "third" };
                commands.parse_command_line(sizeof(args2) / sizeof(char*), args2);
                commands.execute();
                commands.clear();
                AF_END_STRING_TRACING();
                AF_TEST_RESULT("first\nsecond\nthird\n", traces);

                AF_TEST_COMMENT("demo -p first second third -f");
                 AF_START_STRING_TRACING(traces);
                const char* args3[] = { "demo", "-p", "first", "second", "third", "-f" };
                commands.parse_command_line(sizeof(args3) / sizeof(char*), args3);
                simple_flag = commands.has("a_simple_flag");
                additional_arguments = commands.arguments();
                commands.execute();
                commands.clear();
                AF_END_STRING_TRACING();
                AF_TEST_RESULT("first\nsecond\nthird\n", traces);
                AF_TEST_RESULT(true, simple_flag);
                AF_TEST_RESULT("[]", pretty_s(additional_arguments));

                AF_TEST_COMMENT("demo first second third -f -a 12 0.5 fourth");
                AF_START_STRING_TRACING(traces);
                const char* args4[] = { "demo", "first", "second", "third", "-f", "-a", "12", "0.5", "fourth" };
                commands.parse_command_line(sizeof(args4) / sizeof(char*), args4);
                simple_flag = commands.has("a_simple_flag");
                additional_arguments = commands.arguments();
                commands.execute();
                commands.clear();
                AF_END_STRING_TRACING();
                AF_TEST_RESULT("12+0.5=12.5\n", traces);
                AF_TEST_RESULT(true, simple_flag);
                AF_TEST_RESULT("[\"first\",\"second\",\"third\",\"fourth\"]", pretty_s(additional_arguments));

                AF_TEST_COMMENT("demo first -p second third -f -a 12 0.5 fourth");
                AF_START_STRING_TRACING(traces);
                const char* args5[] = { "demo", "first", "-p", "second", "third", "-f", "-a", "12", "0.5", "fourth" };
                commands.parse_command_line(sizeof(args5) / sizeof(char*), args5);
                simple_flag = commands.has("a_simple_flag");
                additional_arguments = commands.arguments();
                commands.execute();
                commands.clear();
                AF_END_STRING_TRACING();
                AF_TEST_RESULT("second\nthird\n12+0.5=12.5\n", traces);
                AF_TEST_RESULT(true, simple_flag);
                AF_TEST_RESULT("[\"first\",\"fourth\"]", pretty_s(additional_arguments));

                AF_TEST_COMMENT("demo first second third -a 12 0.5 fourth -p");
                AF_START_STRING_TRACING(traces);
                const char* args6[] = { "demo", "first", "second", "third", "-a", "12", "0.5", "fourth", "-p" };
                commands.parse_command_line(sizeof(args6) / sizeof(char*), args6);
                simple_flag = commands.has("a_simple_flag");
                additional_arguments = commands.arguments();
                commands.execute();
                commands.clear();
                AF_END_STRING_TRACING();
                AF_TEST_RESULT("12+0.5=12.5\n", traces);
                AF_TEST_RESULT(false, simple_flag);
                AF_TEST_RESULT("[\"first\",\"second\",\"third\",\"fourth\"]", pretty_s(additional_arguments));
            }
        }
    }
}

// uncomment the line below to declare your test set
AF_DECLARE_TEST_SET("ClParsing", cl_parsing, 
    autotelica::examples::cl_parsing::examples<>() , 
    autotelica::examples::cl_parsing::tests<>());

