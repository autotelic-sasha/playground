#pragma once
#include "std_disambiguation.h"
#include "diagnostic_messages.h"
#include <iomanip>
#include <numeric>

namespace autotelica {
    namespace std_pretty_printing {
        namespace std_pretty_printing_impl {
            struct default_pretty_config {
                using inner_container_config_t = default_pretty_config;

                static constexpr char const* const pair_start() { return "{"; }
                static constexpr char const* const pair_separator() { return ","; }
                static constexpr char const* const pair_end() { return "}"; }

                static constexpr char const* const element_separator() { return ","; }
                static constexpr char const* const tab() { return "\t"; }
                static constexpr char const* const new_row() { return "\n"; }

                static constexpr char const* const seq_start() { return "["; }
                static constexpr char const* const seq_end() { return "]"; }

                static constexpr char const* const map_start() { return "{"; }
                static constexpr char const* const map_end() { return "}"; }

                static constexpr char const* const set_start() { return "{"; }
                static constexpr char const* const set_end() { return "}"; }


                static constexpr char const* const column_separator() { return ":"; }
                static constexpr char const* const title_row_separator() { return "="; }
                static constexpr char const* const grid_column_separator() { return "|"; }

                static std::ostream& manipulators(std::ostream& out) {
                    return out << std::boolalpha << std::right;
                }
            };

            template< typename ContainerT, typename ConfigT>
            constexpr char const* const container_start() {
                using namespace std_disambiguation;
                return is_mapish_t<ContainerT>::value ? ConfigT::map_start() : (
                    is_setish_t<ContainerT>::value ? ConfigT::set_start() : ConfigT::seq_start());
            }

            template< typename ContainerT, typename ConfigT>
            constexpr char const* const container_end() {
                using namespace std_disambiguation;
                return is_mapish_t<ContainerT>::value ? ConfigT::map_end() : (
                    is_setish_t<ContainerT>::value ? ConfigT::set_end() : ConfigT::seq_end());
            }


            struct one_per_row : public default_pretty_config {

                using inner_container_config_t = default_pretty_config;

                static constexpr char const* const seq_start() { return "[\n\t"; }
                static constexpr char const* const seq_end() { return "\n]"; }

                static constexpr char const* const map_start() { return "{\n\t"; }
                static constexpr char const* const map_end() { return "\n}"; }

                static constexpr char const* const set_start() { return "{\n\t"; }
                static constexpr char const* const set_end() { return "\n}"; }

                static constexpr char const* const element_separator() { return ",\n\t"; }
            };


            using namespace std_disambiguation;


            // lots of work to be able to deal with arrays, oh well, C++ was never good with arrays
            template<int sz, typename T>
            struct print_array {
                void operator()(std::ostream& os, T const* arr) {
                    os << "[";
                    for (int i = 0; i < sz; ++i) {
                        os << arr[i];
                        if (i != sz - 1)
                            os << ", ";
                    }
                    os << "]";
                }
            };
            template<int sz>
            struct print_array<sz, char> {
                void operator()(std::ostream& os, char const* arr) {
                    os << arr;
                }
            };
            template<int rank, int size, typename T>
            struct traverse_array_dimensions {
                void operator()(std::ostream& os, const T* arr) {
                    const int sz = (int)std::extent<T>();
                    os << "[";
                    for (size_t i = 0; i < size; ++i) {
                        traverse_array_dimensions<rank-1, sz, std::remove_extent_t<T>>()(os, arr[i]);
                        if (i != size - 1)
                            os << ",";
                    }
                    os << "]";
                }
            };
            template<int size, typename T>
            struct traverse_array_dimensions<1, size, T> {
                void operator()(std::ostream& os, const T* arr) {
                     print_array<size, T>()(os, arr);
                }
            };

            template< typename ElemT, typename ConfigT = default_pretty_config,
                typename std::enable_if_t<
                std::is_array<ElemT>::value, int> = 0>
            std::ostream& pretty_print(std::ostream& os, const ElemT& element) {
                const int r = (int)std::rank <ElemT>{};
                using eT = typename std::remove_extent<ElemT>::type;
                const int sz = (int)std::extent< ElemT>();
                traverse_array_dimensions<r, sz, eT>()(os, element);
                return os;
            }

