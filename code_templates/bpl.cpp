#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#include "bpl.h"
#include <experimental/filesystem>
#include "autotelica_core/util/string_util.h"
#include "autotelica_core/util/asserts.h"
#include <vector>
#include <functional>
#include <sstream>
// libstud generates UUIDs, it came from here: https://github.com/libstud/libstud-uuid
// thanks Boris Kolpackov, wherever you are
#include <libstud/uuid.hxx>

using path_t = std::experimental::filesystem::path;
namespace filesystem_n = std::experimental::filesystem;


namespace autotelica {
    namespace {
        class named_values {
            string_util::string_map_nc<std::string, std::string> _values;
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
            bool exists(std::string const& name) const {
                return _values.find(trim(name)) != _values.end();
            }
            void add(std::string const& name, std::string const& value) {
                using namespace string_util;
                auto const nm = trim(name);
                AF_ASSERT(!exists(name),
                    "Duplicate entry in the key-value table: % (keys are not case sensitive)", name);
                _values[nm] = trim(value);
            }
            std::string get(std::string const& name) const {
                using namespace string_util;
                auto const nm = trim(name);
                auto const it = _values.find(nm);
                if(it == _values.end())
                    return "";
                if (is_lowercase(nm))
                    return to_lower(it->second);
                if (is_uppercase(nm))
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
            virtual std::string const& name() const = 0;
        };
        class functions {
            using registry_t = string_util::string_map_nc<std::function<std::shared_ptr<function>()>>;
            static registry_t registry() {
                static registry_t _instance;
                return instance;
            }
        public:
            static bool exists(std::string const& name) {
                return (registry().find(trim(name)) != registry().end());
            }
            static bool register_function(
                std::string const& name,
                std::function<std::shared_ptr<function>()> const& constructor) {
                AF_ASSERT(!exists(name), "Function % is already registered.", name);
                registry()[trim(name)] = constructor;
            }
            static std::shared_ptr<function> create(std::string const& name) {
                if (!exists(name)) return nullptr;
                return registry()[trim(name)]();
            }
        };
        class GUID : public function {
            static std::map<long, std::string> guids() {
                static std::map<long, std::string> _instance;
                return _instance;
            }
        public:
            std::string evaluate(named_values const& values) const override {
                AF_ASSERT(_arguments.size() == 1, "Function GUID takes exactly one argument.");
                long idx = _arguments[0];
                if (guids().find(idx) == guids().end())
                    guids()[idx] = uuid::generate().string();
                return guids()[idx];
            }
            std::string const& name() const override { return "GUID"; }
        };
        static bool register_guid_f = functions::register_function("GUID", 
            []() { return std::shared_ptr<function>(new GUID()); });

