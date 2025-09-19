#pragma once
#include <format>
#include <iostream>

#define log_message(msg, ...) std::cout << std::format(msg, __VA_ARGS__)

#ifdef _DEBUG
#	define log_message_debug(msg, ...) std::cout << std::format(msg, __VA_ARGS__)
#else
#	define log_message_debug(msg, ...)
#endif

#ifdef assert
#   undef assert
#endif

//You cannot possibily have a good library without this
#define WRITE_TO_NULLPTR() *(int*)nullptr = 69420

#define assert(x, msg) if(!(x)) {\
        log_message("[ASSERTION FAILED]: in file: {}, on line: {}, msg: {}", __FILE__, __LINE__, msg);\
    	WRITE_TO_NULLPTR();\
    }

#ifndef defer

//If defer is already defined, the implementation should be very similar

template<class T>
struct Callable
{
    Callable(T&& _func) : func(_func) {}
    ~Callable() {func();}

    const T func;
};

//This exist as a mean to provide the clean defer call defer{}; without
//having to put up with multiple closing bracklets
struct MakeCallable
{
    template<class T>
    Callable<T> operator<<(T&& lambda)
    {
        return Callable<T>(std::forward<T>(lambda));
    }
};

inline MakeCallable __make_callable_util;

#define STRINGIFY(x) #x
#define CONCAT(a, b) a##b
#define CONCAT_WITH(a, b) CONCAT(a, b)
#define defer auto CONCAT_WITH(callable_, __LINE__) = __make_callable_util << [&]()

#endif
