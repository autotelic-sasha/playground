// matrices_expressions.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "kml.h"
#include "json_util.h"
/*
#include <iomanip>
#include <string>
#include <cctype>
#include <math.h>
#include <algorithm>
#include <sstream>
#include <memory>
#include <vector>
#include <map>
#include <unordered_map>
#include "Eigen/Dense"

using Eigen::MatrixXd;
using Eigen::VectorXd;

// kml is a small language for basic matrix operations
// it uses Eigen libraries as a back end and exposes a small subset of their functionality
// it could be useful and can be extended in the future, but main purpose for now is 
// as an example of peg grammars with packrat parsing that can be easily exposed to 
// other applications (e.g. Excel)
// 
// a,b,c1,m2 ... matrices
// basic arithmetic: +, -, *
// matrix operations: |m| - determinant, m^T - transpose, m^-1 - inverse, -m - negative
// name -> [a-zA-Z]+[a-zA-Z0-9]*
// constant_matrix -> name
// constant_number -> ['+'|'-']?[0-9]+[0-9]*('.'*[0-9]*)?             implemented in constant_number
// array_row -> '['constant_number(','constant_number)*']'
// array -> '[' array_row (',' array_row)*'];
// size -> [0-9]+[0-9]*
// 
// argument ->          size                    |
//                      number_expression       | 
//                      matrix_expression       
// 
// arg_list ->          argument                |
//                      argument ',' arg_list
//
// disambiguation between matrix function invocation vs number function invocation vs matrix  (expression) is context sensitive. 
// it is decided based on existence on functions.
// 
// matrix_function_invocation ->   name '(' arg_list ')'              implemented in matrix_function_inv
// number_function_invocation ->   name '(' arg_list ')'              implemented in number_function_inv
// 
// number_mul_term->   constant_number                              |
//                     '(' number_expression ')'                    |   implemented in bracketed_number
//                     number_function_invocation                       implemented in number_function_inv
// 
// number_product ->   number_mul_term (('*'\'/'\e) number_mul_term)* 
//              
// number_term -> ('-'|'+')* number_rhs_term                        |   implemented in negative_number
// 
// number_sum -> number_term (('+'\'-') number_term)?                |   implemented in number_arithmetic
// 
// number_expression -> number_sum                                  |   implemented in constant_number
//                      '|' matrix_expression '|'                       implemented in determinant
//
// matrix_term ->       constant_matrix                             |   implemented in constant_matrix 
//                      matrix_function_invocation                  |   implemented in matrix_function_inv
//                      matrix_term^T                               |   implemented in matrix_unary_op 
//                      matrix_term^-1                              |   implemented in matrix_unary_op 
//                      ('-'|'+') matrix_term                       |   implemented in matrix_unary_op 
//                      '(' matrix_expression ')'                       implemented in bracketed_matrix
//
// matrix_number_term -> matrix_term (('*'\'/') number_expression)? |   implemented in matrix_number_op 
//                       number_expression ('*'\e) matrix_term          implemented in matrix_number_op 
// 
// matrix_product -> matrix_number_term ('*' matrix_number_term)?   |   implemented in matrix_binary_op (dot product)
//                      
// matrix_expression -> matrix_product (('+'\'-') matrix_product)?  |   implemented in matrix_binary_op
//                      array
// 
// assignments -> name '=' matrix_expression ';' (assignenent)*
// 
// expression -> matrix_expression                                  |   implemented in matrix_expression
//               number_expression                                  |   implemented in number_expression
//               
// program -> (assignments)+ expression
//

enum class expression_type {
    number,
    matrix,
    assignment
};

class constant_matrix;
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
std::ostream& operator<<(std::ostream& out, constant_matrix const& cmp);
std::ostream& operator<<(std::ostream& out, std::shared_ptr<constant_matrix> const& cmp);
namespace json_util {
    struct label {
        std::string const& _label;
        size_t& _tabs;
        label(std::string const& label_, size_t& tabs_) :_label(label_), _tabs(tabs_) {}
    };
    std::ostream& operator<<(std::ostream& out, label const& l) {
        out << "\"" << l._label << "\" : ";
        return out;
    }
    struct object_type {
        std::string const& _type;
        size_t& _tabs;
        object_type(std::string const& type_, size_t& tabs_) :_type(type_), _tabs(tabs_) {}
    };
    std::ostream& operator<<(std::ostream& out, object_type const& ot) {
        out << label( "type", ot._tabs ) << "\"" << ot._type << "\"";
        return out;
    }
    struct description {
        std::string const& _description;
        size_t& _tabs;
        description(std::string const& description_, size_t& tabs_) :_description(description_), _tabs(tabs_) {}
    };
    std::ostream& operator<<(std::ostream& out, description const& d) {
        out << label("description", d._tabs) << "\"" << d._description << "\"";
        return out;
    }
    struct next {
        size_t& _tabs;
        next(size_t& tabs_) :_tabs(tabs_) {}
    };
    std::ostream& operator<<(std::ostream& out, next const& n) {
        out << ",\n" << std::string(n._tabs, '\t');
        return out;
    }
    struct start_object {
        size_t& _tabs;
        start_object(size_t& tabs_) :_tabs(tabs_) {}
    };
    std::ostream& operator<<(std::ostream& out, start_object const& so) {
        ++so._tabs;
        out << "{\n" << std::string(so._tabs, '\t');
        return out;
    }
    struct end_object {
        size_t& _tabs;
        end_object(size_t& tabs_) :_tabs(tabs_) {}
    };
    std::ostream& operator<<(std::ostream& out, end_object const& eo) {
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
            size_t& tabs_) :_type(type_), _description(description_),_tabs(tabs_) {}
    };
    std::ostream& operator<<(std::ostream& out, start_expression_object const& eo) {
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
    std::ostream& operator<<(std::ostream& out, start_array const& sa) {
        ++sa._tabs;
        out << "[\n" << std::string(sa._tabs, '\t');
        return out;
    }
    struct end_array {
        size_t& _tabs;
        end_array(size_t& tabs_) :_tabs(tabs_) {}

    };
    std::ostream& operator<<(std::ostream& out, end_array const& ea) {
        --ea._tabs;
        out << "\n" << std::string(ea._tabs, '\t') << "]";
        return out;
    }
    struct start_row {
        size_t& _tabs;
        start_row(size_t& tabs_) :_tabs(tabs_) {}

    };
    std::ostream& operator<<(std::ostream& out, start_row const& sr) {
        out << "[";
        return out;
    }
    struct end_row {
        size_t& _tabs;
        end_row(size_t& tabs_) : _tabs(tabs_) {}

    };
    std::ostream& operator<<(std::ostream& out, end_row const& er) {
        out << "]";
        return out;
    }
    struct next_in_row {
        size_t& _tabs;
        next_in_row(size_t& tabs_) :_tabs(tabs_) {}

    };
    std::ostream& operator<<(std::ostream& out, next_in_row const& nr) {
        out << ",";
        return out;
    }
    template<typename T>
    struct js_t {
        const T& _value;
        size_t& _tabs;
        js_t(const T& value_, size_t& tabs_) :_value(value_), _tabs(tabs_) {}
    };
    template<typename T>
    inline js_t<T> js(const T& value_, size_t& tabs_) {return js_t<T>(value_, tabs_);} 
    
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
    std::ostream& operator<<(std::ostream& out, js_t<constant_matrix> const& j) {
        std::stringstream buff;
        buff << j._value;
        auto s = buff.str();
        std::string r(j._tabs+1, '\t');
        r = "\n" + r;
        size_t i = s.find('\n');
        while (i != std::string::npos) {
            s.replace(i, 1, r);
            i = s.find('\n', i + 1);
        }

        out << "\"" << r << s << "\"\n";
        return out;
    }
    std::ostream& operator<<(std::ostream& out, js_t<std::shared_ptr<constant_matrix>> const& j) {
        std::stringstream buff;
        buff << j._value;
        auto s = buff.str();
        std::string r(j._tabs+1, '\t');
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
    std::ostream& operator<<(std::ostream& out, js_t<int> const& j) {
        out << j._value;
        return out;
    }
    template<>
    std::ostream& operator<<(std::ostream& out, js_t<size_t> const& j) {
        out << j._value;
        return out;
    }
    template<>
    std::ostream& operator<<(std::ostream& out, js_t<double> const& j) {
        out << j._value;
        return out;
    }
    template<>
    std::ostream& operator<<(std::ostream& out, js_t<bool> const& j) {
        out << j._value;
        return out;
    }
    template<>
    std::ostream& operator<<(std::ostream& out, js_t<char> const& j) {
        out << "\"" << j._value << "\"";
        return out;
    }
    template<>
    std::ostream& operator<<(std::ostream& out, js_t<std::string> const& j) {
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
    std::ostream& operator<<(std::ostream& out, labeled_value_t<T> const& lv) {
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
    std::ostream& operator<<(std::ostream& out, result_value_t<T> const& rv) {
        out << label("value", rv._tabs) << js(rv._result_value, rv._tabs);
        return out;
    }

}
using matrix_value_t = MatrixXd;
struct matrix_expression;
struct expression {
    virtual std::string display() const = 0;
    virtual std::string display_with_brackets() const = 0;
    virtual void json_stream(std::ostream& out, size_t& tabs) const = 0;
    
    std::string json_string() const{
        std::stringstream buff;
        size_t tabs = 0;
        json_stream(buff, tabs);
        return buff.str();
    }
    virtual expression_type type() const = 0;
    virtual double number_value() const = 0;
    virtual matrix_value_t const matrix_value() const = 0;
    virtual std::shared_ptr<constant_matrix> const evaluate_matrix() const = 0;

    virtual ~expression() {}
};
struct matrix_expression : public expression {
    expression_type type() const override { return expression_type::matrix; }
    double number_value() const override { throw std::runtime_error("Attempted to evaluate matrix as a number."); return 0; }
};
using matrix_expression_p = std::shared_ptr<matrix_expression>;

class constant_matrix : public matrix_expression {
    matrix_value_t const _value;
public:
    constant_matrix(double const*const value_, size_t const rows, size_t const cols) : 
        _value(Eigen::Map<const Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>>(value_, rows, cols)) {}
    
    constant_matrix(matrix_value_t const& v) :_value(v) {}
    std::string display_tabbed(size_t tabs = 0) const {
        const std::string indent(tabs, '\t');
        std::stringstream buff;
        for (Eigen::Index r = 0; r < _value.rows(); ++r) {
            buff << indent << "|";
            auto const& row = _value.row(r);
            for (Eigen::Index c = 0; c < row.size(); ++c)
                buff << " " << std::setw(10) << std::setprecision(6) << std::setfill(' ') << row[c] << " ";
            buff << "|\n";
        }
        return buff.str();
    }

    std::string display() const override { 
        return display_tabbed();
    }
    std::string display_with_brackets() const override { return display(); }

    void json_stream(std::ostream& out, size_t& tabs) const override {
        using namespace json_util;
        out << start_expression_object("constant_matrix", std::string("\n")+display_tabbed(tabs + 1), tabs) << next(tabs)
            << labeled_value("rows", (int)_value.rows(), tabs) << next(tabs) 
            << labeled_value("columns", (int)_value.cols(), tabs) << next(tabs) 
            << label("data", tabs);
        out << start_array(tabs);
        size_t const total = _value.rows() * _value.cols();
        for (size_t i = 0; i < total; ++i) {
            out << _value.data()[i];
            if (i < total - 1)
                out << next_in_row(tabs);
        }
        out << end_array(tabs);
        if (json_configuration::include_result())
            out << next(tabs) << result_value(*this, tabs);
        out << end_expression_object(tabs);
    }
    std::shared_ptr<constant_matrix> const evaluate_matrix() const override { throw std::runtime_error("Constant matrices cannot be evaluated");}
    matrix_value_t const matrix_value() const override { return _value; }

    static std::shared_ptr<constant_matrix> create(double const* const value_, size_t const rows, size_t const cols) { 
        return std::shared_ptr<constant_matrix>(new constant_matrix{ value_, rows, cols }); }
    
    static std::shared_ptr<constant_matrix> create(matrix_value_t const& v) {
        return std::shared_ptr<constant_matrix>(new constant_matrix{ v });
    }
    static std::shared_ptr<constant_matrix> create(std::vector<std::vector<double>> const& v) {
        using map_t = Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;
        if (v.size() == 0)
            throw std::runtime_error("Cannot initialise a matrix with an empty array.");
        size_t cols = v.front().size();
        map_t mp(v.size(), v.front().size());
        for (size_t i = 0; i < v.size(); ++i) {
            if(v[i].size() != cols)
                throw std::runtime_error("Cannot initialise a matrix with an array that is not rectangular.");
            for (size_t j = 0; j < v.front().size(); ++j)
                mp(i, j) = v[i][j];
        }
        
        return std::shared_ptr<constant_matrix>(new constant_matrix{ mp });
    }

};
std::ostream& operator<<(std::ostream& out, constant_matrix const& cmp) {
    out << cmp.display();
    return out;
}

using constant_matrix_p = std::shared_ptr<constant_matrix>;
std::ostream& operator<<(std::ostream& out, constant_matrix_p const& cmp) {
    out << cmp->display();
    return out;
}
struct non_const_matrix : public matrix_expression {
    matrix_value_t const matrix_value() const override { return evaluate_matrix()->matrix_value(); }
};

constant_matrix_p get_matrix_from_cache(std::string const& name);

class named_matrix : public matrix_expression {
    std::string const _name;
    inline constant_matrix_p const get() const {
        return get_matrix_from_cache(_name);
    }
public:
    named_matrix(std::string const& name_) : _name(name_){
    }

    std::string display() const override {
        std::stringstream buff;
        buff << _name;
        return buff.str();
    }
    std::string display_with_brackets() const override { return display(); }

    void json_stream(std::ostream& out, size_t& tabs) const override {
        using namespace json_util;
        out << start_expression_object("named_matrix", _name, tabs) << next(tabs)
            << labeled_value("name", _name, tabs);
        if (json_configuration::include_result())
            out << next(tabs) << result_value(evaluate_matrix(), tabs);
        out << end_expression_object(tabs);
    }
    std::shared_ptr<constant_matrix> const evaluate_matrix() const override { return get(); }
    matrix_value_t const matrix_value() const override { return evaluate_matrix()->matrix_value(); }
};

struct number_expression : public expression {
    expression_type type() const override { return expression_type::number; }
    std::shared_ptr<constant_matrix> const evaluate_matrix() const override { throw std::runtime_error("Attempted to evaluate number as a matrix."); return nullptr; }
    matrix_value_t const matrix_value() const override { return evaluate_matrix()->matrix_value(); }
};

class constant_number : public number_expression {
    double _value;
public:
    constant_number(double value_) : _value(value_) {}
    std::string display() const override { 
        std::stringstream buff;
        buff << _value;
        return buff.str();
    }
    std::string display_with_brackets() const override { return display(); }

    void json_stream(std::ostream& out, size_t& tabs) const override {
        using namespace json_util;
        out << start_expression_object("constant_number", display(), tabs);
        if (json_configuration::include_result())
            out << next(tabs) << result_value(number_value(), tabs);
        out << end_expression_object(tabs);

    }

    double number_value() const override { return _value; }
    static std::shared_ptr<constant_number> create(double v) {
        return std::shared_ptr<constant_number>(new constant_number{ v });
    }
};

class negative_number : public number_expression {
    std::shared_ptr<number_expression> _number;
public:
    negative_number(std::shared_ptr<number_expression> number_) :_number(number_){
    }

    std::string display() const override {
        std::stringstream buff;
        buff << "-" << _number->display();
        return buff.str();
    }
    std::string display_with_brackets() const override { 
        std::stringstream buff;
        buff << "(" << "-" << _number->display_with_brackets() << ")";
        return buff.str();
    }
    void json_stream(std::ostream& out, size_t& tabs) const override {
        using namespace json_util;
        out << start_expression_object("negative_number", display(), tabs) << next(tabs)
            << labeled_value("number", _number, tabs);
        if (json_configuration::include_result())
            out << next(tabs) << result_value(number_value(), tabs);
        out << end_expression_object(tabs);
    }

    double number_value() const override {
        return -_number->number_value();
    }
};

class bracketed_number : public number_expression {
    std::shared_ptr<number_expression> _number;
public:
    bracketed_number(std::shared_ptr<number_expression> number_) :_number(number_) {
    }

    std::string display() const override {
        std::stringstream buff;
        buff << "(" << _number->display() << ")";
        return buff.str();
    }
    std::string display_with_brackets() const override {
        std::stringstream buff;
        buff << "(" << _number->display_with_brackets() << ")";
        return buff.str();
    }

    void json_stream(std::ostream& out, size_t& tabs) const override {
        using namespace json_util;
        out << start_expression_object("bracketed_number", display(), tabs) << next(tabs) <<
            labeled_value("number", _number, tabs);
        if (json_configuration::include_result())
            out << next(tabs) << result_value(number_value(), tabs);
        out << end_expression_object(tabs);
    }
    double number_value() const override {
        return _number->number_value();
    }
};

class number_arithmetic : public number_expression {
    std::shared_ptr<number_expression> _lhs;
    std::shared_ptr<number_expression> _rhs;
    char _op;
public:
    number_arithmetic(
        std::shared_ptr<number_expression> lhs_,
        std::shared_ptr<number_expression> rhs_,
        char op_) :_lhs(lhs_), _rhs(rhs_), _op(op_) {}

    std::string display() const override {
        std::stringstream buff;
        buff << _lhs->display() << _op << _rhs->display();
        return buff.str();
    }
    std::string display_with_brackets() const override {
        std::stringstream buff;
        buff << "(" << _lhs->display_with_brackets() << _op << _rhs->display_with_brackets() << ")";
        return buff.str();
    }

    void json_stream(std::ostream& out, size_t& tabs) const override {
        using namespace json_util;
        out << start_expression_object("number_arithmetic", display(), tabs) << next(tabs) <<
            labeled_value("lhs", _lhs, tabs) << next(tabs) <<
            labeled_value("rhs", _rhs, tabs) << next(tabs) <<
            labeled_value("op", _op, tabs);
        if (json_configuration::include_result())
            out << next(tabs) << result_value(number_value(), tabs);
        out << end_expression_object(tabs);
    }
    double number_value() const override {
        switch (_op) {
        case '+': return _lhs->number_value() + _rhs->number_value();
        case '-': return _lhs->number_value() - _rhs->number_value();
        case '*': return _lhs->number_value() * _rhs->number_value();
        case '/': return _lhs->number_value() / _rhs->number_value();
        default: throw std::runtime_error("Unknown operand.");
        }

         return 0;// never happens, but compilers moan 
    }
};

class determinant : public number_expression {
    matrix_expression_p _matrix;
public:
    determinant(matrix_expression_p matrix_) :_matrix(matrix_) {}
    std::string display() const override {
        std::stringstream buff;
        buff << "|" << _matrix->display() << "|";
        return buff.str();
    }
    std::string display_with_brackets() const override {
        std::stringstream buff;
        buff << "|" << _matrix->display_with_brackets() << "|";
        return buff.str();
    }

    void json_stream(std::ostream& out, size_t& tabs) const override {
        using namespace json_util;
        out << start_expression_object("determinant", display(), tabs) << next(tabs) <<
            labeled_value("matrix", _matrix, tabs);
        if (json_configuration::include_result())
            out << next(tabs) << result_value(number_value(), tabs);
        out << end_expression_object(tabs);
    }

    double number_value() const override {
        return _matrix->matrix_value().determinant();
    }
};

class bracketed_matrix : public non_const_matrix {
    matrix_expression_p _matrix;
public:
    bracketed_matrix(matrix_expression_p matrix_) :_matrix(matrix_) {
    }

    std::string display() const override {
        std::stringstream buff;
        buff << "(" << _matrix->display() << ")";
        return buff.str();
    }
    std::string display_with_brackets() const override {
        std::stringstream buff;
        buff << "(" << _matrix->display_with_brackets() << ")";
        return buff.str();
    }

    void json_stream(std::ostream& out, size_t& tabs) const override {
        using namespace json_util;
        out << start_expression_object("bracketed_matrix", display(), tabs) << next(tabs) <<
            labeled_value("matrix", _matrix, tabs);
        if (json_configuration::include_result())
            out << next(tabs) << result_value(evaluate_matrix(), tabs);
        out << end_expression_object(tabs);
    }
    std::shared_ptr<constant_matrix> const evaluate_matrix() const override {
        return _matrix->evaluate_matrix(); 
    }
};


enum class unary_matrix_op {
    transpose,
    inverse,
    negative
};
class matrix_unary_op : public non_const_matrix {
    matrix_expression_p _matrix;
    unary_matrix_op _op;
    inline std::string to_string(unary_matrix_op op) const{
        switch (_op) {
        case unary_matrix_op::inverse:
            return "^-1";
        case unary_matrix_op::transpose:
            return "^T";
        case unary_matrix_op::negative:
            return "-";
        default:
            throw std::runtime_error("Unknown matrix unary operation.");
        }
    }
public:
    matrix_unary_op(matrix_expression_p matrix_, unary_matrix_op op_) :_matrix(matrix_), _op(op_) {}
    std::string display() const override { 
        std::stringstream buff;
        switch (_op) {
        case unary_matrix_op::inverse:
            buff << _matrix->display() << to_string(_op);
            break;
        case unary_matrix_op::transpose:
            buff << _matrix->display() << to_string(_op);
            break;
        case unary_matrix_op::negative:
            buff << to_string(_op) << _matrix->display();
            break;
        default:
            throw std::runtime_error("Unknown matrix unary operation.");
        }
        return buff.str();
    }
    std::string display_with_brackets() const override {
        std::stringstream buff;
        {
            switch (_op) {
            case unary_matrix_op::inverse:
                buff << "(" << _matrix->display_with_brackets() << to_string(_op) << ")";
                break;
            case unary_matrix_op::transpose:
                buff << "(" << _matrix->display_with_brackets()<< to_string(_op) << ")";
                break;
            case unary_matrix_op::negative:
                buff << "(" << to_string(_op) << _matrix->display_with_brackets() << ")";
                break;
            default:
                throw std::runtime_error("Unknown matrix unary operation.");
            }
        }
        return buff.str();
    }
    void json_stream(std::ostream& out, size_t& tabs) const override {
        using namespace json_util;
        out << start_expression_object("matrix_unary_op", display(), tabs) << next(tabs) <<
            labeled_value("matrix", _matrix, tabs) << next(tabs) <<
            labeled_value("op", to_string(_op), tabs);
        if (json_configuration::include_result())
            out << next(tabs) << result_value(evaluate_matrix(), tabs);
        out << end_expression_object(tabs);
    }
    std::shared_ptr<constant_matrix> const evaluate_matrix() const override {
        switch (_op) {
        case unary_matrix_op::inverse:
            return constant_matrix::create(_matrix->matrix_value().inverse());
        case unary_matrix_op::transpose:
            return constant_matrix::create(_matrix->matrix_value().transpose());
            break;
        case unary_matrix_op::negative:
            return constant_matrix::create(-_matrix->matrix_value());
        default:
            throw std::runtime_error("Unknown matrix unary operation.");
        }
        return nullptr; 
    }
};
class matrix_number_op : public non_const_matrix {
    matrix_expression_p _matrix;
    std::shared_ptr<number_expression> _number;
    char _op; // * or /
public:
    matrix_number_op(
        matrix_expression_p matrix_,
        std::shared_ptr<number_expression> number_,
        char op_) : _matrix(matrix_), _number(number_), _op(op_) {
    }
    std::string display() const override {
        std::stringstream buff;
        switch (_op) {
        case '*':
            buff << _number->display() << _op << _matrix->display();
            break;
        case '/':
            buff << _matrix->display() << _op << _number->display();
            break;
        default:
            throw std::runtime_error("Unknown operator.");
        }
        return buff.str();
    }
    std::string display_with_brackets() const override {
        std::stringstream buff;
        buff << "(";
        switch (_op) {
        case '*':
            buff << _number->display_with_brackets() << _op << _matrix->display_with_brackets();
            break;
        case '/':
            buff << _matrix->display_with_brackets() << _op << _number->display_with_brackets();
            break;
        default:
            throw std::runtime_error("Unknown operator.");
        }
        buff << ")";
        return buff.str();
    }

    void json_stream(std::ostream& out, size_t& tabs) const override {
        using namespace json_util;
        out << start_expression_object("matrix_number_op", display(), tabs) << next(tabs) <<
            labeled_value("matrix", _matrix, tabs) << next(tabs) <<
            labeled_value("number", _number, tabs) << next(tabs) <<
            labeled_value("op", _op, tabs);
        if (json_configuration::include_result())
            out << next(tabs) << result_value(evaluate_matrix(), tabs);
        out << end_expression_object(tabs);
    }
    std::shared_ptr<constant_matrix> const evaluate_matrix() const override {
        switch (_op) {
        case '*':
            return constant_matrix::create(_number->number_value() * _matrix->matrix_value());
        case '/':
            return constant_matrix::create(_matrix->matrix_value()/ _number->number_value());
        default:
            throw std::runtime_error("Unknown matrix unary operation.");
        }
        return nullptr;
    }
};
class matrix_binary_op : public non_const_matrix {
    matrix_expression_p _lhs;
    matrix_expression_p _rhs;
    char _op; // *, + or -
public:
    matrix_binary_op(
        matrix_expression_p lhs_,
        matrix_expression_p rhs_,
        char op_) : _lhs(lhs_), _rhs(rhs_), _op(op_) {
    }
    void json_stream(std::ostream& out, size_t& tabs) const override {
        using namespace json_util;
        out << start_expression_object("matrix_binary_op", display(), tabs) << next(tabs) <<
            labeled_value("lhs", _lhs, tabs) << next(tabs) <<
            labeled_value("rhs", _rhs, tabs) << next(tabs) <<
            labeled_value("op", _op, tabs);
        if (json_configuration::include_result())
            out << next(tabs) << result_value(evaluate_matrix(), tabs);
        out << end_expression_object(tabs);
    }
    std::string display() const override {
        std::stringstream buff;
        buff << _lhs->display() << _op << _rhs->display();
        return buff.str();
    }
    std::string display_with_brackets() const override {
        std::stringstream buff;
        buff << "(" << _lhs->display_with_brackets() << _op << _rhs->display_with_brackets() << ")";
        return buff.str();
    }
    std::shared_ptr<constant_matrix> const evaluate_matrix() const override {
        switch (_op) {
        case '*':
            return constant_matrix::create(_lhs->matrix_value() * _rhs->matrix_value());
        case '+':
            return constant_matrix::create(_lhs->matrix_value() + _rhs->matrix_value());
        case '-':
            return constant_matrix::create(_lhs->matrix_value() - _rhs->matrix_value());
        default:
            throw std::runtime_error("Unknown matrix unary operation.");
        }
        return nullptr;
    }
};

bool check_function_exists(std::string const& name);
namespace parsing {
    std::shared_ptr<expression> parse_expression(std::string const& code, size_t& dot);
}
std::string to_lower(std::string const& s) {
    std::string out(s);
    std::transform(s.begin(), s.end(), out.begin(), [](char c) {return std::tolower(c); });
    return out;
}

class matrix_cache {
    static std::map<std::string, std::shared_ptr<constant_matrix>>& cache() {
        static std::map<std::string, std::shared_ptr<constant_matrix>> _cache;
        return _cache;
    }
public:
    static void add(std::string const& name, constant_matrix_p const& matrix_) {
        // for prettier error messages, we don't allow functions to only differ by case
        std::string existing;
        std::string l_name(to_lower(name));
        for (auto const& e : cache()) {
            if (to_lower(e.first) == l_name) {
                existing = e.first;
                break;
            }
        }
        if (!existing.empty()) {
            std::stringstream error;
            error << "A matrix with a similar name already exists: " << existing << ". " << name <<
                " differs only in case, please choose a better name for it.";
            throw std::runtime_error(error.str());
        }
        if (check_function_exists(name)) {
            std::stringstream error;
            error << "A function with a name similar to this matrix already exists: " << name <<
                ", it differs only in case, please choose a better name for it.";
            throw std::runtime_error(error.str());
        }

        cache()[name] = matrix_;
    }
    static void add(std::string const& name, std::string const& code) {
        size_t dot(0);
        auto me = parsing::parse_expression(code, dot);
        if(!me) throw std::runtime_error("No parseable expressions found");
        if(me->type() != expression_type::matrix)
            throw std::runtime_error("Expression value is a number, not a matrix.");
        auto cm = std::dynamic_pointer_cast<constant_matrix>(me);
        if (cm)
            add(name, cm);
        else
            add(name, me->evaluate_matrix());
    }
    static constant_matrix_p get(std::string const& name) {
        std::string l_name(to_lower(name));
        for (auto const& e : cache()) {
            if (e.first == name)
                return e.second;
            if (to_lower(e.first) == l_name) {
                std::stringstream error;
                error << "Matrix " << name << " does not exist. Did you mean: " << e.first << "?";
                throw std::runtime_error(error.str());
            }
        }
        std::stringstream error;
        error << "Matrix " << name << " does not exist.";
        throw std::runtime_error(error.str());
    }
    static constant_matrix_p find(std::string const& name) {
        std::string l_name(to_lower(name));
        for (auto const& e : cache()) {
            if (e.first == name)
                return e.second;
        }
        return nullptr;
    }
    static void remove(std::string const& name) {
        if (find(name))
            cache().erase(name);
    }
    static std::string display_matrix(std::string const& name) {
        return get(name)->display();
    }
    static void clear() {
        cache().clear();
    }
    static std::vector<std::string> list() {
        std::vector<std::string> ret;
        for (auto const& e : cache())
            ret.push_back(e.first);
        return ret;
    }
};
constant_matrix_p get_matrix_from_cache(std::string const& name) { return matrix_cache::get(name); }

class assignment : public expression {
    std::string const _name;
    matrix_expression_p _value{ nullptr };
    constant_matrix_p _old_value{ nullptr };
public:
    assignment(std::string const name_, matrix_expression_p value_) : _name(name_), _value(value_) {}
    expression_type type() const override { return expression_type::assignment; }
    double number_value() const override { throw std::runtime_error("Attempted to evaluate assignment as a number."); return 0; }
    matrix_value_t const matrix_value() const override { return evaluate_matrix()->matrix_value(); }
    std::shared_ptr<constant_matrix> const evaluate_matrix() const  override { throw std::runtime_error("Attempted to evaluate assignment as a matrix."); return nullptr; }

    std::string display() const override {
        std::stringstream buff;
        buff << _name << " = ";
        auto const_matrix = std::dynamic_pointer_cast<constant_matrix>(_value);
        if (const_matrix) 
            buff << "\n";
        
        buff << _value->display() << ";";
        return buff.str();
    }
    std::string display_with_brackets() const override { return display(); }
    std::string description(size_t tabs) const {
        auto const_matrix = std::dynamic_pointer_cast<constant_matrix>(_value);
        if (const_matrix) {
            std::stringstream buff;
            buff << _name << " = \n" << const_matrix->display_tabbed(tabs + 1) << std::string(tabs, '\t') << ";";
            return buff.str();
        }
        return display();
    }
    void json_stream(std::ostream& out, size_t& tabs) const override {
        using namespace json_util;
        out << start_expression_object("assignment", description(tabs), tabs) << next(tabs) <<
            labeled_value("name", _name, tabs) << next(tabs) <<
            labeled_value("value", _value, tabs) << next(tabs);
        out << end_expression_object(tabs);
    }

    void add_to_cache() {
        _old_value = matrix_cache::find(_name);
        if (_old_value)
            matrix_cache::remove(_name);
        auto const_matrix = std::dynamic_pointer_cast<constant_matrix>(_value);
        if(const_matrix)
            matrix_cache::add(_name, const_matrix);
        else
            matrix_cache::add(_name, _value->evaluate_matrix());
    }
    void restore_cache() const {
        matrix_cache::remove(_name);
        if (_old_value) 
            matrix_cache::add(_name, _old_value);
        
    }
};
namespace functions {
    enum class return_type_t {
        matrix,
        number
    };

    std::string to_string(return_type_t t) {
        switch (t) {
        case return_type_t::matrix: return "matrix";
        case return_type_t::number: return "number";
        default:
            ;
        }
        return "unknown";
    }

    enum class argument_type_t {
        matrix,
        number,
        size
    };

    std::string to_string(argument_type_t t) {
        switch (t) {
        case argument_type_t::matrix: return "matrix";
        case argument_type_t::number: return "number";
        case argument_type_t::size: return "size";
        default:
            ;
        }
        return "unknown";
    }
    class argument_t {
        // unions are too much paint prior to C++ 17, we gonna waste some memory
        int _size{ 0 };
        std::shared_ptr<number_expression> _number{ nullptr };
        matrix_expression_p _matrix{ nullptr };
        argument_type_t _type;
        void check_type(argument_type_t const t) const { if (t != _type) throw std::runtime_error("Invalid data type cast for argument."); }
    public:
        argument_t(int size_ = 0) : _type(argument_type_t::size), _size(size_) {  }
        argument_t(std::shared_ptr<number_expression> number_) : _type(argument_type_t::number), _number(number_) { }
        argument_t(matrix_expression_p matrix_) : _type(argument_type_t::matrix), _matrix(matrix_) { }

        ~argument_t() {}

        argument_type_t type() const { return _type; }
        inline int size() const {
            check_type(argument_type_t::size);
            return _size;
        }
        inline std::shared_ptr<number_expression> number() const {
            check_type(argument_type_t::number);
            return _number;
        }
        inline matrix_expression_p matrix() const {
            check_type(argument_type_t::matrix);
            return _matrix;
        }
    };

    class function {
        std::string _name;
        std::string _help;
        return_type_t _return_type;
        std::vector<argument_type_t> _argument_types;
    protected:
        void validate_argument_types(std::vector<argument_t> const& arguments) const {
            if (arguments.size() != _argument_types.size()) {
                std::stringstream buff;
                buff << "Wrong number of arguments. Function " << _name << " expects " << _argument_types.size() <<
                    " arguments, but " << arguments.size() << " arguments were provided.";
                throw std::runtime_error(buff.str());
            }
            for (size_t i = 0; i < _argument_types.size(); ++i) {
                if (_argument_types[i] != arguments[i].type()) {
                    std::stringstream buff;
                    buff << "When invoking function " << _name << " argument " << i << " was a " <<
                        to_string(arguments[i].type()) << ", but a " << to_string(_argument_types[i]) <<
                        " was expected.";
                    throw std::runtime_error(buff.str());
                }
            }
        }
    public:
        function(
            std::string name_, 
            std::string help_,
            return_type_t return_type_,
            std::vector<argument_type_t> argument_types_
        ) : _name(name_), _help(help_), _return_type(return_type_), _argument_types(argument_types_){}

        virtual ~function() {}
        inline return_type_t const return_type() const { return _return_type; }
        inline std::vector<argument_type_t> const& argument_types() const { return _argument_types; }
        inline std::string const& name() const { return _name; }
        inline std::string const& help() const { return _help; }
        inline std::string signature() const {
            std::stringstream buff;
            buff << to_string(_return_type) << " " << name() << "(";
            for (size_t i = 0; i < _argument_types.size(); ++i) {
                buff << to_string(_argument_types[i]);
                if (i < _argument_types.size() - 1)
                    buff << ", ";
            }
            buff << ")";
            return buff.str();
        }
    };

    struct number_function : public function {
        number_function(
            std::string name_,
            std::string help_,
            std::vector<argument_type_t> argument_types_
        ) : function(name_, help_,return_type_t::number, argument_types_) {}

        virtual std::shared_ptr<number_expression> evaluate(std::vector<argument_t> const& arguments) const = 0;
    };
    struct matrix_function : public function {
    protected:
        std::string make_name(std::string const& name, int rows, int columns) const {
            std::stringstream buff;
            buff << "__" << name << "_" << rows << "_" << columns;
            return buff.str();
        }
    public:
        matrix_function(
            std::string name_,
            std::string help_,
            std::vector<argument_type_t> argument_types_
        ) : function(name_, help_, return_type_t::matrix, argument_types_) {}

        virtual std::shared_ptr<constant_matrix> evaluate(std::vector<argument_t> const& arguments) const = 0;
    };

    class function_table {
        static std::map<std::string, std::shared_ptr<function>>& table() {
            static std::map<std::string, std::shared_ptr<function>> _inst;
            return _inst;
        }
    public:
        static bool add(std::shared_ptr<function> const& function_) {
            // for prettier error messages, we don't allow functions to only differ by case
            std::string existing;
            std::string l_name(to_lower(function_->name()));
            for (auto const& e : table()) {
                if (to_lower(e.first) == l_name) {
                    existing = e.first;
                    break;
                }
            }
            if (!existing.empty()) {
                std::stringstream error;
                error << "Function with a similar name already exists: " << existing << ". " << function_->name() <<
                    " differs only in case, please choose a better name for it.";
                throw std::runtime_error(error.str());
            }
            table()[function_->name()] = function_;
            return true;
        }
        static std::shared_ptr<function> get(std::string const& name) {
            std::string l_name(to_lower(name));
            for (auto const& e : table()) {
                if (e.first == name)
                    return e.second;
                if (to_lower(e.first) == l_name) {
                    std::stringstream error;
                    error << "Function " << name << " does not exist. Did you mean: " << e.first << "?";
                    throw std::runtime_error(error.str());
                }
            }
            std::stringstream error;
            error << "Function " << name << " does not exist.";
            throw std::runtime_error(error.str());
        }
        static std::shared_ptr<function> find(std::string const& name) {
            std::string l_name(to_lower(name));
            for (auto const& e : table()) {
                if (e.first == name)
                    return e.second;
                if (to_lower(e.first) == l_name) {
                    std::stringstream error;
                    error << "Function " << name << " does not exist. Did you mean: " << e.first << "?";
                    throw std::runtime_error(error.str());
                }
            }
            return nullptr;
        }
        static bool check_exists_nc(std::string const& name) {
            std::string l_name(to_lower(name));
            for (auto const& e : table()) {
                if (to_lower(e.first) == l_name)
                    return true;
            }
            return false;
        }
        static std::map<std::string, std::string> help() {
            std::map<std::string, std::string> ret;
            for (auto const& e : table()) {
                ret[e.second->signature()] = e.second->help();
            }
            return ret;
        }
    };

    struct abs : public number_function {
        abs() : number_function("abs", "Absolute value of a number.", { argument_type_t::number }) {}
        std::shared_ptr<number_expression> evaluate(std::vector<argument_t> const& arguments) const override {
            validate_argument_types(arguments);
            return std::shared_ptr<number_expression>(new constant_number(fabs(arguments[0].number()->number_value())));
        }
    };
    static auto register_abs = function_table::add(std::shared_ptr<function>(new abs()));

    struct trace : public number_function  {
        trace() : number_function("trace", "Computes trace of a matrix.", { argument_type_t::matrix }) {}
        std::shared_ptr<number_expression> evaluate(std::vector<argument_t> const& arguments) const override {
            validate_argument_types(arguments);
            auto const& matrix = arguments[0].matrix()->matrix_value();
            double v = matrix.trace();
            return std::shared_ptr<number_expression>(new constant_number(v));
        }
    };
    static auto register_trace = function_table::add(std::shared_ptr<function>(new trace()));

    class identity : public matrix_function {

    public:
        identity() : matrix_function("identity", "Creates a diagonal matrix of a given size.", { argument_type_t::size, argument_type_t::size }) {
        }
        std::shared_ptr<constant_matrix> evaluate(std::vector<argument_t> const& arguments) const override {
            validate_argument_types(arguments);
            auto const& rows = arguments[0].size();
            auto const& cols = arguments[1].size();
            auto m = constant_matrix::create(Eigen::MatrixXd::Identity(rows, cols));
            return m;
        }
    };
    static auto register_identity = function_table::add(std::shared_ptr<function>(new identity()));

    class zero : public matrix_function {
    public:
        zero() : matrix_function("zero", "Creates a zero matrix of a given size.", { argument_type_t::size, argument_type_t::size }) {
        }
        std::shared_ptr<constant_matrix> evaluate(std::vector<argument_t> const& arguments) const override {
            validate_argument_types(arguments);
            auto const& rows = arguments[0].size();
            auto const& cols = arguments[1].size();
            auto m = constant_matrix::create(Eigen::MatrixXd::Zero(rows, cols));
            return m;
        }
    };
    static auto register_zero = function_table::add(std::shared_ptr<function>(new zero()));

    class random : public matrix_function {
    public:
        random() : matrix_function("random", "Creates a random matrix of a given size.", { argument_type_t::size, argument_type_t::size }) {
        }
        std::shared_ptr<constant_matrix> evaluate(std::vector<argument_t> const& arguments) const override {
            validate_argument_types(arguments);
            auto const& rows = arguments[0].size();
            auto const& cols = arguments[1].size();
            auto m = constant_matrix::create(Eigen::MatrixXd::Random(rows, cols));
            return m;
        }
    };
    static auto register_random = function_table::add(std::shared_ptr<function>(new random()));

    class constant : public matrix_function {
    public:
        constant() : matrix_function("constant", "Creates a constant matrix of a given size.", { 
            argument_type_t::size, argument_type_t::size, argument_type_t::number }) {
        }
        std::shared_ptr<constant_matrix> evaluate(std::vector<argument_t> const& arguments) const override {
            validate_argument_types(arguments);
            auto const& rows = arguments[0].size();
            auto const& cols = arguments[1].size();
            auto const& v = arguments[2].number()->number_value();
            auto m = constant_matrix::create(Eigen::MatrixXd::Constant(rows, cols, v));
            return m;
        }
    };
    static auto register_constant = function_table::add(std::shared_ptr<function>(new constant()));

    class dot : public number_function {
    public:
        dot() : number_function("dot", "Dot product.", { argument_type_t::matrix, argument_type_t::matrix }) {
        }
        std::shared_ptr<number_expression> evaluate(std::vector<argument_t> const& arguments) const override {
            validate_argument_types(arguments);
            auto const& m1 = arguments[0].matrix()->matrix_value();
            auto const& m2 = arguments[0].matrix()->matrix_value();
            VectorXd v1 = m1.reshaped();
            VectorXd v2 = m2.reshaped();
            return constant_number::create(v1.dot(v2));
        }
    };
    static auto register_dot = function_table::add(std::shared_ptr<function>(new dot()));

}

bool check_function_exists(std::string const& name) {
    return functions::function_table::check_exists_nc(name);
}


namespace function_inv {
    std::string display(
        std::string const& function_name,
        std::vector<functions::argument_t> const& arguments
    ){
        std::stringstream buff;
        buff << function_name << "(";
        for (size_t i = 0; i < arguments.size(); ++i) {
            auto const& arg = arguments[i];
            switch (arg.type()) {
            case functions::argument_type_t::size:
                buff << arg.size();
                break;
            case functions::argument_type_t::number:
                buff << arg.number()->display();
                break;
            case functions::argument_type_t::matrix:
                buff << arg.matrix()->display();
                break;
            default:
                throw std::runtime_error("Unknown argument type.");
            }
            if (i < arguments.size() - 1)
                buff << ", ";
        }
        buff << ")";
        return buff.str();
    }
    std::string display_with_brackets(
        std::string const& function_name,
        std::vector<functions::argument_t> const& arguments
    ) {
        std::stringstream buff;
        buff << function_name << "(";
        for (size_t i = 0; i < arguments.size(); ++i) {
            auto const& arg = arguments[i];
            switch (arg.type()) {
            case functions::argument_type_t::size:
                buff << arg.size();
                break;
            case functions::argument_type_t::number:
                buff << arg.number()->display_with_brackets();
                break;
            case functions::argument_type_t::matrix:
                buff << arg.matrix()->display_with_brackets();
                break;
            default:
                throw std::runtime_error("Unknown argument type.");
            }
            if (i < arguments.size() - 1)
                buff << ", ";
        }
        buff << ")";
        return buff.str();
    }
    void json_stream(std::ostream& out, size_t& tabs, 
        std::string const& function_name,
        std::vector<functions::argument_t> const& arguments
        ){
        using namespace json_util;
        out << labeled_value("function", function_name, tabs) << next(tabs) <<
            label("arguments", tabs) <<
            start_array(tabs);
        for (size_t i = 0; i < arguments.size(); ++i) {
            auto const& arg = arguments[i];
            switch (arg.type()) {
            case functions::argument_type_t::size:
                out << js(arg.size(), tabs);
                break;
            case functions::argument_type_t::number:
                out << js(arg.number(), tabs);
                break;
            case functions::argument_type_t::matrix:
                out << js(arg.matrix(), tabs);
                break;
            default:
                throw std::runtime_error("Unknown argument type.");
            }
            if (i < arguments.size() - 1)
                out << next(tabs);
        }
        out << end_array(tabs);
    }
}
class matrix_function_inv : public non_const_matrix {
public:
    std::shared_ptr<functions::matrix_function> _function;
    std::vector<functions::argument_t> _arguments;
public:
    matrix_function_inv(
        std::shared_ptr<functions::matrix_function> function_,
        std::vector<functions::argument_t> const& arguments_) : _function(function_), _arguments(arguments_) {
    }
    
    std::string display() const override { 
        return function_inv::display(_function->name(), _arguments);
    }
    std::string display_with_brackets() const override {
        return function_inv::display_with_brackets(_function->name(), _arguments);
    }

    void json_stream(std::ostream& out, size_t& tabs) const override {
        using namespace json_util;
        out << start_expression_object("matrix_function_inv", display(), tabs) << next(tabs);
        function_inv::json_stream(out, tabs, _function->name(), _arguments);
        if (json_configuration::include_result())
            out << next(tabs) << result_value(evaluate_matrix(), tabs);
        out << end_expression_object(tabs);
    }

    std::shared_ptr<constant_matrix> const evaluate_matrix() const override { return _function->evaluate(_arguments); }
};
class number_function_inv : public number_expression {
public:
    std::shared_ptr<functions::number_function> _function;
    std::vector<functions::argument_t> _arguments;
public:
    number_function_inv(
        std::shared_ptr<functions::number_function> function_,
        std::vector<functions::argument_t> const& arguments_) : _function(function_), _arguments(arguments_) {
    }

    std::string display() const override {
        return function_inv::display(_function->name(), _arguments);
    }
    std::string display_with_brackets() const override {
        return function_inv::display_with_brackets(_function->name(), _arguments);
    }

    void json_stream(std::ostream& out, size_t& tabs) const override {
        using namespace json_util;
        out << start_expression_object("number_function_inv", display(), tabs) << next(tabs);
        function_inv::json_stream(out, tabs, _function->name(), _arguments);
        if (json_configuration::include_result())
            out << next(tabs) << result_value(number_value(), tabs);
        out << end_expression_object(tabs);
    }
    double number_value() const override { return _function->evaluate(_arguments)->number_value(); }
};

namespace parsing {
    std::string error_with_position(std::string const& error, size_t const dot) {
        std::stringstream buff;
        buff << "Syntax error: " << error << " (at position: " << dot << ")";
        return buff.str();
    }

    inline void skip_whitespace(std::string const code, size_t& dot) {
        while (dot < code.size() && std::isspace(code[dot]))
            ++dot;
    }
    inline bool done(std::string const& code, size_t dot) {
        return dot >= code.size();
    }
    inline void complete(std::string const& code, size_t& dot, size_t local_dot) {
        skip_whitespace(code, local_dot);
        dot = local_dot;
    }
    inline bool match(std::string const& code, size_t& dot, std::string const& token) {
        // see if the token appears next in the code
        size_t local_dot(dot);
        for (size_t i = 0; i < token.size(); ++i, ++local_dot) {
            if (code[local_dot] != token[i])
                return false;
        }
        dot = local_dot;
        return true;
    }
    // we often skip past tokens but expect more after them (for all arithmetic)
    // skip_past_token does that and checks if there is more coming
    inline void skip_past_token(std::string const& code, size_t& dot) {
        char c = code[dot];
        ++dot;
        skip_whitespace(code, dot);
        if (done(code, dot))
            throw std::runtime_error(error_with_position(std::string("trailing ") + c, dot));
    }
    bool empty(std::stringstream& buff) {
        return (buff.tellp() == std::streampos(0));
    }
    template<typename T>
    struct parse_result_t {
        T _value{};
        bool _found{ false }; // has the value been succesfully parsed

        operator bool() const { return _found; }
        operator T const& () const { return _value; }
        T const& value() const { return _value; }

        static parse_result_t<T> const& not_found() {
            static parse_result_t<T> _empty;
            return _empty;
        }
        template< typename Other> operator parse_result_t<Other>() const {
            if (!(this->_found))
                return parse_result_t<Other>::not_found();
            throw std::runtime_error("Programming error: no appropriate conversion betwen parse_result types.");
        }
    };
    template<typename T> inline parse_result_t<T> found(T const& _v) { return parse_result_t<T>{_v, true }; }
    template<typename T, typename U> parse_result_t<std::shared_ptr<T>> dynamic_result_cast(parse_result_t<U>& ur) {
        return parse_result_t< std::shared_ptr<T>>{std::dynamic_pointer_cast<T>(ur._value), ur._found };
    }
    parse_result_t<bool> not_found() {
        static parse_result_t<bool> _notfound{ false };
        return _notfound;
    }

    // we only cache results of things that can backtrack and take longer to parse than to look up
    enum class packrat_t {
        bracketed_number_expression,
        number_function_invocation,
        number_mul_term,
        number_product,
        number_rhs_term,
        number_term,
        number_sum,
        number_expression,
        matrix_term,
        matrix_number_term,
        matrix_product,
        matrix_expression
    };

    struct packrat_cache_base {
        virtual ~packrat_cache_base() {}
    };
    template<typename ResultT>
    struct cached_result_t {
        parse_result_t<ResultT> _result;
        size_t _dot{ (size_t)-1 };
    };

    template< typename ResultT>
    class packrat_cache : public packrat_cache_base {
        std::map<size_t, cached_result_t<ResultT>> _results;
    public:
        inline parse_result_t<ResultT>& add(
            size_t dot,
            parse_result_t<ResultT>& result,
            size_t new_dot) {
            _results[dot] = cached_result_t<ResultT>{ result, new_dot };
            return result;
        }
        inline parse_result_t<ResultT> add(
            size_t dot,
            ResultT& result,
            size_t new_dot) {
            parse_result_t<ResultT> ret(found(result));
            _results[dot] = cached_result_t<ResultT>{ ret, new_dot };
            return ret;
        }
        inline parse_result_t<ResultT> not_found(size_t dot) {
            parse_result_t<ResultT> ret(parsing::not_found());
            _results[dot] = cached_result_t<ResultT>{ ret, dot };
            return ret;
        }

        inline bool has(size_t dot) {
            return _results.find(dot) != _results.end();
        }
        inline cached_result_t<ResultT> get(size_t dot) {
            return _results[dot];
        }
        inline parse_result_t<ResultT> update_dot_and_get(size_t& dot) {
            auto& res = _results[dot];
            dot = res._dot;
            return res._result;
        }
    };
    template<typename ResultT>
    inline parse_result_t<ResultT> complete_cached(
        std::string const& code,
        size_t& dot,
        size_t local_dot,
        std::shared_ptr <packrat_cache<ResultT>> cache,
        ResultT& result) {

        skip_whitespace(code, local_dot);
        auto ret = cache->add(dot, result, local_dot);
        dot = local_dot;
        return ret;
    }
    template<typename ResultT>
    inline parse_result_t<ResultT> complete_cached(
        std::string const& code,
        size_t& dot,
        size_t local_dot,
        std::shared_ptr <packrat_cache<ResultT>> cache,
        parse_result_t<ResultT>& result) {

        skip_whitespace(code, local_dot);
        auto ret = cache->add(dot, result, local_dot);
        dot = local_dot;
        return ret;
    }


    template<typename ResultT>
    inline parse_result_t<ResultT> cached_not_found(
        size_t dot,
        std::shared_ptr<packrat_cache<ResultT>> cache) {
        return cache->not_found(dot);
    }

    class packrat_caches {
        using caches_t = std::map<packrat_t, std::shared_ptr<packrat_cache_base>>;
        static caches_t& caches() {
            static caches_t _caches;
            return _caches;
        }
        using root_cache_t = std::unordered_map<std::string, std::shared_ptr<expression>>;
        static root_cache_t& root_cache() {
            static root_cache_t _root_cache;
            return _root_cache;
        }
    public:
        static void clear() {
            caches().clear();
        }

        template< typename ResultT>
        static std::shared_ptr<packrat_cache<ResultT>> get(packrat_t _t) {
            auto it = caches().find(_t);
            if (it != caches().end()) {
                std::shared_ptr<packrat_cache<ResultT>> ret = std::dynamic_pointer_cast<packrat_cache<ResultT>>(it->second);
                if (!ret)
                    throw std::runtime_error("Memoization error: typed cache is of wrong type.");
                return ret;
            }

            std::shared_ptr<packrat_cache<ResultT>> ret(new packrat_cache<ResultT>);
            caches()[_t] = ret;
            return ret;
        }
        static std::shared_ptr<expression> add_root(std::string const& code, std::shared_ptr<expression> root) {
            root_cache()[to_lower(code)] = root;
            return root;
        }
        static std::shared_ptr<expression> get_root(std::string const& code) {
            auto it = root_cache().find(to_lower(code));
            if (it == root_cache().end())
                return nullptr;
            return it->second;
        }
        static void clear_roots() {
            root_cache().clear();
        }

    };

    // during the recursive decent we assume every function starts at a non-whitespace
    // and every function needs to skip any whitespace left after it.
    // 
    // we need some forward declarations, this is a recursive decent after all
    parse_result_t<std::shared_ptr<expression>> get_expression(std::string const& code, size_t& dot);
    parse_result_t<matrix_expression_p> get_matrix_expression(std::string const& code, size_t& dot);
    parse_result_t<std::shared_ptr<number_expression>> get_number_expression(std::string const& code, size_t& dot);

    parse_result_t<std::shared_ptr<number_expression>> get_constant_number(std::string const code, size_t& dot) {
        // parsing constant numbers is pretty boring and done in one pass with no backtracking
        // we won't be cacheing it for now, maybe later, if it pops up in profilers
        size_t local_dot(dot);
        if (done(code, local_dot)) return not_found();

        std::stringstream  buff;
        char c = code[local_dot];
        if (c == '+' || c == '-') {
            buff << c;
            c = code[++local_dot];
        }
        while (std::isdigit(c)) { // we don't need to check for end of string, isdigit will fail for \0
            buff << c;
            c = code[++local_dot];
        }
        if (c == '.') { // c is 0 at the end of string
            buff << c;
            c = code[++local_dot];
            while (std::isdigit(c)) {
                buff << c;
                c = code[++local_dot];
            }
        }
        if (!empty(buff)) {
            complete(code, dot, local_dot);
            std::shared_ptr<number_expression> ret(new constant_number(std::stod(buff.str())));
            return found(ret);
        }
        return not_found();
    }
    parse_result_t<double> get_constant_number_value(std::string const code, size_t& dot) {
        // parsing constant numbers is pretty boring and done in one pass with no backtracking
        // we won't be cacheing it for now, maybe later, if it pops up in profilers
        size_t local_dot(dot);
        if (done(code, local_dot)) return not_found();

        std::stringstream  buff;
        char c = code[local_dot];
        if (c == '+' || c == '-') {
            buff << c;
            c = code[++local_dot];
        }
        while (std::isdigit(c)) { // we don't need to check for end of string, isdigit will fail for \0
            buff << c;
            c = code[++local_dot];
        }
        if (c == '.') { // c is 0 at the end of string
            buff << c;
            c = code[++local_dot];
            while (std::isdigit(c)) {
                buff << c;
                c = code[++local_dot];
            }
        }
        if (!empty(buff)) {
            complete(code, dot, local_dot);
            return found(std::stod(buff.str()));
        }
        return not_found();
    }
    parse_result_t<std::vector<double>> get_constant_row(std::string const code, size_t& dot) {
        // parsing constant numbers is pretty boring and done in one pass with no backtracking
        // we won't be cacheing it for now, maybe later, if it pops up in profilers
        size_t local_dot(dot);
        if (done(code, local_dot)) return not_found();

        char c = code[local_dot];
        if (c != '[') return not_found();
        skip_past_token(code, local_dot);
        std::vector<double> ret;
        auto n = get_constant_number_value(code, local_dot);
        if (!n)
            throw std::runtime_error(error_with_position("expected a number", local_dot));
        while (n) {
            ret.push_back(n);
            c = code[local_dot];
            if (c != ',' || c == ']') break;
            skip_past_token(code, local_dot);
            n = get_constant_number_value(code, local_dot);
            if (!n)
                throw std::runtime_error(error_with_position("expected a number", local_dot));
        }
        if(c!=']')
            throw std::runtime_error(error_with_position("expected ']'", local_dot));
        skip_past_token(code, local_dot);
        
        complete(code, dot, local_dot);
        return found(ret);
    }
    parse_result_t<std::vector<std::vector<double>>> get_constant_array(std::string const code, size_t& dot) {
        // parsing constant numbers is pretty boring and done in one pass with no backtracking
        // we won't be cacheing it for now, maybe later, if it pops up in profilers
        size_t local_dot(dot);
        if (done(code, local_dot)) return not_found();

        char c = code[local_dot];
        if (c != '[') return not_found();
        skip_past_token(code, local_dot);
        std::vector<std::vector<double>> ret;
        auto r = get_constant_row(code, local_dot);
        if (!r)
            throw std::runtime_error(error_with_position("expected a row", local_dot));
        while (r) {
            ret.push_back(r.value());
            c = code[local_dot];
            if (c != ',' || c == ']') break;
            skip_past_token(code, local_dot);
            r = get_constant_row(code, local_dot);
            if (!r)
                throw std::runtime_error(error_with_position("expected a row", local_dot));
        }
        if (c != ']')
            throw std::runtime_error(error_with_position("expected ']'", local_dot));
        skip_past_token(code, local_dot);

        complete(code, dot, local_dot);
        return found(ret);
    }

    parse_result_t<int> get_size(std::string const code, size_t& dot) {
        // parsing constant numbers is pretty boring and done in one pass with no backtracking
        // we won't be cacheing it for now, maybe later, if it pops up in profilers
        size_t local_dot(dot);
        if (done(code, local_dot)) return not_found();

        std::stringstream  buff;
        char c = code[local_dot];
        while (std::isdigit(c)) {// we don't need to check for end of string, isdigit will fail for \0
            buff << c;
            c = code[++local_dot];
        }
        if (!empty(buff)) {
            complete(code, dot, local_dot);
            return found(std::stoi(buff.str()));
        }
        return not_found();
    }
    parse_result_t<std::string> get_name(std::string const code, size_t& dot) {
        // parsing names is pretty boring and done in one pass with no backtracking
        // we won't be cacheing it for now, maybe later, if it pops up in profilers
        size_t local_dot(dot);
        if (done(code, local_dot)) return not_found();

        std::stringstream  buff;
        char c = code[local_dot];
        if (!std::isalpha(c))
            return not_found();
        while (std::isalnum(c)) {// note that we don't need to check for end of string, isalnum will fail for \0
            buff << c;
            c = code[++local_dot];
        }
        if (!empty(buff)) {
            complete(code, dot, local_dot);
            return found(buff.str());
        }
        return not_found();
    }
    parse_result_t<std::shared_ptr<named_matrix>> get_constant_matrix(std::string const& name) {
        return found(std::shared_ptr<named_matrix>(new named_matrix(name)));
    }
    parse_result_t<std::shared_ptr<functions::matrix_function>> get_matrix_function(std::string const& name) {
        auto f = functions::function_table::find(name);
        if (f && f->return_type() == functions::return_type_t::matrix)
            return found(std::dynamic_pointer_cast<functions::matrix_function>(f));
        return not_found();
    }
    parse_result_t<std::shared_ptr<functions::number_function>> get_number_function(std::string const& name) {
        auto f = functions::function_table::find(name);
        if (f && f->return_type() == functions::return_type_t::number)
            return found(std::dynamic_pointer_cast<functions::number_function>(f));
        return not_found();
    }
    parse_result_t<functions::argument_t> get_argument(std::string const& code, size_t& dot, functions::argument_type_t arg_type) {
        // parsing arguments is done with no backtracking, no cacheing is needed
        size_t local_dot(dot);
        if (done(code, local_dot)) return not_found();

        switch (arg_type) {
        case functions::argument_type_t::matrix: {
            auto m = get_matrix_expression(code, local_dot);
            if (!m)
                return not_found();
            complete(code, dot, local_dot);
            return found(functions::argument_t(m.value()));
        }
        case functions::argument_type_t::number: {
            auto n = get_number_expression(code, local_dot);
            if (!n)
                return not_found();
            complete(code, dot, local_dot);
            return found(functions::argument_t(n.value()));
        }
        case functions::argument_type_t::size: {
            auto sz = get_size(code, local_dot);
            if (!sz)
                return not_found();
            complete(code, dot, local_dot);
            return found(functions::argument_t(sz.value()));
        }
        default:
            ;
        }
        return not_found();
    }
    parse_result_t<std::vector<functions::argument_t>> get_arguments(std::string const& code, size_t& dot,
        // parsing arguments is done with no backtracking, no cacheing is needed
        std::vector<functions::argument_type_t>const& arg_types) {
        size_t local_dot(dot);
        if (done(code, local_dot)) return not_found();

        if (code[local_dot] != '(')
            throw std::runtime_error(error_with_position("function arguments not found", local_dot));
        skip_past_token(code, local_dot);

        std::vector<functions::argument_t> ret;
        ret.reserve(arg_types.size());
        for (size_t i = 0; i < arg_types.size(); ++i) {
            skip_whitespace(code, local_dot);
            auto arg = get_argument(code, local_dot, arg_types[i]);
            if (!arg)
                throw std::runtime_error(error_with_position("function argument not found or is of wrong type", local_dot));
            ret.push_back(arg);
            skip_whitespace(code, local_dot);
            if (i != arg_types.size() - 1) {
                if (code[local_dot] != ',')
                    throw std::runtime_error(error_with_position("not enough function arguments", local_dot));
                skip_past_token(code, local_dot);
            }
        }
        skip_whitespace(code, local_dot);
        if (code[local_dot++] != ')')
            throw std::runtime_error(error_with_position("unbalanced closing bracket", local_dot));
        skip_whitespace(code, local_dot);
        complete(code, dot, local_dot);
        return found(ret);
    }
    parse_result_t<std::shared_ptr<number_expression>> get_bracketed_number_expression(std::string const& code, size_t& dot) {
        auto pcache(packrat_caches::get< std::shared_ptr<number_expression>>(packrat_t::bracketed_number_expression));
        if (pcache->has(dot))
            return pcache->update_dot_and_get(dot);
        size_t local_dot(dot);
        if (done(code, local_dot)) return cached_not_found(dot, pcache);

        if (code[local_dot] == '(') {
            skip_past_token(code, local_dot);
            auto n = get_number_expression(code, local_dot);
            if (!n) // ok, this ain't it
                return cached_not_found(dot, pcache);

            if (code[local_dot++] != ')')
                throw std::runtime_error(error_with_position("unbalanced brackets", local_dot));
            skip_whitespace(code, local_dot);
            std::shared_ptr<number_expression> ret(new bracketed_number(n.value()));
            return complete_cached(code, dot, local_dot, pcache, ret);

        }
        return cached_not_found(dot, pcache);
    }
    parse_result_t<std::shared_ptr<number_expression>> get_number_function_invocation(std::string const& code, size_t& dot) {
        auto pcache(packrat_caches::get< std::shared_ptr<number_expression>>(packrat_t::number_function_invocation));
        if (pcache->has(dot))
            return pcache->update_dot_and_get(dot);
        size_t local_dot(dot);
        if (done(code, local_dot)) return cached_not_found(dot, pcache);

        auto name = get_name(code, local_dot);
        if (!name)
            return cached_not_found(dot, pcache);

        auto nf = get_number_function(name.value());
        if (!nf)
            return cached_not_found(dot, pcache);

        auto arguments = get_arguments(code, local_dot, nf.value()->argument_types());
        if (!arguments)
            throw std::runtime_error(error_with_position("function arguments not found", local_dot));

        std::shared_ptr<number_expression> ret(new number_function_inv(nf.value(), arguments.value()));
        return complete_cached(code, dot, local_dot, pcache, ret);
    }

    parse_result_t<std::shared_ptr<number_expression>> get_number_mul_term(std::string const& code, size_t& dot) {
        auto pcache(packrat_caches::get< std::shared_ptr<number_expression>>(packrat_t::number_mul_term));
        if (pcache->has(dot))
            return pcache->update_dot_and_get(dot);

        size_t local_dot(dot);
        if (done(code, local_dot)) return cached_not_found(dot, pcache);

        auto term = get_constant_number(code, local_dot); //first check for constants
        if (!term) {// brackets?
            term = get_bracketed_number_expression(code, local_dot);
            if (!term) {
                term = get_number_function_invocation(code, local_dot);
                if (!term)
                    return cached_not_found(dot, pcache);
            }
        }
        return complete_cached(code, dot, local_dot, pcache, term);
    }
    parse_result_t<std::shared_ptr<number_expression>> get_number_product(std::string const& code, size_t& dot) {
        auto pcache(packrat_caches::get< std::shared_ptr<number_expression>>(packrat_t::number_product));
        if (pcache->has(dot))
            return pcache->update_dot_and_get(dot);

        size_t local_dot(dot);
        if (done(code, local_dot)) return cached_not_found(dot, pcache);

        auto product = get_number_mul_term(code, local_dot);
        if (!product) return cached_not_found(dot, pcache);

        if (!done(code, local_dot)) {
            char op = code[local_dot];
            auto next_term = [&]() {
                if (op == '-' || op == '+' || op == ')')
                    return cached_not_found(dot, pcache);
                if (op == '*' || op == '/')
                    skip_past_token(code, local_dot);
                else
                    op = '*';
                return get_number_mul_term(code, local_dot);
                };
            parse_result_t<std::shared_ptr<number_expression>> next = next_term();
            while (next) {
                product = found(std::shared_ptr<number_expression>(new number_arithmetic(product.value(), next.value(), op)));
                op = code[local_dot];
                next = next_term();
            }
        }
        return complete_cached(code, dot, local_dot, pcache, product);
    }
    parse_result_t<std::shared_ptr<number_expression>> get_number_term(std::string const& code, size_t& dot) {
        // number term is only there to deal with unary + and - for number expressions
        auto pcache(packrat_caches::get< std::shared_ptr<number_expression>>(packrat_t::number_term));
        if (pcache->has(dot))
            return pcache->update_dot_and_get(dot);

        size_t local_dot(dot);
        if (done(code, local_dot)) return cached_not_found(dot, pcache);

        bool is_negative = false;
        char c = code[local_dot];
        while (c == '+' || c == '-') {
            is_negative = (c == '-') ? (!is_negative) : is_negative;
            skip_past_token(code, local_dot); // we always expect more after + or -
            c = code[local_dot];
        }


        auto rhs = get_number_product(code, local_dot);
        if (!rhs)
            return cached_not_found(dot, pcache);
        if (is_negative) {
            std::shared_ptr<number_expression> ret(new negative_number(rhs.value()));
            return complete_cached(code, dot, local_dot, pcache, ret);
        }
        return complete_cached(code, dot, local_dot, pcache, rhs);
    }

    parse_result_t<std::shared_ptr<number_expression>> get_number_sum(std::string const& code, size_t& dot) {
        // number_sum is right-recursive
        auto pcache(packrat_caches::get< std::shared_ptr<number_expression>>(packrat_t::number_sum));
        if (pcache->has(dot))
            return pcache->update_dot_and_get(dot);

        size_t local_dot(dot);
        if (done(code, local_dot)) return cached_not_found(dot, pcache);

        auto sum = get_number_term(code, local_dot);
        if (!sum) return cached_not_found(dot, pcache);
        if (!done(code, local_dot)) {
            char op = code[local_dot];
            auto next_term = [&]() {
                if (op == '-' || op == '+') {
                    skip_past_token(code, local_dot);
                    return get_number_term(code, local_dot);
                }
                return cached_not_found(dot, pcache);
                };

            parse_result_t<std::shared_ptr<number_expression>> next = next_term();
            while (next) {
                sum = found(std::shared_ptr<number_expression>(new number_arithmetic(sum.value(), next.value(), op)));
                op = code[local_dot];
                next = next_term();
            }
        }
        return complete_cached(code, dot, local_dot, pcache, sum);
    }

    parse_result_t<std::shared_ptr<number_expression>> get_number_expression(std::string const& code, size_t& dot) {
        // number expression is either a number sum or a determinant
        // we may be backtracking, so let's cache it
        auto pcache(packrat_caches::get< std::shared_ptr<number_expression>>(packrat_t::number_expression));
        if (pcache->has(dot))
            return pcache->update_dot_and_get(dot);

        size_t local_dot(dot);
        if (done(code, local_dot)) return cached_not_found(dot, pcache);

        auto n = get_number_sum(code, local_dot);
        if (n)
            return complete_cached(code, dot, local_dot, pcache, n);

        // else
        if (code[local_dot] == '|') {
            ++local_dot;
            skip_whitespace(code, local_dot);
            auto m = get_matrix_expression(code, local_dot);
            if (m) {
                skip_whitespace(code, local_dot);
                if (code[local_dot++] != '|')
                    throw std::runtime_error(error_with_position("expected '|'", dot));
                std::shared_ptr<number_expression> ret(new determinant(m.value()));
                return complete_cached(code, dot, local_dot, pcache, ret);
            }
        }
        return cached_not_found(dot, pcache);
    }

    parse_result_t<matrix_expression_p> get_matrix_term_postfix(
        parse_result_t<matrix_expression_p> m,
        std::string const& code, size_t& dot) {
        // matrix postfix operators bind very tightly, they are right recursive, and may backtrack ...
        // but they are real quick to parse (they are short), so we won't be caching them for now         
        size_t local_dot(dot);
        if (!m || done(code, local_dot)) return m;

        matrix_expression_p ret;
        if (match(code, local_dot, "^-1")) { // inverse
            ret = matrix_expression_p(new matrix_unary_op(m.value(), unary_matrix_op::inverse));
        }
        if (match(code, local_dot, "^T")) { // inverse
            ret = matrix_expression_p(new matrix_unary_op(m.value(), unary_matrix_op::transpose));
        }
        if (ret) {
            complete(code, dot, local_dot);
            return get_matrix_term_postfix(found(ret), code, dot); // right recursion
        }
        return m;
    }
    parse_result_t<matrix_expression_p> get_matrix_term(std::string const& code, size_t& dot) {
        // matrix_terms are recursive and complicated, we'll cache them for now
        auto pcache(packrat_caches::get< matrix_expression_p>(packrat_t::matrix_term));
        if (pcache->has(dot))
            return pcache->update_dot_and_get(dot);

        size_t local_dot(dot);
        if (done(code, local_dot)) return cached_not_found(dot, pcache);

        auto name = get_name(code, local_dot);
        if (name) { // constant matrix or function
            auto mf = get_matrix_function(name.value());
            if (mf) {
                auto arguments = get_arguments(code, local_dot, mf.value()->argument_types());
                if (!arguments)
                    throw std::runtime_error(error_with_position("function arguments not found", local_dot));
                matrix_expression_p mfi(new matrix_function_inv(mf.value(), arguments.value()));
                auto ret = get_matrix_term_postfix(found(mfi), code, local_dot);
                return complete_cached(code, dot, local_dot, pcache, ret);
            }
            if(get_number_function(name.value()))
                return not_found();
            auto m = get_constant_matrix(name);
            if (m) {
                auto ret = get_matrix_term_postfix(dynamic_result_cast<matrix_expression>(m), code, local_dot);
                return complete_cached(code, dot, local_dot, pcache, ret);
            }
            return not_found();
        }
        // else
        char c = code[local_dot];
        if (c == '-' || c == '+') {
            bool is_negative = false;
            while (c == '+' || c == '-') {
                is_negative = (c == '-') ? (!is_negative) : is_negative;
                skip_past_token(code, local_dot); // we always expect more after + or -
                c = code[local_dot];
            }
            auto m = get_matrix_term(code, local_dot);
            if (!m)
                return cached_not_found(dot, pcache);
            auto pf = get_matrix_term_postfix(m, code, local_dot); // postfix binds tigher than -
            if (is_negative) {
                matrix_expression_p ret(new matrix_unary_op(pf.value(), unary_matrix_op::negative));
                return complete_cached(code, dot, local_dot, pcache, ret);
            }
            else {
                return complete_cached(code, dot, local_dot, pcache, pf);
            }
        }
        // else
        if (code[local_dot] == '(') {
            skip_past_token(code, local_dot);
            auto m = get_matrix_expression(code, local_dot);
            if (!m) {
                return cached_not_found(dot, pcache);
            }
            else {
                if (code[local_dot++] != ')')
                    throw std::runtime_error(error_with_position("unbalanced brackets", local_dot));
                matrix_expression_p bm(new bracketed_matrix(m.value()));
                auto ret = get_matrix_term_postfix(found(bm), code, local_dot);
                return complete_cached(code, dot, local_dot, pcache, ret);
            }
        }
        return cached_not_found(dot, pcache);
    }

    parse_result_t<matrix_expression_p> get_matrix_number_term(std::string const& code, size_t& dot) {
        // matrix_number_term is recursive and can backtrack, so cacheing
        auto pcache(packrat_caches::get< matrix_expression_p>(packrat_t::matrix_number_term));
        if (pcache->has(dot))
            return pcache->update_dot_and_get(dot);

        size_t local_dot(dot);
        if (done(code, local_dot)) return cached_not_found(dot, pcache);

        auto m = get_matrix_term(code, local_dot);
        if (m) {
            if (!done(code, local_dot)) {
                char c = code[local_dot];
                if (c == '*' || c == '/') {
                    skip_past_token(code, local_dot);
                    auto n = get_number_expression(code, local_dot);
                    if (!n) {
                        if (c == '/')
                            throw std::runtime_error(error_with_position("a number expression expected", local_dot));
                        // this could be a matrix multiplication
                        --local_dot; // falls through to return m
                    }
                    else { // yay, this is an matrix_number_op
                        matrix_expression_p ret(new matrix_number_op(m.value(), n.value(), c));
                        return complete_cached(code, dot, local_dot, pcache, ret);
                    }
                }
            }
            return complete_cached(code, dot, local_dot, pcache, m);
        }
        // else
        auto n = get_number_expression(code, local_dot);
        if (n) {
            if (!done(code, local_dot)) {
                char c = code[local_dot];
                if (c == '*') {
                    skip_past_token(code, local_dot);
                    auto m = get_matrix_term(code, local_dot);
                    if (m) {
                        matrix_expression_p ret(new matrix_number_op(m.value(), n.value(), '*'));
                        return complete_cached(code, dot, local_dot, pcache, ret);
                    }
                }
                else {
                    skip_whitespace(code, local_dot);
                    auto m = get_matrix_term(code, local_dot);
                    if (m) {
                        matrix_expression_p ret(new matrix_number_op(m.value(), n.value(), '*'));
                        return complete_cached(code, dot, local_dot, pcache, ret);
                    }
                }
            }
        }
        return cached_not_found(dot, pcache);
    }
    parse_result_t<matrix_expression_p> get_matrix_product(std::string const& code, size_t& dot) {
        // matrix product is right recursive and can backtrack, cache it
        auto pcache(packrat_caches::get< matrix_expression_p>(packrat_t::matrix_product));
        if (pcache->has(dot))
            return pcache->update_dot_and_get(dot);

        size_t local_dot(dot);
        if (done(code, local_dot)) return cached_not_found(dot, pcache);

        auto product = get_matrix_number_term(code, local_dot);
        if (!product) return cached_not_found(dot, pcache);

        if (!done(code, local_dot)) {
            char op = code[local_dot];
            auto next_term = [&]() {
                if (op == '*') {
                    skip_past_token(code, local_dot);
                    return get_matrix_number_term(code, local_dot);
                }
                return cached_not_found(dot, pcache);
                };

            parse_result_t<matrix_expression_p> next = next_term();
            while (next) {
                product = found(matrix_expression_p(new matrix_binary_op(product.value(), next.value(), op)));
                op = code[local_dot];
                next = next_term();
            }
        }
        return complete_cached(code, dot, local_dot, pcache, product);
    }
    parse_result_t<matrix_expression_p> get_matrix_expression(std::string const& code, size_t& dot) {
        // matrix product is right recursive and can backtrack, cache it
        auto pcache(packrat_caches::get< matrix_expression_p>(packrat_t::matrix_expression));
        if (pcache->has(dot))
            return pcache->update_dot_and_get(dot);

        size_t local_dot(dot);
        if (done(code, local_dot)) return cached_not_found(dot, pcache);
        auto arr = get_constant_array(code, local_dot);
        if (arr) {
            auto expr = std::dynamic_pointer_cast<matrix_expression>(constant_matrix::create(arr.value()));
            return complete_cached(code, dot, local_dot, pcache, expr);
        }
        auto expr = get_matrix_product(code, local_dot);
        if (!expr) return cached_not_found(dot, pcache);

        if (!done(code, local_dot)) {
            char op = code[local_dot];
            auto next_term = [&]() {
                if (op == '+' || op == '-') {
                    skip_past_token(code, local_dot);
                    return get_matrix_product(code, local_dot);
                }
                return cached_not_found(dot, pcache);
                };

            parse_result_t<matrix_expression_p> next = next_term();
            while (next) {
                expr = found(matrix_expression_p(new matrix_binary_op(expr.value(), next.value(), op)));
                op = code[local_dot];
                next = next_term();
            }
        }
        return complete_cached(code, dot, local_dot, pcache, expr);
    }
    std::shared_ptr<expression> parse_expression(std::string const& code, size_t& dot) {
        auto cached = packrat_caches::get_root(code);
        if (cached)
            return cached;

        packrat_caches::clear();
        skip_whitespace(code, dot);
        auto m = get_matrix_expression(code, dot);
        if (m) {
            if (!done(code, dot))
                throw std::runtime_error(error_with_position("trailing code after end of expression", dot));
            return packrat_caches::add_root(code, m.value());
        }
        auto n = get_number_expression(code, dot);
        if (n) {
            if (!done(code, dot))
                throw std::runtime_error(error_with_position("trailing code after end of expression", dot));
            return packrat_caches::add_root(code, n.value());
        }
        throw std::runtime_error(error_with_position("no parseable expressions found", 0));
        return nullptr;
    }


    parse_result_t<std::shared_ptr<assignment>> parse_assignment(std::string const& code, size_t& dot) {
        size_t local_dot(dot);
        if (done(code, local_dot)) return not_found();

        skip_whitespace(code, local_dot);
        auto const name_r = get_name(code, local_dot);
        if (!name_r)
            return not_found();
        if (code[local_dot] != '=')
            return not_found();
        skip_past_token(code, local_dot);
        auto m = get_matrix_expression(code, local_dot);
        if (!m)
            throw std::runtime_error(error_with_position("expected a matrix expression", local_dot));
        if (code[local_dot++] != ';')
            throw std::runtime_error(error_with_position("assignments must end in ';'", local_dot));

        std::shared_ptr<assignment> ret(new assignment(name_r.value(), m.value()));
        complete(code, dot, local_dot);
        return found(ret);
    }
}
struct kml_output {
    constant_matrix_p _matrix;
    double _number;
    bool is_matrix() const { return _matrix != nullptr; }
    bool is_number() const { return !is_matrix(); }
};
using kml_output_p = std::shared_ptr<kml_output>;

std::ostream& operator<<(std::ostream& out, kml_output const& o) {
    if (o.is_matrix())
        out << "\n" << o._matrix->display();
    else
        out << o._number;
    return out;
}
std::ostream& operator<<(std::ostream& out, kml_output_p const& o) {
    out << *o;
    return out;
}


class kml_program {
    std::vector <std::shared_ptr<assignment>> _assignments;
    std::shared_ptr<expression> _expression;
    std::string display_assignments(size_t tabs = 0) const {
        std::stringstream buff;
        std::string indent(tabs, '\t');
        for (auto const& a : _assignments)
            buff << a->description(tabs) << "\n" << indent;
        return buff.str();
    }
    std::string display_assignments_with_brackets() const {
        std::stringstream buff;
        for (auto const& a : _assignments)
            buff << a->display_with_brackets();
        return buff.str();
    }
public:
    kml_program(std::string const& code) {
        parse(code);
    }
    void parse(std::string const& code) {
        using namespace parsing;

        _assignments.clear();
        _expression.reset();
        size_t dot(0);
        skip_whitespace(code, dot);
        auto a = parse_assignment(code, dot);
        while (a) {
            _assignments.push_back(a.value());
            a = parse_assignment(code, dot);
        }
        for (auto const& a : _assignments)
            a->add_to_cache();
        if (done(code, dot))
            return;
        _expression = parse_expression(code, dot);
        for (auto const& a : _assignments)
            a->restore_cache();
    }
    kml_output_p execute() const {
        kml_output_p output;
        for (auto const& a : _assignments) 
            a->add_to_cache();
        if (_expression) {
            if(_expression->type() == expression_type::matrix) 
                output = kml_output_p(new kml_output{ _expression->evaluate_matrix(), 0 });
            else 
                output = kml_output_p(new kml_output{ nullptr, _expression->number_value() });
        }
        for (auto const& a : _assignments)
            a->restore_cache();
        return output;
    }
    std::string display() const {
        for (auto const& a : _assignments)
            a->add_to_cache();
        std::stringstream buff;
        buff << display_assignments();
        if (_expression)
            buff << _expression->display();
        for (auto const& a : _assignments)
            a->restore_cache();
        return buff.str();
    }
    std::string display_with_brackets() const {
        for (auto const& a : _assignments)
            a->add_to_cache();
        std::stringstream buff;
        buff << display_assignments_with_brackets();
        if (_expression)
            buff << _expression->display_with_brackets();
        for (auto const& a : _assignments)
            a->restore_cache();
        return buff.str();
    }
    std::string json_string() const {
        using namespace json_util;
        for (auto const& a : _assignments)
            a->add_to_cache();
        std::stringstream buff;
        size_t tabs(0);
        std::stringstream ass_text;
        buff << start_expression_object("program", display(), tabs) << next(tabs) <<
            label("assignments", tabs);
        if (_assignments.empty())
            buff << "null" << next(tabs);
        else {
            buff << start_expression_object("assignments", display_assignments(tabs), tabs);
            if (!_assignments.empty())
                buff << next(tabs) << label("assignments", tabs) << start_array(tabs);
            for (size_t i = 0; i < _assignments.size(); ++i) {
                _assignments[i]->json_stream(buff, tabs);
                if (i != _assignments.size() - 1)
                    buff << next(tabs);
            }
            if (!_assignments.empty())
                buff << end_array(tabs) << next(tabs);
            buff << end_expression_object(tabs) << next(tabs);
        }
        
        buff << label("expression", tabs);
        if(!_expression)
            buff << "null" << next(tabs);
        else {
            buff << start_expression_object("expression", _expression ? _expression->display() : "", tabs);
            if (_expression) {
                buff << next(tabs) << label("expression", tabs);
                _expression->json_stream(buff, tabs);
            }
            buff << end_expression_object(tabs);
        }
        buff << end_expression_object(tabs);
        for (auto const& a : _assignments)
            a->restore_cache();
        return buff.str();
    }

};

*/

