#pragma once
#include <array>
#include <vector>
#include <deque>
#include <forward_list>
#include <list>

#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>

#include <stack>
#include <queue>
#include <string>
#include <initializer_list>

#include <type_traits>

namespace autotelica {
    namespace std_disambiguation {
        // SFINAE based type detection for std types
        // the core idea for this is addapted from https://stackoverflow.com/questions/35293470/checking-if-a-type-is-a-map 
        // they ain't perfect - but will work in any useful case I can think of

        // Implements a bunch of predicates in the style of std for std containers and such
        // they all follow the same pattern:
        // is_SOMETHING_t<T>::value is meant to be used for SFINAE type disambiguation, for example with std::enable_if_t
        // is_SOMETHING_f<T>(T const& t) is a constexpr function that checks if t satisfies the predicate 
        // 
        // is_pair_t, is_pair_f
        // is_string_t, is_string_f
        // is_vector_t, is_vector_f
        // is_deque_t, is_deque_f
        // is_forward_list_t, is_forward_list_f
        // is_list_t, is_list_f
        //
        // is_set_t, is_set_f
        // is_map_t, is_map_f
        // is_unordered_set_t, is_unordered_set_f
        // is_unordered_map_t, is_unordered_map_f
        //
        // is_stack_t, is_stack_f
        // is_queue_t, is_queue_f
        //
        // ... then, there are some that group these together
        // is_sequence_t, is_sequence_f - sequence containers: vector, deque, forward_list, list
        // is_associative_t, is_associative_f - associative containers: 
        //      set, multiset, map, multimap, unordered_set, unordered_multiset, unordered_map, unordered_multimap
        // is_adaptor_t, is_adaptor_f - adaptors: stack, queue, priority_queue
        // is_setish_t, is_setish_f - set like stuff: set, multiset, unordered_set, unordered_multiset
        // is_mapish_t, is_mapish_f - map like stuff: map, multimap, unordered_map, unordered_multimap
        // is_indexable_t, is_indexable_f - things that can be easily indexed 
        //      vector, deque, forward_list, list, set, multiset, unordered_set, unordered_multiset, string
        // is_non_indexable_t, is_non_indexable_f - things that are harder to index 
        //      map, multimap, unordered_map, unordered_multimap, stack, queue, priority_queue
        // is_container_t, is_container_f - any of the above
        //
        // NOTE: these may fail for polymorphic allocator version in c++ 17
        //       it's a simple extension, a TODO I guess

        // Trickery to make use of SFINAE - can be instantiated only if the listed types exist
        // C++ 17 has this built in (and calls it std::void_t)     
        template<typename ... Ts>
        using enable_if_types_exist = void;


        // pair
        template<typename T, typename U = void>
        struct is_pair_impl : std::false_type {};

        template<typename T>
        struct is_pair_impl<T, enable_if_types_exist<typename T::first_type, typename T::second_type>> : std::true_type {};

        template<typename T>
        struct is_pair_t : is_pair_impl<T>::type {};

        template<typename T>
        constexpr bool is_pair_f(T const& t) { return is_pair_t<T>(); }

        // string, wstring
        template<typename T, typename U = void>\
            struct is_string_impl : std::false_type {};

        template<typename T>
        struct is_string_impl<T, std::enable_if_t<std::is_same< T, 
            std::basic_string<typename T::value_type, typename T::traits_type, typename T::allocator_type>>::value>> : std::true_type {};

        template<typename T>
        struct is_string_t : is_string_impl<T>::type {};

        template<typename T>\
            constexpr bool is_string_f(T const& t) { return is_string_t<T>(); }


        // initializer_list 
        template<typename T, typename U = void>\
            struct is_initializer_list_impl : std::false_type {};

        template<typename T>
        struct is_initializer_list_impl<T, std::enable_if_t<std::is_same< T, 
            std::initializer_list<typename T::value_type>>::value >> : std::true_type {};

        template<typename T>
        struct is_initializer_list_t : is_initializer_list_impl<T>::type {};

        template<typename T>\
            constexpr bool is_initializer_list_f(T const& t) { return is_string_t<T>(); }

#define GCC_PASTING_NAMEPSACE(L, R) L##::##R

#define AF_IMPL_IS_STD_TYPE(TYPE_, ...) \
        template<typename T, typename U = void>\
        struct is_##TYPE_## _impl : std::false_type {};\
        \
        template<typename T>\
        struct is_##TYPE_##_impl<T, std::enable_if_t<std::is_same< T, std:: TYPE_  <__VA_ARGS__>>::value>> : std::true_type {};\
        \
        template<typename T>\
        struct is_##TYPE_##_t : is_##TYPE_##_impl<T>::type {};\
        \
        template<typename T>\
        constexpr bool is_##TYPE_##_f(T const& t) { return is_##TYPE_##_t<T>(); }

