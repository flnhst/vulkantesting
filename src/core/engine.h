#ifndef ENGINE_H
#define ENGINE_H

#include "core/core.h"

class engine
{
public:
    engine();
    ~engine();

    void initialize();

    void destroy();

    int run();

    void stop();

    bool main_loop_running() const { return main_loop_running_; }

private:
    void main_loop_();

    void create_sdl_window_();
    void create_instance_();
    void create_debug_utils_ext_();
    void select_physical_device_();

    void destroy_debug_utils_ext_();
    void destroy_instance_();
    void destroy_sdl_window_();

    bool is_physical_device_suitable_(vk::PhysicalDevice physical_device);

    std::unique_ptr<sdl_window> sdl_window_;

    bool main_loop_running_{ true };
    bool first_tick_{ true };
    std::uint64_t ticks_{ 0 };

    std::vector<const char*> instance_extensions_;
    std::vector<const char*> device_extensions_;
    std::vector<const char*> layers_;

    vk::Instance instance_{ nullptr };
    vk::DispatchLoaderDynamic dispatch_;

    int vulkan_major_{ 1 };
    int vulkan_minor_{ 2 };
    int vulkan_patch_{ 168 };

    std::uint64_t messages_emitted_{ 0 };
    vk::DebugUtilsMessengerEXT debug_utils_messenger_{ nullptr };

    vk::PhysicalDevice physical_device_{ nullptr };
    std::uint32_t queue_family_index_{ 0 };

    friend VkBool32 messenger_callback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageTypes,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData);
};

VkBool32 messenger_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData);

#endif
