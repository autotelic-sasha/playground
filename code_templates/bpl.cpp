#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#include "bpl.h"
#include <experimental/filesystem>
#include "autotelica_core/util/string_util.h"
#include "autotelica_core/util/asserts.h"
#include <vector>
#include <functional>
using path_t = std::experimental::filesystem::path;
namespace filesystem_n = std::experimental::filesystem;


namespace autotelica {
    namespace {
        class named_values {
            std::map<std::string, std::string> _values;
            bool is_lowercase(std::string const& name) {
                using namespace string_util;
                for (auto const c : name) {
                    if (fast_to_lower(c) != c)
                        return false;
                }
                return true;
            }
            bool is_uppercase(std::string const& name) {
                using namespace string_util;
                for (auto const c : name) {
                    if (fast_to_upper(c) != c)
                        return false;
                }
                return true;
            }
        public:
            void add(std::string const& name, std::string const& value) {
                using namespace string_util;
                auto const lk = to_lower(name);
                AF_ASSERT(_values.find(lk) == _values.end(),
                    "Duplicate entry in the key-value table: % (keys are not case sensitive)", name);
                _values[lk] = value;
            }
            std::string get(std::string const& name) {
                using namespace string_util;
                auto const& lk = to_lower(name);
                auto const it = _values.find(lk);
                if(it == _values.end())
                    return "";
                if (is_lowercase(name))
                    return to_lower(it->second);
                if (is_uppercase(name))
                    return to_upper(it->second);
                return it->second;
            }
        };
        enum class argument_t {
            integer,
            floating,
            text
        };
        class argument {
            // we will keep this real simple too
            // unions are painful, so we will waste a little bit of memory
            argument_t _type;
            long _integer{ 0 };
            double _floating{ 0 };
            std::string _text;
        public:
            argument(long i):_type(argument_t::integer), _integer(i){}
            argument(double f) :_type(argument_t::floating), _floating(f) {}
            argument(std::string const& s) :_type(t), _text(s) {}

            long integer() const {
                AF_ASSERT(_type == argument_t::integer, "Argument is of wrong type, expected integer.");
                return _integer;
            }
            double floating() const {
                AF_ASSERT(_type == argument_t::floating, "Argument is of wrong type, expected a floating point number.");
                return _floating;
            }
            std::string const& text() const {
                AF_ASSERT(_type == argument_t::text, "Argument is of wrong type, expected text.");
                return _text;
            }
        };
        class function {
        protected:
            std::vector<argument> _arguments;
        public:
            function(){}
            // functions in bpl are evaluated only once, during parsing
            // it's handy to build them on the fly
            inline void add_argument(long i) {
                _arguments.emplace_back(i);
            }
            inline void add_argument(double f) {
                _arguments.emplace_back(f);
            }
            inline void add_argument(std::string const& s) {
                _arguments.emplace_back(s);
            }

            virtual std::string evaluate(named_values const& values) const = 0;
        };
        class functions {
            using registry_t = string_util::string_map_nc<std::function<std::shared_ptr<function>()>>;
            static registry_t registry() {
                static registry_t _instance;
                return instance;
            }
            public:
                static bool exists(std::string const& name) {
                    return (registry().find(name) != registry().end());
                }
                static bool register_function(
                    std::string const& name,
                    std::function<std::shared_ptr<function>()> const& constructor) {
                    AF_ASSERT(!exists(name), "Function % is already registered.", name);
                    registry()[name] = constructor;
                }
                static std::shared_ptr<function> create(std::string const& name) {
                    if (!exists(name)) return nullptr;
                    return registry()[name];
                }
        }

        inline void skip_whitespace(std::string const code, size_t& dot) {
            while (dot < code.size() && std::isspace(code[dot]))
                ++dot;
        }
        // look-ahead: see if the token appears next in the code
        // if it does, move dot to the one pass the end of the token
        inline bool match(std::string const& code, size_t& dot, std::string const& token) {
            size_t local_dot(dot);
            for (size_t i = 0; i < token.size(); ++i, ++local_dot) {
                if (code[local_dot] != token[i])
                    return false;
            }
            dot = local_dot;
            return true;
        }
        long get_int(std::string const code, size_t& dot, bool& found) {
            // get integer
            // if one is there, otherwise don't
            char* start = const_cast<char*>(code.c_str()) + local_dot;
            char* end(start);
            long i = strtol(start, &end, 10);
            found = (end != start);
            return i;
        }
        double get_double(std::string const code, size_t& dot, bool& found) {
            // get integer
            // if one is there, otherwise don't
            char* start = const_cast<char*>(code.c_str()) + local_dot;
            char* end(start);
            double d = strtod(start, &end);
            found = (end != start);
            return d;
        }
        std::string get_string(std::string const code, size_t& dot, bool& found) {
            found = false;
            size_t local_dot = dot;
            skip_whitespace(code, local_dot);
            if (code[local_dot] != '"')
                return "";
            ++local_dot;
            std::string ret;
            char c = code[local_dot++];
            while (c && c != '"') {
                if (c == '\\' && code[local_dot + 1] == '"') {
                    // quotes can be escaped
                    ++local_dot;
                    ret += c;
                    continue;
                }
                ret += c;
                c = code[local_dot++];
            }
            if (c != '"')
                return "";
            found = true;
            return ret;

        }

    }


    class bpl_impl {
        std::map<std::string, std::string> _kvm;
        // we keep both string and path representation, for speed
        path_t _source_path;
        path_t _target_path;

        path_t make_target_path(
                path_t const& source_path, 
                path_t const& source_folder, 
                path_t const& target_folder) {
            auto const source_p = source_path.string();
            auto const source_f = source_folder.string();
            auto const target_f = target_folder.string();
            
            auto target_p = string_util::replace(source_p, source_f, target_f);
            AF_ASSERT(source_p != target_p,
                "Source folder doesn't seem to be present in the path % (source folder is: %)", source_p, source_f);
            return path_t(target_p);
        }

    public:
        bpl_impl(
            std::string source_path_,
            std::string target_path_,
            std::map<std::string, std::string> kvm_) {

            using namespace string_util;

            _source_path = path_t(source_path_).make_preferred();
            _target_path = path_t(target_path_).make_preferred();
            for (auto const& e : kvm_) {
                auto const lk = to_lower(e.first);
                AF_ASSERT(_kvm.find(lh) == _kvm.end(),
                    "Duplicate entry in the key-value table: % (keys are not case sensitive)", e.first);
                _kvm[lk] = e.second;
            }
            AF_ASSERT(filesystem_n::exists(_source_path), 
                "Source folder % does not exist.", source_path_);

            AF_ASSERT(to_lower(_target_path.string()).find(to_lower(_source_path.string())) == std::string::npos,
                "Target path cannot be a sub-directory of the source path.");

        }
    };

    bpl::bpl(
        std::string source_path_str_,
        std::string target_path_str_,
        std::map<std::string, std::string> kvm_) :
    _impl(new bpl_impl(source_path_, target_path_, kvm_){
    }
}
