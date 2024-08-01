#include "kml.h"
#include "parsing.h"
#include "json_util.h"
std::ostream& operator<<(std::ostream& out, kml_output const& o) {
    if (o.is_matrix())
        out << "\n" << o.matrix_value()->display();
    else
        out << o.number_value();
    return out;
}
std::ostream& operator<<(std::ostream& out, kml_output_p const& o) {
    out << *o;
    return out;
}

std::string kml_program::display_assignments(size_t tabs) const {
    std::stringstream buff;
    std::string indent(tabs, '\t');
    for (auto const& a : _assignments)
        buff << a->description(tabs) << "\n" << indent;
    return buff.str();
}
std::string kml_program::display_assignments_with_brackets() const {
    std::stringstream buff;
    for (auto const& a : _assignments)
        buff << a->display_with_brackets();
    return buff.str();
}
kml_program::kml_program(std::string const& code) {
    parse(code);
}
void kml_program::parse(std::string const& code) {
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
kml_output_p kml_program::execute() const {
    kml_output_p output;
    for (auto const& a : _assignments)
        a->add_to_cache();
    if (_expression) {
        if (_expression->type() == expression_type::matrix)
            output = kml_output_p(new kml_output{ _expression->matrix_value(), 0 });
        else
            output = kml_output_p(new kml_output{ nullptr, _expression->number_value() });
    }
    for (auto const& a : _assignments)
        a->restore_cache();
    return output;
}
std::string kml_program::display() const {
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
std::string kml_program::display_with_brackets() const {
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
std::string kml_program::json_string() const {
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
    if (!_expression)
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
