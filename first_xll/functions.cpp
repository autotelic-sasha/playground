//#include "pch.h"
#include "first_xll_macros.h"
#include <string>
#include "af_xloper.h"
#include <numeric>

//using namespace autotelica::xloper;
//	AF_DECLARE_XLL("Autotelica Testing");

	autotelica::xloper::xl_fast_array::xl_fast_array big_array(double v) {
		autotelica::xloper::xl_fast_array::xl_fast_array ret(100, 100, v);
		return ret;
	}
	AF_DECLARE_EXCEL_FUNCTION(
		big_array,
		"makes a big fast grid",
		v, "value to populate the grid with");

	autotelica::xloper::xl_fast_array::xl_fast_array big_array_add(double v, autotelica::xloper::xl_fast_array::xl_fast_array in) {
		if (in.empty())
			return in;

		for (auto i = in.size() - 1; i >= 0; --i)
			in.values()[i] += v;
		return in;
	}
	AF_DECLARE_EXCEL_FUNCTION(
		big_array_add,
		"increments a big fast grid",
		v, "value to add to the grid",
		grid, "grid to increment");

	std::vector<int> sum_rows(std::vector <std::vector<int>> const& v) {
		std::vector<int> out;
		for (auto const vv : v) {
			int sum = std::accumulate(vv.begin(), vv.end(), 0);
			out.push_back(sum);
		}
		return out;
	}
	AF_DECLARE_EXCEL_FUNCTION(
		sum_rows,
		"returns sum of rows",
		grid, "in grid");


	std::map<std::string, int> 
		sum_rows_m(
			std::map<std::string, std::vector<int>> const& in) {
		std::map<std::string, int> out;
		for (auto const& vv : in) {
			int s = std::accumulate(vv.second.begin(), vv.second.end(), 0);
			out[vv.first] = s;
		}
		return out;
	}
	AF_DECLARE_EXCEL_FUNCTION(
		sum_rows_m,
		"returns sums of rows",
		grid, "in grid");


	//AF_DECLARE_PUBLIC_FUNCTION(
	//	multiply_matrices,
	//	"Fast matrix multiplication",
	//	LHS, "lhs matrix",
	//	RHS, "rhs matrix");

	using namespace autotelica::xloper;
	using namespace autotelica::xloper::xl_util;
	AF_DECLARE_LAMBDA_EXCEL_FUNCTION(
		sum_rows_mt, 
		[](std::map<std::string, std::vector<int>> const& in) {
			return xl_transpose(sum_rows_m(in)); 
		}, 
		"returns sums of rows transposed", 
		grid, 
		"grid");


	std::map<std::string, std::vector<int>> 
		sum_rows_m_append(
			std::map<std::string, std::vector<int>> const& in) {
		std::map<std::string, std::vector<int>> out;
		out = in;
		for (auto const& vv : in) {
			int s = std::accumulate(vv.second.begin(), vv.second.end(), 0);
			out[vv.first].push_back(s);
		}
		return out;
	}
	AF_DECLARE_EXCEL_FUNCTION(
		sum_rows_m_append,
		"returns sums of rows",
		grid, "in grid");

	AF_DECLARE_EXCEL_NAMED_FUNCTION(
		matrix_row_sum, 
		sum_rows_m_append,
		"returns sums of rows",
		grid, "in grid");

	AF_DECLARE_LAMBDA_EXCEL_FUNCTION(
		sum_rows_m_append_t,
		[](std::map<std::string, std::vector<int>> const& in) {
			return xl_transpose(sum_rows_m_append(in));
		},
		"returns sums of rows transposed",
		grid,
		"grid");
	std::vector<int> inc_v(std::vector<int> const& v) {
		std::vector<int> out;
		for (auto const vv : v)
			out.push_back(vv + 1);
		return out;
	}
	AF_DECLARE_EXCEL_FUNCTION(
		inc_v,
		"increments a vector",
		v, "in v");

	int simple(int x) {
		return x+1;
	}
	AF_DECLARE_EXCEL_FUNCTION(
		simple,
		"adds three integers",
		x, "first integer");

	int add_four1(int x, int y, int z, int x1) {
		return x + y + z+x1;
	}
	AF_DECLARE_EXCEL_FUNCTION(
		add_four1,
		"adds four integers",
		x, "first integer",
		y, "second integer",
		z, "third integer",
		x1, "fourth integer" );


	int addthree(int x, int y, int z) {
		return x + y + z;
	}
	AF_DECLARE_EXCEL_FUNCTION(
		addthree,
		"adds three integers",
		x, "first integer",
		y, "second integer",
		z, "third integer");

	int add(int x, int y) {
		using namespace autotelica::xloper;
		// in this case it's faster to not check, so just as an example
		if (xl_util::called_from_wizard())
			return 0;
		return x + y;
	}

	AF_DECLARE_EXCEL_FUNCTION(add,"adds two integers", x, "first integer", y , "second integer");

	int add3(int x, int y, int z) {
		return x + y + z;
	}
	AF_DECLARE_QD_EXCEL_FUNCTION(add3, 3);

	int mult(int x, int y) {
		return x * y * 3;
	}
	AF_DECLARE_QD_EXCEL_FUNCTION(mult, 2);

	using namespace autotelica::xloper::xl_util;
	AF_DECLARE_LAMBDA_EXCEL_FUNCTION(l_add, [](int i, int j) {return i + j; }, "adds two integers", i, "first integer", j, "second integer");
	AF_DECLARE_QD_LAMBDA_EXCEL_FUNCTION(t_inc_v, [](std::vector<int> v) {return xl_transpose(inc_v(v)); }, 1);
	//FIRSTXLL_API  
	//	traits::xl_type_trait<int>::xl_target_type
	//	add2(
	//		traits::xl_type_trait<int>::xl_target_type x,
	//		traits::xl_type_trait<int>::xl_target_type y) {
	//	return add(x, y);
	//}
	//FIRSTXLL_API void WINAPI xlAutoAutoFree12(LPXLOPER12 pXL) {
	//	autotelica::xloper::memory::freeXL(pXL);
	//}
	//FIRSTXLL_API int WINAPI xlAutoOpen(void)
	//{
	//	return autotelica::xloper::registration::xl_f_registry::xlAutoOpen_impl();
	//	//using namespace autotelica::xloper::conversions;
	//	//using namespace autotelica::xloper::memory;
	//	////auto sig = autotelica::xloper::signature::get_signature(add);
	//	////std::string type_string(sig->type_string());
	//	//std::string type_string = autotelica::xloper::signature::get_signature_string(add);
	//	//XLOPER12 xDLL;


	//	//Excel12(xlGetName, &xDLL, 0);
	//	//LPXLOPER12 function_name = to_xl("add_af_impl");

	//	//LPXLOPER12 function_text = to_xl("add");
	//	//LPXLOPER12 signature = to_xl("QJJ");
	//	//LPXLOPER12 argument_names = to_xl("x,y");
	//	//LPXLOPER12 function_type = to_xl(1);// function type, 1 for function, 2 for command
	//	//XLOPER12 xlMissing;
	//	//xlMissing.xltype = xltypeMissing;
	//	//LPXLOPER12 function_category = to_xl("autotelica testing");
	//	//LPXLOPER12 function_help = to_xl("adds two numbers");
	//	//LPXLOPER12 arg1_help = to_xl("first number ");
	//	//LPXLOPER12 arg2_help = to_xl("second number ");
	//	////for (i = 0; i < rgFuncsRows; i++)
	//	////{
	//	//Excel12(xlfRegister, 0, 12,
	//	//	(LPXLOPER12)&xDLL,
	//	//	function_name,
	//	//	signature,
	//	//	function_text,
	//	//	argument_names,
	//	//	function_type, 
	//	//	function_category, // user defined function category
	//	//	&xlMissing, // always nil or missing
	//	//	&xlMissing, // help topic
	//	//	function_help, 
	//	//	arg1_help,
	//	//	arg2_help
	//	//	);
	//	////}

	//	//Excel12(xlFree, 0, 1, (LPXLOPER12)&xDLL);
	//	//freeXL(signature);
	//	//freeXL(function_name);
	//	//freeXL(argument_names);
	//	//freeXL(function_type);
	//	//freeXL(function_help);
	//	//freeXL(function_category);
	//	//freeXL(arg1_help);
	//	//freeXL(arg2_help);
	//	
	//	//return 1;
	//}