            // printing stuff that didn't match any of the forms bellow
            template< typename ElemT, typename ConfigT = default_pretty_config,
                typename std::enable_if_t<
                !is_container_t<ElemT>::value &&
                !is_pair_t<ElemT>::value &&
                !std::is_array<ElemT>::value, int> = 0>
            std::ostream& pretty_print(std::ostream& os, const ElemT& element) {
                os << element;
                return os;
            }
            // printing strings
            template< typename ElemT, typename ConfigT = default_pretty_config,
                typename std::enable_if_t<
                is_string_t<ElemT>::value, int> = 0>
            std::ostream& pretty_print(std::ostream& os, const ElemT& element) {
                os << "\"" << element << "\"";
                return os;
            }
            // printing pairs
            template< typename ElemT, typename ConfigT = default_pretty_config,
                typename std::enable_if_t<
                is_pair_t<ElemT>::value, int > = 0>
            std::ostream& pretty_print(std::ostream& os, const ElemT& element) {
                os << ConfigT::pair_start();
                pretty_print(os, element.first);
                os << ConfigT::pair_separator();
                pretty_print(os, element.second);
                os << ConfigT::pair_end();
                return os;
            }
            // printing containers
            template< typename ContainerT, typename ConfigT = default_pretty_config,
                typename std::enable_if_t<
                is_container_t<ContainerT>::value &&
                !is_mapish_t<ContainerT>::value &&
                !is_string_t<ContainerT>::value &&
                !is_adaptor_t<ContainerT>::value, int> = 0 >
            std::ostream& pretty_print(std::ostream& os, const ContainerT& container) { 
                os << ConfigT::manipulators << container_start<ContainerT, ConfigT>();
                for (auto it = container.begin(); it != container.end(); ++it) {
                    os << ConfigT::manipulators;
                    pretty_print<typename ContainerT::value_type, typename ConfigT::inner_container_config_t>(os, *it);
                    if (std::next(it) != container.end())
                        os << ConfigT::element_separator();
                }
                os << container_end<ContainerT, ConfigT>();
                return os;
            }
            template< typename ContainerT, typename ConfigT = default_pretty_config,
                typename std::enable_if_t<
                is_mapish_t<ContainerT>::value, int> = 0 >
            std::ostream& pretty_print(std::ostream& os, const ContainerT& container) { 
                os << ConfigT::manipulators << container_start<ContainerT, ConfigT>();
                for (auto it = container.begin(); it != container.end(); ++it) {
                    os << ConfigT::manipulators;
                    os << ConfigT::pair_start();
                    pretty_print(os, it->first);
                    os << ConfigT::pair_separator();
                    pretty_print(os, it->second);
                    os << ConfigT::pair_end();
                    if (std::next(it) != container.end())
                        os << ConfigT::element_separator();
                }
                os << container_end<ContainerT, ConfigT>();
                return os;
            }
            // printing stacks and priority queues
            template< typename ContainerT, typename ConfigT = default_pretty_config,
                typename std::enable_if_t<
                is_stack_t<ContainerT>::value ||
                is_priority_queue_t<ContainerT>::value, int> = 0>
            std::ostream & pretty_print(std::ostream & os, const ContainerT & container) {
                ContainerT container_copy(container);
                std::vector<typename ContainerT::value_type> tmp;
                while (!container_copy.empty()) {
                    tmp.push_back(container_copy.top());
                    container_copy.pop();
                }
                pretty_print(os, tmp);
                return os;
            }
            // printing queues
            template< typename ContainerT, typename ConfigT = default_pretty_config,
                typename std::enable_if_t<
                is_queue_t<ContainerT>::value, int> = 0>
            std::ostream& pretty_print(std::ostream& os, const ContainerT& container) {
                ContainerT container_copy(container);
                std::vector<typename ContainerT::value_type> tmp;
                while (!container_copy.empty()) {
                    tmp.push_back(container_copy.front());
                    container_copy.pop();
                }
                pretty_print(os, tmp);
                return os;
            }
            // pretty_t class - a functor invoking prett_print on a container 
            template< typename ContainerT, typename ConfigT = std_pretty_printing_impl::default_pretty_config>
            struct pretty_t {
                ContainerT const& _container;
                pretty_t(ContainerT const& container) :_container(container) {}
                std::ostream& operator()(std::ostream& out) const {
                    return pretty_print<ContainerT, ConfigT>(out, _container);
                }
                std::string str() const {
                    std::stringstream s;
                    operator()(s);
                    return s.str();
                }
            };

