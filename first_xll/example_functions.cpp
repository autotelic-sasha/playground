#include "pch.h"
#include "example_functions.h"
#include <algorithm>
#include <random>
#include <numeric>
// a,b,c1,m2 ... matrices
// basic arithmetic: +, -, *
// matrix operations: |m| - determinant, mT - transpose, m^-1 (or m^) - inverse

namespace af_xll_examples {
	// a bunch of things that we will expose to Excel in the file xll_exports

	// PODs: int, double, float, bool, short, unsigned short, size_t 
	int add2(int i, int j) {
		return i + j;
	}
	int add3(int i0, int i1, int i2) {
		return i0 + i1 + i2;
	}
	int add4(int i0, int i1, int i2, int i3) {
		return i0 + i1 + i2 + i3;
	}
	int add5(int i0, int i1, int i2, int i3, int i4) {
		return i0 + i1 + i2 + i3 + i4;
	}
	int add6(int i0, int i1, int i2, int i3, int i4, int i5) {
		return i0 + i1 + i2 + i3 + i4 + i5;
	}
	int add7(int i0, int i1, int i2, int i3, int i4, int i5,
		int i6) {
		return i0 + i1 + i2 + i3 + i4 + i5 + i6;
	}
	int add8(int i0, int i1, int i2, int i3, int i4, int i5,
		int i6, int i7) {
		return i0 + i1 + i2 + i3 + i4 + i5 + i6 + i7;
	}
	int add9(int i0, int i1, int i2, int i3, int i4, int i5,
		int i6, int i7, int i8) {
		return i0 + i1 + i2 + i3 + i4 + i5 + i6 + i7 + i8;
	}
	int add10(int i0, int i1, int i2, int i3, int i4, int i5,
		int i6, int i7, int i8, int i9) {
		return i0 + i1 + i2 + i3 + i4 + i5 + i6 + i7 + i8 + i9;
	}
	int add11(int i0, int i1, int i2, int i3, int i4, int i5,
		int i6, int i7, int i8, int i9, int i10) {
		return i0 + i1 + i2 + i3 + i4 + i5 + i6 + i7 + i8 + i9 + i10;
	}
	int add12(int i0, int i1, int i2, int i3, int i4, int i5,
		int i6, int i7, int i8, int i9, int i10, int i11) {
		return i0 + i1 + i2 + i3 + i4 + i5 + i6 + i7 + i8 + i9 + i10 + i11;
	}

	double multiply12(double x0, double x1, double x2, double x3, double x4, double x5,
		double x6, double x7, double x8, double x9, double x10, double x11) {
		return x0 * x1 * x2 * x3 * x4 * x5 * x6 * x7 * x8 * x9 * x10 * x11;
	}

	bool exclusive_or(bool b1, bool b2) {
		return b1 ^ b2;
	}

	// strings: std::string, std::wstring
	std::string take_right(std::string const& s, size_t length) {
		if (length >= s.size()) throw std::runtime_error("Not enough characters.");
		return s.substr(0, length);
	}
	std::wstring reverse(std::wstring const& s) {
		std::wstring ret(s.rbegin(), s.rend());
		return ret;
	}


	// containers:
	// vectors and lists  of PODs and strings
	double sum_vector(std::vector<double> const& vec) {
		return std::accumulate(vec.begin(), vec.end(), 0.0);
	}
	std::vector<double> sum_vectors(std::vector<double>& v1, std::vector<double> const& v2) {
		std::vector<double> out;
		size_t all(v1.size() > v2.size() ? v1.size() : v2.size());
		out.reserve(all);
		for (size_t i = 0; i < all; ++i) {
			double x = i >= v1.size() ? 0.0 : v1[i];
			double y = i >= v2.size() ? 0.0 : v2[i];
			out.push_back(x + y);
		}
		return out;
	}
	std::list<std::string> sort_strings(std::list<std::string> const& strings) {
		std::list<std::string> ret{ strings };
		ret.sort();
		return ret;
	}
	bool any(std::vector<bool> const& bools) {
		for (auto const& b : bools)
			if (b) return true;
		return false;
	}
	bool all(std::vector<bool> const& bools) {
		for (auto const& b : bools)
			if (!b) return false;
		return true;
	}


