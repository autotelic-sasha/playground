#include "parsing.h"
#include <sstream>
#include <unordered_map>
#include "string_util.h"
using namespace string_util;
// this is where we hide all the parsing filth
namespace parsing {
    // reporting errors in the code
    std::string error_with_position(std::string const& error, size_t const dot) {
        std::stringstream buff;
        buff << "Syntax error: " << error << " (at position: " << dot << ")";
        return buff.str();
    }

    // helpers for the decent through code
    inline void complete(std::string const& code, size_t& dot, size_t local_dot) {
        skip_whitespace(code, local_dot);
        dot = local_dot;
    }
    inline bool empty(std::stringstream& buff) {
        return (buff.tellp() == std::streampos(0));
    }

    // look-ahead: see if the token appears next in the code
    inline bool match(std::string const& code, size_t& dot, std::string const& token) {
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
    inline void skip_past_token(std::string const& code, size_t& dot, bool can_be_last = false) {
        char c = code[dot];
        ++dot;
        skip_whitespace(code, dot);
        if (!can_be_last && done(code, dot))
            throw std::runtime_error(error_with_position(std::string("trailing ") + c, dot));
    }

    // parse_result_t is for storing results of parsing steps

    template<typename T> inline parse_result_t<T> found(T const& _v) { 
        return parse_result_t<T>{_v, true }; 
    }
    template<typename T, typename U> parse_result_t<std::shared_ptr<T>> dynamic_result_cast(parse_result_t<U>& ur) {
        return parse_result_t< std::shared_ptr<T>>{std::dynamic_pointer_cast<T>(ur._value), ur._found };
    }
    parse_result_t<bool> not_found() {
        static parse_result_t<bool> _notfound{ false };
        return _notfound;
    }

    // packrat caching - basically memoization of parsing function results
    // we only cache results of things that can backtrack and take longer to parse than to look up
    enum class packrat_t { // one entry for each function whose results we memoize
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
        virtual void clear() = 0;
    };
    template<typename ResultT>
    struct cached_result_t {
        parse_result_t<ResultT> _result;
        size_t _dot{ (size_t)(-1)};
    };
    // there is one instance of a cache per memoized function
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
            _results.insert({ dot, cached_result_t<ResultT>{ ret, new_dot } });
            return ret;
        }
        inline parse_result_t<ResultT> not_found(size_t dot) {
            parse_result_t<ResultT> ret(parsing::not_found());
            _results.insert({ dot, cached_result_t<ResultT>{ ret, dot } });
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
        void clear() override {
            _results.clear();
        }
    };
    // when parsing, there is a bit of tidying up to do once a result is found
    // complete_cached amd cached_not_found functions implement this
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

    // cache of caches
    class packrat_caches {
        using caches_t = std::map<packrat_t, std::shared_ptr<packrat_cache_base>>;
        static caches_t& caches() {
            static caches_t _caches;
            return _caches;
        }
        using root_cache_t = std::unordered_map<std::string, expression_p>;
        static root_cache_t& root_cache() {
            static root_cache_t _root_cache;
            return _root_cache;
        }
    public:
        static void clear() {
            for (auto c : caches())
                c.second->clear();
        }

