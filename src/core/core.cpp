#include "core.h"

void assert_vulkan_result(const vk::Result& result, const std::string& failure_message)
{
    if (result != vk::Result::eSuccess)
    {
        throw_exception(fmt::format("Vulkan API call failed: '{}', result = '{}'.", failure_message, vk::to_string(result)));
    }
}

std::optional<std::string> get_environment_variable(const std::string& name)
{
    std::string str_value;

#ifdef WIN32
    constexpr std::size_t buf_size = 2048;

    char value[buf_size];

    auto size = GetEnvironmentVariable(name.c_str(), value, buf_size);

    if (size >= buf_size || size == 0)
    {
        return std::nullopt;
    }

    str_value.assign(value, size);
#else
    auto* value = getenv(name.c_str());

    if (value == nullptr)
    {
        return std::nullopt;
    }

    str_value = value;
#endif

    return str_value;
}

bool has_environment_variable(const std::string& name)
{
    return get_environment_variable(name).has_value();
}
