#pragma once
#include <string>
#include <sstream>
#include <algorithm>
#include <functional>
#include <cstring>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <vector>
#include <type_traits>

namespace autotelica {
	namespace string_util {
		// fast conversion to uppercase (doesn't use locales)
		static char fast_to_upper(char c) {
			switch (c) {
			case 'a': return 'A';
			case 'b': return 'B';
			case 'c': return 'C';
			case 'd': return 'D';
			case 'e': return 'E';
			case 'f': return 'F';
			case 'g': return 'G';
			case 'h': return 'H';
			case 'i': return 'I';
			case 'j': return 'J';
			case 'k': return 'K';
			case 'l': return 'L';
			case 'm': return 'M';
			case 'n': return 'N';
			case 'o': return 'O';
			case 'p': return 'P';
			case 'q': return 'Q';
			case 'r': return 'R';
			case 's': return 'S';
			case 't': return 'T';
			case 'u': return 'U';
			case 'v': return 'V';
			case 'w': return 'W';
			case 'x': return 'X';
			case 'y': return 'Y';
			case 'z': return 'Z';
			default: return c;
			}
		}
		
		// fast conversion to lowercase (doesn't use locales)
		static char fast_to_lower(char c) {
			switch (c) {
			case 'A': return 'a';
			case 'B': return 'b';
			case 'C': return 'c';
			case 'D': return 'd';
			case 'E': return 'e';
			case 'F': return 'f';
			case 'G': return 'g';
			case 'H': return 'h';
			case 'I': return 'i';
			case 'J': return 'j';
			case 'K': return 'k';
			case 'L': return 'l';
			case 'M': return 'm';
			case 'N': return 'n';
			case 'O': return 'o';
			case 'P': return 'p';
			case 'Q': return 'q';
			case 'R': return 'r';
			case 'S': return 's';
			case 'T': return 't';
			case 'U': return 'u';
			case 'V': return 'v';
			case 'W': return 'w';
			case 'X': return 'x';
			case 'Y': return 'y';
			case 'Z': return 'z';
			default: return c;
			}
		}

		// fast case insesitive equals (doesn't use locales)
		static bool fast_no_case_equal(char c1, char c2) {
			switch (c1) {
			case 'a': return c2 == 'a' || c2 == 'A';
			case 'b': return c2 == 'b' || c2 == 'B';
			case 'c': return c2 == 'c' || c2 == 'C';
			case 'd': return c2 == 'd' || c2 == 'D';
			case 'e': return c2 == 'e' || c2 == 'E';
			case 'f': return c2 == 'f' || c2 == 'F';
			case 'g': return c2 == 'g' || c2 == 'G';
			case 'h': return c2 == 'h' || c2 == 'H';
			case 'i': return c2 == 'i' || c2 == 'I';
			case 'j': return c2 == 'j' || c2 == 'J';
			case 'k': return c2 == 'k' || c2 == 'K';
			case 'l': return c2 == 'l' || c2 == 'L';
			case 'm': return c2 == 'm' || c2 == 'M';
			case 'n': return c2 == 'n' || c2 == 'N';
			case 'o': return c2 == 'o' || c2 == 'O';
			case 'p': return c2 == 'p' || c2 == 'P';
			case 'q': return c2 == 'q' || c2 == 'Q';
			case 'r': return c2 == 'r' || c2 == 'R';
			case 's': return c2 == 's' || c2 == 'S';
			case 't': return c2 == 't' || c2 == 'T';
			case 'u': return c2 == 'u' || c2 == 'U';
			case 'v': return c2 == 'v' || c2 == 'V';
			case 'w': return c2 == 'w' || c2 == 'W';
			case 'x': return c2 == 'x' || c2 == 'X';
			case 'y': return c2 == 'y' || c2 == 'Y';
			case 'z': return c2 == 'z' || c2 == 'Z';
			case 'A': return c2 == 'a' || c2 == 'A';
			case 'B': return c2 == 'b' || c2 == 'B';
			case 'C': return c2 == 'c' || c2 == 'C';
			case 'D': return c2 == 'd' || c2 == 'D';
			case 'E': return c2 == 'e' || c2 == 'E';
			case 'F': return c2 == 'f' || c2 == 'F';
			case 'G': return c2 == 'g' || c2 == 'G';
			case 'H': return c2 == 'h' || c2 == 'H';
			case 'I': return c2 == 'i' || c2 == 'I';
			case 'J': return c2 == 'j' || c2 == 'J';
			case 'K': return c2 == 'k' || c2 == 'K';
			case 'L': return c2 == 'l' || c2 == 'L';
			case 'M': return c2 == 'm' || c2 == 'M';
			case 'N': return c2 == 'n' || c2 == 'N';
			case 'O': return c2 == 'o' || c2 == 'O';
			case 'P': return c2 == 'p' || c2 == 'P';
			case 'Q': return c2 == 'q' || c2 == 'Q';
			case 'R': return c2 == 'r' || c2 == 'R';
			case 'S': return c2 == 's' || c2 == 'S';
			case 'T': return c2 == 't' || c2 == 'T';
			case 'U': return c2 == 'u' || c2 == 'U';
			case 'V': return c2 == 'v' || c2 == 'V';
			case 'W': return c2 == 'w' || c2 == 'W';
			case 'X': return c2 == 'x' || c2 == 'X';
			case 'Y': return c2 == 'y' || c2 == 'Y';
			case 'Z': return c2 == 'z' || c2 == 'Z';
			default: return c1 == c2;
			}
		}

