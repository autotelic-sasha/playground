#pragma once
#include <memory>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <fstream>
#include <exception>
#include <type_traits>
#include <functional>
#include "string_util.h"
#include "comparissons.h"
#include "macro_magic.h"
#include "diagnostic_messages.h"
#include "std_pretty_printing.h"

namespace autotelica {
	namespace testing {
	// testing utilities 
		
		// configuration of test runs
		class testing_config {
		public:
			// tests can run in a number of modes
			enum class run_mode {
				testing,		// actual testing, exceptions normally get thrown on errors
				examples,		// examples run just like any code, errors are reported in output, not thrown as exceptions
				recording,		// test macros can record themselves, 
								// in this mode every testing macro outputs itself with correct results as expected result
				plain_csv,		// only execute tests, output results of tests in plain csv format, errors are reported in output, not thrown as exceptions
				excel_csv		// only execute tests, output results of tests in excel csv format, errors are reported in output, not thrown as exceptions
			};
		private:
			run_mode _run_mode;

			// epsilon values for value comparissons
			float _float_epsilon;
			double _double_epsilon;

			testing_config() {
				_reset();
			}
			// testing_config is a singleton
			static std::shared_ptr<testing_config> get() {
				static std::shared_ptr<testing_config> instance(new testing_config());
				return instance;
			}
			void _reset() { // initialisation
				_float_epsilon = sqrt(std::numeric_limits<float>::epsilon());
				_double_epsilon = sqrt(std::numeric_limits<double>::epsilon());
				_run_mode = run_mode::examples;
			}
		public:
			// getters and setters are all that matters
			static float float_epsilon() { return get()->_float_epsilon; }
			static double double_epsilon() { return get()->_double_epsilon; }

			static float set_float_epsilon(float const& f) { return get()->_float_epsilon = f; }
			static double set_double_epsilon(double const& d) { return get()->_double_epsilon = d; }

			static run_mode current_run_mode() { return get()->_run_mode; }
			static void set_run_mode(run_mode mode) { get()->_run_mode = mode; }
			
			static void set_run_mode_examples() { set_run_mode(run_mode::examples); }
			static void set_run_mode_testing() { set_run_mode(run_mode::testing); }
			static void set_run_mode_recording() { set_run_mode(run_mode::recording); }
			static void set_run_mode_plain_csv() { set_run_mode(run_mode::plain_csv); }
			static void set_run_mode_excel_csv() { set_run_mode(run_mode::excel_csv); }


			static bool is_run_mode_examples() { return current_run_mode() == run_mode::examples; }
			static bool is_run_mode_testing() { return current_run_mode() == run_mode::testing; }
			static bool is_run_mode_recording() { return current_run_mode() == run_mode::recording; }
			static bool is_run_mode_plain_csv() { return current_run_mode() == run_mode::plain_csv; }
			static bool is_run_mode_excel_csv() { return current_run_mode() == run_mode::excel_csv; }
			static bool is_run_mode_csv() { return is_run_mode_plain_csv() || is_run_mode_excel_csv(); }

			static bool throw_on_error() { return is_run_mode_testing() && diagnostic_messages::messages::throw_errors(); }
			static bool run_examples() { return is_run_mode_examples(); }

			static void reset() { get()->_reset(); }
		};

		namespace testing_impl {
			// outputing strings when recording is a pain
			// quote_string deals with that pain
			template<typename T>
			struct quote_string {
				T const& _value;
				quote_string(T const& t) : _value(t) {}
				T const& value() const { return _value; }
			};
			template<>
			struct quote_string<const char*> {
				std::string const _value;
				quote_string(const char* const t) : _value(
					string_util::replace(
						string_util::af_format_string("\"%\"", string_util::replace(t, "\"", "\\\"")),
						"\n", "\\n")
				) {
				}
				const char* const value() const { return _value.c_str(); }
			};

			template<>
			struct quote_string<std::string> {
				std::string const _value;
				quote_string(std::string const& t) : _value(
					string_util::replace(
						string_util::af_format_string("std::string(\"%\")", string_util::replace(t, "\"", "\\\"")),
						"\n", "\\n")
				) {
				}
				const char* const value() const { return _value.c_str(); }
			};


			// formatting function call from macro arguments
			// used for various output messages
			static std::string format_function_call(
				bool is_function, 
				const char* const function_name, 
				const char* const arguments = "") {
				if (is_function) {
					return string_util::af_format_string("%(%)", function_name, arguments);
				}
				else {
					return function_name;
				}
			}

