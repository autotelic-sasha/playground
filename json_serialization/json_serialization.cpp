#include <iostream>
// #define _AF_JSON_OPTIMISED true
// #define _AF_JSON_OPTIMISED_STRING_MAPS
// #define _AF_JSON_OPTIMISED_DEFAULT_INITIALISATION
// #define _AF_JSON_VALIDATE_DUPLICATE_KEYS
// #define _AF_JSON_TERSE 
#include "experiments_2.h"

using namespace autotelica::serialization;

struct test1 : public af_serializable {
    int i;
    test1(int i_) :i(i_) {}
    bool operator==(test1 const& rhs) const {
        return i == rhs.i;
    }
    object_description_p serialization() override {
        return object_description(*this).
            member("i", i,2510).
            end_object();
    }
};

int main()
{
    std::shared_ptr<test1> t{ new test1(1991) };
    auto js = json::writer<>::to_string(t);
    std::cout << js << std::endl;
}