		// fast is_lower (doesn't use locales)
		static bool fast_is_lower(char c) {
			switch (c) {
			case 'a':
			case 'b':
			case 'c':
			case 'd':
			case 'e':
			case 'f':
			case 'g':
			case 'h':
			case 'i':
			case 'j':
			case 'k':
			case 'l':
			case 'm':
			case 'n':
			case 'o':
			case 'p':
			case 'q':
			case 'r':
			case 's':
			case 't':
			case 'u':
			case 'v':
			case 'w':
			case 'x':
			case 'y':
			case 'z':
				return true;
			default:
				return false;
			}
		}
		
		// fast is_upper (doesn't use locales)
		static bool fast_is_upper(char c) {
			switch (c) {
			case 'A':
			case 'B':
			case 'C':
			case 'D':
			case 'E':
			case 'F':
			case 'G':
			case 'H':
			case 'I':
			case 'J':
			case 'K':
			case 'L':
			case 'M':
			case 'N':
			case 'O':
			case 'P':
			case 'Q':
			case 'R':
			case 'S':
			case 'T':
			case 'U':
			case 'V':
			case 'W':
			case 'X':
			case 'Y':
			case 'Z':
				return true;
			default:
				return false;
			}
		}
		inline bool is_lowercase(std::string const& name) {
			return std::all_of(name.begin(), name.end(), [](const char c) {return fast_to_lower(c) == c; });
		}
		inline bool is_uppercase(std::string const& name) {
			return std::all_of(name.begin(), name.end(), [](const char c) {return fast_to_upper(c) == c; });
		}

		// fast is_space (doesn't use locales)
		static bool fast_is_space(char c) {
			switch (c) {
			case ' ':
			case '\t':
			case '\v':
			case '\n':
			case '\r':
				return true;
			default:
				return false;
			}
		}
		static bool fast_is_space_or_null(char c) {
			switch (c) {
			case 0:
			case ' ':
			case '\t':
			case '\v':
			case '\n':
			case '\r':
				return true;
			default:
				return false;
			}
		}

		// replace a substring with another substring
		static std::string replace(std::string const& target,
			std::string const& from, std::string const& to) {
			if (target.empty() || from.empty()) return target;
			std::stringstream out;
			for (size_t i = 0; i < target.size(); ) {
				bool matched = true;
				size_t j = 0;
				for (; j < from.size(); ++j) {
					if (i + j >= target.size() || target[i + j] != from[j]) {
						matched = false;
						break;
					}
				}
				if (matched) {
					out << to;
					i += from.size();
				}
				else {
					size_t k = i;
					for (; k <= i + j; ++k) {
						out << target[k];
					}
					i = k;
				}
			}
			return out.str();
		}