	// list of vectors and lists of PODs and strings
	// vectors of vectors and lists  of PODs and strings
	std::vector<int> sum_int_rows(std::vector<std::vector<int>> const& v) {
		std::vector<int> ret( v.size());
		for (size_t i = 0; i < v.size(); ++i)
			ret[i] = std::accumulate(v[i].begin(), v[i].end(), 0);
		return ret;
	}
	std::list<std::vector<double>> random_square(size_t side) {
		std::list<std::vector<double>> ret;
		std::random_device r;
		std::uniform_real_distribution<double> u(-1000000, 1000000);
		for (size_t i = 0; i < side; ++i) {
			ret.push_back({});
			std::vector<double>& back = ret.back();
			back.reserve(side);
			for (size_t j = 0; j < side; ++j) {
				back.push_back(u(r));
			}
		}
		return ret;
	}
	std::list<std::list<std::string>> random_words(size_t side, std::vector<std::string> const& words) {
		std::list<std::list<std::string>> ret;
		std::random_device r;
		std::uniform_int_distribution<size_t> u(0, words.size());
		for (size_t i = 0; i < side; ++i) {
			ret.push_back({});
			auto& back = ret.back();
			for (size_t j = 0; j < side; ++j)
				back.push_back(words[u(r)]);
		}
		return ret;
	}

	std::vector<std::list<std::string>> capitalise(std::vector<std::vector<std::string>> const& words) {
		std::vector<std::list<std::string>> ret;
		for (auto const& row : words) {
			ret.push_back({});
			auto& back = ret.back();
			for (auto const& word : row) {
				std::string cword;
				for (size_t i = 0; i < word.size(); ++i){
					if(i==0)
						cword.push_back(std::toupper(word[i]));
					else
						cword.push_back(std::tolower(word[i]));
				}
				back.push_back(cword);
			}
		}
		return ret;
	}
	std::vector<std::vector<double>> multiply_matrices(
		std::vector<std::vector<double>> const& m1,
		std::vector<std::vector<double>> const& m2) {

		size_t r1 = m1.size();
		size_t c1 = 0;
		for (auto const& row1 : m1) {
			if (row1.size() != c1) {
				if (c1 == 0)
					c1 = row1.size();
				else
					throw std::runtime_error("Matrices must be square, m1 isn't.");
			}
		}
		size_t r2 = m2.size();
		size_t c2 = 0;
		for (auto const& row2 : m2) {
			if (row2.size() != c2) {
				if (c2 == 0)
					c2 = row2.size();
				else
					throw std::runtime_error("Matrices must be square, m2 isn't.");
			}
		}
		if(c1 != r2)
			throw std::runtime_error("These matrices can't be multiplied.");

		std::vector<std::vector<double>> ret;
		for (size_t i = 0; i < r1; ++i) {
			ret.push_back({});
			auto& back = ret.back();
			back.resize(c2);
			for (size_t j = 0; j < c2; ++j) {
				back[j] = 0;
				for (size_t k = 0; k < c1; ++k) {
					back[j] += m1[i][k] * m2[k][j];
				}
			}
		}
		return ret;
	}

	// maps and unordered maps where both key and value are either a POD or a string
	std::map<std::string, std::string>
		sort_rows(std::unordered_map<std::string, std::string> const& in) {
		std::map<std::string, std::string> ret; // red-black tree map is sorted by construction
		for (auto const& e : in)
			ret[e.first] = e.second;
		return ret;
	}

	std::map<std::string, bool>
		negate_rows(std::unordered_map<std::string, bool> const& in) {
		std::map<std::string, bool> ret;
		for (auto const& e : in)
			ret[e.first] = !e.second;
		return ret;
	}


	// maps and unordered maps where keys are PODs or strings and values are vectors or lists of PODs or strings
	std::map<std::string, double>
		sum_named_rows(
			std::map<std::string, std::vector<double>> const& in) {
		std::map<std::string, double> out;
		for (auto const& vv : in) {
			out[vv.first]= std::accumulate(vv.second.begin(), vv.second.end(), 0.0);
		}
		return out;
	}

