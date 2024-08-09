#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#include "bpl_impl.h"
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
#include "autotelica_core/util/include/std_pretty_printing.h"
#include "rapidjson/document.h"
#include "rapidjson/error/en.h"

using path_t = std::experimental::filesystem::path;
namespace filesystem_n = std::experimental::filesystem;
using namespace autotelica::string_util;
using namespace autotelica::std_pretty_printing;

namespace autotelica {
    // files and stuff
    // We don't parse binary files, so something simple for reading and writing.
    // Also, we don't expect text files to be so big to cause problems 
    // when loading them to memory for parsing. 
    std::string read_file(path_t const& path) {
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

    // Special files are like functions for special actions. 
    // At the time of writing this comment, __GITCLONE__ is the only example, 
    // it fetches a content of a github repo while creating a project. 
    // In time, I expect we'll have something that copies folders from elsewhere,
    // maybe some other files from the internet, and who knows what else.
    class special_files {
        // get a repo from github, the address is the only line in the file.
        static void get_from_github(path_t const& gitclone_file, path_t const& target_file) {
            auto content = trim(read_file(gitclone_file));
            AF_ASSERT(!content.empty() && content.find('\n') == std::string::npos,
                "Gitclone file must contain one line and one line only.");

            auto target_folder = target_file.parent_path().make_preferred().string();
            auto command = af_format_string("git clone --depth=1 % %", content, target_folder);

            AF_WARNING("Cloning git repository with command '%'.\nCHECK that it worked, it's really hard to check it programatically, sorry.", command);
            system(command.c_str());
            auto const dot_git_folder = target_file.parent_path().append(".git");
            if (filesystem_n::exists(dot_git_folder)) {
                auto const new_dot_git_folder = target_file.parent_path().append(".original_dot_git");
                filesystem_n::rename(dot_git_folder, new_dot_git_folder);
            }
        }

        // 'specials' is a registry that maps special file names to functions that handle them
        using specials_t = string_map_nc<std::function<void(path_t const&, path_t const&)>>;
        static specials_t const& _specials() {
            static specials_t _specials{ {"__GITCLONE__", get_from_github} };
            return _specials;
        }
    public:
        // check if a file is special
        static bool is_special(path_t const& the_file) {
            auto const fname = the_file.filename().string();
            return (_specials().find(fname) != _specials().end());
        }
        // check is a name refers to a special file
        // it is not completely perfect, but it is only used for creating descriptions
        // simplicity matters more here
        static bool is_special(std::string const& name) {
            if (_specials().find(name) != _specials().end())
                return true;
            return (_specials().find("__" + name + "__") != _specials().end());
        }

        // deal with special files, if they are special
        static bool handle_if_special(path_t const& the_file, path_t const& target_file) {
            auto filename = the_file.filename().string();
            auto const fname = the_file.filename().string();
            auto const& it = _specials().find(fname);
            if (it != _specials().end()) {
                it->second(the_file, target_file);
                return true;
            }
            return false;
        }
    };
    // named values
    // This is a container for the simple replacement values. 
    // It implements the bpl semantics for handling the case of replacemens.
    class named_values {
        string_map_nc<std::string> _values;
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
            if (it == _values.end())
                return "";
            if (is_lowercase(nm))
                return to_lower(it->second);
            if (is_uppercase(nm))
                return to_upper(it->second);
            return it->second;
        }
    };
    // functions
    // simple, not really guaranteed to be universal uuids, but good enough for visual studio
    // using them to create GUIDs in projects
    std::string simple_uuid() {
        std::stringstream ss;
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 15);
        auto generate_hex = [&](size_t digits, bool last = false) {
            for (size_t i = 0; i < digits; i++)
                ss << std::hex << dis(gen);
            if (!last) ss << "-";
        };
        generate_hex(8);
        generate_hex(4);
        generate_hex(4);
        generate_hex(4);
        generate_hex(12, true);
        return to_upper(ss.str());
    }

    // Implementation of functions.
    // Arguments can only be one of three types
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
        argument(long i) :_type(argument_t::integer), _integer(i) {}
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
    // base class for functions
    // just some arguments, added as we parse, and a function to evaluate it
    // oh, also one to report it's name, nice for checking, documenting and all that
    class function {
    protected:
        std::vector<argument> _arguments;
    public:
        function() {}
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
    // a registry of all available functions
    // to add a new function, implement a class derived from 'function'
    // then register it through static initialisation with 'functions'
    class functions {
        // at the core of it is a non-case sensitive mapping
        // of function names to factory functions that create function objects (LOL, but it is all functional :D)
        // ok, ok ... in the lingo: 'functions' is an abstract factory of 'function' objects
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
    // function GUID
    // Creates GUIDs. (or close enough)
    // Takes a single integer argument which is used as an identifier for a GUID
    // During a single run GUID(x) always returns the same guid for the same x. 
    // NOT TRUE between runs. 
    class GUID : public function {
        // there is a static storage for generated guids
        // because we need to be able to repeat them
        // visual studio uses them as handles, so they get repeated
        static std::map<long, std::string>& guids() {
            static std::map<long, std::string> _instance;
            return _instance;
        }
        static std::string const& sname() { // C++ 14 doesn't have static data members, oh well
            static std::string _name{ "GUID" };
            return _name;
        }
    public:
        std::string evaluate(named_values const& values) const override {
            AF_ASSERT(_arguments.size() == 1, "Function GUID takes exactly one argument.");
            long idx = _arguments[0].integer();
            if (guids().find(idx) == guids().end()) 
                guids()[idx] = simple_uuid();
            return guids()[idx];
        }
        std::string const& name() const override { return GUID::sname(); }
    };
    // this is where function GUID is registered. 
    static bool register_guid_f = functions::register_function(
        "GUID",
        []() { return std::shared_ptr<function>(new GUID()); });

    //
    // parsing stuff
    //
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

    // parsing functions
    // All recursive decent functions are required to be nice:
    //      clean up any trailing whitespace
    //      move the dot to the next token that is to be read
    // That way they can all start reading from the dot.

    // function argument parsing, one for each type
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

    // there is a subtlety to text arguments
    // they can be either some random strings, in which case they must be quoted
    // or they can be names from the replacement mapping, in which case they are not quoted
    //      and they get evaluated to their replacement values immediately
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
    // parsing the list of all arguments for a function
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

    // parse and immediately evaluate a replaceable name or a function
    std::string evaluate_name_or_function(
        std::string const& code, size_t& dot,
        std::string const& terminator,
        named_values const& values, bool& found) {
        found = false;
        size_t local_dot = dot;
        skip_whitespace(code, local_dot);
        // there is only two ways the collection of strings for a name stops:
        //   - we reach a terminator
        //   - we reach an open bracket (meaning, this is a function call)
        std::string name;
        char c = code[local_dot++];
        while (c) {
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

        return "";
    }

    //
    // Processing inputs
    //
    // Filenames are easier to process, because there is no escaping
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
                auto value = evaluate_name_or_function(name, dot, "__", values, found);
                if (found) {
                    out << value;
                    AF_ASSERT(lookahead(name, dot, "__"), "Non-terminated replacement.");
                    dot += 2;
                    c = name[dot];
                    continue;
                }
                else {
                    if (strict) {
                        // there is a very confusing (for the parser) use case, additional '_' around the name
                        // that's actually on, and non-strict parsing deals with it just fine
                        // but strict checking needs to be given a chance
                        if(name[dot] != '_')
                            AF_ERROR("Unhandled replacement (%) at character: %", name, dot);
                    }
                    dot -= 2;
                }
            }
            out << c;
            if (c)// c==0 is end of string
                c = name[++dot];
        }
        return out.str();
    }

    // File content allows for escaping symbols, and functions, makes it a little more complicated
    std::string get_escaped(
        std::string const& content,
        size_t& dot
    ) {
        char c = content[dot];
        std::string ret;
        ret += c;
        while (c) {
            if (lookahead(content, dot, "}}}}")) {
                dot += 4;
                ret += "}}";
                break;
            }
            ret += c;
            c = content[++dot];
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
                if (lookahead(content, dot, "{{")) { // {{{{ means we are escaping things
                    ++dot; 
                    out << get_escaped(content, dot);
                    c = content[dot];
                }
                else {
                    bool found = false;
                    auto value = evaluate_name_or_function(content, dot, "}}", values, found);
                    if (found) {
                        out << value;
                        AF_ASSERT(lookahead(content, dot, "}}"), "Non-terminated replacement.");
                        dot += 2;
                        c = content[dot];
                        continue;
                    }
                    else {
                        if (strict) {
                            // there is a very confusing (for the parser) use case, additional '{' around the name
                            // that's actually on, and non-strict parsing deals with it just fine
                            // but strict checking needs to be given a chance
                            if (content[dot] != '{')
                                AF_ERROR("Unhandled replacement at character: %", dot);
                        }

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

    // 
    // Listing required data from inputs
    // 
    // This is helpful to document templates, examine them, and generate blank configuration files.
    // Because of escaping in content, the logic is slightly different for file names and content.
    // In the end it makes it more readable to have two separate functions, even though thery are very similar.
    void list_required_names_for_filename(std::string const& path, string_map_nc<string_set_nc>& sections) {
        size_t dot{ 0 };
        char c = path[dot];
        while (c) {
            if (lookahead(path, dot, "__")) {// is it a name?
                dot += 2;
                if (path[dot] == '_') {// is it a name with additional '_' around it?
                    dot -= 2;// false alarm, roll back
                }
                else {// yesss, it's a name, get it
                    size_t end_of_name = dot;
                    while (path[end_of_name++] && !lookahead(path, end_of_name, "__"))
                        ;
                    AF_ASSERT(path[end_of_name], "Missing closing name underscores.");
                    auto name = path.substr(dot, end_of_name - dot);
                    dot = end_of_name + 2;
                    if (!special_files::is_special(name)) {
                        auto i = name.find('.');// deal with sections
                        if (i == std::string::npos)
                            sections[""].insert(to_lower(trim(name)));
                        else
                            sections[to_lower(trim(name.substr(0, i)))].insert(to_lower(trim(name.substr(i + 1))));
                    }
                }
            }
            if (!path[dot])// all this rolling back an forth ... did we run out of text?
                break;
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
            if (lookahead(content, dot, "{{")) {// is it a name?
                dot += 2;
                if (lookahead(content, dot, "{{")) {// ... or is it escaped?
                    dot += 2;
                }
                else if (content[dot] == '{') {// is a name that is surrounded by braces?
                    dot -= 2; // roll back, false alarm
                }
                else { // yess ... it's a name! get it
                    size_t end_of_name = dot;
                    while (content[end_of_name++] && !lookahead(content, end_of_name, "}}"))
                        ;
                    AF_ASSERT(content[end_of_name], "Missing closing name braces.");
                    auto name = content.substr(dot, end_of_name - dot);
                    dot = end_of_name + 2;
                    if (!ignore_functions || name.find('(') == std::string::npos) { // function calls are not included here
                        auto i = name.find('.');// deal with sections nicely
                        if (i == std::string::npos)
                            sections[""].insert(to_lower(trim(name)));
                        else
                            sections[to_lower(trim(name.substr(0, i)))].insert(to_lower(trim(name.substr(i + 1))));
                    }
                }
            }
            c = content[++dot];
        }
    }

    // config files
    const char* const tag_extensions_to_ignore = "extensions_to_ignore";
    const char* const tag_files_to_ignore = "files_to_ignore";
    const char* const tag_sections = "sections";
    const char* const tag_name = "name";
    const char* const tag_named_values = "named_values";
    const char* const tag_value = "value";
    void parse_string_config(
        std::string const& config,
        named_values& values) {
        if (config.empty())
            return;
        const char comma = ',';
        const char quote = '\"';
        const char equals = '=';
        size_t i = 0;
        while(i<config.size() && config[i] == quote)
            ++i;
        size_t last = config.size() - 1;
        while(last && config[last] == quote)
            --last;
        bool parsing_name = true;
        std::string name;
        std::string value;
        char c = 0;
        while (i <= last) {
            c = config[i];
            switch (c) {
            case comma: {
                AF_ASSERT(!name.empty(), "Empty names are not allowed");
                AF_ASSERT(!parsing_name, "The syntax of names parameter is incorrect.");
                values.add(name, value);
                parsing_name = true;
                name = "";
                value = "";
                }
                break;
            case equals: {
                AF_ASSERT(parsing_name, "The syntax of names parameter is incorrect.");
                parsing_name = false;
                }
                break;
            default:
                if (parsing_name)
                    name += c;
                else
                    value += c;
                break;
            }
            ++i;
        }
        if(!name.empty())
            values.add(name, value);
    }
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
                section = trim(line.substr(1, line.size() - 2));
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
                if (section.empty())
                    values.add(name, line.substr(eq + 1));
                else
                    values.add(section + "." + name, line.substr(eq + 1));
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

        out << "\t\"" << tag_extensions_to_ignore << "\" : \"" << to_csv(extensions_to_ignore_) << "\"," << std::endl;
        out << "\t\"" << tag_files_to_ignore << "\" : \"" << to_csv(files_to_ignore_) << "\"," << std::endl;

        out << "\t\"sections\":[" << std::endl;

        size_t szs = sections.size();
        size_t is = 0;
        for (auto e : sections) {
            ++is;
            out << "\t{" << std::endl;
            out << "\t\t\"name\":\"" << e.first << "\"," << std::endl;
            out << "\t\t\"named_values\" : [" << std::endl;

            size_t sz = e.second.size();
            size_t i = 0;
            for (auto const name : e.second) {
                ++i;
                out << "\t\t\t{\"name\":\"" << name << "\",\"value\":\"\"}";
                if (i < sz)
                    out << ",";
                out << std::endl;
            }
            out << "\t\t]\n\t}";
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
            std::string name = get_json_string(nvo, tag_name);
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

    // 
    // Implementation of all the functionality is in bpl_impl
    // 
    class bpl_impl {
        named_values _values;
        // we keep both string and path representation, for speed
        path_t _source_path;
        path_t _target_path;
        path_t _config_path;
        bool _strict;
        // ignoring only refers to content of files, names are still parsed
        std::vector<std::string> _extensions_to_ignore;
        std::vector<std::string> _files_to_ignore;
        std::vector<std::string> _paths_to_ignore;

        inline bool has(std::vector<std::string> const& v, std::string const& s) {
            // just to make lots of lines shorter
            return (std::find(v.begin(), v.end(), s) != v.end());
        }
        // we cache paths to ignore, so that we can ignore files
        // in ignored folders
        void cache_path_to_ignore(std::string const& p) {
            if (!has(_paths_to_ignore, p))
                _paths_to_ignore.push_back(p);
        }
        bool ignored(path_t const& target_path) {
            auto const path_s = to_lower(target_path.string());
            // is it under a directory that is already being ignored?
            for (auto p : _paths_to_ignore) {
                if (starts_with(path_s, p))
                    return true;
            }
            // is it ignored by extension?
            auto extension = to_lower(target_path.extension().string());
            if (!extension.empty() && !_extensions_to_ignore.empty()) {
                // wildcard matching works with equality too
                for (auto const& pattern : _extensions_to_ignore) {
                    if (wildcard_match(extension, pattern)) {
                        cache_path_to_ignore(path_s);
                        return true;
                    }
                    // in case people specified extensions to ignore including dots at the start
                    if (extension[0] == '.') {
                        auto const shorter = extension.substr(1);
                        if (wildcard_match(shorter, pattern)) {
                            cache_path_to_ignore(path_s);
                            return true;
                        }
                    }
                }
            }
            // is it ignored by filename?
            if (!_files_to_ignore.empty()) {
                // we want to be portable, so normalise file separators
                auto const norm_path = replace(to_lower(target_path.string()), "\\", "/");
                for (auto const& pattern : _files_to_ignore) {
                    auto const norm_pattern = replace(pattern, "\\", "/");
                    if (wildcard_match(norm_path, norm_pattern)) {
                        cache_path_to_ignore(path_s);
                        return true;
                    }
                }
            }
            // well, we tried what we could
            return false;
        }
        // constructing the target path from source path is a busy work
        path_t make_target_path(
            path_t const& source_path,
            path_t const& source_folder,
            path_t const& target_folder) {
            
            // we do some string manipulation here, make things strings
            auto const source_p = source_path.string();
            auto const source_f = source_folder.string();
            auto const target_f = target_folder.string();

            // first we drop the source folder prefix from the path and replace it with target one
            // if we introduces and double separators while doing that, we tidy that up too
            auto target_p = replace(replace(replace(source_p, source_f, target_f), "\\\\", "\\"), "//", "/");
            
            AF_ASSERT(source_p != target_p,
                "Source folder doesn't seem to be present in the path % (source folder is: %)", source_p, source_f);

            // then we handle replacements in file names
            if (!special_files::is_special(source_path))
                target_p = process_filename(target_p, _strict, _values);
            else { // for special files, we don't want to tackle the filename, just the path to it
                auto const target_root_s = process_filename(
                    path_t(target_p).parent_path().make_preferred().string(),
                    _strict, _values);
                target_p = path_t(target_root_s).append(source_path.filename()).string();
            }
            return path_t(target_p);
        }
        
        // this is the main working function
        // here we create files and folders in the target
        void create_target(path_t const& source_path, path_t const& target_path) {
            // better safe than sorry
            AF_ASSERT(!filesystem_n::exists(target_path), "% already exists.", target_path.string());
            
            // directories are just created
            if (filesystem_n::is_directory(source_path)) {
                filesystem_n::create_directories(target_path);
                AF_MESSAGE("Creating directory %.", target_path.string());
            }
            else if (special_files::handle_if_special(source_path, target_path)) {
                // special files have their own handlers
                return;
            }
            else if (ignored(source_path)) {
                // ignored files are just copied
                filesystem_n::copy(source_path, target_path);
                AF_MESSAGE("Creating file %.", target_path.string());
            }
            else {
                // for everything else, we parse the content too
                auto const content = read_file(source_path);
                AF_MESSAGE("Parsing file %.", target_path.string());
                auto const new_content = process_content(content, _strict, _values);
                AF_MESSAGE("Creating file %.", target_path.string());
                write_file(target_path, new_content);
            }
        }

    public:
        bpl_impl(
            std::string const& source_path_,
            std::string const& target_path_,
            std::string const& config_path_,
            bool strict_ = false,
            std::string const& extensions_to_ignore_ = "",
            std::string const& files_to_ignore_ = "") {
            
            _strict = strict_;
            _source_path = path_t(source_path_).make_preferred();
            _target_path = path_t(target_path_).make_preferred();
            _config_path = path_t(config_path_).make_preferred();
            AF_ASSERT(filesystem_n::exists(_source_path),
                "Source folder % does not exist.", source_path_);

            if (filesystem_n::exists(_config_path)) {
                if (wildcard_match(_config_path.string(), "*.json")) {
                    parse_json_config_file(config_path_, _values, _extensions_to_ignore, _files_to_ignore);
                }
                else {
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
            std::string const& extensions_to_ignore_ = "",
            std::string const& files_to_ignore_ = "",
            std::map<std::string, std::string> const& kvm_ = {}) {
            
            _strict = strict_;
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

        bpl_impl(
            std::string const& source_path_,
            std::string const& target_path_,
            bool strict_ = false,
            std::string const& extensions_to_ignore_ = "",
            std::string const& files_to_ignore_ = "",
            std::string const& named_values_s = "") {

            _strict = strict_;
            csv_to_vector(to_lower(extensions_to_ignore_), _extensions_to_ignore);
            csv_to_vector(to_lower(files_to_ignore_), _files_to_ignore);

            _source_path = path_t(source_path_).make_preferred();
            _target_path = path_t(target_path_).make_preferred();
            AF_ASSERT(filesystem_n::exists(_source_path),
                "Source folder % does not exist.", source_path_);

            AF_ASSERT(to_lower(_target_path.string()).find(to_lower(_source_path.string())) == std::string::npos,
                "Target path cannot be a sub-directory of the source path.");

            parse_string_config(named_values_s, _values);
        }
        // generating projects
        // for every file or folder, process the path, create a target
        void generate() {
            using recursive_directory_iterator = filesystem_n::recursive_directory_iterator;
            for (const auto& p : recursive_directory_iterator(_source_path)) {
                try {
                    auto target_path = make_target_path(p.path(), _source_path, _target_path);
                    create_target(p.path(), target_path);
                }
                catch (std::exception& e) {
                    // this is all just to avoid duplicating error messages, AF_ERROR prints them already
                    if (wildcard_match(e.what(), "[*] ERROR: *"))
                        AF_ERROR("Error while processing %.", p.path().string());
                    else
                        AF_ERROR("Error while processing % :\n %", p.path().string(), e.what());
                }
                catch (...) {
                    AF_ERROR("Unknown error while processing %.", p.path().string());
                }
            }
        }

        // generating blank configuration files
        // then you just populate them with values,it's nice
        void generate_config_files() {
            string_map_nc<string_set_nc> sections;
            using recursive_directory_iterator = filesystem_n::recursive_directory_iterator;
            for (const auto& p : recursive_directory_iterator(_source_path)) {
                list_required_names_for_filename(p.path().string(), sections);
                if (special_files::is_special(p) || ignored(p) || filesystem_n::is_directory(p))
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

        // got a new template to deal with? 
        // or one that you wrote but forgot all about?
        // use 'describe' to get information about it.
        void describe() {
            // it's long but boring, adding comments would make it both longer and boringer
            string_map_nc<string_set_nc> to_define;
            std::vector<std::string> functions;
            std::vector<std::string> special_files;

            std::cout << "Template: " << _source_path << std::endl << std::endl;
            using recursive_directory_iterator = filesystem_n::recursive_directory_iterator;
            for (const auto& p : recursive_directory_iterator(_source_path)) {
                string_map_nc<string_set_nc> sections;
                list_required_names_for_filename(p.path().string(), sections);
                if (!(special_files::is_special(p) || ignored(p) || filesystem_n::is_directory(p))){
                    auto const content = read_file(p);
                    list_required_names(content, sections, false);
                }
                auto short_path = replace(p.path().string(), _source_path.string(), "");
                std::cout << short_path << std::endl;
                
                if (!filesystem_n::is_directory(p) && (ignored(p) || special_files::is_special(p)))
                    std::cout << "\t* The content of the file was excluded from parsing." << std::endl;

                if (sections.empty()) {
                    std::cout << "\tNo replacements specified in this path." << std::endl;
                }
                else {
                    std::cout << "\tReplacements:" << std::endl;
                    for (auto const& e : sections) {
                        for (auto const& name : e.second) {
                            std::cout << "\t\t";
                            if (special_files::is_special(name)) {
                                std::cout << "[special file]         ";
                                special_files.push_back(p.path().string());
                            }
                            else if (name.find('(') != std::string::npos) {
                                std::cout << "[function]             ";
                                functions.push_back(name);
                            }
                            else {
                                std::cout << "[needs to be defined]  ";
                                to_define[e.first].insert(name);
                            }
                            if (!e.first.empty())
                                std::cout << e.first << ".";
                            std::cout << name;
                            std::cout << std::endl;
                        }
                    }
                }
            }

            std::cout << "\n\nSpecial files used:" << std::endl;
            for (auto const& f : special_files)
                std::cout << '\t' << f << std::endl;
            std::cout << "\nReplacement functions used:" << std::endl;
            for (auto const& f : functions)
                std::cout << '\t' << f << std::endl;
            std::cout << "\nNames to be defined:" << std::endl;
            for (auto const& e : to_define) {
                auto tabs = "\t";
                if (!e.first.empty()) {
                    std::cout << '\t' << '[' << e.first << ']' << std::endl;
                    tabs = "\t\t";
                }
                for (auto const& name : e.second) {
                    std::cout << tabs << name << std::endl;
                }
            }
        }
    };

    // external interface
    bpl::bpl(
            std::string const& source_path_,
            std::string const& target_path_,
            std::string const& config_path_,
            bool strict_,
            std::string const& extensions_to_ignore_,
            std::string const& files_to_ignore_) :
        _impl(new bpl_impl(
            source_path_, 
            target_path_, 
            config_path_, 
            strict_,
            extensions_to_ignore_,
            files_to_ignore_)) {
    }

    bpl::bpl(
            std::string const& source_path_,
            std::string const& target_path_,
            bool strict_,
            std::string const& extensions_to_ignore_,
            std::string const& files_to_ignore_,
            std::map<std::string, std::string> const& kvm_) :
        _impl(new bpl_impl(
            source_path_,
            target_path_,
            strict_,
            extensions_to_ignore_,
            files_to_ignore_,
            kvm_)) {
    }


    bpl::bpl(
        std::string const& source_path_,
        std::string const& target_path_,
        bool strict_,
        std::string const& extensions_to_ignore_,
        std::string const& files_to_ignore_,
        std::string const& named_values_s):
        _impl(new bpl_impl(
            source_path_,
            target_path_,
            strict_,
            extensions_to_ignore_,
            files_to_ignore_,
            named_values_s)) {
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
