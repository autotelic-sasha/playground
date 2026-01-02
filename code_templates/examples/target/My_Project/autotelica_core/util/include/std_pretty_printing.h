#pragma once
#include "std_disambiguation.h"
#include "diagnostic_messages.h"
#include <iomanip>
#include <numeric>

namespace autotelica {
    namespace std_pretty_printing {
        namespace std_pretty_printing_impl {
            template<typename char_t>
            struct default_pretty_config;
            
            template<>
            struct default_pretty_config<char> {
                using inner_container_config_t = default_pretty_config<char>;
                
                using char_type = char;
                using ostream_t = std::ostream;
                using string_t = std::string;
                using stringstream_t = std::stringstream;

                static constexpr auto pair_start = "{";
                static constexpr auto pair_separator = ",";
                static constexpr auto pair_end = "}";

                static constexpr auto element_separator = ",";
                static constexpr auto tab = "\t";
                static constexpr auto new_row = "\n";
                static constexpr auto quote = "\"";
                static constexpr auto blank = " ";

                static constexpr auto seq_start = "[";
                static constexpr auto seq_end = "]";

                static constexpr auto map_start = "{";
                static constexpr auto map_end = "}";

                static constexpr auto set_start = "{";
                static constexpr auto set_end = "}";


                static constexpr auto column_separator = ":";
                static constexpr auto title_row_separator = "=";
                static constexpr auto grid_column_separator = "|";

                static constexpr auto single_row_prefix = "0 : ";

                static constexpr auto index = "Index";
                static constexpr auto key =   "Key";
                static constexpr auto value = "Value";

                static std::ostream& manipulators(std::ostream& out) {
                    return out << std::boolalpha << std::right;
                }
                
                template<typename container_t>
                static inline const char_type* const container_start(container_t const&) {
                    using namespace std_disambiguation;
                    return  is_mapish_t<container_t>::value ? map_start : (
                        is_setish_t<container_t>::value ? set_start :
                        seq_start);
                }

                template<typename container_t>
                static inline const char_type* const container_end(container_t const&) {
                    using namespace std_disambiguation;
                    return  is_mapish_t<container_t>::value ? map_end : (
                        is_setish_t<container_t>::value ? set_end :
                        seq_end);
                }
            };

            template<>
            struct default_pretty_config<wchar_t> {
                using inner_container_config_t = default_pretty_config<wchar_t>;

                using char_type = wchar_t;
                using ostream_t = std::wostream;
                using string_t = std::wstring;
                using stringstream_t = std::wstringstream;

                static constexpr auto pair_start = L"{";
                static constexpr auto pair_separator = L",";
                static constexpr auto pair_end = L"}";

                static constexpr auto element_separator = L",";
                static constexpr auto tab = L"\t";
                static constexpr auto new_row = L"\n";
                static constexpr auto quote = L"\"";
                static constexpr auto blank = L" ";

                static constexpr auto seq_start = L"[";
                static constexpr auto seq_end = L"]";

                static constexpr auto map_start = L"{";
                static constexpr auto map_end = L"}";

                static constexpr auto set_start = L"{";
                static constexpr auto set_end = L"}";


                static constexpr auto column_separator = L":";
                static constexpr auto title_row_separator = L"=";
                static constexpr auto grid_column_separator = L"|";

                static constexpr auto single_row_prefix = L"0 : ";

                static constexpr auto index = L"Index";
                static constexpr auto key = L"Key";
                static constexpr auto value = L"Value";

                static std::wostream& manipulators(std::wostream& out) {
                    return out << std::boolalpha << std::right;
                }
                template<typename container_t>
                static inline const char_type* const container_start(container_t const&) {
                    using namespace std_disambiguation;
                    return  is_mapish_t<container_t>::value ?   map_start : (
                            is_setish_t<container_t>::value ?   set_start :
                                                                seq_start);
                }