        template< typename ResultT>
        static std::shared_ptr<packrat_cache<ResultT>> get(packrat_t _t) {
            auto it = caches().find(_t);
            if (it != caches().end()) {
                std::shared_ptr<packrat_cache<ResultT>> ret = 
                    std::dynamic_pointer_cast<packrat_cache<ResultT>>(it->second);
                if (!ret)
                    throw std::runtime_error("Memoization error: typed cache is of wrong type.");
                return ret;
            }

            std::shared_ptr<packrat_cache<ResultT>> ret(new packrat_cache<ResultT>());
            caches()[_t] = ret;
            return ret;
        }
        static expression_p add_root(std::string const& code, expression_p root) {
            root_cache()[to_lower(code)] = root;
            return root;
        }
        static expression_p get_root(std::string const& code) {
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
    parse_result_t<number_expression_p> get_number_expression(std::string const& code, size_t& dot);

    parse_result_t<double> get_constant_number_value(std::string const code, size_t& dot) {
        // parsing constant numbers is pretty boring and done in one pass with no backtracking
        // we won't be cacheing it for now, maybe later, if it pops up in profilers
        size_t local_dot(dot);
        if (done(code, local_dot)) return not_found();
        char* start = const_cast<char*>(code.c_str()) + local_dot;
        char* end(start);
        double d = strtod(start, &end);
        if(end == start)
            return not_found();
        local_dot += (end - start);
        complete(code, dot, local_dot);
        return found(d);
    }
    parse_result_t<number_expression_p> get_constant_number(std::string const code, size_t& dot) {
        auto num = get_constant_number_value(code, dot);
        if (num)
            return found(constant_number::create(num.value()));
        return num;
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
        if (c != ']')
            throw std::runtime_error(error_with_position("expected ']'", local_dot));
        skip_past_token(code, local_dot);

        complete(code, dot, local_dot);
        return found(ret);
    }
    parse_result_t<std::vector<std::vector<double>>> get_constant_array(std::string const code, size_t& dot) {
        // parsing constant arrays is pretty boring and done in one pass with no backtracking
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
        skip_past_token(code, local_dot, true);

        complete(code, dot, local_dot);
        return found(ret);
    }

    parse_result_t<long> get_size(std::string const code, size_t& dot) {
        // parsing constant numbers is pretty boring and done in one pass with no backtracking
        // we won't be cacheing it for now, maybe later, if it pops up in profilers
        size_t local_dot(dot);
        if (done(code, local_dot)) return not_found();
        char* start = const_cast<char*>(code.c_str()) + local_dot;
        char* end(start);
        long i = strtol(start, &end, 10);
        if (end == start)
            return not_found();
        local_dot += (end - start);
        complete(code, dot, local_dot);
        return found(i);
    }
    parse_result_t<std::string> get_name(std::string const code, size_t& dot) {
        // parsing names is pretty boring and done in one pass with no backtracking
        // we won't be cacheing it for now, maybe later, if it pops up in profilers
        size_t local_dot(dot);
        if (done(code, local_dot)) return not_found();
        if (!std::isalpha(code[local_dot]))
            return not_found();
        while (std::isalnum(code[++local_dot])) // note that we don't need to check for end of string, isalnum will fail for \0
            ;
        size_t const start = dot;
        size_t const end = local_dot;
        complete(code, dot, local_dot);
        return found(code.substr(start,end));
        //std::stringstream  buff;
        //char c = code[local_dot];
        //if (!std::isalpha(c))
        //    return not_found();
        //while (std::isalnum(c)) {// note that we don't need to check for end of string, isalnum will fail for \0
        //    buff << c;
        //    c = code[++local_dot];
        //}
        //if (!empty(buff)) {
        //    complete(code, dot, local_dot);
        //    return found(buff.str());
        //}
        //return not_found();
    }
    parse_result_t<matrix_expression_p> get_constant_matrix(std::string const& name) {
        return found(named_matrix::create(name));
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
    parse_result_t<number_expression_p> get_bracketed_number_expression(std::string const& code, size_t& dot) {
        auto pcache(packrat_caches::get< number_expression_p>(packrat_t::bracketed_number_expression));
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
            auto ret(bracketed_number::create(n.value()));
            return complete_cached(code, dot, local_dot, pcache, ret);
        }
        return cached_not_found(dot, pcache);
    }
    parse_result_t<number_expression_p> get_number_function_invocation(std::string const& code, size_t& dot) {
        auto pcache(packrat_caches::get<number_expression_p>(packrat_t::number_function_invocation));
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

        auto ret(number_function_inv::create(nf.value(), arguments.value()));
        return complete_cached(code, dot, local_dot, pcache, ret);
    }

    parse_result_t<number_expression_p> get_number_mul_term(std::string const& code, size_t& dot) {
        auto pcache(packrat_caches::get<number_expression_p>(packrat_t::number_mul_term));
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
    parse_result_t<number_expression_p> get_number_product(std::string const& code, size_t& dot) {
        auto pcache(packrat_caches::get<number_expression_p>(packrat_t::number_product));
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
            parse_result_t<number_expression_p> next = next_term();
            while (next) {
                product = found(number_arithmetic::create(product.value(), next.value(), op));
                op = code[local_dot];
                next = next_term();
            }
        }
        return complete_cached(code, dot, local_dot, pcache, product);
    }
    parse_result_t<number_expression_p> get_number_term(std::string const& code, size_t& dot) {
        // number term is only there to deal with unary + and - for number expressions
        auto pcache(packrat_caches::get<number_expression_p>(packrat_t::number_term));
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
            auto ret(negative_number::create(rhs.value()));
            return complete_cached(code, dot, local_dot, pcache, ret);
        }
        return complete_cached(code, dot, local_dot, pcache, rhs);
    }