            // simple table format - optimised for size, rather than speed
            // it traverses the container two or three times to figure out the column sizes
            // could be slow for large containers, just find something better for that

            // element_length functions are used as helpers when calculating column widths in a table
            template< typename ValueT,
                typename std::enable_if_t<
                !std::is_arithmetic<ValueT>::value &&
                !std::is_same<ValueT, std::string>::value, int> = 0>
            size_t element_length(const ValueT& v) {
                return -1;
            };

            template< typename ValueT,
                typename std::enable_if_t<
                std::is_arithmetic<ValueT>::value, int> = 0>
            size_t element_length(const ValueT& v) {
                return std::to_string(v).size();
            };
            template< typename ValueT,
                typename std::enable_if_t<
                is_string_t<ValueT>::value, int> = 0>
            size_t element_length(const ValueT& v) {
                return std::distance(v.begin(), v.end());
            };

            // longest_key and longest_value are widths of the two columns in the table
            template< typename ContainerT,
                typename std::enable_if_t<
                is_indexable_t<ContainerT>::value ||
                is_adaptor_t<ContainerT>::value, int> = 0 >
            size_t longest_key(const ContainerT& container) {
                return std::to_string(std::distance(container.begin(), container.end())).size(); // for indexable containers keys are indices
            }
            template< typename ContainerT,
                typename std::enable_if_t<
                is_mapish_t<ContainerT>::value, int> = 0 >
            size_t longest_key(const ContainerT& container) {
                size_t i = 1;
                for (auto it = container.begin(); it != container.end(); ++it) {
                    const size_t l = element_length(it->first);
                    if (l == -1) return -1;
                    if (l > i) i = l;
                }
                return i;
            }

            template< typename ContainerT,
                typename std::enable_if_t<
                is_indexable_t<ContainerT>::value ||
                is_adaptor_t<ContainerT>::value, int> = 0 >
            size_t longest_value(const ContainerT& container) {
                size_t i = 1;
                for (auto it = container.begin(); it != container.end(); ++it) {
                    const size_t l = element_length(*it);
                    if (l == -1) return -1;
                    if (l > i) i = l;
                }
                return i;
            }
            template< typename ContainerT,
                typename std::enable_if_t<
                is_mapish_t<ContainerT>::value, int> = 0 >
            size_t longest_value(const ContainerT& container) {
                size_t i = 1;
                for (auto it = container.begin(); it != container.end(); ++it) {
                    const size_t l = element_length(it->second);
                    if (l == -1) return -1;
                    if (l > i) i = l;
                }
                return i;
            }

            // default case, stuff that we don't know how to tabularize
            template< typename ContainerT, typename ConfigT = default_pretty_config,
                typename std::enable_if_t<
                !(is_indexable_t<ContainerT>::value &&
                    !is_string_t<ContainerT>::value) &&
                    !is_adaptor_t<ContainerT>::value &&
                    !is_mapish_t<ContainerT>::value &&
                    !is_value_grid_t<ContainerT>::value, int> = 0 >
            std::ostream& table_print(bool first_row_titles, std::ostream& os, const ContainerT& container) {
                os << "0 " << ConfigT::column_separator() << " ";
                pretty_print(os, container);
                os << ConfigT::new_row();
                return os;
            }