			static void csv_column_titles() {
				using namespace diagnostic_messages;
				if (testing_config::is_run_mode_csv()) {
					messages::message(
						string_util::to_csv_row_string(
							testing_config::is_run_mode_excel_csv(),
							"Status",
							"Test code",
							"Actual value",
							"Expected value",
							"Error",
							"File",
							"Line"));
				}
			}
			// reporting success
			static void report_success(
					bool is_function, 
					const char* const function_name, 
					const char* const file_name,
					int line,
					const char* const arguments = "") {
				using namespace diagnostic_messages;

				if (testing_config::is_run_mode_csv()) {
					messages::message(
						string_util::to_csv_row_string(
							testing_config::is_run_mode_excel_csv(),
							"SUCCESS",
							format_function_call(is_function, function_name, arguments),
							"",
							"",
							"",
							file_name,
							line));
				}
				else {
					messages::message(
						"SUCCESS: % ran with no exceptions.",
						format_function_call(is_function, function_name, arguments));

				}
			}
			
			// reporting success of a tests that was meant to throw an exception
			static void report_throw_success(
				bool is_function,
				const char* const function_name,
				const char* const file_name,
				int line, 
				const char* const arguments = "",
				std::string const& exception_what = "") {

				using namespace diagnostic_messages;
				if (testing_config::is_run_mode_csv()) {
					messages::message(
						string_util::to_csv_row_string(
							testing_config::is_run_mode_excel_csv(),
							"SUCCESS",
							format_function_call(is_function, function_name, arguments),
							"Threw an exception as expected.",
							"",
							exception_what,
							file_name,
							line));
				}
				else {
					messages::message(
						"SUCCESS: % has thrown an exception as expected(exception reason was '%').",
						format_function_call(is_function, function_name, arguments),
						exception_what);
				}
			}

			// reporting a succesful comparisson of actual and expected result
			template< typename T>
			static void report_success_result(
					T const result,
					bool is_function,
					const char* const function_name,
					const char* const file_name,
					int line,
					const char* const arguments = "") {
				using namespace diagnostic_messages;
				if (testing_config::is_run_mode_csv()) {
					messages::message(
						string_util::to_csv_row_string(
							testing_config::is_run_mode_excel_csv(),
							"SUCCESS",
							format_function_call(is_function, function_name, arguments),
							result,
							result,
							"",
							file_name,
							line));
				}
				else {
					messages::message(
						"SUCCESS: % ran with no exceptions and returned %.",
						format_function_call(is_function, function_name, arguments),
						quote_string<T>(result).value());
				}
			}
			
			// reporting that an exception was thrown when not expected
			static void report_exception_error(
					bool is_function,
					const char* const function_name, 
					const char* const arguments, 
					const char* const file_name, 
					int line, 
					const char* const exception_what,
					bool throw_exception) {

				using namespace diagnostic_messages;
				if (testing_config::is_run_mode_csv()) {
					messages::message(
						string_util::to_csv_row_string(
							testing_config::is_run_mode_excel_csv(),
							"ERROR",
							format_function_call(is_function, function_name, arguments),
							"",
							"",
							exception_what,
							file_name,
							line));
				}
				else {
					if (throw_exception) {
						messages::error_ex(nullptr, file_name, line,
							"% failed with exception reason: '%'.",
							format_function_call(is_function, function_name, arguments),
							exception_what);
					}
					else {
						messages::error_text_ex(nullptr, file_name, line,
							"% failed with exception reason: '%'.",
							format_function_call(is_function, function_name, arguments),
							exception_what);
					}
				}
			}
			
			// reporting that no exception was throws when one was expected
			static void report_no_exception_error(
					bool is_function,
					const char* const function_name,
					const char* const arguments,
					const char* const file_name,
					int line,
					bool throw_exception) {
				using namespace diagnostic_messages;
				if (testing_config::is_run_mode_csv()) {
					messages::message(
						string_util::to_csv_row_string(
							testing_config::is_run_mode_excel_csv(),
							"ERROR",
							format_function_call(is_function, function_name, arguments),
							"",
							"",
							"Was expected to throw an exception but didn't.",
							file_name,
							line));
				}
				else {
					if (throw_exception) {
						messages::error_ex(nullptr, file_name, line,
							"%  was expected to throw an exception but didn't.",
							format_function_call(is_function, function_name, arguments));
					}
					else {
						messages::error_text_ex(nullptr, file_name, line,
							"%  was expected to throw an exception but didn't.",
							format_function_call(is_function, function_name, arguments));
					}
				}
			}

