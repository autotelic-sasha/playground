#pragma once

#define STRINGIFY_VA_ARGS_0(...) 
#define STRINGIFY_VA_ARGS_1(a) #a
#define STRINGIFY_VA_ARGS_2(a,...)  #a ", " STRINGIFY_VA_ARGS_1(__VA_ARGS__)
#define STRINGIFY_VA_ARGS_3(a,...)  #a ", " STRINGIFY_VA_ARGS_2(__VA_ARGS__)
#define STRINGIFY_VA_ARGS_4(a,...)  #a ", " STRINGIFY_VA_ARGS_3(__VA_ARGS__)
#define STRINGIFY_VA_ARGS_5(a,...)  #a ", " STRINGIFY_VA_ARGS_4(__VA_ARGS__)
#define STRINGIFY_VA_ARGS_6(a,...)  #a ", " STRINGIFY_VA_ARGS_5(__VA_ARGS__)
#define STRINGIFY_VA_ARGS_7(a,...)  #a ", " STRINGIFY_VA_ARGS_6(__VA_ARGS__)
#define STRINGIFY_VA_ARGS_8(a,...)  #a ", " STRINGIFY_VA_ARGS_7(__VA_ARGS__)
#define STRINGIFY_VA_ARGS_9(a,...)  #a ", " STRINGIFY_VA_ARGS_8(__VA_ARGS__)
#define STRINGIFY_VA_ARGS_10(a,...) #a ", " STRINGIFY_VA_ARGS_9(__VA_ARGS__)
#define STRINGIFY_VA_ARGS_11(a,...) #a ", " STRINGIFY_VA_ARGS_10(__VA_ARGS__)
#define STRINGIFY_VA_ARGS_12(a,...) #a ", " STRINGIFY_VA_ARGS_11(__VA_ARGS__)
#define STRINGIFY_VA_ARGS_13(a,...) #a ", " STRINGIFY_VA_ARGS_12(__VA_ARGS__)

#define STRINGIFY_VA_ARGS_N(_13, _12, _11, _10, _9, _8,_7,_6, _5,_4,_3,_2,_1, N, ...) STRINGIFY_VA_ARGS##N

#define STRINGIFY_VA_ARGS(...) STRINGIFY_VA_ARGS_N(__VA_ARGS__,_13, _12, _11, _10, _9, _8,_7,_6, _5,_4,_3,_2,_1 )(__VA_ARGS__)


// create a variable name with __LINE__ appended to the name, weird and ugly
#define NAME_WITH_LINE(s) NAME_WITH_LINEXX(s, __LINE__)
#define NAME_WITH_LINEXX(s, l) NAME_WITH_LINEX(s, l)
#define NAME_WITH_LINEX(s, l) s ## l