                template<typename container_t>
                static inline const char_type* const container_end(container_t const&) {
                    using namespace std_disambiguation;
                    return  is_mapish_t<container_t>::value ?   map_end : (
                            is_setish_t<container_t>::value ?   set_end :
                                                                seq_end);
                }
            };

            template<typename char_t>
            struct one_per_row;

            template<>
            struct one_per_row<char> : public default_pretty_config<char> {

                using inner_container_config_t = default_pretty_config<char>;

                static constexpr auto seq_start = "[\n\t";
                static constexpr auto seq_end = "\n]"; 

                static constexpr auto map_start = "{\n\t"; 
                static constexpr auto map_end = "\n}"; 

                static constexpr auto set_start = "{\n\t"; 
                static constexpr auto set_end = "\n}"; 

                static constexpr auto element_separator = ",\n\t"; 
            };

            template<>
            struct one_per_row<wchar_t> : public default_pretty_config<wchar_t> {

                using inner_container_config_t = default_pretty_config<wchar_t>;

                static constexpr auto seq_start = L"[\n\t";
                static constexpr auto seq_end = L"\n]";

                static constexpr auto map_start = L"{\n\t";
                static constexpr auto map_end = L"\n}";

                static constexpr auto set_start = L"{\n\t";
                static constexpr auto set_end = L"\n}";

                static constexpr auto element_separator = L",\n\t";
            };




            using namespace std_disambiguation;
            // lots of work to be able to deal with arrays, oh well, C++ was never good with arrays
            namespace arrays {
                template<int sz, typename T, typename config_t>
                struct print_array {
                    void operator()(typename config_t::ostream_t& os, T const* arr) {
                        os << config_t::seq_start;
                        for (int i = 0; i < sz; ++i) {
                            os << arr[i];
                            if (i != sz - 1)
                                os << config_t::element_separator;
                        }
                        os << config_t::seq_end;
                    }
                };
                template<int sz, typename config_t>
                struct print_array<sz, typename config_t::char_type, config_t> {
                    void operator()(
                            typename config_t::ostream_t& os, 
                            typename config_t::char_type const* arr) {
                        os << arr;
                    }
                };
                template<int rank, int size, typename T, typename config_t>
                struct traverse_array_dimensions {
                    void operator()(typename config_t::ostream_t& os, const T* arr) {
                        const int sz = (int)std::extent<T>();
                        os << config_t::seq_start;
                        for (size_t i = 0; i < size; ++i) {
                            traverse_array_dimensions<rank - 1, sz, std::remove_extent_t<T>, config_t>()(os, arr[i]);
                            if (i != size - 1)
                                os << config_t::element_separator;
                        }
                        os << config_t::seq_end;
                    }
                };
                template<int size, typename T, typename config_t>
                struct traverse_array_dimensions<1, size, T, config_t> {
                    void operator()(typename config_t::ostream_t& os, const T* arr) {
                        print_array<size, T, config_t>()(os, arr);
                    }
                };
            }
            // now we go a little SFINAE crazy, ha ha ha ha :D 
            
            template<typename T>
            using if_is_array_t = typename std::enable_if_t<
                std::is_array<T>::value, bool>;

            template<typename T>
            using if_is_nothing_else_t = typename std::enable_if_t<
                !is_container_t<T>::value &&
                !is_pair_t<T>::value &&
                !std::is_array<T>::value, bool>;

            template<typename T>
            using if_is_string_t = typename std::enable_if_t<
                is_string_t<T>::value, bool>;

            template<typename T>
            using if_is_pair_t = typename std::enable_if_t<
                is_pair_t<T>::value, bool>;

            template<typename T>
            using if_is_other_container_t = typename std::enable_if_t<
                is_container_t<T>::value &&
                !is_mapish_t<T>::value &&
                !is_string_t<T>::value &&
                !is_adaptor_t<T>::value, bool>;

            template<typename T>
            using if_is_mapish_t = typename std::enable_if_t<
                is_mapish_t<T>::value, bool>;

