// code_templates.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#include <iostream>
#include <experimental/filesystem>
#include "autotelica_core/util/string_util.h"
#include <map>

using path_t = std::experimental::filesystem::path;
namespace filesystem_n = std::experimental::filesystem;

class mintl {
    std::map<std::string, std::string> _kvm;
    std::string _source_path_str;
    std::string _target_path_str;
public:
    mintl(
        std::string source_path_str_,
        std::string target_path_str_,
        std::map<std::string, std::string> kvm_){
    }
};


void check_paths(path_t const& source_folder, path_t const& target_folder) {
    if (autotelica::string_util::to_lower(target_folder.string()).find(autotelica::string_util::to_lower(source_folder.string())) != std::string::npos)
        throw std::runtime_error("Target path cannot be contained within source path.");
}
path_t make_new_path(path_t const& source_path, path_t const& source_folder, path_t const& target_folder) {
    path_t new_path(
        autotelica::string_util::replace(source_path.string(), source_folder.string(), target_folder.string()));
    return new_path;
}
void create_target(path_t const& source_path, path_t const& target_path) {
    if (filesystem_n::is_directory(source_path))
        filesystem_n::create_directories(target_path);
    else
        filesystem_n::copy(source_path, target_path);
}

int main()
{
    auto const source = path_t("C:\\dev\\playing\\share_libraries_template\\shared_library_template").make_preferred();
    auto const target = path_t("C:\\dev\\playing\\share_libraries_template\\out").make_preferred();
    //if (filesystem_n::exists(target))
    //    throw std::runtime_error("Target directory already exists.");

    //filesystem_n::create_directories(target);
    check_paths(source, target);
    using recursive_directory_iterator = filesystem_n::recursive_directory_iterator;
    for (const auto& entry : recursive_directory_iterator(source)) {
        try {
            std::cout << "=======================" << std::endl;
            std::cout << "READING:" << entry << std::endl;
            auto new_path = make_new_path(entry, source, target);
            create_target(entry, new_path);
            std::cout << "CREATED:" << new_path << std::endl;
        }
        catch (std::exception const& e) {
            std::cout << "ERROR: " << e.what() << std::endl;
        }
    }
    
    std::cout << "Hello World!\n";
}


