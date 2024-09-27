#include <iostream>
// #define _AF_JSON_OPTIMISED true
// #define _AF_JSON_OPTIMISED_STRING_MAPS
#define _AF_JSON_OPTIMISED_DEFAULT_INITIALISATION false
// #define _AF_JSON_VALIDATE_DUPLICATE_KEYS
// #define _AF_JSON_TERSE 
#include "experiments_2.h"

using namespace autotelica::serialization;

struct test1 : public af_serializable {
    int i;
    object_description_p _description;

    test1(int i_ = 0) :i(i_) {}
    bool operator==(test1 const& rhs) const {
        return i == rhs.i;
    }
    object_description_p serialization() override {
        if (!_description) {
            _description = object_description(*this).
                member("i", i, 2510).
                end_object();
        }
        return _description;
    }
    static type_description_t<test1>& type_description() {
        static type_description_t<test1> description = 
            type_description_t<test1>().
                member("i", &test1::i, 2510).
            end_object();
        return description;
    }
};
struct test2  {
    int i;
    object_description_p _description;

    test2(int i_ = 0) :i(i_) {}
    bool operator==(test2 const& rhs) const {
        return i == rhs.i;
    }
    static type_description_t<test2>& type_description() {
        static type_description_t<test2> description =
            type_description_t<test2>().
            member("i", &test2::i, 2510).
            end_object();
        return description;
    }
};
int main()
{
    test1 t(1991);
    auto js = json::writer<>::to_string(t);
    test1 t2, t3;
    std::cout << js << std::endl;
    json::reader<>::from_string(t2, js);
    std::cout << js << std::endl;
    json::reader<>::from_string(t3, "{}");
    std::cout << json::writer<>::to_string(t3) << std::endl;

    test2 t_2(1991);
    js = json::writer<>::to_string(t_2);
    test2 t_22, t_23;
    std::cout << js << std::endl;
    json::reader<>::from_string(t_22, js);
    std::cout << js << std::endl;
    json::reader<>::from_string(t_23, "{}");
    std::cout << json::writer<>::to_string(t_23) << std::endl;
}

