#pragma once
#include <iostream>
#include <iomanip>
#include <string>
#include <memory>
#include <vector>
#include "matrix_cache.h"
#include "functions.h"
#include "Eigen/Dense"

using Eigen::MatrixXd;
using Eigen::VectorXd;

// supported types 
// assignment is a specific type, it is basically void type in most languages
// we on purpose want to avoid having to support chained assignments, or using assignment values in expressions
// purely because we don't like that sort of thing
enum class expression_type {
    number,
    matrix,
    assignment
};

// the evaluated value types map to concrete types
using number_value_t = double;
using matrix_value_t = MatrixXd;

// in our expression tree, a matrix value is represented by a constant matrix expression
// having a separate implementation for it helps us optimise by caching matrices and 
// making sure that the moving and copying semantics work like we want them to
class constant_matrix;
using evaluated_matrix_t = std::shared_ptr<constant_matrix>;

// base expression type
struct expression {
    // expression interface
    // expressions act as a union
    virtual expression_type type() const = 0;
    virtual double number_value() const = 0;
    virtual evaluated_matrix_t const matrix_value() const = 0;

    virtual ~expression() {}


    // serialization functions
    virtual std::string display() const = 0;
    virtual std::string display_with_brackets() const = 0;
    virtual void json_stream(std::ostream& out, size_t& tabs) const = 0;

    inline std::string json_string() const {
        std::stringstream buff;
        size_t tabs = 0;
        json_stream(buff, tabs);
        return buff.str();
    }
};
using expression_p = std::shared_ptr<expression>;

// base class for number expressions
struct number_expression : public expression {
    expression_type type() const override { return expression_type::number; }
    evaluated_matrix_t const matrix_value() const override { throw std::runtime_error("Attempted to evaluate number as a matrix."); return nullptr; }
};
using number_expression_p = std::shared_ptr<number_expression>;

// base for matrix expressions
struct matrix_expression : public expression {
    expression_type type() const override { return expression_type::matrix; }
    double number_value() const override { throw std::runtime_error("Attempted to evaluate matrix as a number."); return 0; }
};
using matrix_expression_p = std::shared_ptr<matrix_expression>;

// number expressions
//
// number constants
class constant_number : public number_expression {
    double _value;
public:
    constant_number(double value_);
    double number_value() const override;

    std::string display() const override;
    std::string display_with_brackets() const override;
    void json_stream(std::ostream& out, size_t& tabs) const override;

    static number_expression_p create(double v);
};

// negation of numbers
class negative_number : public number_expression {
    std::shared_ptr<number_expression> _number;
public:
    negative_number(std::shared_ptr<number_expression> number_);
    double number_value() const override;

    std::string display() const override;
    std::string display_with_brackets() const override;
    void json_stream(std::ostream& out, size_t& tabs) const override;

    static number_expression_p create(std::shared_ptr<number_expression> number_);
};

// number expressions in brackets
class bracketed_number : public number_expression {
    std::shared_ptr<number_expression> _number;
public:
    bracketed_number(std::shared_ptr<number_expression> number_);
    double number_value() const override;

    std::string display() const override;
    std::string display_with_brackets() const override;
    void json_stream(std::ostream& out, size_t& tabs) const override;

    static number_expression_p create(std::shared_ptr<number_expression> number_);
};

// all number arithmetic is implemented in a single class
// the structure of the parse tree deals with precedence
class number_arithmetic : public number_expression {
    std::shared_ptr<number_expression> _lhs;
    std::shared_ptr<number_expression> _rhs;
    char _op; // all operators are single characters, might as well use that 
public:
    number_arithmetic(
        std::shared_ptr<number_expression> lhs_,
        std::shared_ptr<number_expression> rhs_,
        char op_);
    double number_value() const override;

    std::string display() const override;
    std::string display_with_brackets() const override;
    void json_stream(std::ostream& out, size_t& tabs) const override;

    static number_expression_p create(
        std::shared_ptr<number_expression> lhs_,
        std::shared_ptr<number_expression> rhs_,
        char op_);
};

// determinant 
class determinant : public number_expression {
    matrix_expression_p _matrix;
public:
    determinant(matrix_expression_p matrix_);
    double number_value() const override;

    std::string display() const override;
    std::string display_with_brackets() const override;
    void json_stream(std::ostream& out, size_t& tabs) const override;

    static number_expression_p create(matrix_expression_p matrix_);
};

// matrices 
//
// constant matrix is a matrix value expression
class constant_matrix : public matrix_expression {
    matrix_value_t const _value;
public:
    constant_matrix(double const* const value_, size_t const rows, size_t const cols);
    constant_matrix(matrix_value_t const& v);
    evaluated_matrix_t const matrix_value() const override; // constant matrices cannot be further evaluated
                                                                             // invoking evaluation on them throws
    matrix_value_t const value() const { return _value; }
    
    std::string display_tabbed(size_t tabs = 0) const;
    std::string display() const override;
    std::string display_with_brackets() const override;
    void json_stream(std::ostream& out, size_t& tabs) const override;

