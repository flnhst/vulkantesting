#ifndef ENGINE_H
#define ENGINE_H

#include "core/core.h"

struct physical_device_info
{
    vk::PhysicalDevice physical_device;

    vk::PhysicalDeviceProperties2 properties;
    vk::PhysicalDeviceFeatures2 features;
    std::vector<vk::QueueFamilyProperties2> queue_families;

    std::vector<uint32_t> graphics_family_queue_indices_;
    std::vector<uint32_t> transfer_family_queue_indices_;

    std::vector<uint32_t> present_family_queue_indices_;
};

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
    void enumerate_physical_devices_();
    void create_surface_();
    void select_physical_device_();
    void create_device_();
    void retrieve_queues_();

    void destroy_device_();
    void destroy_surface_();
    void destroy_debug_utils_ext_();
    void destroy_instance_();
    void destroy_sdl_window_();

    bool is_physical_device_suitable_(const physical_device_info& p_physical_device_info);

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

    std::vector<physical_device_info> physical_device_infos_;

    const physical_device_info* selected_physical_device_info_{ nullptr };

    vk::Device device_{ nullptr };

    std::uint32_t graphics_queue_family_index_{ 0 };
    vk::Queue graphics_queue_{ nullptr };
    std::uint32_t present_queue_family_index_{ 0 };
    vk::Queue present_queue_{ nullptr };

    vk::SurfaceKHR surface_{ nullptr };

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
