#ifndef CORE_H
#define CORE_H

#include <memory>
#include <vector>
#include <string>
#include <cstdlib>
#include <cstdint>
#include <iostream>
#include <thread>
#include <chrono>
#include <set>
#include <optional>
#include <fstream>
#include <semaphore>

#include "log/log.h"

#include "concurrentqueue.h"
#include "blockingconcurrentqueue.h"

#include <libsdlplusplus/context.h>
#include <libsdlplusplus/window.h>

#ifdef WIN32
#include <windows.h>
#endif

#include <csignal>

#ifdef WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#else
#define VK_USE_PLATFORM_XLIB_KHR
#define VK_USE_PLATFORM_WAYLAND_KHR
#endif

#include <vulkan/vulkan.hpp>

#ifndef NDEBUG
#define EVK_ASSERT_RESULT(result, failure_message) ::assert_vulkan_result((result), (failure_message))
#else
#define EVK_ASSERT_RESULT(result, failure_message) ((void)result)
#endif

template <typename Dst, typename Src>
const Dst* vk_ptr(Src& src)
{
    return &(src.operator const Dst & ());
}

void assert_vulkan_result(const vk::Result& result, const std::string& failure_message);

namespace vk
{

constexpr std::uint32_t QueueFamilyIgnored = VK_QUEUE_FAMILY_IGNORED;
constexpr std::uint32_t RemainingArrayLayers = VK_REMAINING_ARRAY_LAYERS;
constexpr std::uint32_t RemainingMipLevels = VK_REMAINING_MIP_LEVELS;

}

inline void vulkantesting_debug_break()
{
#ifdef WIN32
    if (IsDebuggerPresent())
    {
        _CrtDbgBreak();
    }
#else
    std::raise(SIGINT);
#endif
}

std::optional<std::string> get_environment_variable(const std::string& name);

bool has_environment_variable(const std::string& name);

class engine;
class sdl_window;

inline void throw_exception(std::string message)
{
    SPDLOG_CRITICAL("Exception thrown: '{}'.", message);

    throw std::runtime_error(message);
}

std::vector<char> read_file(const std::string& filename);

#endif