		// replace a substring with another substring (case insesitive)
		static std::string replace_nc(std::string const& target,
			std::string const& from, std::string const& to) {
			if (target.empty() || from.empty()) return target;
			std::stringstream out;
			for (size_t i = 0; i < target.size(); ) {
				bool matched = true;
				size_t j = 0;
				for (; j < from.size(); ++j) {
					if (i + j >= target.size() || !fast_no_case_equal(target[i + j], from[j])) {
						matched = false;
						break;
					}
				}
				if (matched) {
					out << to;
					i += from.size();
				}
				else {
					size_t k = i;
					for (; k <= i + j; ++k) {
						out << target[k];
					}
					i = k;
				}
			}
			return out.str();
		}
		// prefix and suffix checks
		inline bool starts_with(std::string const& s, std::string const& prefix) {
			if (prefix.size() > s.size()) return false;
			return (s.compare(0, prefix.size(), prefix) == 0);
		}
		inline bool ends_with(std::string const& s, std::string const& suffix) {
			if (suffix.size() > s.size()) return false;
			return (s.compare(s.size()- suffix.size(), suffix.size(), suffix) == 0);
		}


		// it helps to be able to recognise these. 
		// it's not bullet proof - it assumes something is a wildcard pattern if
		// it contains ? or a *
		static bool is_wildcard_pattern(const char* pattern) {
			size_t sz = strlen(pattern);
			for (size_t i = 0; i < sz; ++i) {
				switch (pattern[i]) {
				case '*':
				case '?':
					return true;
				default:
					;// do nothing
				}
			}
			return false;
		}
		
		// match a wildcard string (using ? for a single character and * for any number of them)
		static bool wildcard_match(
			const char* const text,
			const char* const pattern,
			const size_t szT,
			const size_t szP,
			const bool case_sensitive = false) {

			if (szP == 0)
				return szT == 0;

			auto compare = case_sensitive ?
				[](char const l, char const r) { return l == r; } :
				[](char const l, char const r) { return fast_no_case_equal(l,r); };
			size_t matchStart = 0; // used for '*' matching

			size_t iP = 0, iT = 0; // counters for pattern and text
			for (; iT < szT && iP < szP; ++iT) {
				switch (pattern[iP]) {
				case '?': // matches any single character
					++iP;
					break;
				case '*': // matches none or some characters
					++iP;
					// multiple asterisks are the same as a single one
					for (size_t i = iP; pattern[i] == '*' && i < szP; ++i, ++iP)
						;
					if (iP == szP)//if the pattern ends in '*' it will match whatever's left to match
						return true;
					// loop looks for the match for the remainder of the pattern anywhere   
					// this means that we may have to rewind the starting point of the match
					// 'matchStart' is the starting index for matching within the pattern
					matchStart = iP;
					for (; iP < szP && iT < szT; ++iT) {
						char const c = pattern[iP];
						if (c == '*') {
							break;
						}
						else if (c == '?' || compare(text[iT], c))
							++iP;
						else
							// oups, let * swallow text up to here and start again 
							iP = matchStart;//rewind the pattern

					}
					if (iT == szT)
						return iP == szP || (iP == szP - 1 && pattern[iP] == '*');
					break;
				default:
					if (!compare(text[iT], pattern[iP]))
						return false;
					++iP;
				}
			}
			return (iP == szP && iT == szT);
		}
		
		static bool wildcard_match(
			std::string const& text,
			std::string const& pattern,
			const bool case_sensitive = false) {
			return wildcard_match(text.c_str(), pattern.c_str(), 
				text.size(), pattern.size(), case_sensitive);
		}
		
		// case insesitive equality
		static bool equal_nc(
			const char* const s1,
			size_t l1,
			const char* const s2,
			size_t l2
		) {
			if (l1 != l2) return false;
			for (size_t i = 0; i < l1; ++i)
				if (!fast_no_case_equal(s1[i], s2[i])) return false;
			return true;
		}
		