int main()
{
    matrix_cache::add("m1", "identity(3,3)");
    std::cout << "m1\n" << matrix_cache::display_matrix("m1") << std::endl;
    matrix_cache::add("m2", "random(3,3)");
    std::cout << "m2\n" << matrix_cache::display_matrix("m2") << std::endl;
    matrix_cache::add("m3", "constant(3,3,0.109)");
    std::cout << "m3\n" << matrix_cache::display_matrix("m3") << std::endl;
    matrix_cache::add("m10", "[[1,2,3],[4,5,6],[7,8,9]]");
    std::cout << "m10\n" << matrix_cache::display_matrix("m3") << std::endl;


    for (auto const& m : matrix_cache::list())
        std::cout << m << std::endl;

    for (auto const& f : functions::function_table::help())
        std::cout << f.first << " : " << f.second << std::endl;

    std::vector<std::string> examples = {
        "5-1-2",
        "2(4-2)",
        "1.234-3.4*0.1",
        "2*(4-2)",
        "1*2*3*4",
        "1*2/3*4",
        "1*(2/3)*4",
        "(1*2)/(3*4)",
        "(1*2/3)*4",
        "(1+2)*(3-4)",
        "1+2+(3-4)",
        "1+2+3-4",
        "1+2+3-4*2",
        "(1+2)+(3-4)",
        "(1+2-3*4)",
        "-1",
        "-(1+2)",
        "4/2",
        "(2+2)/3",
        "1.23435",
        "2 1.23435",
        "m1=identity(3,3);|m1|",
        "m2",
        "3m1",
        "m2^T",
        "m2^-1",
        "m2^T^T^-1",
        "m2=random(3,3);m4=constant(3,3,0.901);-m4",
        "-m2^T",
        "-m2^T^T^-1",
        "+m2^T^T^-1",
        "m1+m2",
        "m1+m2-m3*m1",
        "m1+m2-m3+m1-m2",
        "(m1+m2)^-1",
        "(m1+m2)^-1/2",
        "m3/(1+5)",
        "m1*m2+m3",
        "m1*m2+(m3-m1)",
        "identity(3,3)",
        "4identity(3,3)",
        "4*identity(3,3)",
        "m1*identity(3,3)",
        "abs(-4)",
        "abs(-4) * identity(3,3)",
        "abs(-4) random(3,4)",
        "trace(m1)",
        "m5=[[1,2,3],[4,5,6],[7,8,9]];m5"

    };
    json_configuration::include_result(true);
    for (auto const& ex : examples) {
        std::cout << "Testing " << ex << std::endl;
        try {
            kml_program p(ex);
            auto res = p.execute();


            std::cout << p.display_with_brackets() << " : " << p.display() << " = " << res << std::endl;
            std::cout << p.json_string() << std::endl;
        }
        catch (std::runtime_error const& e) {
            std::cout << "Error: " << e.what() << std::endl;
        }
    }
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
