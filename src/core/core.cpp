#include "core.h"

void assert_vulkan_result(const vk::Result& result, const std::string& failure_message)
{
    if (result != vk::Result::eSuccess)
    {
        std::cout << "=========== Vulkan API call failed: '" << failure_message << "', result: '" << vk::to_string(result) << "'." << std::endl;

        throw std::runtime_error(failure_message.c_str());
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
#endif

    return str_value;
}

bool has_environment_variable(const std::string& name)
{
    return get_environment_variable(name).has_value();
}
