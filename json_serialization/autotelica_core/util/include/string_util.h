#pragma once
#include <string>
#include <sstream>
#include <algorithm>
#include <functional>
#include <cstring>
#include <cwchar>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <vector>
#include <type_traits>
#include <locale>
#include <codecvt>

namespace autotelica {
	namespace string_util {
		struct utf8 {
			static std::wstring_convert<std::codecvt_utf8<wchar_t>>& converter() {
				static std::wstring_convert<std::codecvt_utf8<wchar_t>> _instance;
				return _instance;
			}
			static inline std::string convert(std::wstring const& w) {
				return converter().to_bytes(w);
			}
			static inline std::string to_string(std::wstring const& w) {
				return convert(w);
			}
			static inline std::string to_string(std::string const& s) {
				return s;
			}
			static inline std::wstring convert(std::string const& s) {
				return converter().from_bytes(s);
			}
			static inline std::wstring to_wstring(std::string const& s) {
				return convert(s);
			}
			static inline std::wstring to_wstring(std::wstring const& w) {
				return w;
			}
		};
		template<typename string_t>
		string_t utf8_convert(std::string const& in);
		template<typename string_t>
		string_t utf8_convert(std::wstring const& in);
		template<>
		std::string utf8_convert<std::string>(std::string const& in){
			return in;
		}
		template<>
		std::string utf8_convert<std::string>(std::wstring const& in){
			return utf8::convert(in);
		}
		template<>
		std::wstring utf8_convert<std::wstring>(std::string const& in){
			return utf8::convert(in);
		}
		template<>
		std::wstring utf8_convert<std::wstring>(std::wstring const& in){
			return in;
		}