    parse_result_t<number_expression_p> get_number_sum(std::string const& code, size_t& dot) {
        // number_sum is right-recursive
        auto pcache(packrat_caches::get<number_expression_p>(packrat_t::number_sum));
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

            parse_result_t<number_expression_p> next = next_term();
            while (next) {
                sum = found(number_arithmetic::create(sum.value(), next.value(), op));
                op = code[local_dot];
                next = next_term();
            }
        }
        return complete_cached(code, dot, local_dot, pcache, sum);
    }

    parse_result_t<number_expression_p> get_number_expression(std::string const& code, size_t& dot) {
        // number expression is either a number sum or a determinant
        // we may be backtracking, so let's cache it
        auto pcache(packrat_caches::get<number_expression_p>(packrat_t::number_expression));
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
                auto ret(determinant::create(m.value()));
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
            ret = matrix_unary_op::create(m.value(), unary_matrix_op::inverse);
        }
        if (match(code, local_dot, "^T")) { // inverse
            ret = matrix_unary_op::create(m.value(), unary_matrix_op::transpose);
        }
        if (ret) {
            complete(code, dot, local_dot);
            return get_matrix_term_postfix(found(ret), code, dot); // right recursion
        }
        return m;
    }
    parse_result_t<matrix_expression_p> get_matrix_term(std::string const& code, size_t& dot) {
        // matrix_terms are recursive and complicated, we'll cache them for now
        auto pcache(packrat_caches::get<matrix_expression_p>(packrat_t::matrix_term));
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
                auto mfi(matrix_function_inv::create(mf.value(), arguments.value()));
                auto ret = get_matrix_term_postfix(found(mfi), code, local_dot);
                return complete_cached(code, dot, local_dot, pcache, ret);
            }
            if (get_number_function(name.value()))
                return not_found();
            auto m = get_constant_matrix(name);
            if (m) {
                auto ret = get_matrix_term_postfix(m, code, local_dot);
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
                auto ret(matrix_unary_op::create(pf.value(), unary_matrix_op::negative));
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
                auto bm(bracketed_matrix::create(m.value()));
                auto ret = get_matrix_term_postfix(found(bm), code, local_dot);
                return complete_cached(code, dot, local_dot, pcache, ret);
            }
        }
        return cached_not_found(dot, pcache);
    }

    parse_result_t<matrix_expression_p> get_matrix_number_term(std::string const& code, size_t& dot) {
        // matrix_number_term is recursive and can backtrack, so cacheing
        auto pcache(packrat_caches::get<matrix_expression_p>(packrat_t::matrix_number_term));
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
                        auto ret(matrix_number_op::create(m.value(), n.value(), c));
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
                        auto ret(matrix_number_op::create(m.value(), n.value(), '*'));
                        return complete_cached(code, dot, local_dot, pcache, ret);
                    }
                }
                else {
                    skip_whitespace(code, local_dot);
                    auto m = get_matrix_term(code, local_dot);
                    if (m) {
                        auto ret(matrix_number_op::create(m.value(), n.value(), '*'));
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
                product = found(matrix_binary_op::create(product.value(), next.value(), op));
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
            auto expr = constant_matrix::create(arr.value());
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
                expr = found(matrix_binary_op::create(expr.value(), next.value(), op));
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


    parse_result_t<assignment_p> parse_assignment(std::string const& code, size_t& dot) {
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

        auto ret(assignment::create(name_r.value(), m.value()));
        complete(code, dot, local_dot);
        return found(ret);
    }
}






