#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#include "bpl.h"
#include <experimental/filesystem>
#include <vector>
#include <functional>
#include <sstream>
#include <istream>
#include <algorithm>
#include <random>
#include <cstdlib>
#include "autotelica_core/util/include/string_util.h"
#include "autotelica_core/util/include/asserts.h"

#include "rapidjson/document.h"
#include "rapidjson/error/en.h"

using path_t = std::experimental::filesystem::path;
namespace filesystem_n = std::experimental::filesystem;
using namespace autotelica::string_util;

namespace autotelica {
    namespace {
        std::string generate_hex(size_t len) {
            std::stringstream ss;
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(0, 15);

            for (size_t i = 0; i < len; i++) {
                ss << std::hex << dis(gen);
            }
            return ss.str();
        }
        std::string simple_uuid() {
            // simple, not really guaranteed to be universal uuids, but good enough for visual studio
            std::stringstream ss;
            ss << generate_hex(8) << "-" <<
                generate_hex(4) << "-" <<
                generate_hex(4) << "-" <<
                generate_hex(4) << "-" <<
                generate_hex(12);
            return to_upper(ss.str());
        }

        class named_values {
            string_map_nc<std::string> _values;
            bool is_lowercase(std::string const& name) const {
                for (auto const c : name) {
                    if (fast_to_lower(c) != c)
                        return false;
                }
                return true;
            }
            bool is_uppercase(std::string const& name) const {
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
                auto const nm = trim(name);
                AF_ASSERT(!exists(name),
                    "Duplicate entry in the key-value table: % (keys are not case sensitive)", name);
                _values[nm] = trim(value);
            }
            std::string get(std::string const& name) const {
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
            argument(std::string const& s) :_type(argument_t::text), _text(s) {}

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
            using registry_t = string_map_nc<std::function<std::shared_ptr<function>()>>;
            static registry_t& registry() {
                static registry_t _instance;
                return _instance;
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
                return true;
            }
            static std::shared_ptr<function> create(std::string const& name) {
                if (!exists(name)) return nullptr;
                return registry()[trim(name)]();
            }
        };
        class GUID : public function {
            static std::map<long, std::string>& guids() {
                static std::map<long, std::string> _instance;
                return _instance;
            }
            static std::string const& sname() {
                static std::string _name{ "GUID" };
                return _name;
            }
        public:
            std::string evaluate(named_values const& values) const override {
                AF_ASSERT(_arguments.size() == 1, "Function GUID takes exactly one argument.");
                long idx = _arguments[0].integer();
                if (guids().find(idx) == guids().end()) {
                    guids()[idx] = simple_uuid();
                }
                return guids()[idx];
            }
            std::string const& name() const override { return GUID::sname(); }
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
                        ++local_dot;
                        skip_whitespace(code, local_dot);
                        c = code[local_dot];
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
                double f = get_floating_arg(code, local_dot, found);
                if (found) {
                    func->add_argument(f);
                    advance_after_arg();
                    continue;
                }
                std::string s = get_string_arg(code, local_dot, found);
                if (found) {
                    func->add_argument(s);
                    advance_after_arg();
                    continue;
                }
                std::string n = get_name_arg(code, local_dot, found, values);
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
            found = false;
            size_t local_dot = dot;
            skip_whitespace(code, local_dot);
            // there is only two ways the collection of strings for a name stops:
            //   - we reach a terminator
            //   - we reach an open bracket (meaning, this is a function call)
            std::string name;
            char c = code[local_dot++];
            while (c){
                if (lookahead(code, local_dot, terminator)) {
                    name += c;
                    break;
                }
                if (c == '(') {
                    // is this a function call?
                    auto f = functions::create(trim(name));
                    if (f) {
                        local_dot;//skip the bracket
                        add_arguments(code, local_dot, f, values);
                        AF_ASSERT(lookahead(code, local_dot, terminator),
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
            if (values.exists(name)) {
                dot = local_dot;
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
                if (lookahead(name, dot, "__")) {
                    dot += 2;
                    bool found = false;
                    auto value = evaluate_name_or_function(name, dot, strict, "__", values, found);
                    if (found) {
                        out << value;
                        AF_ASSERT(lookahead(name, dot, "__"), "Non-terminated replacement.");
                        dot += 2;
                        c = name[dot];
                        continue;
                    }
                    else {
                        dot -= 2;
                    }
                }
                out << c;
                if(c)// c==0 is end of string
                    c = name[++dot];
            }
            return out.str();
        }
        std::string get_escaped(
            std::string const& content,
            size_t& dot
        ) {
            char c = content[++dot];
            std::string ret;
            ret += c;
            while (c) {
                if (lookahead(content, dot, "}}}}")) {
                    dot += 4;
                    ret += "}}";
                    break;
                }
            }
            AF_ASSERT(c, "Missing closing braces.");
            return ret;
        }
        std::string process_content(
                std::string const& content,
                bool const strict,
                named_values const& values) {
            size_t dot{ 0 };
            std::stringstream out;
            char c = content[dot];
            while (c) {
                if (lookahead(content, dot, "{{")) {
                    dot += 2;
                    if (lookahead(content, dot, "{{")) {
                        dot += 2;
                        out << get_escaped(content, dot); 
                        c = content[dot];
                    }
                    else {
                        bool found = false;
                        auto value = evaluate_name_or_function(content, dot, strict, "}}", values, found);
                        if (found) {
                            out << value;
                            AF_ASSERT(lookahead(content, dot, "}}"), "Non-terminated replacement.");
                            dot += 2;
                            c = content[dot];
                            continue;
                        }
                        else {
                            dot -= 2;
                        }
                    }
                }
                if (c) {
                    out << c;
                    c = content[++dot];
                }
            }
            return out.str();
        }
        void list_required_names_for_filename(std::string const& path, string_map_nc<string_set_nc>& sections) {
            size_t dot{ 0 };
            char c = path[dot];
            while (c) {
                if (lookahead(path, dot, "__")) {
                    dot += 2;
                    if (path[dot] == '_') {
                        dot -= 2;
                    }
                    else {
                        size_t end_of_name = dot;
                        while (path[end_of_name++] && !lookahead(path, end_of_name, "__"))
                            ;
                        AF_ASSERT(path[end_of_name], "Missing closing name underscores.");
                        auto name = path.substr(dot, end_of_name - dot);
                        dot = end_of_name + 2;
                        auto i = name.find('.');
                        if (i == std::string::npos)
                            sections[""].insert(trim(name));
                        else
                            sections[trim(name.substr(0, i))].insert(trim(name.substr(i+1)));
                    }
                    if(!path[dot])
                        break;
                }
                c = path[++dot];
            }
        }
        void list_required_names(
                std::string const& content, 
                string_map_nc<string_set_nc>& sections, 
                bool ignore_functions = true) {
            size_t dot{ 0 };
            char c = content[dot];
            while (c) {
                if (lookahead(content, dot, "{{")) {
                    dot += 2;
                    if (lookahead(content, dot, "{{")) {
                        dot += 2;
                    }
                    else if (content[dot] == '{') {
                        dot -= 2;
                    }
                    else {
                        size_t end_of_name = dot;
                        while (content[end_of_name++] && !lookahead(content, end_of_name, "}}"))
                            ;
                        AF_ASSERT(content[end_of_name], "Missing closing name braces.");
                        auto name = content.substr(dot, end_of_name - dot);
                        dot = end_of_name + 2;
                        if (!ignore_functions || name.find('(') == std::string::npos) { // function calls are not included here
                            auto i = name.find('.');
                            if (i == std::string::npos)
                                sections[""].insert(trim(name));
                            else
                                sections[trim(name.substr(0, i))].insert(trim(name.substr(i + 1)));
                        }
                    }
                }
                c = content[++dot];
            }
        }

        // files and stuff
        std::string read_file(path_t const& path){
            std::ifstream f(path);
            const auto sz = filesystem_n::file_size(path);
            std::string result(sz, '\0');
            f.read(&(result[0]), sz);
            return result;
        }
        void write_file(path_t const& path, std::string const& s) {
            std::ofstream f(path);
            f << s;
        }

        class special_files {
            static void get_from_github(path_t const& gitclone_file, path_t const& target_file) {
                auto content = trim(read_file(gitclone_file));
                AF_ASSERT(!content.empty() && content.find('\n') == std::string::npos,
                    "Gitclone file must contain one line and one line only.");
                
                auto target_folder = target_file.parent_path().make_preferred().string();
                auto command = af_format_string("git clone % %", content, target_folder);

                AF_WARNING("Cloning git repository with command '%'.\nCHECK that it worked, it's really hard to check it programatically, sorry.", command);
                system(command.c_str());
            }
            static constexpr const char * const _gitclone = "__GITCLONE__";
        public:
            static bool is_special(path_t const& the_file) {
                static string_set_nc _specials{ _gitclone };
                auto const fname = the_file.filename().string();
                return (_specials.find(fname) != _specials.end());
            }
            static bool handle_if_special(path_t const& the_file, path_t const& target_file) {
                auto filename = the_file.filename().string();
                if (equal_nc(filename, _gitclone)) {
                    get_from_github(the_file, target_file);
                    return true;
                }
                return false;
            }
        };
        // config files
        const char* const tag_extensions_to_ignore = "extensions_to_ignore";
        const char* const tag_files_to_ignore = "files_to_ignore";
        const char* const tag_sections = "sections";
        const char* const tag_name = "name";
        const char* const tag_named_values = "named_values";
        const char* const tag_value = "value";

        bool parse_ini_config_file(
                path_t const& path,
                named_values& values,
                std::vector<std::string>& extensions_to_ignore, 
                std::vector<std::string>& files_to_ignore) {
            std::string section;
            std::string line;
            std::ifstream f(path);
            // every line is a comment, a section start, or a value
            while (std::getline(f, line)) {
                line = trim(line);
                if (line.empty()) continue;
                if (line[0] == ';') continue; // ignore comments
                if (line[0] == '[') {//section
                    AF_ASSERT(line.back() == ']', "Missing ']' when parsing ini file.");
                    section = "";
                    section = trim(line.substr(1, line.size()-2));
                    continue;
                }
                const auto eq = line.find('=');
                AF_ASSERT(eq != std::string::npos, "Missing '='  when parsing ini file.");
                auto const name = trim(line.substr(0, eq));
                if (section.empty() && name == tag_extensions_to_ignore)
                    csv_to_vector(to_lower(line.substr(eq + 1)), extensions_to_ignore);
                else if (section.empty() && name == tag_files_to_ignore)
                    csv_to_vector(to_lower(line.substr(eq + 1)), files_to_ignore);
                else {
                    if(section.empty())
                        values.add(name, line.substr(eq + 1));
                    else
                        values.add(section+"."+name, line.substr(eq + 1));
                }
            }
            return true;
        }
        std::string create_template_ini(
            string_map_nc<string_set_nc> const& sections,
            std::vector<std::string> const& extensions_to_ignore_,
            std::vector<std::string> const& files_to_ignore_) {
            std::stringstream out;
            out << tag_extensions_to_ignore << " =  " << to_csv(extensions_to_ignore_) << std::endl;
            out << tag_files_to_ignore << " =  " << to_csv(files_to_ignore_) << std::endl;
            for (auto e : sections) {
                if (!e.first.empty()) {
                    out << "[" << e.first << "]" << std::endl;
                }
                for (auto const name : e.second) {
                    out << name << " = " << std::endl;
                }
            }
            return out.str();
        }
        std::string create_template_json(
            string_map_nc<string_set_nc> const& sections, 
            std::vector<std::string> const& extensions_to_ignore_,
            std::vector<std::string> const& files_to_ignore_
            ) {
            std::stringstream out;
            out << "{" << std::endl;

            out << "\t\"" << tag_extensions_to_ignore << "\" : \""<< to_csv(extensions_to_ignore_) << "\"," << std::endl;
            out << "\t\"" << tag_files_to_ignore << "\" : \""<< to_csv(files_to_ignore_) << "\"," << std::endl;

            out << "\t\"sections\":[" << std::endl;

            size_t szs = sections.size();
            size_t is = 0;
            for (auto e : sections) {
                ++is;
                out << "\t{" << std::endl;
                out << "\t\t\"name\":\"" << e.first << "\"," << std::endl;
                out << "\t\t\t\"named_values\" : [" << std::endl;

                size_t sz = e.second.size();
                size_t i = 0;
                for (auto const name : e.second) {
                    ++i;
                    out << "\t\t\t\t{\"name\":\"" << name << "\",\"value\":\"\"}";
                    if (i < sz)
                        out << ",";
                    out << std::endl;
                }
                out << "\t\t\t]\n\t}";
                if (is < szs)
                    out << ",";
                out << std::endl;
            }
            out << "\t]" << std::endl;
            out << "}" << std::endl;
            return out.str();
        }

        std::string get_json_string(
            rapidjson::Value const& o,
            const char* const tag
        ) {
            AF_ASSERT(o[tag].IsString(), "% value must be a string.", tag);
            return o[tag].GetString();
        }
        void parse_json_named_values(
            std::string const& section,
            rapidjson::Value const& v,
            named_values& values
        ) {
            AF_ASSERT(v.IsArray(), "% block must be an array.", tag_named_values);
            for (auto const& nv : v.GetArray()) {
                AF_ASSERT(nv.IsObject(), "% array members must be object.", tag_named_values);
                auto const nvo = nv.GetObject();
                AF_ASSERT(nvo.HasMember(tag_name) && nvo.HasMember(tag_value), 
                    "% objects must contain names and values.", tag_named_values);
                std::string name = get_json_string(nvo,tag_name);
                std::string value = get_json_string(nvo, tag_value);
                if (section.empty())
                    values.add(name, value);
                else
                    values.add(section + "." + name, value);
            }
        }
        bool parse_json_config_file(
                path_t const& path,
                named_values& values,
                std::vector<std::string>& extensions_to_ignore,
                std::vector<std::string>& files_to_ignore) {
            std::string content = read_file(path);
            rapidjson::Document d;
            d.ParseInsitu(&content[0]);
            if (d.HasParseError()) {
                auto error = rapidjson::GetParseError_En(d.GetParseError());
                AF_ERROR("Failed to parse json [at %]: %", d.GetErrorOffset(), error);
                return false;
            }
            AF_ASSERT(d.IsObject(), "Top level JSON must be an object.");
            auto const top = d.GetObject();
            if (top.HasMember(tag_extensions_to_ignore)) {
                csv_to_vector(to_lower(get_json_string(top, tag_extensions_to_ignore)), extensions_to_ignore);
            }
            if (top.HasMember(tag_files_to_ignore)) {
                csv_to_vector(to_lower(get_json_string(top, tag_files_to_ignore)), files_to_ignore);
            }
            if (top.HasMember(tag_named_values)) {
                parse_json_named_values("", top[tag_named_values], values);
            }
            if (top.HasMember(tag_sections)) {
                AF_ASSERT(top[tag_sections].IsArray(), "% block must be an array.", tag_sections);
                for (auto const& js_section : top[tag_sections].GetArray()) {
                    AF_ASSERT(js_section.IsObject(), "% members must be objects.", tag_sections);
                    auto const jso = js_section.GetObject();
                    std::string section;
                    if (jso.HasMember(tag_name))
                        section = get_json_string(js_section, tag_name);
                    if (jso.HasMember(tag_named_values))
                        parse_json_named_values(section, jso[tag_named_values], values);
                }
            }
            return true;
        }
    }


    class bpl_impl {
        named_values _values;
        // we keep both string and path representation, for speed
        path_t _source_path;
        path_t _target_path;
        path_t _config_path;
        bool _strict;
        bool _force;
        // ignoring only refers to content of files, names are still parsed
        std::vector<std::string> _extensions_to_ignore;
        std::vector<std::string> _files_to_ignore;
        std::vector<std::string> _paths_to_ignore;

        bool starts_with(std::string const& s, std::string const& prefix) {
            for (size_t i = 0; i < prefix.size(); ++i)
                if (s[i] != prefix[i])
                    return false;
            return true;
        }
        void cache_path_to_ignore(std::string const& p) {
            if (std::find(_paths_to_ignore.begin(), _paths_to_ignore.end(), p) == _paths_to_ignore.end())
                _paths_to_ignore.push_back(p);
        }
        bool ignored(path_t const& target_path) {
            if (_extensions_to_ignore.empty() &&
                _files_to_ignore.empty())
                return false;
            auto const path_s = to_lower(target_path.string());
            auto const extension = to_lower(target_path.extension().string());
            if (std::find(_extensions_to_ignore.begin(), _extensions_to_ignore.end(), extension) != _extensions_to_ignore.end()) {
                cache_path_to_ignore(path_s);
                return true;
            }
            auto const fname = to_lower(target_path.filename().string());
            if (std::find(_files_to_ignore.begin(), _files_to_ignore.end(), fname) != _files_to_ignore.end()) {
                cache_path_to_ignore(path_s);
                return true;
            }
            for (auto const& pattern : _files_to_ignore) {
                if(wildcard_match(fname, pattern)) {
                    cache_path_to_ignore(path_s);
                    return true;
                }
            }
            
            for (auto p : _paths_to_ignore) {
                if(starts_with(path_s, p))
                    return true;
            }
            return false;
        }
        path_t make_target_path(
                path_t const& source_path, 
                path_t const& source_folder, 
                path_t const& target_folder) {
            auto const source_p = source_path.string();
            auto const source_f = source_folder.string();
            auto const target_f = target_folder.string();
            
            auto target_p = replace(replace(replace(source_p, source_f, target_f), "\\\\","\\"), "//", "/");
            AF_ASSERT(source_p != target_p,
                "Source folder doesn't seem to be present in the path % (source folder is: %)", source_p, source_f);

            if(!special_files::is_special(source_path))
                target_p = process_filename(target_p, _strict, _values);
            else {
                auto const target_root_s = process_filename(
                    path_t(target_p).parent_path().make_preferred().string(),
                    _strict, _values);
                target_p = path_t(target_root_s).append(source_path.filename()).string();
            }
            return path_t(target_p);
        }
        void copy_target(path_t const& source_path, path_t const& target_path) {
            AF_ASSERT(_force || !filesystem_n::exists(target_path),
                "% already exists.", target_path.string());
            if (filesystem_n::is_directory(source_path))
                filesystem_n::create_directories(target_path);
            else if(ignored(target_path))
                filesystem_n::copy(source_path, target_path);
            else {
                if (special_files::handle_if_special(source_path, target_path))
                    return;
                auto const content = read_file(source_path);
                auto const new_content = process_content(content, _strict, _values);
                write_file(target_path, new_content);
            }
        }

    public:
        bpl_impl(
                std::string const& source_path_,
                std::string const& target_path_,
                std::string const& config_path_,
                bool strict_ = false,
                bool force_ = false,
                std::string const& extensions_to_ignore_ = "",
                std::string const& files_to_ignore_ = "") {
            _strict = strict_;
            _force = force_;
            _source_path = path_t(source_path_).make_preferred();
            _target_path = path_t(target_path_).make_preferred();
            _config_path = path_t(config_path_).make_preferred();
            AF_ASSERT(filesystem_n::exists(_source_path),
                "Source folder % does not exist.", source_path_);
            
            if (filesystem_n::exists(_config_path)) {
                if (wildcard_match(_config_path.string(), "*.json")) {
                    parse_json_config_file(config_path_, _values, _extensions_to_ignore, _files_to_ignore);
                }
                else{
                    parse_ini_config_file(config_path_, _values, _extensions_to_ignore, _files_to_ignore);
                }
            }
            if (!extensions_to_ignore_.empty())
                csv_to_vector(to_lower(extensions_to_ignore_), _extensions_to_ignore);
            if (!files_to_ignore_.empty())
                csv_to_vector(to_lower(files_to_ignore_), _files_to_ignore);
        }
            
        bpl_impl(
                std::string const& source_path_,
                std::string const& target_path_,
                bool strict_ = false, 
                bool force_ = false,
                std::string const& extensions_to_ignore_ = "",
                std::string const& files_to_ignore_ = "",
                std::map<std::string, std::string> const& kvm_ = {}) {
            _strict = strict_;
            _force = force_;
            csv_to_vector(to_lower(extensions_to_ignore_), _extensions_to_ignore);
            csv_to_vector(to_lower(files_to_ignore_), _files_to_ignore);

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

        void generate() {
            using recursive_directory_iterator = filesystem_n::recursive_directory_iterator;
            for (const auto& p : recursive_directory_iterator(_source_path)) {
                try {
                    auto target_path = make_target_path(p.path(), _source_path, _target_path);
                    copy_target(p.path(), target_path);
                }
                catch (std::exception& e) {
                    if(wildcard_match(e.what(), "[*] ERROR: *"))
                        AF_ERROR("Error while processing %.", p.path().string());
                    else
                        AF_ERROR("Error while processing % :\n %", p.path().string(), e.what());
                }
                catch (...) {
                    AF_ERROR("Unknown error while processing %.", p.path().string());
                }
            }
        }

        void generate_config_files() {
            string_map_nc<string_set_nc> sections;
            using recursive_directory_iterator = filesystem_n::recursive_directory_iterator;
            for (const auto& p : recursive_directory_iterator(_source_path)) {
                list_required_names_for_filename(p.path().string(), sections);
                if(ignored(p) || filesystem_n::is_directory(p))
                    continue;
                auto const content = read_file(p);
                list_required_names(content, sections);
            }

            if (wildcard_match(_config_path.string(), "*.json")) {
                std::string json = create_template_json(sections, _extensions_to_ignore, _files_to_ignore);
                write_file(_config_path, json);
            }
            else {
                std::string ini = create_template_ini(sections, _extensions_to_ignore, _files_to_ignore);
                write_file(_config_path, ini);
            }
        }
        void describe() {
            using recursive_directory_iterator = filesystem_n::recursive_directory_iterator;
            for (const auto& p : recursive_directory_iterator(_source_path)) {
                string_map_nc<string_set_nc> sections;
                list_required_names_for_filename(p.path().string(), sections);
                if (!(ignored(p) || filesystem_n::is_directory(p))) {
                    auto const content = read_file(p);
                    list_required_names(content, sections, false);
                }
                if (!sections.empty()) {
                    std::cout << p.path().string() << std::endl;
                    for (auto const& e : sections) {
                        for (auto const& name : e.second) {
                            std::cout << "\t";
                            if (!e.first.empty())
                                std::cout << e.first << ".";
                            std::cout << name << std::endl;
                        }
                    }
                }
            }

        }


    };


    bpl::bpl(
            std::string const& source_path_,
            std::string const& target_path_,
            std::string const& config_path_,
            bool strict_,
            bool force_,
            std::string const& extensions_to_ignore_,
            std::string const& files_to_ignore_) :
        _impl(new bpl_impl(
            source_path_, 
            target_path_, 
            config_path_, 
            strict_,
            force_,
            extensions_to_ignore_,
            files_to_ignore_)) {
    }

    bpl::bpl(
            std::string const& source_path_,
            std::string const& target_path_,
            bool strict_,
            bool force_,
            std::string const& extensions_to_ignore_,
            std::string const& files_to_ignore_,
            std::map<std::string, std::string> const& kvm_) :
        _impl(new bpl_impl(
            source_path_,
            target_path_,
            strict_,
            force_,
            extensions_to_ignore_,
            files_to_ignore_,
            kvm_)) {
    }

    void bpl::generate() {
        _impl->generate();
    }
    void bpl::generate_config_files() {
        _impl->generate_config_files();
    }
    void bpl::describe() {
        _impl->describe();
    }
}