            template<typename T>
            using if_is_stack_or_pq_t = typename std::enable_if_t<
                is_stack_t<T>::value ||
                is_priority_queue_t<T>::value, bool>;

            template<typename T>
            using if_is_queue_t = typename std::enable_if_t<
                is_queue_t<T>::value, bool>;

            // printing arrays
            template<   typename ostream_t,
                        typename elem_t, 
                        typename config_t = default_pretty_config<typename ostream_t::char_type>,
                        if_is_array_t<elem_t> = true>
            ostream_t& pretty_print(ostream_t& os, const elem_t& element) {
                const int r = (int)std::rank <elem_t>{};
                using eT = typename std::remove_extent<elem_t>::type;
                const int sz = (int)std::extent< elem_t>();
                arrays::traverse_array_dimensions<r, sz, eT, config_t>()(os, element);
                return os;
            }

            // printing stuff that didn't match any of the forms bellow
            template<   typename ostream_t,
                        typename elem_t,
                        typename config_t = default_pretty_config<typename ostream_t::char_type>,
                        if_is_nothing_else_t<elem_t> = true>
            ostream_t& pretty_print(ostream_t& os, const elem_t& element) {
                os << element;
                return os;
            }
            // printing strings
            template<   typename ostream_t,
                        typename elem_t,
                        typename config_t = default_pretty_config<typename ostream_t::char_type>,
                        if_is_string_t<elem_t> = true >
            ostream_t& pretty_print(ostream_t& os, const elem_t& element) {
                os << config_t::quote << element << config_t::quote;
                return os;
            }
            // printing pairs
            template<   typename ostream_t,
                        typename elem_t,
                        typename config_t = default_pretty_config<typename ostream_t::char_type>,
                        if_is_pair_t<elem_t> = true >
            ostream_t& pretty_print(ostream_t& os, const elem_t& element) {
                os << config_t::pair_start;
                pretty_print(os, element.first);
                os << config_t::pair_separator;
                pretty_print(os, element.second);
                os << config_t::pair_end;
                return os;
            }
            // printing containers
            template<   typename ostream_t,
                        typename container_t,
                        typename config_t = default_pretty_config<typename ostream_t::char_type>,
                        if_is_other_container_t<container_t> = true >
            ostream_t& pretty_print(ostream_t& os, const container_t& container) {
                os << config_t::manipulators << config_t::container_start(container);
                for (auto it = container.begin(); it != container.end(); ++it) {
                    os << config_t::manipulators;
                    pretty_print<ostream_t, typename container_t::value_type, typename config_t::inner_container_config_t>(os, *it);
                    if (std::next(it) != container.end())
                        os << config_t::element_separator;
                }
                os << config_t::container_end(container);
                return os;
            }
            // printing maps
            template<   typename ostream_t,
                        typename container_t,
                        typename config_t = default_pretty_config<typename ostream_t::char_type>,
                        if_is_mapish_t<container_t> = true >
            ostream_t& pretty_print(ostream_t& os, const container_t& container) {
                os << config_t::manipulators << config_t::container_start(container);
                for (auto it = container.begin(); it != container.end(); ++it) {
                    os << config_t::manipulators;
                    os << config_t::pair_start;
                    pretty_print(os, it->first);
                    os << config_t::pair_separator;
                    pretty_print(os, it->second);
                    os << config_t::pair_end;
                    if (std::next(it) != container.end())
                        os << config_t::element_separator;
                }
                os << config_t::container_end(container);
                return os;
            }
            // printing stacks and priority queues
            template<   typename ostream_t,
                        typename container_t,
                        typename config_t = default_pretty_config<typename ostream_t::char_type>,
                        if_is_stack_or_pq_t<container_t> = true >
            ostream_t& pretty_print(ostream_t& os, const container_t& container) {
                container_t container_copy(container);
                std::vector<typename container_t::value_type> tmp;
                while (!container_copy.empty()) {
                    tmp.push_back(container_copy.top());
                    container_copy.pop();
                }
                pretty_print(os, tmp);
                return os;
            }
            // printing queues
            template<   typename ostream_t,
                        typename container_t,
                        typename config_t = default_pretty_config<typename ostream_t::char_type>,
                        if_is_queue_t<container_t> = true >
            ostream_t& pretty_print(ostream_t& os, const container_t& container) {
                container_t container_copy(container);
                std::vector<typename container_t::value_type> tmp;
                while (!container_copy.empty()) {
                    tmp.push_back(container_copy.front());
                    container_copy.pop();
                }
                pretty_print(os, tmp);
                return os;
            }
            // pretty_t class - a functor invoking pretty_print on a container 
            template<   typename container_t, 
                        typename config_t = default_pretty_config<char>>
            struct pretty_t {
                using string_t = typename config_t::string_t;
                using stringstream_t = typename config_t::stringstream_t;

