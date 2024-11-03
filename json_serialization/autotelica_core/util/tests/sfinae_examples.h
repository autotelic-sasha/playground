#pragma once
#include "testing_util.h"
#include "sfinae_util.h"

namespace autotelica {
    namespace examples {
        namespace sfinae { 
            using namespace autotelica::sfinae;
            // simple_type_switch function implement static polymorphism
            // the logic filters the overload version based on type predicates

            // a version to deal with c type strings
            template<typename T, if_cstring_t<T> = true>
            std::string simple_type_switch(T const t) {
                return "C STRING";
            }

            // a floating point number version
            template<typename T, if_floating_point_t<T> = true>
            std::string simple_type_switch(T const t) {
                return "FLOATING";
            }

            // a complex predicate that picks up some sequence types
            // it has to explicitly exclude floating points and c-string
            // to avoid ambiguity with the overloads above 
            // (because vectors and lists can be constructed in so many ways)
            template<typename T>
            using can_build_sequence_t =
                all_of_t<
                    not_t<std::is_floating_point<T>>,
                    not_t<is_cstring_t<T>>,
                    any_of_t<
                        is_initializer_list_t<T>,
                        std::is_constructible<std::vector<T>, T>,
                        std::is_constructible<std::list<T>, T>>>;

            template<typename T, if_t<can_build_sequence_t<T>> = true>
            std::string simple_type_switch(T const t) {
                return "SEQUENCE";
            }

            
            // a complex predicate to deal with default implementation
            // it just says "anything that is not covered by previously used predicates"
            // sadly, we have to list all the things excluded explicitly
            template<typename T>
            using anything_else_t =
                none_of_t<
                is_cstring_t<T>,
                std::is_floating_point<T>,
                can_build_sequence_t<T>
                >;

            template<typename T, if_t<anything_else_t<T>> = true>
            std::string simple_type_switch(T const t) {
                return "OTHER";
            }

            // How to use checks for members
            struct A {
                static double static_f(double d) { return d + 1; }

                double member_f(double d1, double d2) { return d1 + d2; }

                template<typename T>
                T templated_f1(T t1) { return ++t1; }

                template<typename T1, typename T2 >
                static T1 templated_f2(T1 t1, T2 t2) { return t1 + t2; }

            };
            _AF_DECLARE_HAS_MEMBER(static_f);
            _AF_DECLARE_HAS_MEMBER(member_f);
            _AF_DECLARE_HAS_MEMBER(nonexistent_f);
            _AF_DECLARE_HAS_MEMBER(templated_f1, int);
            _AF_DECLARE_HAS_MEMBER(templated_f2, int, int);
            _AF_DECLARE_HAS_MEMBER(templated_f3, int);

            template< bool = true> // declaring it as a template is a way to work aroud c++ limitations about declaring things in headers
            void examples() {

            }
            template< bool = true> // declaring it as a template is a way to work aroud c++ limitations about declaring things in headers
            void tests() {
                AF_TEST_RESULT("FLOATING", simple_type_switch(0.109));
                AF_TEST_RESULT("C STRING", simple_type_switch("hello"));
                auto t = { 1,2,3 };
                AF_TEST_RESULT("SEQUENCE", simple_type_switch(t));
                std::map<int, int> m = { {1,2},{3,4} };
                AF_TEST_RESULT("OTHER", simple_type_switch(m));
                AF_TEST_RESULT(true, has_static_f_t<A>::value);
                AF_TEST_RESULT(true, has_member_f_t<A>::value);
                AF_TEST_RESULT(false, has_nonexistent_f_t<A>::value);
                AF_TEST_RESULT(true, has_templated_f1_t<A>::value);
                AF_TEST_RESULT(true, has_templated_f2_t<A>::value);
                AF_TEST_RESULT(false, has_templated_f3_t<A>::value);

            }
        }
    }
}

AF_DECLARE_TEST_SET("SFINAE", sfinae, autotelica::examples::sfinae::examples<>() , autotelica::examples::sfinae::tests<>());

