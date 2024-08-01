#pragma once
#include "expression_tree.h"
namespace parsing {
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

	expression_p parse_expression(std::string const& code, size_t& dot);
	parse_result_t<assignment_p> parse_assignment(std::string const& code, size_t& dot);

    inline void skip_whitespace(std::string const code, size_t& dot) {
        while (dot < code.size() && std::isspace(code[dot]))
            ++dot;
    }

    inline bool done(std::string const& code, size_t dot) {
        return dot >= code.size();
    }
}