                container_t const& _container;

                pretty_t(container_t const& container) :_container(container) {}
                
                template<typename ostream_t>
                ostream_t& operator()(ostream_t& out) const {
                    return pretty_print<ostream_t, container_t, config_t>(out, _container);
                }
                string_t str() const {
                    stringstream_t s;
                    operator()(s);
                    return s.str();
                }
            };

            // simple table format - optimised for size, rather than speed
            // it traverses the container two or three times to figure out the column sizes
            // could be slow for large containers, just find something better for that

            // element_length functions are used as helpers when calculating column widths in a table
            namespace container_arithmetic {
                template< typename value_t,
                    typename std::enable_if_t<
                    !std::is_arithmetic<value_t>::value &&
                    !is_string_t<value_t>::value, bool> = true>
                size_t element_length(const value_t& v) {
                    return -1;
                };

                template< typename value_t,
                    typename std::enable_if_t<
                    std::is_arithmetic<value_t>::value, bool> = true>
                size_t element_length(const value_t& v) {
                    return std::to_string(v).size();
                };
                template< typename value_t,
                    typename std::enable_if_t<
                    is_string_t<value_t>::value, bool> = true>
                size_t element_length(const value_t& v) {
                    return std::distance(v.begin(), v.end());
                };

                // longest_key and longest_value are widths of the two columns in the table
                template< typename container_t,
                    typename std::enable_if_t<
                    is_indexable_t<container_t>::value ||
                    is_adaptor_t<container_t>::value, bool> = true >
                size_t longest_key(const container_t & container) {
                    return std::to_string(std::distance(container.begin(), container.end())).size(); // for indexable containers keys are indices
                }
                template< typename container_t,
                    typename std::enable_if_t<
                    is_mapish_t<container_t>::value, bool> = true >
                size_t longest_key(const container_t& container) {
                    size_t i = 1;
                    for (auto it = container.begin(); it != container.end(); ++it) {
                        const size_t l = element_length(it->first);
                        if (l == -1) return -1;
                        if (l > i) i = l;
                    }
                    return i;
                }

                template< typename container_t,
                    typename std::enable_if_t<
                    is_indexable_t<container_t>::value ||
                    is_adaptor_t<container_t>::value, bool> = true >
                size_t longest_value(const container_t & container) {
                    size_t i = 1;
                    for (auto it = container.begin(); it != container.end(); ++it) {
                        const size_t l = element_length(*it);
                        if (l == -1) return -1;
                        if (l > i) i = l;
                    }
                    return i;
                }
                template< typename container_t,
                    typename std::enable_if_t<
                    is_mapish_t<container_t>::value, bool> = true >
                size_t longest_value(const container_t& container) {
                    size_t i = 1;
                    for (auto it = container.begin(); it != container.end(); ++it) {
                        const size_t l = element_length(it->second);
                        if (l == -1) return -1;
                        if (l > i) i = l;
                    }
                    return i;
                }
            }
            // default case, stuff that we don't know how to tabularize
            template<typename T>
            using if_is_default_table_t = typename std::enable_if_t<
                !(is_indexable_t<T>::value && !is_string_t<T>::value) &&
                !is_adaptor_t<T>::value &&
                !is_mapish_t<T>::value &&
                !is_value_grid_t<T>::value, bool>;
            