    static matrix_expression_p create(double const* const value_, size_t const rows, size_t const cols);
    static matrix_expression_p create(matrix_value_t const& v);
    static evaluated_matrix_t create_value(matrix_value_t const& v);
    static matrix_expression_p create(std::vector<std::vector<double>> const& v);

};
inline std::ostream& operator<<(std::ostream& out, constant_matrix const& cmp) {
    out << cmp.display();
    return out;
}
inline std::ostream& operator<<(std::ostream& out, constant_matrix_p const& cmp) {
    out << cmp->display();
    return out;
}

// matrices can be named, we cache them
class named_matrix : public matrix_expression {
    std::string const _name;
    inline constant_matrix_p const get() const {
        return get_matrix_from_cache(_name);
    }
public:
    named_matrix(std::string const& name_);
    evaluated_matrix_t const matrix_value() const override;
    
    std::string display() const override;
    std::string display_with_brackets() const override;
    void json_stream(std::ostream& out, size_t& tabs) const override;
    
    static matrix_expression_p create(std::string const& name_);
};

// bracketed matrix expression
class bracketed_matrix : public matrix_expression {
    matrix_expression_p _matrix;
public:
    bracketed_matrix(matrix_expression_p matrix_);
    evaluated_matrix_t const matrix_value() const override;

    std::string display() const override;
    std::string display_with_brackets() const override;
    void json_stream(std::ostream& out, size_t& tabs) const override;
    
    static matrix_expression_p create(matrix_expression_p matrix_);
};


// unary matrix operations are all in one class
enum class unary_matrix_op {
    transpose,
    inverse,
    negative
};
class matrix_unary_op : public matrix_expression {
    matrix_expression_p _matrix;
    unary_matrix_op _op;

public:
    matrix_unary_op(matrix_expression_p matrix_, unary_matrix_op op_);
    evaluated_matrix_t const matrix_value() const override;
    
    std::string display() const override;
    std::string display_with_brackets() const override;
    void json_stream(std::ostream& out, size_t& tabs) const override;
    static matrix_expression_p create(matrix_expression_p matrix_, unary_matrix_op op_);
};

// matrix number operations
class matrix_number_op : public matrix_expression {
    matrix_expression_p _matrix;
    number_expression_p _number;
    char _op; // * or /
public:
    matrix_number_op(
        matrix_expression_p matrix_,
        std::shared_ptr<number_expression> number_,
        char op_);
    evaluated_matrix_t const matrix_value() const override;

    std::string display() const override;
    std::string display_with_brackets() const override;
    void json_stream(std::ostream& out, size_t& tabs) const override;

    static matrix_expression_p create(
        matrix_expression_p matrix_,
        number_expression_p number_,
        char op_);
};

// all matrix binary operations are implemented in one class
// the parse tree takes care of presedence
class matrix_binary_op : public matrix_expression {
    matrix_expression_p _lhs;
    matrix_expression_p _rhs;
    char _op; // *, + or -
public:
    matrix_binary_op(
        matrix_expression_p lhs_,
        matrix_expression_p rhs_,
        char op_);
    evaluated_matrix_t const matrix_value() const override;
    
    void json_stream(std::ostream& out, size_t& tabs) const override;
    std::string display() const override;
    std::string display_with_brackets() const override;
    
    static matrix_expression_p create(
        matrix_expression_p lhs_,
        matrix_expression_p rhs_,
        char op_);
};

// matrix assignment (we cache these)
class assignment : public expression {
    std::string const _name;
    matrix_expression_p _value{ nullptr };
    constant_matrix_p _old_value{ nullptr };
public:
    assignment(std::string const name_, matrix_expression_p value_);
    expression_type type() const override;
    // assignments cannot be evaluated, both functions below throw
    double number_value() const override;
    evaluated_matrix_t const matrix_value() const override;

    std::string display() const override;
    std::string display_with_brackets() const override;
    std::string description(size_t tabs) const;
    void json_stream(std::ostream& out, size_t& tabs) const override;

    void add_to_cache();
    void restore_cache() const;
    
    static std::shared_ptr<assignment> create(
        std::string const name_,
        matrix_expression_p value_);

};
using assignment_p = std::shared_ptr<assignment>;

// invocation of matrix functions
class matrix_function_inv : public matrix_expression {
public:
    std::shared_ptr<functions::matrix_function> _function;
    std::vector<functions::argument_t> _arguments;
public:
    matrix_function_inv(
        std::shared_ptr<functions::matrix_function> function_,
        std::vector<functions::argument_t> const& arguments_);
    evaluated_matrix_t const matrix_value() const override;

    std::string display() const override;
    std::string display_with_brackets() const override;
    void json_stream(std::ostream& out, size_t& tabs) const override;

    static matrix_expression_p create(
        std::shared_ptr<functions::matrix_function> function_,
        std::vector<functions::argument_t> const& arguments_);
};

// invocation of number functions
class number_function_inv : public number_expression {
public:
    std::shared_ptr<functions::number_function> _function;
    std::vector<functions::argument_t> _arguments;
public:
    number_function_inv(
        std::shared_ptr<functions::number_function> function_,
        std::vector<functions::argument_t> const& arguments_);
    
    double number_value() const override;

    std::string display() const override;
    std::string display_with_brackets() const override;

    void json_stream(std::ostream& out, size_t& tabs) const override;
    
    static number_expression_p create(
        std::shared_ptr<functions::number_function> function_,
        std::vector<functions::argument_t> const& arguments_);
};