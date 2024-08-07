#include "{{libname}}_api.h"

int add(int i, int j) {
	return i + j;
}
int subtract(int i, int j) {
	return i - j;
}
int mul(int i, int j){
	return i*j;
}
adder::adder(int i_, int j_) :_i(i_), _j(j_) {}
int adder::sum_product(int k) { return k * sum(); }
