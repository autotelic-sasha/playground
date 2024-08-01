#pragma once
#include <map>
#include <memory>
#include <string>
#include <vector>
class constant_matrix;
using constant_matrix_p = std::shared_ptr<constant_matrix>;

class matrix_cache {
    static std::map<std::string, std::shared_ptr<constant_matrix>>& cache();
public:
    static void add(std::string const& name, constant_matrix_p const& matrix_);
    static void add(std::string const& name, std::string const& code);
    static constant_matrix_p get(std::string const& name);
    static constant_matrix_p find(std::string const& name);
    static void remove(std::string const& name);
    static std::string display_matrix(std::string const& name);
    static void clear();
    static std::vector<std::string> list();
};
inline constant_matrix_p get_matrix_from_cache(std::string const& name) { return matrix_cache::get(name); }

