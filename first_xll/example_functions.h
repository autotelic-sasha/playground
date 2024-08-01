#pragma once
#include "af_xloper.h"
namespace af_xll_examples {
	// a bunch of things that we will expose to Excel in the file xll_exports
	
	// PODs: int, double, float, bool, short, unsigned short, size_t 
	int add2(int i, int j);
	int add3(int i0, int i1, int i2);
	int add4(int i0, int i1, int i2, int i3);
	int add5(int i0, int i1, int i2, int i3, int i4);
	int add6(int i0, int i1, int i2, int i3, int i4, int i5);
	int add7(int i0, int i1, int i2, int i3, int i4, int i5,
		int i6);
	int add8(int i0, int i1, int i2, int i3, int i4, int i5,
		int i6, int i7);
	int add9(int i0, int i1, int i2, int i3, int i4, int i5,
		int i6, int i7, int i8);
	int add10(int i0, int i1, int i2, int i3, int i4, int i5,
		int i6, int i7, int i8, int i9);
	int add11(int i0, int i1, int i2, int i3, int i4, int i5,
		int i6, int i7, int i8, int i9, int i10);
	int add12(int i0, int i1, int i2, int i3, int i4, int i5, 
		int i6, int i7, int i8, int i9, int i10, int i11 );

	inline double multiply2(float x, float y) { return x * y; }

	double multiply12(double x0, double x1, double x2, double x3, double x4, double x5, 
		double x6, double x7, double x8, double x9, double x10, double x11 );

	bool exclusive_or(bool b1, bool b2);
	// we will test other types by doing more interesting things that include more interesting other types
	
	// strings: std::string, std::wstring
	std::string take_right(std::string const& s, size_t length);
	std::wstring reverse(std::wstring const& s);
	inline size_t length(std::string const& s) { return s.size(); }
	inline std::string concatenate(std::string const& s1, std::string const s2) { return s1 + " " + s2; }
	inline std::string hello() { return "hello"; }
	
	// containers:
	// vectors and lists  of PODs and strings
	double sum_vector(std::vector<double> const& vec);
	std::vector<double> sum_vectors(std::vector<double>& v1, std::vector<double> const& v2);
	std::list<std::string> sort_strings(std::list<std::string> const& strings);
	bool any(std::vector<bool> const& bools);
	bool all(std::vector<bool> const& bools);

	// list of vectors and lists of PODs and strings
	// vectors of vectors and lists  of PODs and strings
	std::vector<int> sum_int_rows(std::vector <std::vector<int>> const& v);
	std::list<std::vector<double>> random_square(size_t side);// volatile
	std::list<std::list<std::string>> random_words(size_t side, std::vector<std::string> const& words);// volatile
	std::vector<std::list<std::string>> capitalise(std::vector<std::vector<std::string>> const& words);
	std::vector<std::vector<double>> multiply_matrices(std::vector<std::vector<double>> const& m1, std::vector<std::vector<double>> const& m2);

	// maps and unordered maps where both key and value are either a POD or a string
	std::map<std::string, std::string>
		sort_rows(std::unordered_map<std::string, std::string> const& in);

	std::map<std::string, bool>
		negate_rows(std::unordered_map<std::string, bool> const& in);

	 
	// maps and unordered maps where keys are PODs or strings and values are vectors or lists of PODs or strings
	std::map<std::string, double>
		sum_named_rows(
			std::map<std::string, std::vector<double>> const& in);

	std::unordered_map<std::string, std::string>
		concatenate_named_rows(
			std::map<std::string, std::list<std::string>> const& in);


	// dates
	std::string to_iso(double d);
	std::string format_date(double d, std::string const& format);

	using namespace autotelica::xloper::xl_util;
	// variants
	std::string to_string(xl_variant const& in);
	// containers of variants
	inline size_t count_list(std::list<xl_variant> const& l) { return l.size(); }
	// nvps
	xl_nvp make_nvp(std::vector<std::string> const& labels, std::vector<xl_variant> const& values);

	// tables
	xl_table_cs add_count(xl_table_cs const& in);
	xl_table extract_columns(xl_table const& in, std::vector<std::string> const& column_names);
	// fast_array
	xl_fast_array fast_add_matrices(xl_fast_array const& m1, xl_fast_array const& m2);

	/// utilities 
	// object caches
	std::string create_plant(
		std::string const& name,
		size_t fragrancy_level,
		std::string const& colour,
		size_t spread_side, 
		std::string const season);

	std::string create_plant_from_nvp(xl_nvp const& values);

	xl_nvp display_plant(std::string const& name);
	std::string describe_plant(std::string const& name);

	std::string generate_garden(std::string const& name, size_t const& width, size_t const& length);
	xl_table show_garden(std::string const& name);
	 
	// alert
	std::string say_hello(std::string const& name);
	// transposing

	// error handling

	
	// extended functionality - your own additions
	LPXLOPER12 transpose_and_merge(LPXLOPER12 m1, LPXLOPER12 m2);
};