#include <iostream>
#include "{{libname}}_api.h"

int main()
{
    std::cout << add(2,3) << std::endl;
    std::cout << mul(2,3) << std::endl;
    adder a(5, 6);
    std::cout << a.sum() << std::endl;
    std::cout << a.sum_product(2) << std::endl;
}

