#include "expression_tree.h"
#include "json_util.h"

// constant number
constant_number::constant_number(double value_) : _value(value_) {
}
double constant_number::number_value() const{
    return _value;
}
std::string constant_number::display() const{
    std::stringstream buff;
    buff << _value;
    return buff.str();
}
std::string constant_number::display_with_brackets() const{ 
    return display(); 
}
void constant_number::json_stream(std::ostream& out, size_t& tabs) const{
    using namespace json_util;
    out << start_expression_object("constant_number", display(), tabs);
    if (json_configuration::include_result())
        out << next(tabs) << result_value(number_value(), tabs);
    out << end_expression_object(tabs);
}
number_expression_p constant_number::create(double v) {
    return number_expression_p(new constant_number{ v });
}

// negative_number
negative_number::negative_number(std::shared_ptr<number_expression> number_) :_number(number_) {
}
double negative_number::number_value() const{
    return -_number->number_value();
}
std::string negative_number::display() const{
    std::stringstream buff;
    buff << "-" << _number->display();
    return buff.str();
}
std::string negative_number::display_with_brackets() const{
    std::stringstream buff;
    buff << "(" << "-" << _number->display_with_brackets() << ")";
    return buff.str();
}
void negative_number::json_stream(std::ostream& out, size_t& tabs) const{
    using namespace json_util;
    out << start_expression_object("negative_number", display(), tabs) << next(tabs)
        << labeled_value("number", _number, tabs);
    if (json_configuration::include_result())
        out << next(tabs) << result_value(number_value(), tabs);
    out << end_expression_object(tabs);
}


number_expression_p negative_number::create(std::shared_ptr<number_expression> number_) {
    return number_expression_p(new negative_number{ number_ });
}

// bracketed_number
bracketed_number::bracketed_number(std::shared_ptr<number_expression> number_) :_number(number_) {
}
double bracketed_number::number_value() const{
    return _number->number_value();
}

std::string bracketed_number::display() const{
    std::stringstream buff;
    buff << "(" << _number->display() << ")";
    return buff.str();
}
std::string bracketed_number::display_with_brackets() const{
    std::stringstream buff;
    buff << "(" << _number->display_with_brackets() << ")";
    return buff.str();
}

void bracketed_number::json_stream(std::ostream& out, size_t& tabs) const{
    using namespace json_util;
    out << start_expression_object("bracketed_number", display(), tabs) << next(tabs) <<
        labeled_value("number", _number, tabs);
    if (json_configuration::include_result())
        out << next(tabs) << result_value(number_value(), tabs);
    out << end_expression_object(tabs);
}
number_expression_p bracketed_number::create(std::shared_ptr<number_expression> number_) {
    return number_expression_p(new bracketed_number{ number_ });
}

// number_arithmetic

number_arithmetic::number_arithmetic(
    std::shared_ptr<number_expression> lhs_,
    std::shared_ptr<number_expression> rhs_,
    char op_) :_lhs(lhs_), _rhs(rhs_), _op(op_) {}

double number_arithmetic::number_value() const{
    switch (_op) {
    case '+': return _lhs->number_value() + _rhs->number_value();
    case '-': return _lhs->number_value() - _rhs->number_value();
    case '*': return _lhs->number_value() * _rhs->number_value();
    case '/': return _lhs->number_value() / _rhs->number_value();
    default: throw std::runtime_error("Unknown operand.");
    }

    return 0;// never happens, but compilers moan 
}

std::string number_arithmetic::display() const{
    std::stringstream buff;
    buff << _lhs->display() << _op << _rhs->display();
    return buff.str();
}
std::string number_arithmetic::display_with_brackets() const{
    std::stringstream buff;
    buff << "(" << _lhs->display_with_brackets() << _op << _rhs->display_with_brackets() << ")";
    return buff.str();
}

