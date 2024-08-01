#include "functions.h"
#include "string_util.h"
#include "expression_tree.h"

using namespace string_util;

namespace functions {

    std::string to_string(return_type_t t) {
        switch (t) {
        case return_type_t::matrix: return "matrix";
        case return_type_t::number: return "number";
        default:
            ;
        }
        return "unknown";
    }
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

    
    // arguments
    void argument_t::check_type(argument_type_t const t) const { 
        if (t != _type) 
            throw std::runtime_error("Invalid data type cast for argument."); 
    }
    argument_t::argument_t(long size_) : 
        _type(argument_type_t::size), _size(size_) {  }
    argument_t::argument_t(number_expression_p number_) : 
        _type(argument_type_t::number), _number(number_) { }
    argument_t::argument_t(matrix_expression_p matrix_) : 
        _type(argument_type_t::matrix), _matrix(matrix_) { }

    // functions
    void function::validate_argument_types(std::vector<argument_t> const& arguments) const {
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
    function::function(
            std::string name_,
            std::string help_,
            return_type_t return_type_,
            std::vector<argument_type_t> argument_types_
        ) : 
        _name(name_), 
        _help(help_), 
        _return_type(return_type_), 
        _argument_types(argument_types_) {
    }

    function::~function() {
    }
    std::string function::signature() const {
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

    std::string matrix_function::make_name(std::string const& name, int rows, int columns) const {
        std::stringstream buff;
        buff << "__" << name << "_" << rows << "_" << columns;
        return buff.str();
    }

    // function_table
    std::map<std::string, std::shared_ptr<function>>& function_table::table() {
        static std::map<std::string, std::shared_ptr<function>> _inst;
        return _inst;
    }
    bool function_table::add(std::shared_ptr<function> const& function_) {
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
    std::shared_ptr<function> function_table::get(std::string const& name) {
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
    std::shared_ptr<function> function_table::find(std::string const& name) {
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
    bool function_table::check_exists_nc(std::string const& name) {
        std::string l_name(to_lower(name));
        for (auto const& e : table()) {
            if (to_lower(e.first) == l_name)
                return true;
        }
        return false;
    }
    std::map<std::string, std::string> function_table::help() {
        std::map<std::string, std::string> ret;
        for (auto const& e : table()) {
            ret[e.second->signature()] = e.second->help();
        }
        return ret;
    }

#define REGISTER_FUNCTION( NAME ) static auto __register_##NAME = function_table::add(std::shared_ptr<function>(new NAME()));

    // function implementations
    struct abs : public number_function {
        abs() : number_function("abs", "Absolute value of a number.", { argument_type_t::number }) {}
        number_expression_p evaluate(std::vector<argument_t> const& arguments) const override {
            validate_argument_types(arguments);
            return number_expression_p(new constant_number(fabs(arguments[0].number()->number_value())));
        }
    };
    REGISTER_FUNCTION(abs);

    struct trace : public number_function {
        trace() : number_function("trace", "Computes trace of a matrix.", { argument_type_t::matrix }) {}
        number_expression_p evaluate(std::vector<argument_t> const& arguments) const override {
            validate_argument_types(arguments);
            auto const& matrix = arguments[0].matrix()->matrix_value();
            double v = matrix->value().trace();
            return number_expression_p(new constant_number(v));
        }
    };
    REGISTER_FUNCTION(trace);

    class identity : public matrix_function {

    public:
        identity() : matrix_function("identity", "Creates a diagonal matrix of a given size.", { argument_type_t::size, argument_type_t::size }) {
        }
        matrix_result_p evaluate(std::vector<argument_t> const& arguments) const override {
            validate_argument_types(arguments);
            auto const& rows = arguments[0].size();
            auto const& cols = arguments[1].size();
            auto m = constant_matrix::create_value(Eigen::MatrixXd::Identity(rows, cols));
            return m;
        }
    };
    REGISTER_FUNCTION(identity);

    class zero : public matrix_function {
    public:
        zero() : matrix_function("zero", "Creates a zero matrix of a given size.", { argument_type_t::size, argument_type_t::size }) {
        }
        matrix_result_p evaluate(std::vector<argument_t> const& arguments) const override {
            validate_argument_types(arguments);
            auto const& rows = arguments[0].size();
            auto const& cols = arguments[1].size();
            auto m = constant_matrix::create_value(Eigen::MatrixXd::Zero(rows, cols));
            return m;
        }
    };
    REGISTER_FUNCTION(zero);

    class random : public matrix_function {
    public:
        random() : matrix_function("random", "Creates a random matrix of a given size.", { argument_type_t::size, argument_type_t::size }) {
        }
        matrix_result_p evaluate(std::vector<argument_t> const& arguments) const override {
            validate_argument_types(arguments);
            auto const& rows = arguments[0].size();
            auto const& cols = arguments[1].size();
            auto m = constant_matrix::create_value(Eigen::MatrixXd::Random(rows, cols));
            return m;
        }
    };
    REGISTER_FUNCTION(random);

    class constant : public matrix_function {
    public:
        constant() : matrix_function("constant", "Creates a constant matrix of a given size.", {
            argument_type_t::size, argument_type_t::size, argument_type_t::number }) {
        }
        matrix_result_p evaluate(std::vector<argument_t> const& arguments) const override {
            validate_argument_types(arguments);
            auto const& rows = arguments[0].size();
            auto const& cols = arguments[1].size();
            auto const& v = arguments[2].number()->number_value();
            auto m = constant_matrix::create_value(Eigen::MatrixXd::Constant(rows, cols, v));
            return m;
        }
    };
    REGISTER_FUNCTION(constant);

    class dot : public number_function {
    public:
        dot() : number_function("dot", "Dot product.", { argument_type_t::matrix, argument_type_t::matrix }) {
        }
        number_expression_p evaluate(std::vector<argument_t> const& arguments) const override {
            validate_argument_types(arguments);
            auto const& m1 = arguments[0].matrix()->matrix_value();
            auto const& m2 = arguments[0].matrix()->matrix_value();
            VectorXd v1 = m1->value().reshaped();
            VectorXd v2 = m2->value().reshaped();
            return constant_number::create(v1.dot(v2));
        }
    };
    REGISTER_FUNCTION(dot);
}