	std::unordered_map<std::string, std::string>
		concatenate_named_rows(
			std::map<std::string, std::list<std::string>> const& in) {
		std::unordered_map<std::string, std::string> ret;
		for (auto const& e : in) {
			ret[e.first] = std::accumulate(e.second.begin(), e.second.end(), std::string(), 
				[](std::string const& l, std::string const& r) { return l + std::string(" ") + r; });
		}
		return ret;
	}


	// dates
	std::string to_iso(double d) {
		return autotelica::xloper::xl_util::xl_date::xl_date_to_iso8601_date(d);
	}
	std::string format_date(double d, std::string const& format) {
		return autotelica::xloper::xl_util::xl_date::format_xl_date(d, format);
	}

	
	// variants
	std::string to_string(xl_variant const& in) {
		std::string ret;
		switch(in.type()) {
		case xl_variant::xl_type::xl_typeNum:
		case xl_variant::xl_type::xl_typeStr:
		case xl_variant::xl_type::xl_typeInt:
			ret = in.to_string();
			break;
		case xl_variant::xl_type::xl_typeBool:
			ret = in.get_bool() ? "true" : "false";
			break;
		case xl_variant::xl_type::xl_typeErr:
			ret = "ERROR: ";
			ret += std::to_string(in.get_error());
			break;
			ret = std::to_string(in.get_int());
			break;
		case xl_variant::xl_type::xl_typeMulti: {
			std::stringstream str;
			str << "{";
			auto const& multi = in.get_multi();
			for (size_t r = 0; r < multi.size(); ++r) {
				auto const& row = multi[r];
				str << "{";
				for (size_t c = 0; c < row.size(); ++c) {
					str << to_string(row[c]);
					if (c < row.size() - 1) str << ",";
				}
				str << "}";
				if (r < multi.size() - 1) str << ",";
			}
			str << "}";
			ret = str.str();
			break;
		}
		case xl_variant::xl_type::xl_typeMissing:
			ret = "MISSING";
			break;
		case xl_variant::xl_type::xl_typeNil:
			ret = "NUL";
			break;
		case xl_variant::xl_type::xl_typeNone:
			ret = "NONE";
			break;
		default:
			ret = "UNKNOWN";
			break;
		}
		return ret;
	}
	// nvps
	xl_nvp make_nvp(std::vector<std::string> const& labels, std::vector<xl_variant> const& values) {
		if (labels.size() != values.size())
			throw std::runtime_error("Labels and values must be of same size.");
		xl_nvp ret;
		for (size_t i = 0; i < labels.size(); ++i) {
			ret.add(labels[i], values[i]);
		}
		return ret;
	}

	// tables
	xl_table_cs add_count(xl_table_cs const& in) {
		xl_table_cs ret(in);
		ret.headings().push_back("Count");
		for (size_t i = 0; i < ret.rows(); ++i) {
			int count = 0;
			for (auto const& value : ret.row(i)) {
				if (value.type() != xl_variant::xl_type::xl_typeErr &&
					value.type() != xl_variant::xl_type::xl_typeMissing &&
					value.type() != xl_variant::xl_type::xl_typeNil &&
					value.type() != xl_variant::xl_type::xl_typeNone)
					++count;
			}
			ret.row(i).push_back(xl_variant(count));
		}
		return ret;
	}
	xl_table extract_columns(xl_table const& in, std::vector<std::string> const& column_names) {
		xl_table ret;
		for (auto const& col_name : column_names) {
			auto cols = in.get_column(col_name);
			for (auto const& col : cols)
				ret.add_column(col);
		}
		return ret;
	}
	// fast_array
	xl_fast_array fast_add_matrices(xl_fast_array m1, xl_fast_array m2) {
		if (m1.size() != m2.size())
			throw std::runtime_error("Matrices must be of same size.");
		for (int64_t i = 0; i < m1.size(); ++i)
			m1.unsafe_get(i) += m2.unsafe_get(i);
		return m1;
	}

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
	std::string say_hello(std::string const& name) {
		std::string hello("Hello ");
		hello += name;
		autotelica::xloper::xl_util::alert(hello, "Hi");
		return hello;
	}
	// transposing

	// error handling


	// extended functionality - your own additions
	LPXLOPER12 transpose_and_merge(LPXLOPER12 m1, LPXLOPER12 m2);
};