            // tabularizing indexable stuff
            template< typename ContainerT, typename ConfigT = default_pretty_config,
                std::enable_if_t<
                    is_indexable_t<ContainerT>::value &&
                    !is_string_t<ContainerT>::value &&
                    !is_value_grid_t<ContainerT>::value, int> = 0 >
            std::ostream& table_print(bool first_row_titles, std::ostream& os, const ContainerT& container) {
                os << ConfigT::manipulators;
                size_t row = 0;
                const std::string col_1_title("Index");
                const std::string col_2_title("Value");
                const size_t l_width = first_row_titles?std::max(longest_key(container), col_1_title.size()):longest_key(container);
                const size_t r_width = first_row_titles ? std::max(longest_value(container), col_2_title.size()) : longest_value(container);
                if (first_row_titles) {
                    size_t total_length = l_width + r_width + 3;
                    std::string line(total_length, ConfigT::title_row_separator()[0]);
                    os << std::left << std::setw(l_width) << col_1_title 
                        << " " << ConfigT::column_separator() << " " 
                        << std::left << std::setw(r_width) << col_2_title 
                        << ConfigT::new_row() << line << ConfigT::new_row();
                }
                for (auto it = container.begin(); it != container.end(); ++it, ++row) {
                    os << ConfigT::manipulators << std::left << std::setw(l_width);
                    os << row << " " << ConfigT::column_separator() << " ";
                    os << ConfigT::manipulators << std::right << std::setw(r_width);
                    pretty_print<typename ContainerT::value_type, typename ConfigT::inner_container_config_t>(os, *it);
                    os << ConfigT::new_row();
                }
                return os;
            }

            // tabularizing stacks and priority queues
            template< typename ContainerT, typename ConfigT = default_pretty_config,
                typename std::enable_if_t<
                    is_stack_t<ContainerT>::value ||
                    is_priority_queue_t<ContainerT>::value, int> = 0 >
            std::ostream & table_print(bool first_row_titles, std::ostream & os, const ContainerT & container) {
                ContainerT container_copy(container);
                std::vector<typename ContainerT::value_type> tmp;
                while (!container_copy.empty()) {
                    tmp.push_back(container_copy.top());
                    container_copy.pop();
                }
                table_print(first_row_titles, os, tmp);
                return os;
            }

            // tabularizing queues
            template< typename ContainerT, typename ConfigT = default_pretty_config,
                typename std::enable_if_t<
                    is_queue_t<ContainerT>::value, int> = 0 >
            std::ostream& table_print(bool first_row_titles, std::ostream& os, const ContainerT& container) {
                ContainerT container_copy(container);
                std::vector<typename ContainerT::value_type> tmp;
                while (!container_copy.empty()) {
                    tmp.push_back(container_copy.front());
                    container_copy.pop();
                }
                table_print(first_row_titles, os, tmp);
                return os;
            }

            // tabularizing maps
            template< typename ContainerT, typename ConfigT = default_pretty_config,
                typename std::enable_if_t<
                    is_mapish_t<ContainerT>::value, int> = 0 >
            std::ostream& table_print(bool first_row_titles, std::ostream& os, const ContainerT& container) {
                const std::string col_1_title("Key");
                const std::string col_2_title("Value");
                const size_t l_width = first_row_titles ? std::max(longest_key(container), col_1_title.size()) : longest_key(container);
                const size_t r_width = first_row_titles ? std::max(longest_value(container), col_2_title.size()) : longest_value(container);
                if (first_row_titles) {
                    size_t total_length = l_width + r_width + 3;
                    std::string line(total_length, ConfigT::title_row_separator()[0]);
                    os << std::left << std::setw(l_width) << col_1_title
                        << " " << ConfigT::column_separator() << " "
                        << std::left << std::setw(r_width) << col_2_title
                        << ConfigT::new_row() << line << ConfigT::new_row();
                }
                for (auto it = container.begin(); it != container.end(); ++it) {
                    os << ConfigT::manipulators << std::left << std::setw(l_width);
                    os << it->first << " " << ConfigT::column_separator() << " ";
                    os << ConfigT::manipulators << std::right << std::setw(r_width);
                    pretty_print<typename ContainerT::mapped_type, typename ConfigT::inner_container_config_t>(os, it->second);
                    os << ConfigT::new_row();
                }
                return os;
            }



