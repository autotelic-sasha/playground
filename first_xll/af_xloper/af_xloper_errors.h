#pragma once
#include "af_xloper/af_xloper_util.h"
#include <functional>

#pragma warning ( disable : 26495)// known problem in visual studio, it doesn't like union constructors
namespace autotelica {
	namespace xloper {
	namespace errors {
		// there a few options on what to do with errors and empty values
		// all wrapped up in this namespace
		static LPXLOPER12 xlpError(DWORD errCode) {
			LPXLOPER12 out = inner::xl_memory::new_xloper12();
			inner::xl_type_ops::set_xl_type(*out, xltypeErr);
			out->val.err = errCode;
			return out;
		}

		class error_policy {
			// by default, all exceptions get transformed to #N/A
			// this helps spreadsheets work, because ISERROR function will detect these properly
			// but for debugging it may be helpful to display more information
			// then we can set the policy to show "rich" information
			// by returning strings prefixed by #ERR and containing exception text
			bool _rich_error_text; // when an exception is caught, return "ERR:" + exception.what()

			bool _propagate_errors; // when there is an error in the input XLOPER12, return it as function output
			bool _interpret_missing; // missing values are coerced to default values for types

			// disabling wizard calls is not strictly speaking about error handling
			// we have it here because it is really not worth adding complexity with a separate configuration class
			// but we will pretend that it's because slow execution is a kind of an error
			bool _disable_wizard_calls; // do not evaluate functions if invoked from the function wizard
		private:
			error_policy(
				bool rich_error_text_,
				bool propagate_errors_,
				bool interpret_missing_,
				bool disable_wizard_calls_) :
				_rich_error_text(rich_error_text_),
				_propagate_errors(propagate_errors_),
				_interpret_missing(interpret_missing_),
				_disable_wizard_calls(disable_wizard_calls_) {
			}
			static error_policy& instance() {
				static error_policy _instance{ false, true, true, true };
				return _instance;
			}
		public:
			static bool rich_error_text() {
				return instance()._rich_error_text;
			}
			static void set_rich_error_text(bool rich_error_text_) {
				instance()._rich_error_text = rich_error_text_;
			}
			static bool propagate_errors() {
				return instance()._propagate_errors;
			}
			static void set_propagate_errors(bool propagate_errors_) {
				instance()._propagate_errors = propagate_errors_;
			}
			static bool interpret_missing() {
				return instance()._interpret_missing;
			}
			static void set_interpret_missing(bool interpret_missing_) {
				instance()._interpret_missing = interpret_missing_;
			}
			static bool disable_wizard_calls() {
				return instance()._disable_wizard_calls;
			}
			static void set_disable_wizard_calls(bool disable_wizard_calls_) {
				instance()._disable_wizard_calls = disable_wizard_calls_;
			}
			static void set(
				bool rich_error_text_,
				bool propagate_errors_,
				bool interpret_missing_,
				bool disable_wizard_calls_) {
				set_rich_error_text(rich_error_text_);
				set_propagate_errors(propagate_errors_);
				set_interpret_missing(interpret_missing_);
				set_disable_wizard_calls(disable_wizard_calls_);
			}

			// to make these work more easily with excel, we tag the configuration parameters
			// helps a little in case we want to add something else later
			//										tag				getter							setter
			using tags_map = std::unordered_map < std::string, std::pair<std::function<bool()>, std::function<void(bool)>>>;

			static tags_map const& tags() {
				static tags_map _tags = {
					{"RichErrorText", {rich_error_text, set_rich_error_text}},
					{"PropagateErrors", {propagate_errors, set_propagate_errors}},
					{"InterpretMissing", {interpret_missing, set_interpret_missing}},
					{"DisableWizardCalls", {disable_wizard_calls, set_disable_wizard_calls}} };

				return _tags;
			}

		};
		// these are exposed by the XLL by default
		static bool af_xl_configure_error_policy(
			bool rich_error_text_,
			bool propagate_errors_,
			bool interpret_missing_,
			bool disable_wizard_calls_) {
			error_policy::set(rich_error_text_, propagate_errors_, interpret_missing_, disable_wizard_calls_);
			return true;
		}