		static bool equal_nc(
			std::string const& s1,
			std::string const& s2
			) {
			return equal_nc(s1.c_str(), s1.size(), s2.c_str(), s2.size());
		}
		
		static bool equal_nc(
			std::string const& s1,
			const char* const s2
		) {
			return equal_nc(s1.c_str(), s1.size(), s2, strlen(s2));
		}
		
		static bool equal_nc(
			const char* const s1,
			std::string const& s2
			) {
			return equal_nc(s1, strlen(s1), s2.c_str(), s2.size());
		}
		
		static bool equal_nc(
			const char* const s1,
			const char* const s2
		) {
			return equal_nc(s1, strlen(s1), s2, strlen(s2));
		}

		// convert a string to upper case
		static std::string to_upper(std::string const& s) {
			std::string out;
			std::transform(s.begin(), s.end(), std::back_inserter(out), 
				[](const char c) { return fast_to_upper(c); });
			return out;
		}
		
		// convert a string to lower case
		static std::string to_lower(std::string const& s) {
			std::string out;
			std::transform(s.begin(), s.end(), std::back_inserter(out), 
				[](const char c) { return fast_to_lower(c); });
			return out;
		}

		// case insesitive less for strings
		struct less_nc_t : std::binary_function<std::string, std::string, bool>
		{
			bool operator() (const std::string& s1, const std::string& s2) const {
				return std::lexicographical_compare
				(s1.begin(), s1.end(),   
					s2.begin(), s2.end(),
					[](const char c1, const char c2) {return (fast_to_lower(c1) < fast_to_lower(c2)); });  // comparison
			}
		};

		static bool less_nc(const std::string& s1, const std::string& s2) {
			return less_nc_t()(s1, s2);
		}

		struct equal_nc_t : std::binary_function<std::string, std::string, bool>
		{
			bool operator() (const std::string& s1, const std::string& s2) const {
				return equal_nc(s1, s2);  // comparison
			}
		};
		struct hash_nc_t {
			size_t operator()(std::string const& s) const {
				return std::hash<std::string>()(to_lower(s));
			}
		};

		// case insesitive set of strings
		using string_set_nc = std::set<std::string, less_nc_t>;
		
		// case insesitive hash-set of strings
		using string_unordered_set_nc = std::unordered_set<std::string, hash_nc_t, equal_nc_t>;

		// case insesitive strings map 
		template < typename ValueT >
		using string_map_nc = std::map<std::string, ValueT, less_nc_t>;

		// case insesitive strings hash map 
		template < typename ValueT >
		using string_unordered_map_nc = std::unordered_map<std::string, ValueT, hash_nc_t, equal_nc_t>;

		// var_printf is a variadic template version of sprintf
		// based on cpp reference example: https://en.cppreference.com/w/cpp/language/parameter_pack
		// replaces '%' with next argument, to print '%' use '%%'
		static void var_printf(std::ostream& out, const char* format){ // base function
			out << std::boolalpha << format;
		}

		template<typename T, typename... Targs>
		static void var_printf(std::ostream& out, const char* format, T value, Targs... Fargs){

			for (; *format != '\0'; format++)
			{
				if (*format == '\\' && *(format + 1) != '%') {
					format++;
				}
				else if (*format == '%')
				{
					out << std::boolalpha << value;
					var_printf(out, format + 1, Fargs...); // recursive call
					return;
				}
				out << std::boolalpha << *format;
			}
		}

