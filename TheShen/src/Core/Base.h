#pragma once

#include <memory>
#include <vector>
#include "Core/Log.h"

#define SHEN_BIND_EVENT_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }

#define SHEN_CORE_ASSERT(x,...){if(!(x)) {SHEN_CORE_ERROR("Assertion Failed: {0}",__VA_ARGS__);__debugbreak();}} 
#define SHEN_CLIENT_ASSERT(x,...){if(!(x)) {SHEN_CLIENT_ERROR("Assertion Failed: {0}",__VA_ARGS__);__debugbreak();}} 

#define BIT(x) (1 << x)

template<typename T>
using Scope = std::unique_ptr<T>;
template<typename T, typename ... Args>
constexpr Scope<T> CreateScope(Args&& ... args)
{
	return std::make_unique<T>(std::forward<Args>(args)...);
}

template<typename T>
using Ref = std::shared_ptr<T>;
template<typename T, typename ... Args>
constexpr Ref<T> CreateRef(Args&& ... args)
{
	return std::make_shared<T>(std::forward<Args>(args)...);
}