            template<typename T>
            using if_is_indexable_simple_t = typename std::enable_if_t<
                is_indexable_t<T>::value &&
                !is_string_t<T>::value &&
                !is_value_grid_t<T>::value, bool>;

            template<typename T>
            using if_is_value_grid_t = typename std::enable_if_t<
                is_value_grid_t<T>::value, bool>;

            template<   typename ostream_t,
                        typename container_t, 
                        typename config_t = default_pretty_config<typename ostream_t::char_type>,
                        if_is_default_table_t<container_t> = true >
            ostream_t& table_print(bool first_row_titles, ostream_t& os, const container_t& container) {
                os << config_t::single_row_prefix;
                pretty_print(os, container);
                os << config_t::new_row;
                return os;
            }

            // tabularizing indexable stuff
            template<   typename ostream_t,
                        typename container_t,
                        typename config_t = default_pretty_config<typename ostream_t::char_type>,
                        if_is_indexable_simple_t<container_t> = true >
            ostream_t& table_print(bool first_row_titles, ostream_t& os, const container_t& container) {
                using namespace container_arithmetic;
                using string_t = typename config_t::string_t;
                
                os << config_t::manipulators;
                size_t row = 0;
                const string_t col_1_title(config_t::index);
                const string_t col_2_title(config_t::value);
                const size_t l_width = first_row_titles?std::max(longest_key(container), col_1_title.size()):longest_key(container);
                const size_t r_width = first_row_titles ? std::max(longest_value(container), col_2_title.size()) : longest_value(container);
                if (first_row_titles) {
                    size_t total_length = l_width + r_width + 3;
                    string_t line(total_length, config_t::title_row_separator[0]);
                    os << std::left << std::setw(l_width) << col_1_title 
                        << config_t::blank << config_t::column_separator << config_t::blank
                        << std::left << std::setw(r_width) << col_2_title 
                        << config_t::new_row << line << config_t::new_row;
                }
                for (auto it = container.begin(); it != container.end(); ++it, ++row) {
                    os << config_t::manipulators << std::left << std::setw(l_width);
                    os << row << config_t::blank << config_t::column_separator << config_t::blank;
                    os << config_t::manipulators << std::right << std::setw(r_width);
                    pretty_print<ostream_t, typename container_t::value_type, typename config_t::inner_container_config_t>(os, *it);
                    os << config_t::new_row;
                }
                return os;
            }

            // tabularizing stacks and priority queues
            template<   typename ostream_t,
                        typename container_t,
                        typename config_t = default_pretty_config<typename ostream_t::char_type>,
                        if_is_stack_or_pq_t<container_t> = true >
            ostream_t& table_print(bool first_row_titles, ostream_t& os, const container_t& container) {
                container_t container_copy(container);
                std::vector<typename container_t::value_type> tmp;
                while (!container_copy.empty()) {
                    tmp.push_back(container_copy.top());
                    container_copy.pop();
                }
                table_print(first_row_titles, os, tmp);
                return os;
            }

            // tabularizing queues
            template<   typename ostream_t,
                        typename container_t,
                        typename config_t = default_pretty_config<typename ostream_t::char_type>,
                        if_is_queue_t<container_t> = true >
            ostream_t& table_print(bool first_row_titles, ostream_t& os, const container_t& container) {
                container_t container_copy(container);
                std::vector<typename container_t::value_type> tmp;
                while (!container_copy.empty()) {
                    tmp.push_back(container_copy.front());
                    container_copy.pop();
                }
                table_print(first_row_titles, os, tmp);
                return os;
            }