		// string interpolation, like modern languages do it
		template<typename... Targs>
		static std::string af_format_string(const char* format, Targs... Fargs) {
			std::stringstream s;
			var_printf(s, format, Fargs...);
			return s.str();
		}

		
		// trim left 
		inline void ltrim_s(std::string& s) {
			s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
				return !fast_is_space(ch);
				}));
		}
		inline std::string ltrim(std::string const& s) {
			std::string sout(s);
			ltrim_s(sout);
			return sout;
		}
		inline void ltrim_s(std::string& s, char c) {
			s.erase(s.begin(), std::find_if(s.begin(), s.end(), [&](unsigned char ch) {
				return ch!=c;
				}));
		}
		inline std::string ltrim(std::string const& s, char c) {
			std::string sout(s);
			ltrim_s(sout, c);
			return sout;
		}
		// trim right
		inline void rtrim_s(std::string& s) {
			s.erase(std::find_if(s.rbegin(), s.rend(), [&](unsigned char ch) {
				return !fast_is_space_or_null(ch);
				}).base(), s.end());
		}
		inline std::string rtrim(std::string const& s) {
			std::string sout(s);
			rtrim_s(sout);
			return sout;
		}
		inline void rtrim_s(std::string& s, char c) {
			s.erase(std::find_if(s.rbegin(), s.rend(), [&](unsigned char ch) {
				return ch != c;
				}).base(), s.end());
		}
		inline std::string rtrim(std::string const& s, char c) {
			std::string sout(s);
			rtrim_s(sout, c);
			return sout;
		}

		// trim
		inline void trim_s(std::string& s) {
			ltrim_s(s);
			rtrim_s(s);
		}
		inline std::string trim(std::string const& s) {
			std::string sout(s);
			trim_s(sout);
			return sout;
		}	
		inline void trim_s(std::string& s, char c) {
			ltrim_s(s, c);
			rtrim_s(s, c);
		}
		inline std::string trim(std::string const& s, char c) {
			std::string sout(s);
			trim_s(sout, c);
			return sout;
		}

		namespace string_util_impl {
			// formatting csv strings properly (so that Excel can read them) 
			// is somewhat involved, so here's couple of function to help
			template<typename T>
			struct excel_escape_string {
				T const& _value;
				excel_escape_string(T const& t) : _value(t) {}
				T const& value() const { return _value; }
			};
			template<>
			struct excel_escape_string  <const char*> {
				std::string _value;
				excel_escape_string(const char* const t) : _value(t) {
					if (_value.empty()) return;
					_value = replace(trim(_value), "\"", "\"\"");
					if (_value.find(',') != std::string::npos ||
						_value.find('"') != std::string::npos ||
						_value.find(':') != std::string::npos ||
						_value.find('\n') != std::string::npos) {
						_value = "\"" + _value + "\"";
					}
					if (// excel is real weird about ':'
						_value.find(':') != std::string::npos &&
						_value.find('\n') == std::string::npos) {
						_value = replace(_value, ":", "::");
					}
				}
				const char* const value() const { return _value.c_str(); }
			};

			template<>
			struct excel_escape_string  <std::string> {
				std::string const _value;
				excel_escape_string(std::string const& t) :
					_value(excel_escape_string <const char*>(t.c_str()).value()) {
				}
				const char* const value() const { return _value.c_str(); }
			};


			template<typename T>
			struct csv_escape_string {
				T const& _value;
				csv_escape_string(T const& t) : _value(t) {}
				T const& value() const { return _value; }
			};
			template<>
			struct csv_escape_string <const char*> {
				std::string _value;
				csv_escape_string(const char* const t) : _value(t) {
					if (_value.empty()) return;
					_value = trim(_value);
					if (_value.find(',') != std::string::npos ||
						_value.find('\n') != std::string::npos) {
						_value = "\"" + _value + "\"";
					}
				}
				const char* const value() const { return _value.c_str(); }
			};

			template<>
			struct csv_escape_string <std::string> {
				std::string const _value;
				csv_escape_string(std::string const& t) :
					_value(csv_escape_string<const char*>(t.c_str()).value()) {
				}
				const char* const value() const { return _value.c_str(); }
			};
		}

		template< typename T, typename U = void>
		struct convert_if_string_literal_impl {
			using type = T;
		};
		template< typename T>
		struct convert_if_string_literal_impl< T, std::enable_if_t< std::is_constructible< std::string, T>::value>> {
			using type = std::string ;
		};
		template< typename T>
		struct convert_if_string_literal : public convert_if_string_literal_impl<T> {};


		template<typename ValueT>
		static std::ostream& to_csv_row(std::ostream& out, bool excel_format, ValueT const& v) {
			using namespace string_util_impl;
			using ConvertedValueT = typename convert_if_string_literal<decltype(v)>::type;
			const ConvertedValueT vc = v;
			out << std::boolalpha <<
				(excel_format ? excel_escape_string<ConvertedValueT>(vc).value() :
					csv_escape_string<ConvertedValueT>(vc).value());
			return out;
		}

		template<typename T, typename... Targs>
		static std::ostream& to_csv_row(std::ostream& out, bool excel_format, T const& value, Targs const&... Fargs) {
			to_csv_row(out, excel_format, value) << ",";
			to_csv_row(out, excel_format, Fargs...);
			return out;
		}

		template<typename... Targs>
		static std::string to_csv_row_string(bool excel_format,Targs const&... Fargs) {
			std::stringstream sout;
			to_csv_row(sout, excel_format, Fargs...);
			return sout.str();
		}

		template<typename... Targs>
		static std::string to_plain_csv_row_string(Targs const&... Fargs) {
			std::stringstream sout;
			to_csv_row(sout, false, Fargs...);
			return sout.str();
		}

		template<typename... Targs>
		static std::string to_excel_csv_row_string(Targs const&... Fargs) {
			std::stringstream sout;
			to_csv_row(sout, true, Fargs...);
			return sout.str();
		}


		template<typename ValueT>
		static std::string to_csv(std::vector<ValueT> const& values, bool excel_format = false) {
			if (values.empty())
				return "";
			std::stringstream sout;
			const size_t last = values.size()-1;
			size_t i = 0;
			for (auto const& v : values) {
				to_csv_row(sout, excel_format, v);
				if (i < last)
					sout << ",";
				++i;
			}
			return sout.str();
		}

		template< typename ValueT>
		static std::string to_csv(std::vector <std::vector<ValueT>> const& values, bool excel_format = false) {
			if (values.empty())
				return "";
			std::stringstream sout;
			const size_t last = values.size() - 1;
			size_t i = 0;
			for (auto const& row : values) {
				sout << to_csv(row, excel_format);
				if (i < last)
					sout << "\n";
				++i;
			}
			return sout.str();
		}


		template< typename T>
		inline T convert_string(std::string const&);
		template< >
		inline std::string convert_string<std::string>(std::string const& s) { return s; }
		template< >
		inline int convert_string<int>(std::string const& s) {return std::stoi(s);}
		template< >
		inline long convert_string<long>(std::string const& s) { return std::stol(s); }
		template< >
		inline long long convert_string<long long>(std::string const& s) { return std::stoll(s); }
		template< >
		inline unsigned long convert_string<unsigned long>(std::string const& s) { return std::stoul(s); }
		template< >
		inline unsigned long long convert_string<unsigned long long>(std::string const& s) { return std::stoull(s); }
		template< >
		inline float convert_string<float>(std::string const& s) { return std::stof(s); }
		template< >
		inline double convert_string<double>(std::string const& s) { return std::stod(s); }
		template< >
		inline long double convert_string<long double>(std::string const& s) { return std::stold(s); }
		template< >
		inline bool convert_string<bool>(std::string const& s) { return equal_nc(trim(s), "true")?true:false; }

		template<typename T>
		static std::vector<T> csv_to_vector(std::string const& csv) {
			std::vector<T> out;
			std::string acc;
			for (auto const c : csv) {
				switch (c) {
				case ',':
					out.push_back(convert_string<T>(acc));
					acc.clear();
					break;
				default:
					acc.push_back(c);
					break;
				}
			}
			if (!acc.empty())
				out.push_back(trim(acc));
			return out;
		}

		template<>
		std::vector<std::string> csv_to_vector<std::string>(std::string const& csv) {
			std::vector<std::string> out;
			std::string acc;
			for (auto const c : csv) {
				switch (c) {
				case ',':
					out.push_back(trim(acc));
					acc.clear();
					break;
				default:
					acc.push_back(c);
					break;
				}
			}
			if (!acc.empty())
				out.push_back(trim(acc));
			return out;
		}

		template<typename T>
		inline void csv_to_vector(std::string const& csv, std::vector<T>& target) {
			target = csv_to_vector<T>(csv);
		}
	}
}