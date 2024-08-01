#pragma once
#include <iostream>
#include <iomanip>
#include <string>
#include <memory>
class constant_matrix;
// helpers for streaming out to json
class json_configuration {
    bool _include_result = { false };
    static json_configuration& instance() {
        static json_configuration config;
        return config;
    }
public:
    static bool include_result() { return instance()._include_result; }
    static json_configuration& include_result(bool include_result_) {
        instance()._include_result = include_result_;
        return instance();
    }
};
namespace json_util {
    struct label {
        std::string const& _label;
        size_t& _tabs;
        label(std::string const& label_, size_t& tabs_) :_label(label_), _tabs(tabs_) {}
    };
    inline std::ostream& operator<<(std::ostream& out, label const& l) {
        out << "\"" << l._label << "\" : ";
        return out;
    }

    struct object_type {
        std::string const& _type;
        size_t& _tabs;
        object_type(std::string const& type_, size_t& tabs_) :_type(type_), _tabs(tabs_) {}
    };
    inline std::ostream& operator<<(std::ostream& out, object_type const& ot) {
        out << label("type", ot._tabs) << "\"" << ot._type << "\"";
        return out;
    }

    struct description {
        std::string const& _description;
        size_t& _tabs;
        description(std::string const& description_, size_t& tabs_) :_description(description_), _tabs(tabs_) {}
    };
    inline std::ostream& operator<<(std::ostream& out, description const& d) {
        out << label("description", d._tabs) << "\"" << d._description << "\"";
        return out;
    }

    struct next {
        size_t& _tabs;
        next(size_t& tabs_) :_tabs(tabs_) {}
    };
    inline std::ostream& operator<<(std::ostream& out, next const& n) {
        out << ",\n" << std::string(n._tabs, '\t');
        return out;
    }

    struct start_object {
        size_t& _tabs;
        start_object(size_t& tabs_) :_tabs(tabs_) {}
    };
    inline std::ostream& operator<<(std::ostream& out, start_object const& so) {
        ++so._tabs;
        out << "{\n" << std::string(so._tabs, '\t');
        return out;
    }

    struct end_object {
        size_t& _tabs;
        end_object(size_t& tabs_) :_tabs(tabs_) {}
    };
    inline std::ostream& operator<<(std::ostream& out, end_object const& eo) {
        --eo._tabs;
        out << "\n" << std::string(eo._tabs, '\t') << "}";
        return out;
    }

    struct start_expression_object {
        std::string const& _type;
        std::string const& _description;
        size_t& _tabs;
        start_expression_object(
            std::string const& type_,
            std::string const& description_,
            size_t& tabs_) :_type(type_), _description(description_), _tabs(tabs_) {}
    };
    inline std::ostream& operator<<(std::ostream& out, start_expression_object const& eo) {
        out <<
            start_object(eo._tabs) <<
            object_type(eo._type, eo._tabs) << next(eo._tabs) <<
            description(eo._description, eo._tabs);
        return out;
    }
    using end_expression_object = end_object;

    struct start_array {
        size_t& _tabs;
        start_array(size_t& tabs_) : _tabs(tabs_) {}

    };
    inline std::ostream& operator<<(std::ostream& out, start_array const& sa) {
        ++sa._tabs;
        out << "[\n" << std::string(sa._tabs, '\t');
        return out;
    }

    struct end_array {
        size_t& _tabs;
        end_array(size_t& tabs_) :_tabs(tabs_) {}

    };
    inline std::ostream& operator<<(std::ostream& out, end_array const& ea) {
        --ea._tabs;
        out << "\n" << std::string(ea._tabs, '\t') << "]";
        return out;
    }
    
    struct start_row {
        size_t& _tabs;
        start_row(size_t& tabs_) :_tabs(tabs_) {}

    };
    inline std::ostream& operator<<(std::ostream& out, start_row const& sr) {
        out << "[";
        return out;
    }
    
    struct end_row {
        size_t& _tabs;
        end_row(size_t& tabs_) : _tabs(tabs_) {}

    };
    inline std::ostream& operator<<(std::ostream& out, end_row const& er) {
        out << "]";
        return out;
    }
    
