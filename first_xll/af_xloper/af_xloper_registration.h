#pragma once
#include "af_xloper/af_xloper_signature.h"

namespace autotelica {
	namespace xloper {
	// xl_registration of functions depends on the function signature
	// the monster switch statement is ugly, but does it efficiently
	namespace xl_registration {
		struct xl_function_data {
			std::string const _function_xl_category;
			std::string const _function_xl_name;
			std::string const _function_name;
			std::string const _signature;
			std::string const _function_help;
			std::string const _argument_names;
			std::vector<std::string> const _arguments_help;

			void xlfRegister_impl(XLOPER12 const& xDLL, std::string const& function_category_) const {
				using namespace xl_conversions;
				using namespace xl_inner::xl_constants;
				using namespace xl_data;

				auto_pxl function_category{ to_xlp(function_category_) };
				auto_pxl function_xl_name{ to_xlp(_function_xl_name) };
				auto_pxl function_name{ to_xlp(_function_name) };
				auto_pxl signature{ to_xlp(_signature) };
				auto_pxl argument_names{ to_xlp(_argument_names) };
				auto_pxl function_help{ to_xlp(_function_help) };
				auto_pxl function_type{ to_xlp(1) };
				int total_params = 10 + (int)_arguments_help.size() + 1;//1 extra to fix excel trimming help strings
				XLOPER12 res;
				switch (_arguments_help.size()) {
				case 0:
					Excel12(xlfRegister, &res, total_params, (LPXLOPER12)&xDLL,
						function_name, signature, function_xl_name, argument_names, function_type, function_category,
						&xlMissing(), &xlMissing(), function_help, &xlEmptyString());
					break;
				case 1:
					Excel12(xlfRegister, &res, total_params, (LPXLOPER12)&xDLL,
						function_name, signature, function_xl_name, argument_names, function_type, function_category,
						&xlMissing(), &xlMissing(), function_help,
						auto_pxl(to_xlp(_arguments_help[0])), &xlEmptyString());
					break;
				case 2:
					Excel12(xlfRegister, &res, total_params, (LPXLOPER12)&xDLL,
						function_name, signature, function_xl_name, argument_names, function_type, function_category,
						&xlMissing(), &xlMissing(), function_help,
						auto_pxl(to_xlp(_arguments_help[0])), auto_pxl(to_xlp(_arguments_help[1])), &xlEmptyString());
					break;
				case 3:
					Excel12(xlfRegister, &res, total_params, (LPXLOPER12)&xDLL,
						function_name, signature, function_xl_name, argument_names, function_type, function_category,
						&xlMissing(), &xlMissing(), function_help,
						auto_pxl(to_xlp(_arguments_help[0])), auto_pxl(to_xlp(_arguments_help[1])),
						auto_pxl(to_xlp(_arguments_help[2])), &xlEmptyString());
					break;
				case 4:
					Excel12(xlfRegister, &res, total_params, (LPXLOPER12)&xDLL,
						function_name, signature, function_xl_name, argument_names, function_type, function_category,
						&xlMissing(), &xlMissing(), function_help,
						auto_pxl(to_xlp(_arguments_help[0])), auto_pxl(to_xlp(_arguments_help[1])),
						auto_pxl(to_xlp(_arguments_help[2])), auto_pxl(to_xlp(_arguments_help[3])), &xlEmptyString());
					break;
				case 5:
					Excel12(xlfRegister, &res, total_params, (LPXLOPER12)&xDLL,
						function_name, signature, function_xl_name, argument_names, function_type, function_category,
						&xlMissing(), &xlMissing(), function_help,
						auto_pxl(to_xlp(_arguments_help[0])), auto_pxl(to_xlp(_arguments_help[1])),
						auto_pxl(to_xlp(_arguments_help[2])), auto_pxl(to_xlp(_arguments_help[3])),
						auto_pxl(to_xlp(_arguments_help[4])), &xlEmptyString());
					break;
				case 6:
					Excel12(xlfRegister, &res, total_params, (LPXLOPER12)&xDLL,
						function_name, signature, function_xl_name, argument_names, function_type, function_category,
						&xlMissing(), &xlMissing(), function_help,
						auto_pxl(to_xlp(_arguments_help[0])), auto_pxl(to_xlp(_arguments_help[1])),
						auto_pxl(to_xlp(_arguments_help[2])), auto_pxl(to_xlp(_arguments_help[3])),
						auto_pxl(to_xlp(_arguments_help[4])), auto_pxl(to_xlp(_arguments_help[5])), &xlEmptyString());
					break;
				case 7:
					Excel12(xlfRegister, &res, total_params, (LPXLOPER12)&xDLL,
						function_name, signature, function_xl_name, argument_names, function_type, function_category,
						&xlMissing(), &xlMissing(), function_help,
						auto_pxl(to_xlp(_arguments_help[0])), auto_pxl(to_xlp(_arguments_help[1])),
						auto_pxl(to_xlp(_arguments_help[2])), auto_pxl(to_xlp(_arguments_help[3])),
						auto_pxl(to_xlp(_arguments_help[4])), auto_pxl(to_xlp(_arguments_help[5])),
						auto_pxl(to_xlp(_arguments_help[6])), &xlEmptyString());
					break;
				case 8:
					Excel12(xlfRegister, &res, total_params, (LPXLOPER12)&xDLL,
						function_name, signature, function_xl_name, argument_names, function_type, function_category,
						&xlMissing(), &xlMissing(), function_help,
						auto_pxl(to_xlp(_arguments_help[0])), auto_pxl(to_xlp(_arguments_help[1])),
						auto_pxl(to_xlp(_arguments_help[2])), auto_pxl(to_xlp(_arguments_help[3])),
						auto_pxl(to_xlp(_arguments_help[4])), auto_pxl(to_xlp(_arguments_help[5])),
						auto_pxl(to_xlp(_arguments_help[6])), auto_pxl(to_xlp(_arguments_help[7])), &xlEmptyString());
					break;
				case 9:
					Excel12(xlfRegister, &res, total_params, (LPXLOPER12)&xDLL,
						function_name, signature, function_xl_name, argument_names, function_type, function_category,
						&xlMissing(), &xlMissing(), function_help,
						auto_pxl(to_xlp(_arguments_help[0])), auto_pxl(to_xlp(_arguments_help[1])),
						auto_pxl(to_xlp(_arguments_help[2])), auto_pxl(to_xlp(_arguments_help[3])),
						auto_pxl(to_xlp(_arguments_help[4])), auto_pxl(to_xlp(_arguments_help[5])),
						auto_pxl(to_xlp(_arguments_help[6])), auto_pxl(to_xlp(_arguments_help[7])),
						auto_pxl(to_xlp(_arguments_help[8])), &xlEmptyString());
					break;
				case 10:
					Excel12(xlfRegister, &res, total_params, (LPXLOPER12)&xDLL,
						function_name, signature, function_xl_name, argument_names, function_type, function_category,
						&xlMissing(), &xlMissing(), function_help,
						auto_pxl(to_xlp(_arguments_help[0])), auto_pxl(to_xlp(_arguments_help[1])),
						auto_pxl(to_xlp(_arguments_help[2])), auto_pxl(to_xlp(_arguments_help[3])),
						auto_pxl(to_xlp(_arguments_help[4])), auto_pxl(to_xlp(_arguments_help[5])),
						auto_pxl(to_xlp(_arguments_help[6])), auto_pxl(to_xlp(_arguments_help[7])),
						auto_pxl(to_xlp(_arguments_help[8])), auto_pxl(to_xlp(_arguments_help[9])), &xlEmptyString());
					break;
				case 11:
					Excel12(xlfRegister, &res, total_params, (LPXLOPER12)&xDLL,
						function_name, signature, function_xl_name, argument_names, function_type, function_category,
						&xlMissing(), &xlMissing(), function_help,
						auto_pxl(to_xlp(_arguments_help[0])), auto_pxl(to_xlp(_arguments_help[1])),
						auto_pxl(to_xlp(_arguments_help[2])), auto_pxl(to_xlp(_arguments_help[3])),
						auto_pxl(to_xlp(_arguments_help[4])), auto_pxl(to_xlp(_arguments_help[5])),
						auto_pxl(to_xlp(_arguments_help[6])), auto_pxl(to_xlp(_arguments_help[7])),
						auto_pxl(to_xlp(_arguments_help[8])), auto_pxl(to_xlp(_arguments_help[9])),
						auto_pxl(to_xlp(_arguments_help[10])), &xlEmptyString());
					break;
				case 12:
					Excel12(xlfRegister, &res, total_params, (LPXLOPER12)&xDLL,
						function_name, signature, function_xl_name, argument_names, function_type, function_category,
						&xlMissing(), &xlMissing(), function_help,
						auto_pxl(to_xlp(_arguments_help[0])), auto_pxl(to_xlp(_arguments_help[1])),
						auto_pxl(to_xlp(_arguments_help[2])), auto_pxl(to_xlp(_arguments_help[3])),
						auto_pxl(to_xlp(_arguments_help[4])), auto_pxl(to_xlp(_arguments_help[5])),
						auto_pxl(to_xlp(_arguments_help[6])), auto_pxl(to_xlp(_arguments_help[7])),
						auto_pxl(to_xlp(_arguments_help[8])), auto_pxl(to_xlp(_arguments_help[9])),
						auto_pxl(to_xlp(_arguments_help[10])), auto_pxl(to_xlp(_arguments_help[11])), &xlEmptyString());
					break;
				default:
					break;

				}

			}
		};
		// registry of functions, this is how we make the single line macros work
		// all the parameters for xlfRegister for each function are stored here
		class xl_f_registry {
			std::vector<std::shared_ptr< xl_function_data > > _functions;
			std::string _function_category;
			xl_f_registry() {}
			static std::shared_ptr<xl_f_registry> instance() {
				static std::shared_ptr<xl_f_registry> _instance(new xl_f_registry());
				return _instance;
			}
		public:
			static bool set_function_category(std::string const& function_category) {
				instance()->_function_category = function_category;
				return true;
			}
			static std::string const& get_function_category() {
				return instance()->_function_category;
			}

