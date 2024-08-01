#pragma once
#include <string>
#include <memory>
#include <vector>
#include <sstream>
#include <map>
struct number_expression;
using number_expression_p = std::shared_ptr<number_expression>;
struct matrix_expression;
using matrix_expression_p = std::shared_ptr<matrix_expression>;
class constant_matrix;
using matrix_result_p = std::shared_ptr<constant_matrix>;

namespace functions {
    // return types of functions
    enum class return_type_t {
        matrix,
        number
    };
    std::string to_string(return_type_t t);
    
    // function argument types
    enum class argument_type_t {
        matrix,
        number,
        size
    };
    std::string to_string(argument_type_t t);

    class argument_t {
        // unions are too much pain prior to C++ 17, we gonna waste some memory
        long _size{ 0 };
        number_expression_p _number{ nullptr };
        matrix_expression_p _matrix{ nullptr };
        argument_type_t _type{ argument_type_t::size };
        void check_type(argument_type_t const t) const;
    public:
        argument_t(long size_ = 0);
        argument_t(number_expression_p number_);
        argument_t(matrix_expression_p matrix_);

        inline argument_type_t type() const { return _type; }
        inline int size() const {
            check_type(argument_type_t::size);
            return _size;
        }
        inline number_expression_p number() const {
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
        void validate_argument_types(std::vector<argument_t> const& arguments) const;
    public:
        function(
            std::string name_,
            std::string help_,
            return_type_t return_type_,
            std::vector<argument_type_t> argument_types_
        );

        virtual ~function();
        inline return_type_t const return_type() const { return _return_type; }
        inline std::vector<argument_type_t> const& argument_types() const { return _argument_types; }
        inline std::string const& name() const { return _name; }
        inline std::string const& help() const { return _help; }
        std::string signature() const;
    };

    // functions returning numbers
    struct number_function : public function {
        number_function(
            std::string name_,
            std::string help_,
            std::vector<argument_type_t> argument_types_
        ) : function(name_, help_, return_type_t::number, argument_types_) {}

        virtual number_expression_p evaluate(std::vector<argument_t> const& arguments) const = 0;
    };
    
    // functions returning matrices
    struct matrix_function : public function {
    protected:
        std::string make_name(std::string const& name, int rows, int columns) const;
    public:
        matrix_function(
            std::string name_,
            std::string help_,
            std::vector<argument_type_t> argument_types_
        ) : function(name_, help_, return_type_t::matrix, argument_types_) {}

        virtual matrix_result_p evaluate(std::vector<argument_t> const& arguments) const = 0;
    };

    class function_table {
        static std::map<std::string, std::shared_ptr<function>>& table();
    public:
        static bool add(std::shared_ptr<function> const& function_);
        static std::shared_ptr<function> get(std::string const& name);
        static std::shared_ptr<function> find(std::string const& name);
        static bool check_exists_nc(std::string const& name);
        static std::map<std::string, std::string> help();
    };


}

inline bool check_function_exists(std::string const& name) {
    return functions::function_table::check_exists_nc(name);
}