			// reporting that there was a difference between actual and expected value
			template< typename T > 
			static void report_value_error(
					T const expected, T const actual,
					bool is_function,
					const char* const function_name, 
					const char* const arguments,
					const char* const file_name, 
					int line, 
					bool throw_on_difference) {
				using namespace diagnostic_messages;
				if (testing_config::is_run_mode_csv()) {
					messages::message(
						string_util::to_csv_row_string(
							testing_config::is_run_mode_excel_csv(),
							"ERROR",
							format_function_call(is_function, function_name, arguments),
							actual,
							expected,
							"Actual and expected values don't match.",
							file_name,
							line));
				}
				else {
					if (throw_on_difference) {
						messages::error_ex(nullptr, file_name, line,
							"%  returned % but expected value was: %.",
							format_function_call(is_function, function_name, arguments),
							quote_string<T>(actual).value(),
							quote_string<T>(expected).value()
						);
					}
					else {
						messages::error_text_ex(nullptr, file_name, line,
							"%  returned % but expected value was: %.",
							format_function_call(is_function, function_name, arguments),
							quote_string<T>(actual).value(),
							quote_string<T>(expected).value()
						);
					}
				}
			}

			// recording macros			
			static void record_macro(
					const char* const macro_name, 
					const char* const function_name, 
					const char* const arguments = nullptr) {
				using namespace diagnostic_messages;
				timestamp_disabler _t;
				if (arguments) {
					messages::message(
						"\t%(%, %);",
						macro_name, function_name, arguments);
				}
				else {
					messages::message(
						"\t%(%);",
						macro_name, function_name);
				}
			}

			// recording macros with expected results
			template< typename T >
			static void record_macro_with_result(
					T const expected, 
					const char* const macro_name, 
					const char* const function_name, 
					const char* const arguments = nullptr) {
				using namespace diagnostic_messages;
				timestamp_disabler _t;
				if (arguments) {
					messages::message(
						"\t%(%, %, %);",
						macro_name, quote_string< T >(expected).value(),
						function_name, arguments);
				}
				else {
					messages::message(
						"\t%(%, %);",
						macro_name, quote_string< T >(expected).value(),
						function_name);
				}
			}

			// reporting the result of a function or expression or a snippet
			template< typename T >
			static void report_result(
					T  const expected, T const actual,
					bool is_function,
					const char* const function_name, const char* const arguments,
					const char* const file_name, int line, bool throw_on_difference) {
				using namespace comparissons;
				const double double_eps = comparissons_config::double_epsilon();
				const float float_eps = comparissons_config::float_epsilon();
				
				comparissons_config::set_double_epsilon(testing_config::double_epsilon());
				comparissons_config::set_float_epsilon(testing_config::float_epsilon());
				
				if (are_equal_f(expected, actual))
					report_success_result(
						actual, 
						is_function, 
						function_name, 
						file_name,line,
						arguments);
				else
					report_value_error(
						expected, actual,
						is_function,
						function_name, arguments,
						file_name, line, throw_on_difference);
				
				comparissons_config::set_double_epsilon(double_eps);
				comparissons_config::set_float_epsilon(float_eps);
			}

			
		}

		// base class for all test runners
		struct test_runner_base {
			virtual ~test_runner_base() {}
			virtual const char* const name() const = 0;
			virtual void run_examples() const = 0;
			virtual void run_tests()  const = 0;
			virtual void run_recording()  const = 0;
		};

		// all_tests is a registry for tests
		class all_tests {
			struct test_record {
				std::shared_ptr< test_runner_base > _runner;
				std::string _file;
				std::string _class;
			};
			string_util::string_map_nc<test_record> _tests;

			all_tests(){}
			static all_tests& get() {
				static all_tests _instance;
				return _instance;
			}
			void add_impl(
				const char* const name, 
				std::shared_ptr< test_runner_base > runner, 
				std::string const& file_, 
				std::string const& class_
				) {
				if (_tests.find(name) != _tests.end()) {
					throw std::runtime_error(
						string_util::af_format_string("Test runner % is already registered", name));
				}
				_tests[name] = test_record{ runner, file_, class_ };
			}
			std::vector<test_record>
			find_tests(std::string const& pattern) {
				// the key can be:
				// 1. name
				// 2. class name
				// 3. file name
				// 4. a wildcard string for any of those
				
				// a cool thing about wildcard matches is that they will do an exact match too
				std::vector<test_record> out;
				for (auto const& r : _tests) {
					if (string_util::wildcard_match(r.first, pattern))
						out.push_back(r.second);
				}
				if (!out.empty()) return out;
				for (auto const& r : _tests) {
					if (string_util::wildcard_match(r.second._class, pattern))
						out.push_back(r.second);
				}
				if (!out.empty()) return out;
				for (auto const& r : _tests) {
					if (string_util::wildcard_match(r.second._file, pattern))
						out.push_back(r.second);
				}
				if (out.empty()) {
					throw std::runtime_error(
						string_util::af_format_string("Test runner % is not registered", pattern));
				}
				return out;

			}

