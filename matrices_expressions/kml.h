#pragma once
#include <vector>
#include <string>
#include <memory>
#include <iostream>
#include "expression_tree.h"

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
// matrix_function_invocation ->   name '(' arg_list ')'                    implemented in matrix_function_inv
// number_function_invocation ->   name '(' arg_list ')'                    implemented in number_function_inv
// 
// number_mul_term->   constant_number                                  |
//                     '(' number_expression ')'                        |   implemented in bracketed_number
//                     number_function_invocation                           implemented in number_function_inv
// 
// number_product ->   number_mul_term (('*'\'/'\e) number_mul_term)* 
//              
// number_term -> ('-'|'+')* number_product                             |   implemented in negative_number
// 
// number_sum -> number_term (('+'\'-') number_term)?                   |   implemented in number_arithmetic
// 
// number_expression ->     number_sum                                  |   implemented in constant_number
//                      '|' matrix_expression '|'                           implemented in determinant
//
// matrix_term ->       constant_matrix                                 |   implemented in constant_matrix 
//                      matrix_function_invocation                      |   implemented in matrix_function_inv
//                      matrix_term^T                                   |   implemented in matrix_unary_op 
//                      matrix_term^-1                                  |   implemented in matrix_unary_op 
//                      ('-'|'+') matrix_term                           |   implemented in matrix_unary_op 
//                      '(' matrix_expression ')'                           implemented in bracketed_matrix
//
// matrix_number_term -> matrix_term (('*'\'/') number_expression)?     |   implemented in matrix_number_op 
//                       number_expression ('*'\e) matrix_term              implemented in matrix_number_op 
// 
// matrix_product -> matrix_number_term ('*' matrix_number_term)?       |   implemented in matrix_binary_op (dot product)
//                      
// matrix_expression -> matrix_product (('+'\'-') matrix_product)?      |   implemented in matrix_binary_op
//                      array
// 
// assignments -> name '=' matrix_expression ';' (assignenent)*
// 
// expression -> matrix_expression                                      |   implemented in matrix_expression
//               number_expression                                          implemented in number_expression
//               
// program -> (assignments)+ expression
//




class kml_output {
    constant_matrix_p _matrix;
    double _number;
public:
    kml_output(constant_matrix_p matrix_, double number_) : _matrix(matrix_), _number(number_) {}
    inline bool is_matrix() const { return _matrix != nullptr; }
    inline bool is_number() const { return !is_matrix(); }

    inline double number_value() const { return _number; }
    inline constant_matrix_p matrix_value() const { return _matrix; }

};
using kml_output_p = std::shared_ptr<kml_output>;

std::ostream& operator<<(std::ostream& out, kml_output const& o);
std::ostream& operator<<(std::ostream& out, kml_output_p const& o);

class kml_program {
    std::vector <assignment_p> _assignments;
    expression_p _expression;
    std::string display_assignments(size_t tabs = 0) const;
    std::string display_assignments_with_brackets() const;
    void parse(std::string const& code);
public:
    kml_program(std::string const& code);
    kml_output_p execute() const;
    std::string display() const;
    std::string display_with_brackets() const;
    std::string json_string() const;
};