		static std::unordered_map<std::string, bool> af_xl_display_error_policy() {
			std::unordered_map<std::string, bool>  out;
			for (auto const& tag : error_policy::tags()) {
				out[tag.first] = (tag.second.first)();
			}
			return out;
		}

		static std::unordered_map<std::string, bool>
			af_xl_configure_error_policy_ex(std::unordered_map<std::string, bool> const& policy_details) {
			for (auto const& in : policy_details) {
				auto it = error_policy::tags().find(in.first);
				if (it == error_policy::tags().end())
					throw std::runtime_error(std::string("Unknown configuration tag: ") + in.first);
				(it->second.second)(in.second);
			}
			return af_xl_display_error_policy();
		}


		// depending on the policy, we translate error text in various ways
		// that is all embedded in here
		static inline LPXLOPER12 translate_error(const char* const error_text) {
			static constexpr auto error_prefix = "#ERR: ";
			if (!error_policy::rich_error_text())
				return const_cast<LPXLOPER12>(&inner::xl_constants::xlNA());

			try {
				std::string err(error_text);
				err = error_prefix + err;
				return inner::xl_strings::xlpString(err);
			}
			catch (...) {
				return const_cast<LPXLOPER12>(&inner::xl_constants::xlNull());
			}
		}
		static LPXLOPER12 translate_error(std::exception const& error_exception) {
			return translate_error(error_exception.what());
		}
		static LPXLOPER12 translate_error() {
			return translate_error("Unknown error.");
		}

		// we handle errors by using exceptions later in the code
		// this is to wrap XLOPER12 errors, so we can propagate them
		class xloper_exception : public std::exception {
			LPXLOPER12 _errorXlp;
		public:
			xloper_exception(DWORD errCode) :_errorXlp(xlpError(errCode)) {}
			xloper_exception(const XLOPER12* const xlp) :_errorXlp(xlpError(xlp->val.err)) {}
			const char* what() const {
				switch (_errorXlp->val.err) {
				case xlerrNull: return "Excel Error: Null";
				case xlerrDiv0: return "Excel Error: Division by 0";
				case xlerrValue: return "Excel Error: Value error";
				case xlerrRef: return "Excel Error: Invalid Reference";
				case xlerrName: return "Excel Error: Invalid Name";
				case xlerrNum: return "Excel Error: Invalid Number";
				case xlerrNA: return "Excel Error: NA";
				case xlerrGettingData: return "Excel Error: Error Getting Data";
				default: return "Excel Error: Unknown Excel Error";
				}
			}
			LPXLOPER12 errorXlp() const {
				if (error_policy::propagate_errors())
					return _errorXlp;
				else
					return translate_error(*this);
			}
		};



	}

		
	namespace util {
		// finally, some common checks
		static void check_null_xlp(const XLOPER12* const xlp) {
			using namespace errors;
			if (!xlp)
				if (error_policy::rich_error_text())
					throw std::runtime_error("NULL input.");
				else
					throw errors::xloper_exception(&inner::xl_constants::xlNull());
		}
		static void check_input_xl(XLOPER12 const& xl) {
			using namespace errors;
			if (xl.xltype & xltypeErr) {
				if (error_policy::propagate_errors())
					throw xloper_exception(&xl);
				else if (error_policy::rich_error_text())
					throw std::runtime_error("Error in input.");
				else
					throw xloper_exception(&inner::xl_constants::xlNA());
			}
		}
		static void check_wizard() {
			using namespace errors;
			if (error_policy::disable_wizard_calls() &&
				util::called_from_wizard())
				throw xloper_exception(&inner::xl_constants::xlNA());
		}
	}

	}
}