			void list_tests_impl() const {
				using namespace diagnostic_messages;
				using namespace std_pretty_printing;
				timestamp_disabler _t;
				std::vector <std::vector<std::string>> t;
				t.push_back({ "Test Set", "Class", "File" });
				for (auto const& k : _tests)
					t.push_back({ k.first, k.second._class, k.second._file });
				messages::message(std_pretty_printing::table_s(t, true));
			}
			std::string get_file_impl(std::string const& name) const {
				auto const& it = _tests.find(name);
				if (it != _tests.end())
					return it->second._file;
				return "";
			}
			std::string get_class_impl(std::string const& name) const {
				auto const& it = _tests.find(name);
				if (it != _tests.end())
					return it->second._class;
				return "";
			}
			void run_examples_impl(std::string const& name) {
				auto const tests = find_tests(name);
				for(auto const& t : tests)
					t._runner->run_examples();
			}
			void run_tests_impl(std::string const& name) {
				auto const tests = find_tests(name);
				for (auto const& t : tests)
					t._runner->run_tests();
			}
			void run_recording_impl(std::string const& name) {
				auto const tests = find_tests(name);
				for (auto const& t : tests)
					t._runner->run_recording();
			}

			void run_all_examples_impl() {
				for (auto const& k : _tests)
					k.second._runner->run_examples();
			}
			void run_all_tests_impl() {
				for (auto const& k : _tests)
					k.second._runner->run_tests();
			}
			void run_all_recording_impl() {
				for (auto const& k : _tests)
					k.second._runner->run_recording();
			}

			// register a new test set
			static void add(
				const char* const name,
				std::shared_ptr< test_runner_base > runner,
				std::string const& file_,
				std::string const& class_) {
				get().add_impl(name, runner, file_, class_);
			}
		public:
			// list all available test sets
			static void list_tests() {
				get().list_tests_impl();
			}

			// get a file where a test set is implemented
			static std::string get_file(std::string const& name) {
				return get().get_file_impl(name);
			}
			// get a class that implements a test set
			static std::string get_class(std::string const& name) {
				return get().get_class_impl(name);
			}

			// run named test set as examples
			static void examples(std::string const& name) {
				get().run_examples_impl(name);
			}
			// run named test set as tests
			static void tests(std::string const& name) {
				get().run_tests_impl(name);
			}
			// record the named test set
			static void record(std::string const& name) {
				get().run_recording_impl(name);
			}

			
			// run named test sets as examples
			static void run_examples(std::vector<std::string> const& names) {
				for(auto const & name : names)
					get().run_examples_impl(name);
			}
			// run named test sets as tests
			static void run_tests(std::vector<std::string> const& names) {
				for (auto const& name : names)
					get().run_tests_impl(name);
			}
			// record the named test sets
			static void record_tests(std::vector<std::string> const& names) {
				for (auto const& name : names)
					get().run_recording_impl(name);
			}

			// run all test sets as examples
			static void run_all_examples() {
				get().run_all_examples_impl();
			}
			// run all test sets as tests
			static void run_all_tests() {
				get().run_all_tests_impl();
			}
			// record all test sets
			static void record_all() {
				get().run_all_recording_impl();
			}
			
			// register a new test set
			template< typename TestT >
			static bool register_tests(const char* const class_name) {
				using test_set_name_f = decltype(TestT::test_set_name); // fails if test_set_name is not implemented
				using test_set_file_f = decltype(TestT::test_set_file); // fails if test_set_file is not implemented
				using test_examples_f = decltype(TestT::af_examples);      // fails if examples is not implemented
				
				static_assert(std::is_constructible< std::function<const char* const()>, test_set_name_f>::value, 
					"Test class must implement 'static const char* const test_set_name()' function");
				static_assert(std::is_constructible< std::function<const char* const()>, test_set_file_f>::value, 
					"Test class must implement 'static const char* const test_set_file()' function");
				static_assert(std::is_constructible< std::function<void()>, test_examples_f>::value, 
					"Test class must implement 'static void examples()' function");
				add(TestT::test_set_name(), std::shared_ptr< test_runner_base>(new TestT()), TestT::test_set_file(), class_name);
				return true;
			}
		};


		namespace testing_impl {
			// print the header and footer for test set before running it
			struct test_set_printer {
				test_set_printer(std::string const& test_set, std::string const& file_name, std::string const& class_name) {
					using namespace diagnostic_messages;
					if (testing_config::is_run_mode_csv()) {
						messages::message(
							string_util::to_csv_row_string(
								testing_config::is_run_mode_excel_csv(),
								"", 
								test_set,
								"",
								"",
								"",
								file_name
								));
					}
					else {
						if (!class_name.empty())
							print::title(
								string_util::af_format_string("Test set % (%)", test_set, class_name));
						else
							print::title(string_util::af_format_string("Test set %", test_set));

						if (!file_name.empty())
							print::underline(file_name);
					}
				}
				~test_set_printer() {
					using namespace diagnostic_messages;
					if (!testing_config::is_run_mode_csv()) 
						print::line();
				}
			};