            // tabularizing maps
            template<   typename ostream_t,
                        typename container_t,
                        typename config_t = default_pretty_config<typename ostream_t::char_type>,
                        if_is_mapish_t<container_t> = true >
            ostream_t& table_print(bool first_row_titles, ostream_t& os, const container_t& container) {
                using namespace container_arithmetic;
                using string_t = typename config_t::string_t;
                const string_t col_1_title(config_t::key);
                const string_t col_2_title(config_t::value);
                const size_t l_width = first_row_titles ? std::max(longest_key(container), col_1_title.size()) : longest_key(container);
                const size_t r_width = first_row_titles ? std::max(longest_value(container), col_2_title.size()) : longest_value(container);
                if (first_row_titles) {
                    size_t total_length = l_width + r_width + 3;
                    string_t line(total_length, config_t::title_row_separator[0]);
                    os << std::left << std::setw(l_width) << col_1_title
                        << config_t::blank << config_t::column_separator << config_t::blank
                        << std::left << std::setw(r_width) << col_2_title
                        << config_t::new_row << line << config_t::new_row;
                }
                for (auto it = container.begin(); it != container.end(); ++it) {
                    os << config_t::manipulators << std::left << std::setw(l_width);
                    os << it->first << config_t::blank << config_t::column_separator << config_t::blank;
                    os << config_t::manipulators << std::right << std::setw(r_width);
                    pretty_print<ostream_t, typename container_t::mapped_type, typename config_t::inner_container_config_t>(os, it->second);
                    os << config_t::new_row;
                }
                return os;
            }

            // tabularizing sequence of sequence of strings
            template<   typename ostream_t,
                        typename container_t,
                        typename config_t = default_pretty_config<typename ostream_t::char_type>,
                        if_is_value_grid_t<container_t> = true >
            ostream_t& table_print(bool first_row_titles, ostream_t& os, const container_t& container) {
                // we're going to do this in two passes, I don't think that can be avoided
                // first pass to figure out the number of columns and their width
                // second to do the actual work
                using namespace container_arithmetic;
                using string_t = typename config_t::string_t;
                std::vector<size_t> column_widths;
                const size_t default_width = 8;// why 8? kinda nice, no other reason
                size_t max_width = 0;
                for (auto const& row : container) {
                    size_t col = 0;
                    for (auto const& e : row) {
                        size_t w = element_length(e);
                        if (w == -1) w = default_width;
                        if (w > max_width) max_width = w;
                        if (column_widths.size() <= col)
                            column_widths.push_back(w);
                        else {
                            if (w > column_widths[col]) column_widths[col] = w;
                        }
                        ++col;
                    }
                }
                
                // at this point, column_widths is guaranteed to be the same size as the longest row
                int row_i = 0;
                for (auto const& row : container) {
                    size_t col = 0;
                    for (auto const& e : row) {
                        os << std::left << std::setw(column_widths[col]) << e;
                        if (col < column_widths.size() - 1) 
                            os << config_t::blank << config_t::grid_column_separator << config_t::blank;
                        ++col;
                    }
                    // pad missing columns
                    for (; col < column_widths.size(); ++col) {
                        string_t empty_cell(column_widths[col], config_t::blank[0]);
                        os << empty_cell;
                        if (col < column_widths.size() - 1)
                            os << config_t::blank << config_t::grid_column_separator << config_t::blank;
                    }
                    os << config_t::new_row;
                    if (first_row_titles && row_i == 0) {
                        size_t total_length = std::accumulate(column_widths.begin(), column_widths.end(), size_t(0)) +
                            3 * column_widths.size();
                        string_t line(total_length, config_t::title_row_separator[0]);
                        os << line << config_t::new_row;
                    }
                    ++row_i;
                }
                return os;
            }



            // functor to invoke tabularization on its only member
            template<   typename container_t, 
                        typename config_t = default_pretty_config<char>>
            struct table_t {
                using string_t = typename config_t::string_t;
                using stringstream_t = typename config_t::stringstream_t;

                container_t const& _container;
                bool _first_row_titles;
                
