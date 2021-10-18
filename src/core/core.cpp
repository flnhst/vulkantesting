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

std::vector<char> read_file(const std::string& filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        throw_exception(fmt::format("Could not open file '{}'.", filename));
    }

    const std::size_t file_size = static_cast<std::size_t>(file.tellg());

    std::vector<char> buffer(file_size);

    file.seekg(0);

    file.read(buffer.data(), file_size);

    file.close();

    return buffer;
}