			struct scoped_csv_format {
				testing_config::run_mode _current_run_mode;
				
				scoped_csv_format(bool is_excel) :
					_current_run_mode(testing_config::current_run_mode()){
					if (is_excel)
						testing_config::set_run_mode_excel_csv();
					else
						testing_config::set_run_mode_plain_csv();
				}
				~scoped_csv_format() {
					testing_config::set_run_mode(_current_run_mode);
				}
			};
		}

		
		// implementation of a test runner behaviour
		// curiously recursive templates, yay :) 
		template< typename CuriousBaseT >
		struct test_runner : public test_runner_base {
			
			// run as examples
			static void af_examples() {
				testing_impl::test_set_printer printer(
					CuriousBaseT::test_set_name(), 
					all_tests::get_file(CuriousBaseT::test_set_name()).c_str(),
					all_tests::get_class(CuriousBaseT::test_set_name()).c_str());
				CuriousBaseT::af_examples();
			}

			// run as tests
			static void af_tests() {
				testing_config::set_run_mode_testing();
				af_examples();
				testing_config::set_run_mode_examples();
			}

			// let macros record themselves
			static void af_record() {
				testing_config::set_run_mode_recording();
				using namespace diagnostic_messages;
				print::underline("Recording");
				try {
					af_examples();
				}
				catch (std::exception const& e) {
					messages::error_text(
						"An exception was thrown during recording of tests. The error was: %.",
						e.what());
				}
				catch (...) {
					messages::error_text(
						"An unknown exception was thrown during recording of tests.");
				}
				testing_config::set_run_mode_examples();
			}