                table_t(container_t const& container, bool first_row_titles = false) :
                    _container(container),
                    _first_row_titles(first_row_titles){}
                
                template<typename ostream_t>
                ostream_t& operator()(ostream_t& out) const {
                    return table_print<ostream_t, container_t, config_t>(_first_row_titles, out, _container);
                }
                string_t str() const {
                    stringstream_t s;
                    operator()(s);
                    return s.str();
                }
            };
        }


        // pretty functions works as stream modifiers
        template<typename container_t>
        std_pretty_printing_impl::pretty_t<container_t> 
            pretty(container_t const& container) { 
                return std_pretty_printing_impl::pretty_t<container_t>(container); 
        }

        template<typename container_t, typename config_t >
        std_pretty_printing_impl::pretty_t<container_t, config_t>
            pretty(container_t const& container, config_t const& config) { 
                return std_pretty_printing_impl::pretty_t<container_t, config_t>(container);
        }


        // streaming of prettified containers
        template<typename ostream_t, typename container_t>
        ostream_t& operator<<(
                ostream_t& out, 
                std_pretty_printing_impl::pretty_t<
                    container_t, 
                    std_pretty_printing_impl::default_pretty_config<typename ostream_t::char_type>> const& p) {
            return p(out);
        }

        // prettifying into strings
        template< typename container_t, typename char_t >
        typename std::basic_string<char_t> pretty_s(container_t const& container, char_t const& unused) {
            using config_t = std_pretty_printing_impl::default_pretty_config<char_t>;
            
            typename config_t::stringstream_t out;
            out << pretty(container, config_t());
            return out.str();
        }
        template<typename container_t>
        std::string pretty_s(container_t const& container) {
            return pretty_s(container, ' ');
        }
        template<typename container_t>
        std::wstring pretty_w(container_t const& container) {
            return pretty_s(container, L' ');
        }
        // alternative configuration for pretty printing, one value per row
        template<typename char_t>
        using one_per_row_t = std_pretty_printing_impl::one_per_row<char>;
        using one_per_row = std_pretty_printing_impl::one_per_row<char>;
        using one_per_row_w = std_pretty_printing_impl::one_per_row<wchar_t>;

        // simple table format - optimised for size, rather than speed
        // it traverses the container two or three times to figure out the column sizes
        // could be slow for large containers, just find something better for that

        // table functions works as stream modifiers
        template< typename container_t>
        std_pretty_printing_impl::table_t<container_t>
            table(container_t const& container, bool first_row_titles = false) {
                return std_pretty_printing_impl::table_t<container_t>(container, first_row_titles); 
        }

        template< typename container_t, typename config_t >
        std_pretty_printing_impl::table_t<container_t, config_t>
            table(container_t const& container, config_t const& config, bool first_row_titles = false) {
                return std_pretty_printing_impl::table_t<container_t, config_t>(container, first_row_titles); 
        }

        // streaming of tabularized containers
        template< typename ostream_t, typename container_t>
        ostream_t& operator<<(
            ostream_t& out, 
            std_pretty_printing_impl::table_t<
                container_t, 
                std_pretty_printing_impl::default_pretty_config<typename ostream_t::char_type>> const& p) {
            return p(out);
        }

        // tabularizing into strings

        template< typename container_t, typename char_t >
        std::basic_string<char_t> table_s(container_t const& container, bool first_row_titles, char_t const& unused) {
            using config_t = std_pretty_printing_impl::default_pretty_config<char_t>;
            std_pretty_printing_impl::table_t <
                container_t,config_t> const p(container, first_row_titles);
            return p.str();
        }
        template< typename container_t>
        std::string table_s(container_t const& container, bool first_row_titles = false) {
            return table_s(container, first_row_titles, ' ');
        }
        template< typename container_t>
        std::wstring table_w(container_t const& container, bool first_row_titles = false) {
            return table_s(container, first_row_titles, L' ');
        }
    }
}