void number_arithmetic::json_stream(std::ostream& out, size_t& tabs) const{
    using namespace json_util;
    out << start_expression_object("number_arithmetic", display(), tabs) << next(tabs) <<
        labeled_value("lhs", _lhs, tabs) << next(tabs) <<
        labeled_value("rhs", _rhs, tabs) << next(tabs) <<
        labeled_value("op", _op, tabs);
    if (json_configuration::include_result())
        out << next(tabs) << result_value(number_value(), tabs);
    out << end_expression_object(tabs);
}
number_expression_p number_arithmetic::create(
    std::shared_ptr<number_expression> lhs_,
    std::shared_ptr<number_expression> rhs_,
    char op_) {
    return number_expression_p(new number_arithmetic{ lhs_, rhs_, op_ });
}

// determinant
determinant::determinant(matrix_expression_p matrix_) :_matrix(matrix_) {}
double determinant::number_value() const{
    return _matrix->matrix_value()->value().determinant();
}

std::string determinant::display() const{
    std::stringstream buff;
    buff << "|" << _matrix->display() << "|";
    return buff.str();
}
std::string determinant::display_with_brackets() const{
    std::stringstream buff;
    buff << "|" << _matrix->display_with_brackets() << "|";
    return buff.str();
}
void determinant::json_stream(std::ostream& out, size_t& tabs) const{
    using namespace json_util;
    out << start_expression_object("determinant", display(), tabs) << next(tabs) <<
        labeled_value("matrix", _matrix, tabs);
    if (json_configuration::include_result())
        out << next(tabs) << result_value(number_value(), tabs);
    out << end_expression_object(tabs);
}

number_expression_p determinant::create(matrix_expression_p matrix_) {
    return number_expression_p(new determinant{ matrix_ });
}

// constant matrix
using matrix_value_instance_t = Eigen::Map<const Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>>;

constant_matrix::constant_matrix(double const* const value_, size_t const rows, size_t const cols) :
        _value(matrix_value_instance_t(value_, rows, cols)) {}

