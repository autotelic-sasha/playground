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
/*struct test1 : public af_serializable {
    int i;
    object_description_p _description;

    test1(int i_ = 0) :i(i_) {}
    bool operator==(test1 const& rhs) const {
        return i == rhs.i;
    }
    object_description_p object_description() override {
        if (!_description) {
            _description = begin_object(*this).
                member("i", i, 2510).
                end_object();
        }
        return _description;
    }
};*/
struct test2  {
    int i;
    double d;
    std::vector<int> ints;
    json_handler_p<test2> _handler_cache;

    test2(
        int i_ = 0, 
        double d_ = 0,
        std::vector<int> const& ints_ = {7,8}) :
            i(i_), d(d_), ints(ints_), _handler_cache(nullptr){}
    bool operator==(test2 const& rhs) const {
        return i == rhs.i;
    }

    template<typename serialization_factory_t>
    static type_description_t<test2, serialization_factory_t>  const& type_description() {
        static auto description = 
            begin_object<test2, serialization_factory_t>().
                member("i", &test2::i, 2510).
                member("d", &test2::d, 1.991).
                member("ints", &test2::ints, {1,2,3}).
            end_object();
        return description;
    }

    AF_IMPLEMENTS_TYPE_DESCRIPTION_FACTORY;

    //virtual type_description_factory_p type_description_factory() {
    //    return make_type_description_factory(*this);
    //}

    virtual json_handler_p<test2> json_handler(
            default_p<test2> default_ = nullptr) {

        if (!_handler_cache)
            _handler_cache = make_json_handler_from_type_description(
                    this, 
                    default_, 
                    type_description<impl::serialization_factory>());
        else // TODO: if you don't do weird stuff, this is probably unnecessary, defaults should be per instance
            _handler_cache->set_default(default_);

        return _handler_cache;
    }

};

int main()
{
    try {
        //test1 t(1991);
        //auto js = json::writer<>::to_string(t);
        //test1 t2, t3;
        //std::cout << js << std::endl;
        //json::reader<>::from_string(t2, js);
        //std::cout << js << std::endl;
        //json::reader<>::from_string(t3, "{}");
        //std::cout << json::writer<>::to_string(t3) << std::endl;*/
        //autotelica::serialization::traits::default_p<double> a;
        //bool is = std::is_function<decltype(test2::type_description)>::value;
        test2 t_2(1991);

        //bool is = autotelica::serialization::util::predicates::is_serializable_object_t<test2>::value;
        //bool is2 = autotelica::serialization::util::predicates::has_object_description_t<test2>::value;
        //
        auto js = autotelica::json::writer<>::to_string(t_2);
        test2 t_22, t_23;
        std::cout << js << std::endl;
        autotelica::json::reader<>::from_string(t_22, js);
        std::cout << js << std::endl;
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