		template<typename string_t>
		string_t utf8_convert(std::wstring const& in);


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
		static wchar_t fast_to_upper(wchar_t c) {
			switch (c) {
			case L'a': return L'A';
			case L'b': return L'B';
			case L'c': return L'C';
			case L'd': return L'D';
			case L'e': return L'E';
			case L'f': return L'F';
			case L'g': return L'G';
			case L'h': return L'H';
			case L'i': return L'I';
			case L'j': return L'J';
			case L'k': return L'K';
			case L'l': return L'L';
			case L'm': return L'M';
			case L'n': return L'N';
			case L'o': return L'O';
			case L'p': return L'P';
			case L'q': return L'Q';
			case L'r': return L'R';
			case L's': return L'S';
			case L't': return L'T';
			case L'u': return L'U';
			case L'v': return L'V';
			case L'w': return L'W';
			case L'x': return L'X';
			case L'y': return L'Y';
			case L'z': return L'Z';
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
		static wchar_t fast_to_lower(wchar_t c) {
			switch (c) {
			case L'A': return L'a';
			case L'B': return L'b';
			case L'C': return L'c';
			case L'D': return L'd';
			case L'E': return L'e';
			case L'F': return L'f';
			case L'G': return L'g';
			case L'H': return L'h';
			case L'I': return L'i';
			case L'J': return L'j';
			case L'K': return L'k';
			case L'L': return L'l';
			case L'M': return L'm';
			case L'N': return L'n';
			case L'O': return L'o';
			case L'P': return L'p';
			case L'Q': return L'q';
			case L'R': return L'r';
			case L'S': return L's';
			case L'T': return L't';
			case L'U': return L'u';
			case L'V': return L'v';
			case L'W': return L'w';
			case L'X': return L'x';
			case L'Y': return L'y';
			case L'Z': return L'z';
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
		static bool fast_no_case_equal(wchar_t c1, wchar_t c2) {
			switch (c1) {
			case L'a': return c2 == L'a' || c2 == L'A';
			case L'b': return c2 == L'b' || c2 == L'B';
			case L'c': return c2 == L'c' || c2 == L'C';
			case L'd': return c2 == L'd' || c2 == L'D';
			case L'e': return c2 == L'e' || c2 == L'E';
			case L'f': return c2 == L'f' || c2 == L'F';
			case L'g': return c2 == L'g' || c2 == L'G';
			case L'h': return c2 == L'h' || c2 == L'H';
			case L'i': return c2 == L'i' || c2 == L'I';
			case L'j': return c2 == L'j' || c2 == L'J';
			case L'k': return c2 == L'k' || c2 == L'K';
			case L'l': return c2 == L'l' || c2 == L'L';
			case L'm': return c2 == L'm' || c2 == L'M';
			case L'n': return c2 == L'n' || c2 == L'N';
			case L'o': return c2 == L'o' || c2 == L'O';
			case L'p': return c2 == L'p' || c2 == L'P';
			case L'q': return c2 == L'q' || c2 == L'Q';
			case L'r': return c2 == L'r' || c2 == L'R';
			case L's': return c2 == L's' || c2 == L'S';
			case L't': return c2 == L't' || c2 == L'T';
			case L'u': return c2 == L'u' || c2 == L'U';
			case L'v': return c2 == L'v' || c2 == L'V';
			case L'w': return c2 == L'w' || c2 == L'W';
			case L'x': return c2 == L'x' || c2 == L'X';
			case L'y': return c2 == L'y' || c2 == L'Y';
			case L'z': return c2 == L'z' || c2 == L'Z';
			case L'A': return c2 == L'a' || c2 == L'A';
			case L'B': return c2 == L'b' || c2 == L'B';
			case L'C': return c2 == L'c' || c2 == L'C';
			case L'D': return c2 == L'd' || c2 == L'D';
			case L'E': return c2 == L'e' || c2 == L'E';
			case L'F': return c2 == L'f' || c2 == L'F';
			case L'G': return c2 == L'g' || c2 == L'G';
			case L'H': return c2 == L'h' || c2 == L'H';
			case L'I': return c2 == L'i' || c2 == L'I';
			case L'J': return c2 == L'j' || c2 == L'J';
			case L'K': return c2 == L'k' || c2 == L'K';
			case L'L': return c2 == L'l' || c2 == L'L';
			case L'M': return c2 == L'm' || c2 == L'M';
			case L'N': return c2 == L'n' || c2 == L'N';
			case L'O': return c2 == L'o' || c2 == L'O';
			case L'P': return c2 == L'p' || c2 == L'P';
			case L'Q': return c2 == L'q' || c2 == L'Q';
			case L'R': return c2 == L'r' || c2 == L'R';
			case L'S': return c2 == L's' || c2 == L'S';
			case L'T': return c2 == L't' || c2 == L'T';
			case L'U': return c2 == L'u' || c2 == L'U';
			case L'V': return c2 == L'v' || c2 == L'V';
			case L'W': return c2 == L'w' || c2 == L'W';
			case L'X': return c2 == L'x' || c2 == L'X';
			case L'Y': return c2 == L'y' || c2 == L'Y';
			case L'Z': return c2 == L'z' || c2 == L'Z';
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
		static bool fast_is_lower(wchar_t c) {
			switch (c) {
			case L'a':
			case L'b':
			case L'c':
			case L'd':
			case L'e':
			case L'f':
			case L'g':
			case L'h':
			case L'i':
			case L'j':
			case L'k':
			case L'l':
			case L'm':
			case L'n':
			case L'o':
			case L'p':
			case L'q':
			case L'r':
			case L's':
			case L't':
			case L'u':
			case L'v':
			case L'w':
			case L'x':
			case L'y':
			case L'z':
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
		static bool fast_is_upper(wchar_t c) {
			switch (c) {
			case L'A':
			case L'B':
			case L'C':
			case L'D':
			case L'E':
			case L'F':
			case L'G':
			case L'H':
			case L'I':
			case L'J':
			case L'K':
			case L'L':
			case L'M':
			case L'N':
			case L'O':
			case L'P':
			case L'Q':
			case L'R':
			case L'S':
			case L'T':
			case L'U':
			case L'V':
			case L'W':
			case L'X':
			case L'Y':
			case L'Z':
				return true;
			default:
				return false;
			}
		}
		template<typename char_t>
		inline bool is_lowercase_(std::basic_string<char_t> const& name) {
			return std::all_of(name.begin(), name.end(), [](const char_t c) {return fast_to_lower(c) == c; });
		}
		inline bool is_lowercase(std::string const& name) {
			return is_lowercase_<char>(name);
		}
		inline bool is_lowercase(std::wstring const& name) {
			return is_lowercase_<wchar_t>(name);
		}
		template<typename char_t>
		inline bool is_uppercase_(std::basic_string<char_t> const& name) {
			return std::all_of(name.begin(), name.end(), [](const char_t c) {return fast_to_upper(c) == c; });
		}
		inline bool is_uppercase(std::string const& name) {
			return is_uppercase_<char>(name);
		}
		inline bool is_uppercase(std::wstring const& name) {
			return is_uppercase_<wchar_t>(name);
		}

		// fast is_space (doesn't use locales)
		inline bool fast_is_space(char c) {
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
		inline bool fast_is_space(wchar_t c) {
			switch (c) {
			case L' ':
			case L'\t':
			case L'\v':
			case L'\n':
			case L'\r':
				return true;
			default:
				return false;
			}
		}
		inline bool fast_is_space_or_null(char c) {
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
		inline bool fast_is_space_or_null(wchar_t c) {
			switch (c) {
			case 0:
			case L' ':
			case L'\t':
			case L'\v':
			case L'\n':
			case L'\r':
				return true;
			default:
				return false;
			}
		}

		inline std::size_t string_length(const char* s){
			return strlen(s);
		}
		inline std::size_t string_length(const wchar_t* s){
			return wcslen(s);
		}

		// replace a substring with another substring
		template<typename char_t>
		static std::basic_string<char_t> replace_(
				std::basic_string<char_t> const& target,
				std::basic_string<char_t> const& from, 
				std::basic_string<char_t> const& to) {
			if (target.empty() || from.empty()) return target;
			std::basic_stringstream<char_t> out;
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
		// we have to instantiate them like this so that they work with string literals
		inline std::string replace(std::string const& target, std::string const& from, std::string const& to) {
			return replace_<char>(target, from, to);
		}
		inline std::wstring replace(std::wstring const& target, std::wstring const& from, std::wstring const& to) {
			return replace_<wchar_t>(target, from, to);
		}
		// replace a substring with another substring (case insesitive)
		template<typename char_t>
		static std::basic_string<char_t> replace_nc_(
				std::basic_string<char_t> const& target,
				std::basic_string<char_t> const& from, 
				std::basic_string<char_t> const& to) {
			if (target.empty() || from.empty()) return target;
			std::basic_stringstream<char_t> out;
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
		// we have to instantiate them like this so that they work with string literals
		inline std::string replace_nc(std::string const& target, std::string const& from, std::string const& to) {
			return replace_nc_<char>(target, from, to);
		}
		inline std::wstring replace_nc(std::wstring const& target, std::wstring const& from, std::wstring const& to) {
			return replace_nc_<wchar_t>(target, from, to);
		}

		// prefix and suffix checks
		template<typename char_t>
		inline bool starts_with_(std::basic_string<char_t> const& s, std::basic_string<char_t> const& prefix) {
			if (prefix.size() > s.size()) return false;
			return (s.compare(0, prefix.size(), prefix) == 0);
		}
		inline bool starts_with(std::string const& s, std::string const& prefix) {
			return starts_with_<char>(s, prefix);
		}
		inline bool starts_with(std::wstring const& s, std::wstring const& prefix) {
			return starts_with_<wchar_t>(s, prefix);
		}
		template<typename char_t>
		inline bool ends_with_(std::basic_string<char_t> const& s, std::basic_string<char_t> const& suffix) {
			if (suffix.size() > s.size()) return false;
			return (s.compare(s.size()- suffix.size(), suffix.size(), suffix) == 0);
		}
		inline bool ends_with(std::string const& s, std::string const& prefix) {
			return ends_with_<char>(s, prefix);
		}
		inline bool ends_with(std::wstring const& s, std::wstring const& prefix) {
			return ends_with_<wchar_t>(s, prefix);
		}


		// it helps to be able to recognise these. 
		// it's not bullet proof - it assumes something is a wildcard pattern if
		// it contains ? or a *
		template<typename char_t>
		struct wildcard_symbols;

		template<>
		struct wildcard_symbols<char>{
			static constexpr auto any = '*';
			static constexpr auto one = '?';
		};
		template<>
		struct wildcard_symbols<wchar_t>{
			static constexpr auto any = L'*';
			static constexpr auto one = L'?';
		};

		template<typename char_t>
		static bool is_wildcard_pattern(const char_t* pattern) {
			while(char_t c = *pattern++) {
				switch (c) {
				case wildcard_symbols<char_t>::any:
				case wildcard_symbols<char_t>::one:
					return true;
				default:
					;// do nothing
				}
			}
			return false;
		}
		
		// match a wildcard string (using ? for a single character and * for any number of them)
		template<typename char_t>
		static bool wildcard_match(
			const char_t* const text,
			const char_t* const pattern,
			const size_t szT,
			const size_t szP,
			const bool case_sensitive = false) {

			if (szP == 0)
				return szT == 0;

			auto compare = case_sensitive ?
				[](char_t const l, char_t const r) { return l == r; } :
				[](char_t const l, char_t const r) { return fast_no_case_equal(l,r); };
			size_t matchStart = 0; // used for '*' matching

			size_t iP = 0, iT = 0; // counters for pattern and text
			for (; iT < szT && iP < szP; ++iT) {
				switch (pattern[iP]) {
				case wildcard_symbols<char_t>::one: // matches any single character
					++iP;
					break;
				case wildcard_symbols<char_t>::any: // matches none or some characters
					++iP;
					// multiple asterisks are the same as a single one
					for (size_t i = iP; pattern[i] == wildcard_symbols<char_t>::any && i < szP; ++i, ++iP)
						;
					if (iP == szP)//if the pattern ends in '*' it will match whatever's left to match
						return true;
					// loop looks for the match for the remainder of the pattern anywhere   
					// this means that we may have to rewind the starting point of the match
					// 'matchStart' is the starting index for matching within the pattern
					matchStart = iP;
					for (; iP < szP && iT < szT; ++iT) {
						char_t const c = pattern[iP];
						if (c == wildcard_symbols<char_t>::any) {
							break;
						}
						else if (c == wildcard_symbols<char_t>::one || compare(text[iT], c))
							++iP;
						else
							// oups, let * swallow text up to here and start again 
							iP = matchStart;//rewind the pattern

					}
					if (iT == szT)
						return iP == szP || (iP == szP - 1 && pattern[iP] == wildcard_symbols<char_t>::any);
					break;
				default:
					if (!compare(text[iT], pattern[iP]))
						return false;
					++iP;
				}
			}
			return (iP == szP && iT == szT);
		}
		template<typename char_t>
		inline bool wildcard_match_(
			std::basic_string<char_t> const& text,
			std::basic_string<char_t> const& pattern,
			const bool case_sensitive = false) {
			return wildcard_match(text.c_str(), pattern.c_str(), 
				text.size(), pattern.size(), case_sensitive);
		}
		inline bool wildcard_match(std::string const& text,std::string const& pattern,const bool case_sensitive = false) {
			return wildcard_match_<char>(text, pattern, case_sensitive);
		}
		inline bool wildcard_match(std::wstring const& text, std::wstring const& pattern, const bool case_sensitive = false) {
			return wildcard_match_<wchar_t>(text, pattern, case_sensitive);
		}
		// case insesitive equality
		template<typename char_t>
		static bool equal_nc(
			const char_t* const s1,
			size_t l1,
			const char_t* const s2,
			size_t l2
		) {
			if (l1 != l2) return false;
			for (size_t i = 0; i < l1; ++i)
				if (!fast_no_case_equal(s1[i], s2[i])) return false;
			return true;
		}
		
		template<typename char_t>
		inline bool equal_nc_(std::basic_string<char_t> const& s1, std::basic_string<char_t> const& s2) {
			return equal_nc(s1.c_str(), s1.size(), s2.c_str(), s2.size());
		}
		inline bool equal_nc(std::string const& s1, std::string const& s2) {
			return equal_nc_<char>(s1, s2);
		}
		inline bool equal_nc(std::wstring const& s1, std::wstring const& s2) {
			return equal_nc_<wchar_t>(s1, s2);
		}
		template<typename char_t>
		inline bool equal_nc_(std::basic_string<char_t> const& s1,const char_t* const s2) {
			return equal_nc(s1.c_str(), s1.size(), s2, string_length(s2));
		}
		inline bool equal_nc(std::string const& s1, const char* const s2) {
			return equal_nc_<char>(s1, s2);
		}
		inline bool equal_nc(std::wstring const& s1, const wchar_t* const s2) {
			return equal_nc_<wchar_t>(s1, s2);
		}

		template<typename char_t>
		inline bool equal_nc_(const char_t* const s1,std::basic_string<char_t> const& s2) {
			return equal_nc<char_t>(s1, string_length(s1), s2.c_str(), s2.size());
		}
		inline bool equal_nc(const char* const s1, std::string const& s2) {
			return equal_nc_<char>(s1, s2);
		}
		inline bool equal_nc(const wchar_t* const s1, std::wstring const& s2) {
			return equal_nc_<wchar_t>(s1, s2);
		}
		template<typename char_t>
		inline bool equal_nc_(const char_t* const s1,const char_t* const s2) {
			return equal_nc(s1, string_length(s1), s2, string_length(s2));
		}
		inline bool equal_nc(const char* const s1, const char* const s2) {
			return equal_nc_<char>(s1, s2);
		}
		inline bool equal_nc(const wchar_t* const s1, const wchar_t* const s2) {
			return equal_nc_<wchar_t>(s1, s2);
		}

		// convert a string to upper case
		template<typename char_t>
		inline std::basic_string<char_t> to_upper_(std::basic_string<char_t> const& s) {
			std::basic_string<char_t> out;
			std::transform(s.begin(), s.end(), std::back_inserter(out), 
				[](const char_t c) { return fast_to_upper(c); });
			return out;
		}
		inline std::string to_upper(std::string const& s) {
			return to_upper_<char>(s);
		}
		inline std::wstring to_upper(std::wstring const& s) {
			return to_upper_<wchar_t>(s);
		}

		template<typename char_t>
		static void to_upper_inpl_(std::basic_string<char_t>& s) {
			for(auto& c : s)
				c = fast_to_upper(c);
		}
		inline void to_upper_inpl(std::string& s) {
			to_upper_inpl_<char>(s);
		}
		inline void to_upper_inpl(std::wstring& s) {
			to_upper_inpl_<wchar_t>(s);
		}
		// convert a string to lower case
		template<typename char_t>
		inline std::basic_string<char_t> to_lower_(std::basic_string<char_t> const& s) {
			std::basic_string<char_t> out;
			std::transform(s.begin(), s.end(), std::back_inserter(out), 
				[](const char_t c) { return fast_to_lower(c); });
			return out;
		}
		inline std::string to_lower(std::string const& s) {
			return to_lower_<char>(s);
		}
		inline std::wstring to_lower(std::wstring const& s) {
			return to_lower_<wchar_t>(s);
		}
		template<typename char_t>
		inline void to_lower_inpl_(std::basic_string<char_t>& s) {
			for(auto& c : s)
				c = fast_to_lower(c);
		}
		inline void to_lower_inpl(std::string& s) {
			to_lower_inpl_<char>(s);
		}
		inline void to_lower_inpl(std::wstring& s) {
			to_lower_inpl_<wchar_t>(s);
		}
		// case insesitive less for strings
		template<typename char_t>
		struct less_nc_t : std::binary_function<std::basic_string<char_t>, std::basic_string<char_t>, bool>
		{
			bool operator() (std::basic_string<char_t> const& s1, std::basic_string<char_t> const& s2) const {
				return std::lexicographical_compare
				(s1.begin(), s1.end(),   
					s2.begin(), s2.end(),
					[](const char_t c1, const char_t c2) {return (fast_to_lower(c1) < fast_to_lower(c2)); });  // comparison
			}
		};

		template<typename char_t>
		inline bool less_nc(std::basic_string<char_t> const& s1, std::basic_string<char_t> const& s2) {
			return less_nc_t<char_t>()(s1, s2);
		}

		
		template<typename char_t>
		struct equal_nc_t : std::binary_function<std::basic_string<char_t>, std::basic_string<char_t>, bool>
		{
			bool operator() (std::basic_string<char_t> const& s1, std::basic_string<char_t> const& s2) const {
				return equal_nc(s1, s2);  // comparison
			}
		};
		template<typename char_t>
		struct hash_nc_t {
			size_t operator()(std::basic_string<char_t> const& s) const {
				return std::hash<std::basic_string<char_t>>()(to_lower(s));
			}
		};

		// case insesitive set of strings
		template<typename string_t = std::string>
		using set_nc = std::set<string_t, less_nc_t<typename string_t::value_type>>;
		
		// case insesitive hash-set of strings
		template<typename string_t = std::string>
		using unordered_set_nc = std::unordered_set<string_t, hash_nc_t<typename string_t::value_type>, equal_nc_t<typename string_t::value_type>>;

		// case insesitive strings map 
		template < typename ValueT, typename string_t = std::string >
		using map_nc = std::map<string_t, ValueT, less_nc_t<typename string_t::value_type>>;

		// case insesitive strings hash map 
		template < typename ValueT, typename string_t = std::string >
		using unordered_map_nc = std::unordered_map<string_t, ValueT, hash_nc_t<typename string_t::value_type>, equal_nc_t<typename string_t::value_type>>;

		// var_printf is a variadic template version of sprintf
		// based on cpp reference example: https://en.cppreference.com/w/cpp/language/parameter_pack
		// replaces '%' with next argument, to print '%' use '%%'
		template<typename char_t>
		struct varprint_symbols;

		template<>
		struct varprint_symbols<char>{
			static constexpr auto null = '\0';
			static constexpr auto escape = '\\';
			static constexpr auto replace = '%';
		};
		template<>
		struct varprint_symbols<wchar_t>{
			static constexpr auto null = L'\0';
			static constexpr auto escape = L'\\';
			static constexpr auto replace = L'%';
		};
		template<typename char_t>
		static void var_printf(std::basic_ostream<char_t>& out, const char_t* format){ // base function
			out << std::boolalpha << format;
		}

		template<typename char_t, typename T, typename... Targs>
		static void var_printf(std::basic_ostream<char_t>& out, const char_t* format, T value, Targs... Fargs){

			for (; *format != varprint_symbols<char_t>::null; format++)
			{
				if (*format == varprint_symbols<char_t>::escape && *(format + 1) != varprint_symbols<char_t>::replace) {
					format++;
				}
				else if (*format == varprint_symbols<char_t>::replace)
				{
					out << std::boolalpha << value;
					var_printf(out, format + 1, Fargs...); // recursive call
					return;
				}
				out << std::boolalpha << *format;
			}
		}

		// string interpolation, like modern languages do it
		template<typename char_t, typename... Targs>
		inline std::basic_string<char_t> af_format_string(const char_t* format, Targs... Fargs) {
			std::basic_stringstream<char_t> s;
			var_printf(s, format, Fargs...);
			return s.str();
		}

		
		// trim left 
		template<typename char_t>
		inline void ltrim_inpl_(std::basic_string<char_t>& s) {
			s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](char_t ch) {
				return !fast_is_space(ch);
				}));
		}
		inline void ltrim_inpl(std::string& s) {
			ltrim_inpl_<char>(s);
		}
		inline void ltrim_inpl(std::wstring& s) {
			ltrim_inpl_<wchar_t>(s);
		}

		template<typename char_t>
		inline std::basic_string<char_t> ltrim_(std::basic_string<char_t> const& s) {
			std::basic_string<char_t> sout(s);
			ltrim_inpl(sout);
			return sout;
		}
		inline std::string ltrim(std::string const& s) {
			return ltrim_<char>(s);
		}
		inline std::wstring ltrim(std::wstring const& s) {
			return ltrim_<wchar_t>(s);
		}
		template<typename char_t>
		inline void ltrim_inpl_(std::basic_string<char_t>& s, char_t c) {
			s.erase(s.begin(), std::find_if(s.begin(), s.end(), [&](char_t ch) {
				return ch!=c;
				}));
		}
		inline void ltrim_inpl(std::string& s, char c) {
			ltrim_inpl_<char>(s, c);
		}
		inline void ltrim_inpl(std::wstring& s, wchar_t c) {
			ltrim_inpl_<wchar_t>(s, c);
		}
		template<typename char_t>
		inline std::basic_string<char_t> ltrim_(std::basic_string<char_t> const& s, char_t c) {
			std::basic_string<char_t> sout(s);
			ltrim_inpl(sout, c);
			return sout;
		}
		inline std::string ltrim(std::string const& s, char c) {
			return ltrim_<char>(s, c);
		}
		inline std::wstring ltrim(std::wstring const& s, wchar_t c) {
			return ltrim_<wchar_t>(s, c);
		}
		// trim right
		template<typename char_t>
		inline void rtrim_inpl_(std::basic_string<char_t>& s) {
			s.erase(std::find_if(s.rbegin(), s.rend(), [&](char_t ch) {
				return !fast_is_space_or_null(ch);
				}).base(), s.end());
		}
		inline void rtrim_inpl(std::string& s) {
			rtrim_inpl_<char>(s);
		}
		inline void rtrim_inpl(std::wstring& s) {
			rtrim_inpl_<wchar_t>(s);
		}
		template<typename char_t>
		inline std::basic_string<char_t> rtrim_(std::basic_string<char_t> const& s) {
			std::basic_string<char_t> sout(s);
			rtrim_inpl(sout);
			return sout;
		}
		inline std::string rtrim(std::string const& s) {
			return rtrim_<char>(s);
		}
		inline std::wstring rtrim(std::wstring const& s) {
			return rtrim_<wchar_t>(s);
		}
		
		template<typename char_t>
		inline void rtrim_inpl_(std::basic_string<char_t>& s, char_t c) {
			s.erase(std::find_if(s.rbegin(), s.rend(), [&](char_t ch) {
				return ch != c;
				}).base(), s.end());
		}
		inline void rtrim_inpl(std::string& s, char c) {
			rtrim_inpl_<char>(s, c);
		}
		inline void rtrim_inpl(std::wstring& s, wchar_t c) {
			rtrim_inpl_<wchar_t>(s, c);
		}
		template<typename char_t>
		inline std::basic_string<char_t> rtrim_(std::basic_string<char_t> const& s, char_t c) {
			std::basic_string<char_t> sout(s);
			rtrim_inpl(sout, c);
			return sout;
		}
		inline std::string rtrim(std::string const& s, char c) {
			return rtrim_<char>(s, c);
		}
		inline std::wstring rtrim(std::wstring const& s, wchar_t c) {
			return rtrim_<wchar_t>(s, c);
		}

		// trim
		template<typename char_t>
		inline void trim_inpl_(std::basic_string<char_t>& s) {
			ltrim_inpl(s);
			rtrim_inpl(s);
		}
		inline void trim_inpl(std::string& s) {
			trim_inpl_<char>(s);
		}
		inline void trim_inpl(std::wstring& s) {
			trim_inpl_<wchar_t>(s);
		}
		
		template<typename char_t>
		inline std::basic_string<char_t> trim_(std::basic_string<char_t> const& s) {
			std::basic_string<char_t> sout(s);
			trim_inpl(sout);
			return sout;
		}
		inline std::string trim(std::string const& s) {
			return trim_<char>(s);
		}
		inline std::wstring trim(std::wstring const& s) {
			return trim_<wchar_t>(s);
		}

		template<typename char_t>
		inline void trim_inpl(std::basic_string<char_t>& s, char_t c) {
			ltrim_inpl(s, c);
			rtrim_inpl(s, c);
		}
		template<typename char_t>
		inline std::basic_string<char_t> trim_(std::basic_string<char_t> const& s, char_t c) {
			std::basic_string<char_t> sout(s);
			trim_inpl(sout, c);
			return sout;
		}
		inline std::string trim(std::string const& s, char c) {
			return trim_<char>(s, c);
		}
		inline std::wstring trim(std::wstring const& s, wchar_t c) {
			return trim_<wchar_t>(s, c);
		}

		namespace string_util_impl {
			template<typename char_t>
			struct csv_symbols;

			template<>
			struct csv_symbols<char>{
				static constexpr auto comma = ',';
				static constexpr auto quote = '"';
				static constexpr auto quote_s = "\"";
				static constexpr auto double_quote_s = "\"\"";
				static constexpr auto newline = '\n';
				static constexpr auto newline_s = "\n";
				static constexpr auto colon = ':';
				static constexpr auto colon_s = ":";
				static constexpr auto double_colon_s = "::";
				static constexpr auto empty_s = "";
			};
			template<>
			struct csv_symbols<wchar_t>{
				static constexpr auto comma = L',';
				static constexpr auto quote = L'"';
				static constexpr auto quote_s = L"\"";
				static constexpr auto newline = L'\n';
				static constexpr auto newline_s = L"\n";
				static constexpr auto colon = L':';
				static constexpr auto colon_s = L":";
				static constexpr auto double_colon_s = L"::";
				static constexpr auto empty_s = L"";
			};
			// formatting csv strings properly (so that Excel can read them) 
			// is somewhat involved, so here's couple of function to help
			template<typename char_t, typename string_t>
			struct excel_escape_string {
				string_t const& _value;
				excel_escape_string(string_t const& t) : _value(t) {}
				string_t const& value() const { return _value; }
			};
			template<typename char_t>
			struct excel_escape_string<char_t, const char_t*> {
				std::basic_string<char_t> _value;
				
				excel_escape_string(const char_t* const t) : _value(t) {
					using symbols = csv_symbols<char_t>;
					if (_value.empty()) return;
					_value = replace(trim(_value), symbols::quote_s, symbols::double_quote_s);
					if (_value.find(symbols::comma) != std::string::npos ||
						_value.find(symbols::quote) != std::string::npos ||
						_value.find(symbols::colon) != std::string::npos ||
						_value.find(symbols::newline) != std::string::npos) {
						_value = symbols::quote_s + _value + symbols::quote_s;
					}
					if (// excel is real weird about ':'
						_value.find(symbols::colon) != std::string::npos &&
						_value.find(symbols::newline) == std::string::npos) {
						_value = replace(_value, symbols::colon_s, symbols::double_colon_s);
					}
				}
				const char_t* const value() const { return _value.c_str(); }
			};

			template<typename char_t>
			struct excel_escape_string<char_t, std::basic_string<char_t>> {
				std::basic_string<char_t> const _value;
				excel_escape_string(std::basic_string<char_t> const& t) :
					_value(excel_escape_string<char_t, const char_t*>(t.c_str()).value()) {
				}
				 std::basic_string<char_t> const& value() const { return _value; }
			};


			template<typename char_t, typename string_t>
			struct csv_escape_string {
				string_t const& _value;
				csv_escape_string(string_t const& t) : _value(t) {}
				string_t const& value() const { return _value; }
			};
			template<typename char_t>
			struct csv_escape_string<char_t, const char_t*> {
				std::basic_string<char_t> _value;
				csv_escape_string(const char_t* const t) : _value(t) {
					using symbols = csv_symbols<char_t>;
					if (_value.empty()) return;
					_value = trim(_value);
					if (_value.find(symbols::comma) != std::string::npos ||
						_value.find(symbols::newline) != std::string::npos) {
						_value = symbols::quote_s + _value + symbols::quote_s;
					}
				}
				const char_t* const value() const { return _value.c_str(); }
			};
			template<typename char_t>
			struct csv_escape_string<char_t, std::basic_string<char_t>> {
				std::basic_string<char_t> const _value;
				csv_escape_string(std::basic_string<char_t> const& t) :
					_value(csv_escape_string<char_t, const char_t*>(t.c_str()).value()) {
				}
				std::basic_string<char_t> const& value() const { return _value; }
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
		struct convert_if_string_literal_impl< T, std::enable_if_t< std::is_constructible< std::wstring, T>::value>> {
			using type = std::wstring ;
		};
		template< typename T>
		struct convert_if_string_literal : public convert_if_string_literal_impl<T> {};

		template<typename char_t, typename ValueT>
		static std::basic_ostream<char_t>& to_csv_row_(std::basic_ostream<char_t>& out, bool excel_format, ValueT const& v) {
			using namespace string_util_impl;
			using ConvertedValueT = typename convert_if_string_literal<decltype(v)>::type;
			const ConvertedValueT vc = v;
			out << std::boolalpha <<
				(excel_format ? excel_escape_string<char_t, ConvertedValueT>(vc).value() :
					csv_escape_string<char_t, ConvertedValueT>(vc).value());
			return out;
		}

		template<typename char_t, typename T, typename... Targs>
		static std::basic_ostream<char_t>& to_csv_row_(std::basic_ostream<char_t>& out, bool excel_format, T const& value, Targs const&... Fargs) {
			using symbols = string_util_impl::csv_symbols<char_t>;
			to_csv_row_(out, excel_format, value) << symbols::comma;
			to_csv_row_(out, excel_format, Fargs...);
			return out;
		}
		template<typename T, typename... Targs>
		inline std::ostream& to_csv_row(std::ostream& out, bool excel_format, T const& value, Targs const&... Fargs) {
			return to_csv_row_<char, T, Targs...>(out, excel_format, value, Fargs...);
		}
		template<typename T, typename... Targs>
		inline std::wostream& to_csv_row(std::wostream& out, bool excel_format, T const& value, Targs const&... Fargs) {
			return to_csv_row_<wchar_t, Targs...>(out, excel_format, value, Fargs...);
		}

		template<typename char_t, typename... Targs>
		inline std::basic_string<char_t> to_csv_row_string_(bool excel_format,Targs const&... Fargs) {
			std::basic_stringstream<char_t> sout;
			to_csv_row(sout, excel_format, Fargs...);
			return sout.str();
		}

		template<typename... Targs>
		inline std::string to_csv_row_string(bool excel_format, Targs const&... Fargs) {
			return to_csv_row_string_<char, Targs...>(excel_format, Fargs...);
		}
		template<typename... Targs>
		inline std::wstring to_csv_row_wstring(bool excel_format, Targs const&... Fargs) {
			return to_csv_row_string_<wchar_t, Targs...>(excel_format, Fargs...);
		}

		template<typename char_t, typename... Targs>
		inline std::basic_string<char_t> to_plain_csv_row_string_(Targs const&... Fargs) {
			std::basic_stringstream<char_t> sout;
			to_csv_row(sout, false, Fargs...);
			return sout.str();
		}
		template<typename... Targs>
		inline std::string to_plain_csv_row_string(Targs const&... Fargs) {
			return to_plain_csv_row_string_<char, Targs...>(Fargs...);
		}
		template<typename... Targs>
		inline std::wstring to_plain_csv_row_wstring(Targs const&... Fargs) {
			return to_plain_csv_row_string_<wchar_t, Targs...>(Fargs...);
		}

		template<typename char_t, typename... Targs>
		static std::basic_string<char_t> to_excel_csv_row_string_(Targs const&... Fargs) {
			std::basic_stringstream<char_t> sout;
			to_csv_row(sout, true, Fargs...);
			return sout.str();
		}
		template<typename... Targs>
		inline std::string to_excel_csv_row_string(Targs const&... Fargs) {
			return to_excel_csv_row_string_<char, Targs...>(Fargs...);
		}
		template<typename... Targs>
		inline std::wstring to_excel_csv_row_wstring(Targs const&... Fargs) {
			return to_excel_csv_row_string_<wchar_t, Targs...>(Fargs...);
		}

		template<typename ValueT, typename string_t = std::string>
		static string_t to_csv(std::vector<ValueT> const& values, bool excel_format = false) {
			using char_t = typename string_t::value_type;
			using symbols = string_util_impl::csv_symbols<char_t>;

			if (values.empty())
				return symbols::empty_s;
			std::basic_stringstream<char_t> sout;
			const size_t last = values.size()-1;
			size_t i = 0;
			for (auto const& v : values) {
				to_csv_row(sout, excel_format, v);
				if (i < last)
					sout << symbols::comma;
				++i;
			}
			return sout.str();
		}

		template< typename ValueT, typename string_t = std::string>
		static string_t to_csv(std::vector <std::vector<ValueT>> const& values, bool excel_format = false) {
			using char_t = typename string_t::value_type;
			using symbols = string_util_impl::csv_symbols<char_t>;
			if (values.empty())
				return symbols::empty_s;
			std::basic_stringstream<char_t> sout;
			const size_t last = values.size() - 1;
			size_t i = 0;
			for (auto const& row : values) {
				sout << to_csv(row, excel_format);
				if (i < last)
					sout << symbols::newline_s;
				++i;
			}
			return sout.str();
		}


		template< typename T>
		inline T convert_string(std::string const&);
		template< typename T>
		inline T convert_string(std::wstring const&);
		template<>
		inline std::string convert_string<std::string>(std::string const& s) { return s; }
		template<>
		inline std::wstring convert_string<std::wstring>(std::wstring const& s) { return s; }

		template<>
		inline int convert_string<int>(std::string const& s) {return std::stoi(s);}
		template<>
		inline int convert_string<int>(std::wstring const& s) { return std::stoi(s); }

		template<>
		inline long convert_string<long>(std::string const& s) { return std::stol(s); }
		template<>
		inline long convert_string<long>(std::wstring const& s) { return std::stol(s); }

		template<>
		inline long long convert_string<long long>(std::string const& s) { return std::stoll(s); }
		template<>
		inline long long convert_string<long long>(std::wstring const& s) { return std::stoll(s); }

		template<>
		inline unsigned long convert_string<unsigned long>(std::string const& s) { return std::stoul(s); }
		template<>
		inline unsigned long convert_string<unsigned long>(std::wstring const& s) { return std::stoul(s); }

		template<>
		inline unsigned long long convert_string<unsigned long long>(std::string const& s) { return std::stoull(s); }
		template<>
		inline unsigned long long convert_string<unsigned long long>(std::wstring const& s) { return std::stoull(s); }


		template<>
		inline float convert_string<float>(std::string const& s) { return std::stof(s); }
		template<>
		inline float convert_string<float>(std::wstring const& s) { return std::stof(s); }


		template<>
		inline double convert_string<double>(std::string const& s) { return std::stod(s); }
		template<>
		inline double convert_string<double>(std::wstring const& s) { return std::stod(s); }

		template<>
		inline long double convert_string<long double>(std::string const& s) { return std::stold(s); }
		template<>
		inline long double convert_string<long double>(std::wstring const& s) { return std::stold(s); }


		template<>
		inline bool convert_string<bool>(std::string const& s) { return equal_nc(trim(s), "true")?true:false; }
		template<>
		inline bool convert_string<bool>(std::wstring const& s) { return equal_nc(trim(s), L"true") ? true : false; }

		template<typename T, typename string_t>
		static void csv_to_vector(string_t const& csv, std::vector<T>& out) {
			using char_t = typename string_t::value_type;
			using symbols = string_util_impl::csv_symbols<char_t>;
			string_t acc;
			for (auto const c : csv) {
				switch (c) {
				case symbols::comma:
					out.push_back(convert_string<T>(trim(acc)));
					acc.clear();
					break;
				default:
					acc.push_back(c);
					break;
				}
			}
			if (!acc.empty())
				out.push_back(trim(acc));
		}

		template<typename T>
		inline std::vector<T> csv_to_vector(std::string const& csv) {
			std::vector<T> target;
			csv_to_vector(csv, target);
			return target;
		}
		template<typename T>
		inline std::vector<T> csv_to_vector(std::wstring const& csv) {
			std::vector<T> target;
			csv_to_vector(csv, target);
			return target;
		}

		template<typename string_t>
		struct to_string_t;

		template<>
		struct to_string_t<std::string> {
			template<typename value_t>
			static std::string convert(value_t const& v) {
				return std::to_string(v);
			}
		};
		template<>
		struct to_string_t<std::wstring> {
			template<typename value_t>
			static std::wstring convert(value_t const& v) {
				return std::to_wstring(v);
			}
		};
	}
}