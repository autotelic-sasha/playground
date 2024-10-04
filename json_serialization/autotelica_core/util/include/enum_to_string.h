#pragma once
#include <string>
#include <type_traits>
#include "asserts.h"
#include "string_util.h"
#include "diagnostic_messages.h"

namespace autotelica{
    namespace enum_to_string{

        template<typename enum_t, typename string_t = std::string>
        struct enum_to_string {
            static_assert(std::is_enum<enum_t>::value, "Enum to string mapping can only be implemented for enum types.");

            using key_t = string_t;
            using char_t = typename string_t::value_type;
            using mapping_t = std::map<key_t, enum_t>;

        private: 
            static inline bool equal(string_t const& l, const char_t* const r) {
                return (l.c_str() == r || l.compare(r) == 0);
            }
            static inline bool equal(string_t const& l, string_t const& r) {
                return (l.c_str() == r.c_str() || l.compare(r) == 0);
            }
            static inline mapping_t& the_mapping() {
                static mapping_t _the_mapping;
                return _the_mapping;
            }
        public:

            static bool add(key_t const& key, enum_t val) {
                using namespace autotelica::diagnostic_messages;
                using namespace autotelica::string_util;
                for (auto const& p : the_mapping()) {
                    if(equal(p.first, key) && p.second != val)
                        messages::error(utf8_convert<string_t>(
                            "Name % was already declared previously for a different enum value, "
                            "or this enum value was aleady associated with a different name."), key);
                }
                the_mapping()[key] = val;
                return true;
            }
            static enum_t convert(const char_t* const str) {
                
                for (auto const& p : the_mapping()) {
                    if (equal(p.first, str))
                        return p.second;
                }
                throw std::runtime_error("Key not found when converting enum values.");
                return enum_t(- 1);// never happens, just to keep compilers quiet
            }
            static enum_t convert(key_t const& str) {
                auto it = the_mapping().find(str);
                if(it == the_mapping().end())
                    throw std::runtime_error("Key not found when converting enum values.");
                for (auto const& p : the_mapping()) {
                    if (str.c_str() == p.first.c_str() || 
                        p.first.compare(str) == 0)
                        return p.second;
                }
                throw std::runtime_error("Key not found when converting enum values.");
                return enum_t(- 1);// never happens, just to keep compilers quiet
            }
            static key_t const& convert(enum_t const e) {
                for (auto const& p : the_mapping()) {
                    if (p.second == e)
                        return p.first;
                }
                throw std::runtime_error("Value not found when converting enum values.");
                static key_t empty;
                return empty;// never happens, just to keep compilers quiet
            }
            static bool register_enum_tags(enum_t const value, const char_t* const tag) {
                add(tag, value);
                return true;
            }
            template<typename... Targs>
            static bool register_enum_tags(enum_t const value, const char_t* const tag, Targs... Fargs) {
                add(tag, value);
                constexpr std::size_t n = sizeof...(Fargs);
                if(n!=0)
                    return register_enum_tags(Fargs...);
                return true;
            }
        };

        template<typename enum_t, typename string_t = std::string>
        string_t const to_string_t(enum_t const e) {
            return enum_to_string<enum_t, string_t>::convert(e);
        }
        template<typename enum_t>
        std::string const to_string(enum_t const e) {
            try {
                return to_string_t<enum_t, std::string>(e);
            }
            catch (std::runtime_error const&) {
                using namespace autotelica::string_util;
                return utf8::to_string(to_string_t<enum_t, std::wstring>(e));
            }
        }
        template<typename enum_t>
        std::wstring const to_wstring(enum_t const e) {
            try {
                return to_string_t<enum_t, std::wstring>(e);
            }
            catch (std::runtime_error const&) {
                using namespace autotelica::string_util;
                return utf8::to_wstring(to_string_t<enum_t, std::string>(e));
            }
        }

        template<typename enum_t>
        void to_string(std::string& out, enum_t const e) {
            try {
                out = to_string(e);
            }
            catch (std::runtime_error const&) {
                using namespace autotelica::string_util;
                out = utf8::to_string(to_wstring(e));
            }
        }
        template<typename enum_t>
        void to_string(std::wstring& out, enum_t const e) {
            try {
                out = to_wstring(e);
            }
            catch (std::runtime_error const&) {
                using namespace autotelica::string_util;
                out = utf8::to_wstring(to_string(e));
            }
        }

        template<typename enum_t>
        enum_t to_enum(std::string const& s) {
            try {
                return enum_to_string<enum_t, std::string>::convert(s);
            }
            catch (std::runtime_error const&) {
                using namespace autotelica::string_util;
                return enum_to_string<enum_t, std::wstring>::convert(utf8::to_wstring(s));
            }
        }
        template<typename enum_t>
        enum_t to_enum(std::wstring const& s) {
            try {
                return enum_to_string<enum_t, std::wstring>::convert(s);
            }
            catch (std::runtime_error const&) {
                using namespace autotelica::string_util;
                return enum_to_string<enum_t, std::string>::convert(utf8::to_string(s));
            }
        }
        template<typename enum_t>
        enum_t to_enum(const char* const s) {
            try {
                return enum_to_string<enum_t, std::string>::convert(s);
            }
            catch (std::runtime_error const&) {
                using namespace autotelica::string_util;
                return enum_to_string<enum_t, std::wstring>::convert(utf8::to_wstring(s));
            }
        }
        template<typename enum_t>
        enum_t to_enum(const wchar_t* const s) {
            try {
                return enum_to_string<enum_t, std::wstring>::convert(s);
            }
            catch (std::runtime_error const&) {
                using namespace autotelica::string_util;
                return enum_to_string<enum_t, std::string>::convert(utf8::to_string(s));
            }
        }

        template<typename enum_t, typename string_t>
        void to_enum(enum_t& out, string_t const& s) {
            out = to_enum<enum_t>(s);
        }
        template<typename enum_t, typename string_t>
        void to_enum(enum_t& out, const typename string_t::value_type* const s) {
            out = to_enum<enum_t>(s);
        }

    }
}

#define AF_ENUM_TO_STRING_T(enum_t, string_t, ...) \
    namespace __af_string_translation_##enum_t_namespace{\
        static bool __af_string_translation_##enum_t = autotelica::enum_to_string::enum_to_string<enum_t, string_t>::register_enum_tags(__VA_ARGS__);\
    };\
    inline std::ostream& operator<<(std::ostream& out, enum_t const& e) {\
        out << #enum_t << "::" << autotelica::enum_to_string::to_string(e); \
        return out;\
    }\
    inline std::wostream& operator<<(std::wostream& out, enum_t const& e) {\
        out << #enum_t << "::" << autotelica::enum_to_string::to_wstring(e); \
        return out;\
    }

#define AF_ENUM_TO_STRING(enum_t, ...) AF_ENUM_TO_STRING_T(enum_t, std::string, __VA_ARGS__)
#define AF_ENUM_TO_STRING_W(enum_t, ...) AF_ENUM_TO_STRING_T(enum_t, std::wstring, __VA_ARGS__)