        // parsing stuff
        inline void skip_whitespace(std::string const code, size_t& dot) {
            while (dot < code.size() && std::isspace(code[dot]))
                ++dot;
        }
        // look-ahead: see if the token appears next in the code
        inline bool lookahead(std::string const& code, size_t const& dot, std::string const& token) {
            size_t local_dot(dot);
            for (size_t i = 0; i < token.size(); ++i, ++local_dot) {
                if (code[local_dot] != token[i])
                    return false;
            }
            return true;
        }
        long get_integer_arg(std::string const code, size_t& dot, bool& found) {
            // get integer
            // if one is there, otherwise don't
            char* start = const_cast<char*>(code.c_str()) + dot;
            char* end(start);
            long i = strtol(start, &end, 10);
            found = (end != start);
            if (found) {
                dot += (end - start);
                skip_whitespace(code, dot);
            }
            return i;
        }
        double get_floating_arg(std::string const code, size_t& dot, bool& found) {
            // get integer
            // if one is there, otherwise don't
            char* start = const_cast<char*>(code.c_str()) + dot;
            char* end(start);
            double d = strtod(start, &end);
            found = (end != start);
            if (found) {
                dot += (end - start);
                skip_whitespace(code, dot);
            }
            return d;
        }
        std::string get_string_arg(std::string const code, size_t& dot, bool& found) {
            found = false;
            size_t local_dot = dot;
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
            dot = local_dot;
            skip_whitespace(code, dot);
            found = true;
            return ret;
        }
        std::string get_name_arg(
            std::string const code, size_t& dot, 
            bool& found, named_values const& values) {
            
            found = false;
            size_t local_dot = dot;
            skip_whitespace(code, local_dot);
            // argument is terminated by a ',' or a ')'
            // a common error would be to forget a ')', so we will check for '}' and eos
            std::string s;
            char c = code[local_dot++];
            while (
                c != ',' &&
                c != ')' &&
                c != 0 &&
                c != '}' 
                ) {
                s += c;
                c = code[local_dot++];
            }
            // why did we stop?
            AF_ASSERT(c != 0 && c != '}', "Missing closing bracket.");
            // put back the terminator
            --local_dot;
            s = trim(s);
            if (!values.exists(s))
                return "";
            dot = local_dot;
            skip_whitespace(code, dot);
            found = true;
            return s;
        }
        void add_arguments(
            std::string const code, size_t& dot, 
            std::shared_ptr<function> func, named_values const& values) {
            // we assume we are past the brackeet here
            skip_whitespace(code, dot);
            size_t local_dot = dot;
            char c = code[local_dot];

            auto advance_after_arg = [&]() {
                    c = code[local_dot];
                    if (c == ',') {
                        skip_whitespace(code, local_dot);
                        c = code[local_dot]
                    }
                };
            while (c != 0 && c != '}' && c != ')') {
                bool found = false;
                long l = get_integer_arg(code, local_dot, found);
                if (found) {
                    func->add_argument(l);
                    advance_after_arg();
                    continue;
                }
                double f = get_floating_argument(code, local_dot, found);
                if (found) {
                    func->add_argument(f);
                    advance_after_arg();
                    continue;
                }
                std::string s = get_string_argument(code, local_dot, found);
                if (found) {
                    func->add_argument(s);
                    advance_after_arg();
                    continue;
                }
                std::string n = get_name_argument(code, local_dot, found, values);
                if (found) {
                    func->add_argument(n);
                    advance_after_arg();
                    continue;
                }
                AF_ERROR("Unknown argument type for function %.", func->name());
            }
            AF_ASSERT(c == ')', "Missing closing bracket when parsing arguments for %.", func->name());
            ++local_dot;
            dot = local_dot;
        }
        std::string evaluate_name_or_function(
                std::string const& code, size_t& dot, 
                bool strict, std::string const& terminator, 
                named_values const& values, bool& found) {
            found == false;
            size_t local_dot = dot;
            skip_whitespace(code, local_dot);
            // there is only two ways the collection of strings for a name stops:
            //   - we reach a terminator
            //   - we reach an open bracket (meaning, this is a function call)
            std::string name;
            char c = code[local_dot];
            while (!match(code, local_dot, terminator)) {
                if (c == '(') {
                    // is this a function call?
                    auto f = functions::create(trim(name));
                    if (f) {
                        ++local_dot;//skip the bracket
                        add_arguments(code, local_dot, f, values);
                        AF_ASSERT(match(code, local_dot, terminator),
                            "Trailing characters after function % is invoked.", f->name());
                        dot = local_dot;
                        found = true;
                        return f->evaluate(values);
                    }
                }
                name += c;
                c = code[local_dot++];
            }
            
            // if we are here, it's just a name
            if (values.exists(trim(name))) {
                dot == local_dot;
                found = true;
                return values.get(name);
            }
            if(strict)
                AF_ERROR("Unhandled replacement at character: %", dot);
            return "";
        }
        std::string process_filename(
                std::string const& name, 
                bool const strict, 
                named_values const& values) {
            size_t dot{ 0 };
            std::stringstream out;
            char c = name[dot];
            while (c) {
                if (match(name, dot, "__")) {
                    dot += 2;
                    bool found = false;
                    auto value = evaluate_name_or_function(name, dot, strict, "__", values, found);
                    if (found) {
                        out << value;
                        AF_ASSERT(match(name, dot, "__"), "Non-terminated replacement.");
                        dot += 2;
                        continue;
                    }
                }
                out << c;
                c = name[++dot];
            }
            return out.str();
        }
        std::string process_content(
            std::string const& name,
            bool const strict,
            named_values const& values) {
            size_t dot{ 0 };
            std::stringstream out;
            char c = name[dot];
            bool escaping = false;
            while (c) {
                if (!escaping && match(name, dot, "{{")) {
                    dot += 2;
                    bool found = false;
                    auto value = evaluate_name_or_function(name, dot, strict, "}}", values, found);
                    if (found) {
                        out << value;
                        AF_ASSERT(match(name, dot, "}}"), "Non-terminated replacement.");
                        dot += 2;
                        continue;
                    }
                }
                out << c;
                escaping = (c == '\\');
                c = name[++dot];
            }
            return out.str();
        }

    }


    class bpl_impl {
        named_values _values;
        // we keep both string and path representation, for speed
        path_t _source_path;
        path_t _target_path;
        bool _strict;

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
            target_p = process_filename(target_p, _strict, _values);
            return path_t(target_p);
        }

    public:
        bpl_impl(
            std::string const& source_path_,
            std::string const& target_path_,
            bool strict_ = false, 
            std::map<std::string, std::string> const& kvm_ = {}) {

            using namespace string_util;
            _strict = strict_;
            _source_path = path_t(source_path_).make_preferred();
            _target_path = path_t(target_path_).make_preferred();
            for (auto const& e : kvm_) {
                _values.add(e.first, e.second);
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