        // TODO: std::array is missing
        AF_IMPL_IS_STD_TYPE(vector, typename T::value_type, typename T::allocator_type)
        AF_IMPL_IS_STD_TYPE(deque, typename T::value_type, typename T::allocator_type)
        AF_IMPL_IS_STD_TYPE(forward_list, typename T::value_type, typename T::allocator_type)
        AF_IMPL_IS_STD_TYPE(list, typename T::value_type, typename T::allocator_type)

        AF_IMPL_IS_STD_TYPE(set, typename T::key_type, typename T::key_compare, typename T::allocator_type)
        AF_IMPL_IS_STD_TYPE(multiset, typename T::key_type, typename T::key_compare, typename T::allocator_type)
        AF_IMPL_IS_STD_TYPE(map, typename T::key_type, typename T::mapped_type, typename T::key_compare, typename T::allocator_type)
        AF_IMPL_IS_STD_TYPE(multimap, typename T::key_type, typename T::mapped_type, typename T::key_compare, typename T::allocator_type)

        AF_IMPL_IS_STD_TYPE(unordered_set, typename T::key_type, typename T::hasher, typename T::key_equal, typename T::allocator_type)
        AF_IMPL_IS_STD_TYPE(unordered_multiset, typename T::key_type, typename T::hasher, typename T::key_equal, typename T::allocator_type)
        AF_IMPL_IS_STD_TYPE(unordered_map, typename T::key_type, typename T::mapped_type, typename T::hasher, typename T::key_equal, typename T::allocator_type)
        AF_IMPL_IS_STD_TYPE(unordered_multimap, typename T::key_type, typename T::mapped_type, typename T::hasher, typename T::key_equal, typename T::allocator_type)


        AF_IMPL_IS_STD_TYPE(stack, typename T::value_type, typename T::container_type)
        AF_IMPL_IS_STD_TYPE(queue, typename T::value_type, typename T::container_type)
        AF_IMPL_IS_STD_TYPE(priority_queue, typename T::value_type, typename T::container_type, typename T::value_compare)

#define AF_IMPL_IS_STD_KIND(NAME_, CONDITION_) \
            template<typename T, typename U = void>\
            struct is_ ## NAME_ ## _impl : std::false_type {};\
            \
            template<typename T> \
            struct is_ ## NAME_ ## _impl < T,\
                std::enable_if_t < CONDITION_ >> : std::true_type {};\
            \
            template<typename T>\
            struct is_ ## NAME_ ## _t : is_ ## NAME_ ## _impl<T>::type {};\
            \
            template<typename T>\
            constexpr bool is_ ## NAME_ ## _f(T const& t) { return is_ ## NAME_ ## _t<T>(); }


        AF_IMPL_IS_STD_KIND(sequence, is_vector_t<T>::value || is_deque_t<T>::value || is_forward_list_t<T>::value || is_list_t<T>::value);
        AF_IMPL_IS_STD_KIND(setish, is_set_t<T>::value || is_multiset_t<T>::value || is_unordered_set_t<T>::value || is_unordered_multiset_t<T>::value);
        AF_IMPL_IS_STD_KIND(mapish, is_map_t<T>::value || is_multimap_t<T>::value || is_unordered_map_t<T>::value || is_unordered_multimap_t<T>::value);
        AF_IMPL_IS_STD_KIND(associative, is_setish_t<T>::value || is_mapish_t<T>::value);
        AF_IMPL_IS_STD_KIND(adaptor, is_stack_t<T>::value || is_queue_t<T>::value || is_priority_queue_t<T>::value);
        AF_IMPL_IS_STD_KIND(indexable, is_sequence_t<T>::value || is_setish_t<T>::value || is_string_t<T>::value);
        AF_IMPL_IS_STD_KIND(container, is_indexable_t<T>::value || is_mapish_t<T>::value || is_adaptor_t<T>::value);
        AF_IMPL_IS_STD_KIND(value_grid,
            is_sequence_t<T>::value&& is_sequence_t<typename T::value_type>::value &&
            (is_string_t<typename T::value_type::value_type>::value ||
                !is_container_t<typename T::value_type::value_type>::value));
    }
}