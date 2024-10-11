#include <iostream>
// #define _AF_JSON_OPTIMISED true
// #define _AF_JSON_OPTIMISED_STRING_MAPS
// #define _AF_JSON_OPTIMISED_DEFAULT_INITIALISATION false
// #define _AF_JSON_VALIDATE_DUPLICATE_KEYS
// #define _AF_SERIALIZATION_TERSE 
#include "json_serialization.h"

using namespace autotelica::type_description;

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

    test2(int i_ = 0, double d_ = 0) :i(i_), d(d_) {}
    bool operator==(test2 const& rhs) const {
        return i == rhs.i;
    }
    static type_description_t const& type_description() {
        static auto description = 
            begin_object<test2>().
                member("i", &test2::i, 2510).
                member("d", &test2::d, 1.991).
            end_object();
        return description;
    }

    //inline json::handler_p get_json_handler(
    //        default_description_p default_ = nullptr) {
    //    if (!_json_handler_cached || _default_value_cached != default_) {
    //        _json_handler_cached = json_handlers_factory::make_object_handler(
    //            type_description(), *this, default_);
    //        _default_value_cached = default_;
    //    }
    //    return _json_handler_cached;
    //}

};

_AF_DECLARE_HAS_STATIC_MEMBER(type_description);

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

    //bool is = std::is_function<decltype(test2::type_description)>::value;

    bool is = has_static_type_description<test2>::value;
    test2 t_2(1991);
    //auto js = autotelica::json::writer<>::to_string(t_2);
    //test2 t_22, t_23;
    //std::cout << js << std::endl;
    //autotelica::json::reader<>::from_string(t_22, js);
    //std::cout << js << std::endl;
    //autotelica::json::reader<>::from_string(t_23, "{\"d\":2.0}");
    //std::cout << autotelica::json::writer<>::to_string(t_23) << std::endl;
}

