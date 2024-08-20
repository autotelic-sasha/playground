#include "{{libname}}_api.h"

int {{libname}}_add(int i, int j) {
	return i + j;
}

{{libname}}_adder::{{libname}}_adder(int i_, int j_) :_i(i_), _j(j_) {}
int {{libname}}_adder::sum() { return {{libname}}_add( _i, _j); }
