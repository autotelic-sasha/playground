#include <sstream>
#include "matrix_cache.h"
#include "parsing.h"
#include "string_util.h"
#include "functions.h"
using namespace string_util;

std::map<std::string, std::shared_ptr<constant_matrix>>& matrix_cache::cache() {
    static std::map<std::string, std::shared_ptr<constant_matrix>> _cache;
    return _cache;
}
void matrix_cache::add(std::string const& name, constant_matrix_p const& matrix_) {
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
void matrix_cache::add(std::string const& name, std::string const& code) {
    size_t dot(0);
    auto me = parsing::parse_expression(code, dot);
    if (!me) throw std::runtime_error("No parseable expressions found");
    if (me->type() != expression_type::matrix)
        throw std::runtime_error("Expression value is a number, not a matrix.");
    auto cm = std::dynamic_pointer_cast<constant_matrix>(me);
    if (cm)
        add(name, cm);
    else
        add(name, me->matrix_value());
}
constant_matrix_p matrix_cache::get(std::string const& name) {
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
constant_matrix_p matrix_cache::find(std::string const& name) {
    std::string l_name(to_lower(name));
    for (auto const& e : cache()) {
        if (e.first == name)
            return e.second;
    }
    return nullptr;
}
void matrix_cache::remove(std::string const& name) {
    if (find(name))
        cache().erase(name);
}
std::string matrix_cache::display_matrix(std::string const& name) {
    return get(name)->display();
}
void matrix_cache::clear() {
    cache().clear();
}
std::vector<std::string> matrix_cache::list() {
    std::vector<std::string> ret;
    for (auto const& e : cache())
        ret.push_back(e.first);
    return ret;
}