			// v table part, to make the registry work
			const char* const name() const override { return CuriousBaseT::test_set_name(); }
			void run_examples() const override { af_examples(); }
			void run_tests()  const override { af_tests(); }
			void run_recording() const override { af_record(); }
		};

// setup the needed boring members of a test set runner class
#define AF_TEST_SET( test_set_name_ )    \
	static constexpr char const* test_set_name() { return test_set_name_; } \
	static constexpr char const*  test_set_file() { return __FILE__; }

#define af_test_set( test_set_name_ ) AF_TEST_SET( test_set_name_ )

#define AF_DECLARE_TEST_SET_CLASS( test_set_struct_, test_set_name_ )\
	struct test_set_struct_ : autotelica::testing::test_runner<test_set_struct_> {\
		static constexpr char const* test_set_name() { return test_set_name_; } \
		static constexpr char const* test_set_file() { return __FILE__; }

#define af_declare_test_set_class( test_set_name_, test_set_struct_ ) AF_DECLARE_TEST_SET_CLASS( test_set_name_, test_set_struct_ )


#define AF_END_TEST_SET_CLASS_DECLARATION(  ) };

#define af_end_test_set_class_declaration(  ) AF_END_TEST_SET_CLASS_DECLARATION(  )

#define AF_DECLARE_TEST_SET( description, test_namespace, examples_function, test_function )\
namespace autotelica {\
    namespace examples {\
        namespace test_namespace {\
            AF_DECLARE_TEST_SET_CLASS(run, description);\
            static void af_examples() {\
                AF_START_EXAMPLES_ONLY();\
                examples_function;\
                AF_END_EXAMPLES_ONLY();\
                test_function;\
            }\
            AF_END_TEST_SET_CLASS_DECLARATION();\
        }\
    }\
}

#define af_declare_test_set( description, test_namespace, examples_function, test_function) AF_DECLARE_TEST_SET( description, test_namespace, examples_function, test_function )\

// register a test set
#define AF_REGISTER_TEST_SET( test_namespace )  namespace {\
		static bool NAME_WITH_LINE(__test_registration_) = \
			autotelica::testing::all_tests::register_tests<autotelica::examples::test_namespace::run>( #test_namespace );\
	}

#define af_register_test_set( test_namespace ) AF_REGISTER_TEST_SET( test_namespace )

// list all registered test sets 
#define AF_LIST_TEST_SETS( ) autotelica::testing::all_tests::list_tests();

#define af_list_test_sets( ) AF_LIST_TEST_SETS( ) 

// run named test sets as examples
#define AF_RUN_EXAMPLES(...) \
	autotelica::testing::testing_impl::csv_column_titles();\
	autotelica::testing::all_tests::run_examples( { __VA_ARGS__ } )

#define af_run_examples(...) AF_RUN_EXAMPLES(__VA_ARGS__)

// run named test sets as examples
#define AF_RUN_TESTS(...) \
	autotelica::testing::testing_impl::csv_column_titles();\
	autotelica::testing::all_tests::run_tests( { __VA_ARGS__ } )

#define af_run_tests(...) AF_RUN_TESTS(__VA_ARGS__)

// record named test sets 
#define AF_RECORD_TESTS(...) autotelica::testing::all_tests::record_tests( { __VA_ARGS__ } )

#define af_record_tests(...) AF_RECORD_TESTS(__VA_ARGS__)

// run all registered test sets as examples
#define AF_RUN_ALL_EXAMPLES( ) \
	autotelica::testing::testing_impl::csv_column_titles();\
	autotelica::testing::all_tests::run_all_examples(  )

#define af_run_all_examples( ) AF_RUN_ALL_EXAMPLES( )

// run all registered test sets as tests
#define AF_RUN_ALL_TESTS( ) \
	autotelica::testing::testing_impl::csv_column_titles();\
	autotelica::testing::all_tests::run_all_tests(  )

#define af_run_all_tests( ) AF_RUN_ALL_TESTS( )

// record all registered test sets
#define AF_RECORD_ALL_TESTS( ) autotelica::testing::all_tests::record_all(  )

#define af_record_all_tests( ) AF_RECORD_ALL_TESTS( )

// to trace all output in csv format suround the block where tests sets are executed 
// with AF_START_CSV_TRACING and AF_END_CSV_TRACING; or package it all up within AF_CSV_TRACING parameter
// Also ... excel gets weird about interpreting csv field value types, so a bit of special handling is needed;
// hence the overloaded versions too.
#define AF_START_CSV_TRACING( excel_specific ) {\
	autotelica::diagnostic_messages::timestamp_disabler	NAME_WITH_LINE(__disabler);\
	auto NAME_WITH_LINE(__csv_tracer){autotelica::testing::testing_impl::scoped_csv_format(excel_specific)};

#define af_start_csv_tracing( ) AF_START_CSV_TRACING( )

#define AF_END_CSV_TRACING( ) }

#define af_end_csv_tracing( ) AF_END_CSV_TRACING( )

#define AF_CSV_TRACING( excel_specific, snippet ) \
	AF_START_CSV_TRACING(excel_specific);\
	{ snippet; };\
	AF_END_CSV_TRACING();

#define af_csv_tracing( excel_specific, snippet ) AF_CSV_TRACING( excel_specific, snippet )

#define AF_START_EXCEL_CSV_TRACING( ) AF_START_CSV_TRACING( true ) 

#define af_start_excel_csv_tracing( ) AF_START_EXCEL_CSV_TRACING( )

#define AF_END_EXCEL_CSV_TRACING( ) }

#define af_end_excel_csv_tracing( ) AF_END_EXCEL_CSV_TRACING( )

#define AF_EXCEL_CSV_TRACING( snippet ) AF_CSV_TRACING( true, snippet )

#define af_excel_csv_tracing( snippet ) AF_EXCEL_CSV_TRACING( snippet )

#define AF_START_PLAIN_CSV_TRACING( ) AF_START_CSV_TRACING( false ) 

#define af_start_plain_csv_tracing( ) AF_START_PLAIN_CSV_TRACING( )

#define AF_END_PLAIN_CSV_TRACING( ) }

#define af_end_plain_csv_tracing( ) AF_END_PLAIN_CSV_TRACING( )

#define AF_PLAIN_CSV_TRACING( snippet ) AF_CSV_TRACING( false, snippet )

#define af_plain_csv_tracing( snippet ) AF_PLAIN_CSV_TRACING( snippet )

// to trace all output to a file suround the block where tests sets are executed 
// with AF_START_FILE_TRACING and AF_END_FILE_TRACING; or package it all up within AF_FILE_TRACING parameters
#define AF_START_FILE_TRACING( file_name ) {\
	auto NAME_WITH_LINE(__tracer){autotelica::diagnostic_messages::file_message_handler::make_scoped( file_name )};

#define af_start_file_tracing( file_name ) AF_START_FILE_TRACING( file_name )

#define AF_END_FILE_TRACING( ) }

#define af_end_file_tracing( ) AF_END_FILE_TRACING( )

#define AF_FILE_TRACING( file_name, snippet ) \
	AF_START_FILE_TRACING(file_name);\
	{ snippet; };\
	AF_END_FILE_TRACING();

#define af_file_tracing( file_name, snippet ) AF_FILE_TRACING( file_name, snippet )


// to trace all output to a string suround the block where tests sets are executed 
// with AF_START_STRING_TRACING and AF_END_STRING_TRACING; or package it all up within AF_STRING_TRACING parameters
#define AF_START_STRING_TRACING( string_reference ) {\
	auto NAME_WITH_LINE(__tracer){autotelica::diagnostic_messages::string_message_handler::make_scoped( string_reference )};

#define af_start_string_tracing( string_reference ) AF_START_STRING_TRACING( string_reference ) 

#define AF_END_STRING_TRACING( ) }

#define af_end_string_tracing( ) AF_END_STRING_TRACING( )

#define AF_STRING_TRACING( string_reference, snippet ) \
	AF_START_STRING_TRACING(string_reference);\
	{ snippet; };\
	AF_END_STRING_TRACING();

#define af_string_tracing( string_reference, snippet ) AF_STRING_TRACING( string_reference, snippet )


// mark code as only executable during example runs surround it with
//  AF_START_EXAMPLES_ONLY() and AF_END_EXAMPLES_ONLY()
#define AF_START_EXAMPLES_ONLY() { if(autotelica::testing::testing_config::run_examples()) {

#define af_start_examples_only() AF_START_EXAMPLES_ONLY()

#define AF_END_EXAMPLES_ONLY() }}

#define af_end_examples_only() AF_END_EXAMPLES_ONLY()



// test that a function with listed arguments runs without exceptions
#define AF_TEST_FUNCTION(function,...) \
		{\
			using __af_conf = autotelica::testing::testing_config;\
			namespace __af_impl = autotelica::testing::testing_impl;\
			if(!__af_conf::is_run_mode_recording())\
			{\
				try{\
					function(__VA_ARGS__);\
					__af_impl::report_success(true, #function, __FILE__, __LINE__, STRINGIFY_VA_ARGS(__VA_ARGS__));\
				}\
				catch (std::exception const& e) {\
					__af_impl::report_exception_error(true, #function, STRINGIFY_VA_ARGS(__VA_ARGS__), __FILE__, __LINE__, e.what(),__af_conf::throw_on_error());\
				}\
				catch (...) {\
					__af_impl::report_exception_error(true, #function, STRINGIFY_VA_ARGS(__VA_ARGS__), __FILE__, __LINE__, "Unknown exception", __af_conf::throw_on_error());\
				}\
			}\
			else\
			{\
				__af_impl::record_macro("AF_TEST_FUNCTION", #function, STRINGIFY_VA_ARGS(__VA_ARGS__));\
			}\
		}

#define af_test_function(function,...) AF_TEST_FUNCTION(function,__VA_ARGS__)

// test that a function with listed arguments throws an exception
#define AF_TEST_FUNCTION_THROWS(function,...) \
		{\
			using __af_conf = autotelica::testing::testing_config;\
			namespace __af_impl = autotelica::testing::testing_impl;\
			if(!__af_conf::is_run_mode_recording())\
			{\
				try{\
					function(__VA_ARGS__);\
					__af_impl::report_no_exception_error(true, #function, STRINGIFY_VA_ARGS(__VA_ARGS__), \
						__FILE__, __LINE__, __af_conf::throw_on_error());\
				}\
				catch (std::exception const& e) {\
					__af_impl::report_throw_success(true, #function, __FILE__, __LINE__, STRINGIFY_VA_ARGS(__VA_ARGS__), e.what());\
				}\
				catch (...) {\
					__af_impl::report_throw_success(true, #function, __FILE__, __LINE__, STRINGIFY_VA_ARGS(__VA_ARGS__), "Unknown exception");\
				}\
			}\
			else\
			{\
				__af_impl::record_macro("AF_TEST_FUNCTION_THROWS", #function, STRINGIFY_VA_ARGS(__VA_ARGS__));\
			}\
		}

#define af_test_function_throws(function,...) AF_TEST_FUNCTION_THROWS(function,__VA_ARGS__)


// test that the result of the function with listed arguments is as expected
#define AF_TEST_FUNCTION_RESULT(expected_result, function,...) \
		{\
			using __af_conf = autotelica::testing::testing_config;\
			namespace __af_impl = autotelica::testing::testing_impl;\
			if(!__af_conf::is_run_mode_recording())\
			{\
				try{\
					auto __af_l = [&](){ return function(__VA_ARGS__) ;};\
					static_assert(std::is_convertible<decltype(expected_result),decltype(__af_l())>::value,\
						"AF TESTS ERROR: Expected and actual value are not of compatible types.");\
					decltype(__af_l()) converted_expected(expected_result);\
					__af_impl::report_result(converted_expected, __af_l(), true, #function, STRINGIFY_VA_ARGS(__VA_ARGS__),\
						__FILE__, __LINE__, __af_conf::throw_on_error());\
				}\
				catch (std::exception const& e) {\
					__af_impl::report_exception_error(true, #function, STRINGIFY_VA_ARGS(__VA_ARGS__), __FILE__, __LINE__, e.what(), __af_conf::throw_on_error());\
				}\
				catch (...) {\
					__af_impl::report_exception_error(true, #function, STRINGIFY_VA_ARGS(__VA_ARGS__), __FILE__, __LINE__, "Unknown exception", __af_conf::throw_on_error());\
				}\
			}\
			else\
			{\
				__af_impl::record_macro_with_result(function(__VA_ARGS__), "AF_TEST_FUNCTION_RESULT", #function, STRINGIFY_VA_ARGS(__VA_ARGS__));\
			}\
		}

#define af_test_function_result(expected_result, function,...) AF_TEST_FUNCTION_RESULT(expected_result, function,__VA_ARGS__)

// test that a code snippet runs without exceptions
#define AF_TEST( snippet ) \
		{\
			using __af_conf = autotelica::testing::testing_config;\
			namespace __af_impl = autotelica::testing::testing_impl;\
			if(!__af_conf::is_run_mode_recording())\
			{\
				try{\
					auto __af_l = [&](){ snippet ;};\
					__af_l();\
					__af_impl::report_success(false, #snippet, __FILE__, __LINE__);\
				}\
				catch (std::exception const& e) {\
					__af_impl::report_exception_error(false, #snippet, "", __FILE__, __LINE__, e.what(), __af_conf::throw_on_error()); \
				}\
				catch (...) {\
					__af_impl::report_exception_error(false, #snippet, "", __FILE__, __LINE__, "Unknown exception", __af_conf::throw_on_error());\
				}\
			}\
			else\
			{\
				__af_impl::record_macro("AF_TEST", #snippet);\
			}\
		}

#define af_test( snippet ) AF_TEST( snippet )


// test that a code snippet throws exceptions
#define AF_TEST_THROWS(snippet) \
		{\
			using __af_conf = autotelica::testing::testing_config;\
			namespace __af_impl = autotelica::testing::testing_impl;\
			if(!__af_conf::is_run_mode_recording())\
			{\
				try{\
					auto __af_l = [&](){ snippet ;};\
					__af_l();\
					__af_impl::report_no_exception_error(false, #snippet, "", __FILE__, __LINE__, __af_conf::throw_on_error());\
				}\
				catch (std::exception const& e) {\
					__af_impl::report_throw_success(false, #snippet, __FILE__, __LINE__, "", e.what());\
				}\
				catch (...) {\
					__af_impl::report_throw_success(false, #snippet, __FILE__, __LINE__, "", "Unknown exception");\
				}\
			}\
			else\
			{\
				__af_impl::record_macro("AF_TEST_THROWS", #snippet);\
			}\
		}

#define af_test_throws(snippet) AF_TEST_THROWS(snippet)

// test that the result of an expression is as expected
#define AF_TEST_RESULT( expected_result, expression ) \
		{\
			using __af_conf = autotelica::testing::testing_config;\
			namespace __af_impl = autotelica::testing::testing_impl;\
			if(!__af_conf::is_run_mode_recording())\
			{\
				try{\
					auto __af_l = [&](){ return (expression);};\
					static_assert(std::is_convertible<decltype(expected_result),decltype(__af_l())>::value,\
						"AF TESTS ERROR: Expected and actual value are not of compatible types.");\
					decltype(__af_l()) converted_expected(expected_result);\
					__af_impl::report_result(converted_expected, __af_l(), false, #expression, "",\
						__FILE__, __LINE__, __af_conf::throw_on_error());\
				}\
				catch (std::exception const& e) {\
					__af_impl::report_exception_error(false, #expression, "", __FILE__, __LINE__, e.what(), __af_conf::throw_on_error()); \
				}\
				catch (...) {\
					__af_impl::report_exception_error(false, #expression, "", __FILE__, __LINE__, "Unknown exception", __af_conf::throw_on_error());\
				}\
			}\
			else\
			{\
				auto __af_l = [&](){ return (expression);};\
				__af_impl::record_macro_with_result(__af_l(), "AF_TEST_RESULT", #expression);\
			}\
		}

#define af_test_result( expected_result, expression ) AF_TEST_RESULT( expected_result, expression ) 

// add a comment to the test output
#define AF_TEST_COMMENT( comment ) {\
		namespace __af_dm = autotelica::diagnostic_messages;\
		using __af_conf = autotelica::testing::testing_config;\
		namespace __af_impl = autotelica::testing::testing_impl;\
		if (__af_conf::is_run_mode_csv()){\
		}\
		else if(__af_conf::is_run_mode_recording()){\
				__af_dm::timestamp_disabler _t;\
				__af_dm::messages::message(\
					"\n\t//% \n\tAF_TEST_COMMENT( % );",\
					comment, #comment);\
		}\
		else{\
			__af_dm::print::underline(comment, true);\
		}\
	}

#define af_test_comment( comment ) AF_TEST_COMMENT( comment )

	}
}