    struct next_in_row {
        size_t& _tabs;
        next_in_row(size_t& tabs_) :_tabs(tabs_) {}

    };
    inline std::ostream& operator<<(std::ostream& out, next_in_row const& nr) {
        out << ",";
        return out;
    }
    
    // js_t is a wrapper for value types
    template<typename T>
    struct js_t {
        const T& _value;
        size_t& _tabs;
        js_t(const T& value_, size_t& tabs_) :_value(value_), _tabs(tabs_) {}
    };
    template<typename T>
    inline js_t<T> js(const T& value_, size_t& tabs_) { return js_t<T>(value_, tabs_); }

    template<typename T>
    std::ostream& operator<<(std::ostream& out, js_t<T> const& j) {
        j._value.json_stream(out, j._tabs);
        return out;
    }
    template<typename T>
    std::ostream& operator<<(std::ostream& out, js_t<std::shared_ptr<T>> const& j) {
        j._value->json_stream(out, j._tabs);
        return out;
    }
    inline std::ostream& operator<<(std::ostream& out, js_t<constant_matrix> const& j) {
        std::stringstream buff;
        buff << j._value;
        auto s = buff.str();
        std::string r(j._tabs + 1, '\t');
        r = "\n" + r;
        size_t i = s.find('\n');
        while (i != std::string::npos) {
            s.replace(i, 1, r);
            i = s.find('\n', i + 1);
        }

        out << "\"" << r << s << "\"\n";
        return out;
    }
    inline std::ostream& operator<<(std::ostream& out, js_t<std::shared_ptr<constant_matrix>> const& j) {
        std::stringstream buff;
        buff << j._value;
        auto s = buff.str();
        std::string r(j._tabs + 1, '\t');
        r = "\n" + r;
        size_t i = s.find('\n');
        while (i != std::string::npos) {
            s.replace(i, 1, r);
            i = s.find('\n', i + 1);
        }

        out << "\"" << r << s << "\"\n";
        return out;
    }

    template<>
    inline std::ostream& operator<<(std::ostream& out, js_t<int> const& j) {
        out << j._value;
        return out;
    }
    template<>
    inline std::ostream& operator<<(std::ostream& out, js_t<size_t> const& j) {
        out << j._value;
        return out;
    }
    template<>
    inline std::ostream& operator<<(std::ostream& out, js_t<double> const& j) {
        out << j._value;
        return out;
    }
    template<>
    inline std::ostream& operator<<(std::ostream& out, js_t<bool> const& j) {
        out << j._value;
        return out;
    }
    template<>
    inline std::ostream& operator<<(std::ostream& out, js_t<char> const& j) {
        out << "\"" << j._value << "\"";
        return out;
    }
    template<>
    inline std::ostream& operator<<(std::ostream& out, js_t<std::string> const& j) {
        out << "\"" << j._value << "\"";
        return out;
    }

    template<typename T>
    struct labeled_value_t {
        std::string const& _label;
        T const& _value;
        size_t& _tabs;
        labeled_value_t(
            std::string const& label_,
            T const& value_,
            size_t& tabs_) :_label(label_), _value(value_), _tabs(tabs_) {}
    };
    template<typename T>
    inline labeled_value_t<T> labeled_value(std::string const& label_, T const& value_, size_t& tabs_) {
        return labeled_value_t<T>(label_, value_, tabs_);
    }
    template<typename T>
    inline std::ostream& operator<<(std::ostream& out, labeled_value_t<T> const& lv) {
        out << label(lv._label, lv._tabs) << js(lv._value, lv._tabs);
        return out;
    }
    
    template<typename T>
    struct result_value_t {
        T const& _result_value;
        size_t& _tabs;
        result_value_t(T const& result_value_, size_t& tabs_) :_result_value(result_value_), _tabs(tabs_) {}
    };
    template<typename T>
    inline result_value_t<T> result_value(T const& value_, size_t& tabs_) {
        return result_value_t<T>(value_, tabs_);
    }
    template<typename T>
    inline std::ostream& operator<<(std::ostream& out, result_value_t<T> const& rv) {
        out << label("value", rv._tabs) << js(rv._result_value, rv._tabs);
        return out;
    }
}
