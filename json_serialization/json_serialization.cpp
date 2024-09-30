#include <iostream>
// #define _AF_JSON_OPTIMISED true
// #define _AF_JSON_OPTIMISED_STRING_MAPS
#define _AF_JSON_OPTIMISED_DEFAULT_INITIALISATION false
// #define _AF_JSON_VALIDATE_DUPLICATE_KEYS
// #define _AF_JSON_TERSE 
#include "experiments_2.h"

using namespace autotelica::serialization;

//struct test1 : public af_serializable {
//    int i;
//    object_description_p _description;
//
//    test1(int i_ = 0) :i(i_) {}
//    bool operator==(test1 const& rhs) const {
//        return i == rhs.i;
//    }
//    object_description_p object_description() override {
//        if (!_description) {
//            _description = begin_object(*this).
//                member("i", i, 2510).
//                end_object();
//        }
//        return _description;
//    }
//};
struct test2  {
    int i;
    double d;
    json::handler_p _json_handler_cached;
    json::default_value_p _default_value_cached;

    test2(int i_ = 0, double d_ = 0) :i(i_), d(d_) {}
    bool operator==(test2 const& rhs) const {
        return i == rhs.i;
    }
    static type_description_t const& type_description() {
        static auto description = 
            begin_object<test2>().
                member("i", &test2::i, 2510).
                member("d", &test2::d, 1991.1025).
            end_object();
        return description;
    }
    inline object_description_p object_description() {
        return type_description().for_object(*this);
    }

    //inline json::handler_p get_json_handler(
    //        json::default_value_p default_ = nullptr) {
    //    if (!_json_handler_cached || _default_value_cached != default_) {
    //        _json_handler_cached = object_description()->create_json_handler(default_);
    //        _default_value_cached = default_;
    //    }
    //    return _json_handler_cached;
    //}

};
int main()
{
    /*test1 t(1991);
    auto js = json::writer<>::to_string(t);
    test1 t2, t3;
    std::cout << js << std::endl;
    json::reader<>::from_string(t2, js);
    std::cout << js << std::endl;
    json::reader<>::from_string(t3, "{}");
    std::cout << json::writer<>::to_string(t3) << std::endl;*/

    test2 t_2(1991);
    auto js = json::writer<>::to_string(t_2);
    test2 t_22, t_23;
    std::cout << js << std::endl;
    json::reader<>::from_string(t_22, js);
    std::cout << js << std::endl;
    json::reader<>::from_string(t_23, "{}");
    std::cout << json::writer<>::to_string(t_23) << std::endl;
}

