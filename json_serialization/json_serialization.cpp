#define _CRT_SECURE_NO_WARNINGS 

#include <iostream>
#define _AF_JSON_OPTIMISED true
// #define _AF_JSON_OPTIMISED_STRING_MAPS
// #define _AF_JSON_OPTIMISED_DEFAULT_INITIALISATION false
// #define _AF_JSON_VALIDATE_DUPLICATE_KEYS
// #define _AF_SERIALIZATION_TERSE 
#include "json_serialization.h"

using namespace autotelica::type_description;
using namespace autotelica::json;

struct test1 {
    std::vector<std::string> _strings;
    std::map<int, std::string> _map;
    std::map<std::string, std::string> _string_map;

    template<typename serialization_factory_t>
    static type_description_t<serialization_factory_t>  const& type_description() {
        static const auto description =
            begin_object<test1, serialization_factory_t>("test1", 2).
                member("strings", &test1::_strings, {"one", "two","three"}).
                member("map", &test1::_map, { {1,"one"}, {2,"two"} }).
                member("string_map", &test1::_string_map, { {"one","1"}, {"two","2"} }).
            end_object();
        return description;
    }

};

struct test2 {
    int i;
    double d;
    std::vector<int> ints;
    std::vector<std::list<int>> intsints;

    test2(
        int i_ = 0, 
        double d_ = 0,
        std::vector<int> const& ints_ = {7,8}) :
            i(i_), d(d_), ints(ints_){}
    bool operator==(test2 const& rhs) const {
        return i == rhs.i;
    }


    // TODO: test how inheritance will work
    template<typename serialization_factory_t>
    static type_description_t<serialization_factory_t>  const& type_description() {
        static const auto description =
            begin_object<test2, serialization_factory_t>("test2", 1).
                member("i", &test2::i, 2510).
                member("d", &test2::d, 1.991).
                member("ints", &test2::ints, {1,2,3}).
                member("intsints", &test2::intsints, { { 1,2,3 }, { 4,5,6 } }).
            end_object();
        return description;
    }
    // DOCUMENT: one of these must be there for dynamically resolved types
    // best to have both (so that things other than JSON also work)
    AF_IMPLEMENTS_DYNAMIC_TYPE_DESCRIPTION;
    AF_IMPLEMENTS_JSON_HANDLER(test2);

};

int main()
{
    try {

        test1 t_1;
        std::cout << autotelica::json::writer<>::to_string(t_1) << std::endl;

        test2 t_2(1991);

        auto js2 = autotelica::json::writer<>::to_string(t_2);
        test2 t_22, t_23;
        std::cout << js2 << std::endl;
        autotelica::json::reader<>::from_string(t_22, js2);
        std::cout << js2 << std::endl;
        autotelica::json::reader<>::from_string(t_23, "{\"d\":2.0, \"ints\":null}");
        std::cout << autotelica::json::writer<>::to_string(t_23) << std::endl;
    }
    catch (std::runtime_error& e) {
        std::cout << "ERROR CAUGHT: " << e.what() << std::endl;
    }
    catch (...) {
        std::cout << "UNKNOWN ERROR CAUGHT " << std::endl;
    }
}