			static bool register_function(
				std::string const& function_xl_category,
				std::string const& function_xl_name,
				std::string const& function_name,
				std::string const& signature,
				size_t const number_of_arguments,
				bool is_volatile = false,
				std::string const& function_help = "",
				std::string const& argument_names = "",
				std::vector<std::string> const& arguments_help = {}
			) {
				// excel hates fucntions that end in numbers
				std::string f_xl_name(function_xl_name);
				if (isdigit(f_xl_name.back()))
					f_xl_name += "_";
				// function help, argument names, and help strings arrive 
				// packaged in the variadic parameters: function_help, argument0_name, argument0_help, argument1_name, argument1_help ...
				// if not supplied, we can construct sensible defaults
				std::string f_help(function_help.empty() ? function_xl_name : function_help);
				std::string arg_names(argument_names);
				if (arg_names.empty()) {
					std::stringstream sout;
					for (size_t i = 0; i < number_of_arguments; ++i) {
						sout << "arg_" << i;
						if (i != number_of_arguments - 1)
							sout << ",";
					}
					arg_names = sout.str();
				}
				std::vector<std::string> args_help(arguments_help);
				if (args_help.empty()) {
					for (size_t i = 0; i < number_of_arguments; ++i) {
						std::stringstream sout;
						sout << "argument " << i;
						args_help.push_back(sout.str());
					}
				}

				std::shared_ptr<xl_function_data> fd(new xl_function_data{
					function_xl_category,
					f_xl_name,
					function_name,
					is_volatile ? (signature + "!") : signature,
					f_help,
					arg_names,
					args_help
					});
				instance()->_functions.push_back(fd);
				return true;
			}

			// implementation of xlAutoOpen
			static int xlAutoOpen_impl(void) {
				using namespace autotelica::xloper::xl_conversions;
				auto inst = instance();
				std::string f_category(inst->_function_category.empty() ? "af test functions" : inst->_function_category);
				XLOPER12 xDLL;
				Excel12(xlGetName, &xDLL, 0);
				for (auto const& func : inst->_functions) {
					auto const& category = (func->_function_xl_category == "__GLOBAL_CATEGORY__") ?
						f_category : func->_function_xl_category;
					func->xlfRegister_impl(xDLL, category);
				}
				Excel12(xlFree, 0, 1, (LPXLOPER12)&xDLL);
				return 1;
			}
		};

	}

	}
}