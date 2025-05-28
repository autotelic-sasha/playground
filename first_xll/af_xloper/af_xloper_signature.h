#pragma once
#include "af_xloper/af_xloper_traits.h"

namespace autotelica {
	namespace xloper {

	namespace xl_signature {
		// deducing the signature of functions that we are exposing
		// and building Excel type strings to register these functions is a bit of template magic

		template<typename RetT>
		void append_to_signature_string(std::string& out) {
			xl_traits::append_xl_type_string<RetT>(out);
		}
		template<typename RetT, typename Arg1T, typename ... ArgTs>
		void append_to_signature_string(std::string& out) {
			xl_traits::append_xl_type_string<RetT>(out);
			append_to_signature_string<Arg1T, ArgTs...>(out);
		}
		template<typename RetT>
		std::string get_signature_string(RetT(*)()) {
			std::string ret;
			xl_traits::append_xl_ret_type_string <RetT>(ret);
			return ret;
		}
		template<typename RetT, typename ... ArgTs>
		std::string get_signature_string(RetT(*)(ArgTs...)) {
			std::string ret;
			xl_traits::append_xl_ret_type_string <RetT>(ret);
			append_to_signature_string<ArgTs...>(ret);
			return ret;
		}

		// quick and dirty functions and lambdas only ever deal with XLOPER12s, in and out
		template<typename RetT>
		void append_to_signature_string_qd(std::string& out) {
			out += "Q";
		}
		template<typename RetT, typename Arg1T, typename ... ArgTs>
		void append_to_signature_string_qd(std::string& out) {
			out += "Q";
			append_to_signature_string_qd<Arg1T, ArgTs...>(out);
		}
		template<typename RetT>
		std::string get_signature_string_qd(RetT(*)()) {
			std::string ret("Q");
			return ret;
		}
		template<typename RetT, typename ... ArgTs>
		std::string get_signature_string_qd(RetT(*)(ArgTs...)) {
			std::string ret("Q");
			append_to_signature_string_qd<ArgTs...>(ret);
			return ret;
		}
		template<typename RetT, typename Class>
		std::string get_lambda_signature_string_impl(RetT(Class::*)() const) {
			std::string ret("Q");
			return ret;
		}
		template<typename RetT, typename Class, typename ... ArgTs>
		std::string get_lambda_signature_string_impl(RetT(Class::*)(ArgTs...) const) {
			std::string ret("Q");
			append_to_signature_string_qd<ArgTs...>(ret);
			return ret;
		}
		template<typename LambdaT>
		std::string get_lambda_signature_string() {
			return get_lambda_signature_string_impl(&LambdaT::operator());
		}

		// magical template magic to deduce return and parameter types
		template<typename T> struct func_types;

		template<typename R>
		struct func_types<R(*)(void)> {
			using ReturnType = R;
			static constexpr size_t number_of_arguments = 0;
		};

		template<typename R, typename ... ArgTs>
		struct func_types<R(*)(ArgTs...)> {
			using ReturnType = R;
			static constexpr size_t number_of_arguments = sizeof...(ArgTs);

			using TypesTuple = std::tuple<ArgTs...>;
			template <int N>
			using ArgType = typename std::tuple_element<N, TypesTuple>::type;
		};

		template<typename T> struct mem_func_types;

		template<typename R, typename Class>
		struct mem_func_types<R(Class::*)(void) const> {
			using ReturnType = R;
			const size_t number_of_arguments = 0;
		};

		template<typename R, typename Class, typename ... ArgTs>
		struct mem_func_types<R(Class::*)(ArgTs...) const> {
			using ReturnType = R;
			static constexpr size_t number_of_arguments = sizeof...(ArgTs);

			using TypesTuple = std::tuple<ArgTs...>;
			template <int N>
			using ArgType = typename std::tuple_element<N, TypesTuple>::type;
		};

		template<typename Lambda>
		using lambda_func_types = mem_func_types<decltype(&Lambda::operator())>;
	}

	}
}