constant_matrix::constant_matrix(matrix_value_t const& v) :_value(v) {}
std::string constant_matrix::display_tabbed(size_t tabs) const {
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

std::string constant_matrix::display() const{
    return display_tabbed();
}
std::string constant_matrix::display_with_brackets() const{ return display(); }

void constant_matrix::json_stream(std::ostream& out, size_t& tabs) const{
    using namespace json_util;
    out << start_expression_object("constant_matrix", std::string("\n") + display_tabbed(tabs + 1), tabs) << next(tabs)
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

evaluated_matrix_t const constant_matrix::matrix_value() const{ 
    throw std::runtime_error("Constant matrices cannot be evaluated"); 
}
matrix_expression_p constant_matrix::create(double const* const value_, size_t const rows, size_t const cols) {
    return matrix_expression_p(new constant_matrix{ value_, rows, cols });
}

matrix_expression_p constant_matrix::create(matrix_value_t const& v) {
    return matrix_expression_p(new constant_matrix{ v });
}
evaluated_matrix_t constant_matrix::create_value(matrix_value_t const& v) {
    return evaluated_matrix_t(new constant_matrix{ v });
}
matrix_expression_p constant_matrix::create(std::vector<std::vector<double>> const& v) {
    using matrix_t = Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;
    if (v.size() == 0)
        throw std::runtime_error("Cannot initialise a matrix with an empty array.");
    size_t cols = v.front().size();
    matrix_t mp(v.size(), v.front().size());
    for (size_t i = 0; i < v.size(); ++i) {
        if (v[i].size() != cols)
            throw std::runtime_error("Cannot initialise a matrix with an array that is not rectangular.");
        for (size_t j = 0; j < v.front().size(); ++j)
            mp(i, j) = v[i][j];
    }

    return matrix_expression_p(new constant_matrix{ mp });
}

// named_matrix
named_matrix::named_matrix(std::string const& name_) : _name(name_) {
}
evaluated_matrix_t const named_matrix::matrix_value() const{ 
    return get(); 
}
std::string named_matrix::display() const{
    std::stringstream buff;
    buff << _name;
    return buff.str();
}
std::string named_matrix::display_with_brackets() const{ 
    return display(); 
}
void named_matrix::json_stream(std::ostream& out, size_t& tabs) const{
    using namespace json_util;
    out << start_expression_object("named_matrix", _name, tabs) << next(tabs)
        << labeled_value("name", _name, tabs);
    if (json_configuration::include_result())
        out << next(tabs) << result_value(matrix_value(), tabs);
    out << end_expression_object(tabs);
}
matrix_expression_p named_matrix::create(std::string const& name_) {
    return matrix_expression_p(new named_matrix{ name_ });
}

//bracketed_matrix
bracketed_matrix::bracketed_matrix(matrix_expression_p matrix_) :_matrix(matrix_) {
}
evaluated_matrix_t const bracketed_matrix::matrix_value() const{
    return _matrix->matrix_value();
}
std::string bracketed_matrix::display() const{
    std::stringstream buff;
    buff << "(" << _matrix->display() << ")";
    return buff.str();
}
std::string bracketed_matrix::display_with_brackets() const{
    std::stringstream buff;
    buff << "(" << _matrix->display_with_brackets() << ")";
    return buff.str();
}
void bracketed_matrix::json_stream(std::ostream& out, size_t& tabs) const{
    using namespace json_util;
    out << start_expression_object("bracketed_matrix", display(), tabs) << next(tabs) <<
        labeled_value("matrix", _matrix, tabs);
    if (json_configuration::include_result())
        out << next(tabs) << result_value(matrix_value(), tabs);
    out << end_expression_object(tabs);
}
matrix_expression_p bracketed_matrix::create(matrix_expression_p matrix_) {
    return matrix_expression_p(new bracketed_matrix{ matrix_ });
}

// matrix_unary_op
namespace {
    inline std::string to_string(unary_matrix_op op) {
        switch (op) {
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
}
matrix_unary_op::matrix_unary_op(matrix_expression_p matrix_, unary_matrix_op op_) :
    _matrix(matrix_), _op(op_) {
}
std::string matrix_unary_op::display() const{
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
std::string matrix_unary_op::display_with_brackets() const{
    std::stringstream buff;
    {
        switch (_op) {
        case unary_matrix_op::inverse:
            buff << "(" << _matrix->display_with_brackets() << to_string(_op) << ")";
            break;
        case unary_matrix_op::transpose:
            buff << "(" << _matrix->display_with_brackets() << to_string(_op) << ")";
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
void matrix_unary_op::json_stream(std::ostream& out, size_t& tabs) const{
    using namespace json_util;
    out << start_expression_object("matrix_unary_op", display(), tabs) << next(tabs) <<
        labeled_value("matrix", _matrix, tabs) << next(tabs) <<
        labeled_value("op", to_string(_op), tabs);
    if (json_configuration::include_result())
        out << next(tabs) << result_value(matrix_value(), tabs);
    out << end_expression_object(tabs);
}
evaluated_matrix_t const matrix_unary_op::matrix_value() const{
    switch (_op) {
    case unary_matrix_op::inverse:
        return constant_matrix::create_value(_matrix->matrix_value()->value().inverse());
    case unary_matrix_op::transpose:
        return constant_matrix::create_value(_matrix->matrix_value()->value().transpose());
        break;
    case unary_matrix_op::negative:
        return constant_matrix::create_value(-_matrix->matrix_value()->value());
    default:
        throw std::runtime_error("Unknown matrix unary operation.");
    }
    return nullptr;
}
matrix_expression_p matrix_unary_op::create(matrix_expression_p matrix_, unary_matrix_op op_) {
    return matrix_expression_p(new matrix_unary_op{ matrix_, op_ });
}

// matrix number operations
matrix_number_op::matrix_number_op(
    matrix_expression_p matrix_,
    std::shared_ptr<number_expression> number_,
    char op_) : _matrix(matrix_), _number(number_), _op(op_) {
}
evaluated_matrix_t const matrix_number_op::matrix_value() const{
    switch (_op) {
    case '*':
        return constant_matrix::create_value(_number->number_value() * _matrix->matrix_value()->value());
    case '/':
        return constant_matrix::create_value(_matrix->matrix_value()->value() / _number->number_value());
    default:
        throw std::runtime_error("Unknown matrix unary operation.");
    }
    return nullptr;
}

std::string matrix_number_op::display() const{
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
std::string matrix_number_op::display_with_brackets() const{
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
void matrix_number_op::json_stream(std::ostream& out, size_t& tabs) const{
    using namespace json_util;
    out << start_expression_object("matrix_number_op", display(), tabs) << next(tabs) <<
        labeled_value("matrix", _matrix, tabs) << next(tabs) <<
        labeled_value("number", _number, tabs) << next(tabs) <<
        labeled_value("op", _op, tabs);
    if (json_configuration::include_result())
        out << next(tabs) << result_value(matrix_value(), tabs);
    out << end_expression_object(tabs);
}
matrix_expression_p matrix_number_op::create(
        matrix_expression_p matrix_,
        std::shared_ptr<number_expression> number_,
        char op_) {
    return matrix_expression_p(new matrix_number_op{ matrix_, number_, op_ });
}
// matrix_binary_op
matrix_binary_op::matrix_binary_op(
    matrix_expression_p lhs_,
    matrix_expression_p rhs_,
    char op_) : _lhs(lhs_), _rhs(rhs_), _op(op_) {
}

evaluated_matrix_t const matrix_binary_op::matrix_value() const {
    switch (_op) {
    case '*':
        return constant_matrix::create_value(_lhs->matrix_value()->value() * _rhs->matrix_value()->value());
    case '+':
        return constant_matrix::create_value(_lhs->matrix_value()->value() + _rhs->matrix_value()->value());
    case '-':
        return constant_matrix::create_value(_lhs->matrix_value()->value() - _rhs->matrix_value()->value());
    default:
        throw std::runtime_error("Unknown matrix unary operation.");
    }
    return nullptr;
}
void matrix_binary_op::json_stream(std::ostream& out, size_t& tabs) const{
    using namespace json_util;
    out << start_expression_object("matrix_binary_op", display(), tabs) << next(tabs) <<
        labeled_value("lhs", _lhs, tabs) << next(tabs) <<
        labeled_value("rhs", _rhs, tabs) << next(tabs) <<
        labeled_value("op", _op, tabs);
    if (json_configuration::include_result())
        out << next(tabs) << result_value(matrix_value(), tabs);
    out << end_expression_object(tabs);
}
std::string matrix_binary_op::display() const{
    std::stringstream buff;
    buff << _lhs->display() << _op << _rhs->display();
    return buff.str();
}
std::string matrix_binary_op::display_with_brackets() const{
    std::stringstream buff;
    buff << "(" << _lhs->display_with_brackets() << _op << _rhs->display_with_brackets() << ")";
    return buff.str();
}
matrix_expression_p matrix_binary_op::create(
    matrix_expression_p lhs_,
    matrix_expression_p rhs_,
    char op_) {
    return matrix_expression_p(new matrix_binary_op{ lhs_, rhs_, op_ });
}

// assignment
assignment::assignment(std::string const name_, matrix_expression_p value_) : 
    _name(name_), 
    _value(value_) {}
expression_type assignment::type() const{ 
    return expression_type::assignment; 
}
double assignment::number_value() const{ 
    throw std::runtime_error("Attempted to evaluate assignment as a number."); 
    return 0; 
}
evaluated_matrix_t const assignment::matrix_value() const {
    throw std::runtime_error("Attempted to evaluate assignment as a matrix."); 
    return nullptr; 
}

std::string assignment::display() const{
    std::stringstream buff;
    buff << _name << " = ";
    auto const_matrix = std::dynamic_pointer_cast<constant_matrix>(_value);
    if (const_matrix)
        buff << "\n";

    buff << _value->display() << ";";
    return buff.str();
}
std::string assignment::display_with_brackets() const{ 
    return display(); 
}
std::string assignment::description(size_t tabs) const {
    auto const_matrix = std::dynamic_pointer_cast<constant_matrix>(_value);
    if (const_matrix) {
        std::stringstream buff;
        buff << _name << " = \n" << const_matrix->display_tabbed(tabs + 1) << std::string(tabs, '\t') << ";";
        return buff.str();
    }
    return display();
}
void assignment::json_stream(std::ostream& out, size_t& tabs) const{
    using namespace json_util;
    out << start_expression_object("assignment", description(tabs), tabs) << next(tabs) <<
        labeled_value("name", _name, tabs) << next(tabs) <<
        labeled_value("value", _value, tabs) << next(tabs);
    out << end_expression_object(tabs);
}

void assignment::add_to_cache() {
    _old_value = matrix_cache::find(_name);
    if (_old_value)
        matrix_cache::remove(_name);
    auto const_matrix = std::dynamic_pointer_cast<constant_matrix>(_value);
    if (const_matrix)
        matrix_cache::add(_name, const_matrix);
    else
        matrix_cache::add(_name, _value->matrix_value());
}
void assignment::restore_cache() const {
    matrix_cache::remove(_name);
    if (_old_value)
        matrix_cache::add(_name, _old_value);

}
std::shared_ptr<assignment> assignment::create(
    std::string const name_, 
    matrix_expression_p value_) {
    return std::shared_ptr<assignment>(new assignment{ name_, value_ });
}


namespace function_inv {
    std::string display(
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
    ) {
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

// matrix_function_inv
matrix_function_inv::matrix_function_inv(
    std::shared_ptr<functions::matrix_function> function_,
    std::vector<functions::argument_t> const& arguments_) : 
    _function(function_), _arguments(arguments_) {
}
evaluated_matrix_t const matrix_function_inv::matrix_value() const {
    return _function->evaluate(_arguments); 
}
std::string matrix_function_inv::display() const{
    return function_inv::display(_function->name(), _arguments);
}
std::string matrix_function_inv::display_with_brackets() const{
    return function_inv::display_with_brackets(_function->name(), _arguments);
}
void matrix_function_inv::json_stream(std::ostream& out, size_t& tabs) const{
    using namespace json_util;
    out << start_expression_object("matrix_function_inv", display(), tabs) << next(tabs);
    function_inv::json_stream(out, tabs, _function->name(), _arguments);
    if (json_configuration::include_result())
        out << next(tabs) << result_value(matrix_value(), tabs);
    out << end_expression_object(tabs);
}
matrix_expression_p matrix_function_inv::create(
    std::shared_ptr<functions::matrix_function> function_,
    std::vector<functions::argument_t> const& arguments_) {
    return matrix_expression_p(new matrix_function_inv{ function_, arguments_ });
}

// number_function_inv
number_function_inv::number_function_inv(
    std::shared_ptr<functions::number_function> function_,
    std::vector<functions::argument_t> const& arguments_) : _function(function_), _arguments(arguments_) {
}
double number_function_inv::number_value() const{ 
    return _function->evaluate(_arguments)->number_value(); 
}
std::string number_function_inv::display() const{
    return function_inv::display(_function->name(), _arguments);
}
std::string number_function_inv::display_with_brackets() const{
    return function_inv::display_with_brackets(_function->name(), _arguments);
}
void number_function_inv::json_stream(std::ostream& out, size_t& tabs) const{
    using namespace json_util;
    out << start_expression_object("number_function_inv", display(), tabs) << next(tabs);
    function_inv::json_stream(out, tabs, _function->name(), _arguments);
    if (json_configuration::include_result())
        out << next(tabs) << result_value(number_value(), tabs);
    out << end_expression_object(tabs);
}
number_expression_p number_function_inv::create(
    std::shared_ptr<functions::number_function> function_,
    std::vector<functions::argument_t> const& arguments_) {
    return number_expression_p(new number_function_inv{ function_, arguments_ });
}