            // tabularizing sequence of sequence of strings
            template< typename ContainerT, typename ConfigT = default_pretty_config,
                typename std::enable_if_t<
                    is_value_grid_t<ContainerT>::value, int> = 0 >
            std::ostream& table_print(bool first_row_titles, std::ostream& os, const ContainerT& container) {
                // we're going to do this in two passes, I don't think that can be avoided
                // first pass to figure out the number of columns and their width
                // second to do the actual work
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
                            os << " " << ConfigT::grid_column_separator() << " ";
                        ++col;
                    }
                    // pad missing columns
                    for (; col < column_widths.size(); ++col) {
                        std::string empty_cell(column_widths[col], ' ');
                        os << empty_cell;
                        if (col < column_widths.size() - 1)
                            os << " " << ConfigT::grid_column_separator() << " ";
                    }
                    os << ConfigT::new_row();
                    if (first_row_titles && row_i == 0) {
                        size_t total_length = std::accumulate(column_widths.begin(), column_widths.end(), size_t(0)) +
                            3 * column_widths.size();
                        std::string line(total_length, ConfigT::title_row_separator()[0]);
                        os << line << ConfigT::new_row();
                    }
                    ++row_i;
                }
                return os;
            }



            // functor to invoke tabularization on its only member
            template< typename ContainerT, typename ConfigT = default_pretty_config>
            struct table_t {
                ContainerT const& _container;
                bool _first_row_titles;
                table_t(ContainerT const& container, bool first_row_titles = false) :
                    _container(container),
                    _first_row_titles(first_row_titles){}
                std::ostream& operator()(std::ostream& out) const {
                    return table_print<ContainerT, ConfigT>(_first_row_titles, out, _container);
                }
                std::string str() const {
                    std::stringstream s;
                    operator()(s);
                    return s.str();
                }
            };
        }


        // pretty functions works as stream modifiers
        template< typename ContainerT>
        std_pretty_printing_impl::pretty_t<ContainerT>
            pretty(ContainerT const& container) { 
                return std_pretty_printing_impl::pretty_t<ContainerT>(container); 
        }

        template< typename ContainerT, typename ConfigT >
        std_pretty_printing_impl::pretty_t<ContainerT, ConfigT>
            pretty(ContainerT const& container, ConfigT const& config) { 
                return std_pretty_printing_impl::pretty_t<ContainerT, ConfigT>(container); 
        }

        // streaming of prettified containers
        template< typename ContainerT, typename ConfigT>
        std::ostream& operator<<(std::ostream& out, std_pretty_printing_impl::pretty_t<ContainerT, ConfigT> const& p) {
            return p(out);
        }

        // prettifying into strings
        template< typename ContainerT>
        std::string pretty_s(ContainerT const& container) {
            std::stringstream out;
            out << pretty(container);
            return out.str();
        }

        template< typename ContainerT, typename ConfigT >
        std::string pretty_s(ContainerT const& container, ConfigT const& config) {
            std::stringstream out;
            out << pretty(container, config);
            return out.str();
        }

        // alternative configuration for pretty printing, one value per row
        using one_per_row = std_pretty_printing_impl::one_per_row;

        // simple table format - optimised for size, rather than speed
        // it traverses the container two or three times to figure out the column sizes
        // could be slow for large containers, just find something better for that

        // table functions works as stream modifiers
        template< typename ContainerT>
        std_pretty_printing_impl::table_t<ContainerT>
            table(ContainerT const& container, bool first_row_titles = false) {
                return std_pretty_printing_impl::table_t<ContainerT>(container, first_row_titles); 
        }

        template< typename ContainerT, typename ConfigT >
        std_pretty_printing_impl::table_t<ContainerT, ConfigT>
            table(ContainerT const& container, ConfigT const& config, bool first_row_titles = false) {
                return std_pretty_printing_impl::table_t<ContainerT, ConfigT>(container, first_row_titles); 
        }

        // streaming of tabularized containers
        template< typename ContainerT, typename ConfigT>
        std::ostream& operator<<(std::ostream& out, std_pretty_printing_impl::table_t<ContainerT, ConfigT> const& p) {
            return p(out);
        }

        // tabularizing into strings
        template< typename ContainerT>
        std::string table_s(ContainerT const& container, bool first_row_titles = false) {
            std::stringstream out;
            out << table(container, first_row_titles);
            return out.str();
        }

        template< typename ContainerT, typename ConfigT >
        std::string table_s(ContainerT const& container, ConfigT const& config, bool first_row_titles = false) {
            std::stringstream out;
            out << table(container, config, first_row_titles);
            return out.str();
        }
    }
}
