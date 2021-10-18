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

struct swapchain_info
{
    vk::SurfaceCapabilities2KHR capabilities;
    std::vector<vk::SurfaceFormat2KHR> surface_formats;
    std::vector<vk::PresentModeKHR> present_modes;

    vk::SurfaceFormat2KHR chosen_surface_format;
    vk::PresentModeKHR chosen_present_mode;
    vk::Extent2D chosen_extent;
    std::uint32_t chosen_image_count;
};

struct swapchain_image
{
    vk::Image image;
    vk::ImageView image_view;
};

class engine
{
public:
    static constexpr int VULKAN_MAJOR{ 1 };
    static constexpr int VULKAN_MINOR{ 2 };
    static constexpr int VULKAN_PATCH{ 168 };

    static constexpr vk::Format PREFERRED_FORMAT{ vk::Format::eB8G8R8A8Srgb };
    static constexpr vk::ColorSpaceKHR PREFERRED_COLOR_SPACE{ vk::ColorSpaceKHR::eSrgbNonlinear };
#ifdef WIN32
    static constexpr vk::PresentModeKHR PREFERRED_PRESENT_MODE{ vk::PresentModeKHR::eMailbox };
#else
    static constexpr vk::PresentModeKHR PREFERRED_PRESENT_MODE{ vk::PresentModeKHR::eFifo };
#endif
    static constexpr std::uint32_t PREFERRED_EXTRA_IMAGE_COUNT{ 1 };

    static constexpr const char* VERT_SHADER_FILENAME{ "spv/vert.spv" };
    static constexpr const char* FRAG_SHADER_FILENAME{ "spv/frag.spv" };

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
    void query_swapchain_support_();
    void create_swapchain_();
    void retrieve_swapchain_images_();
    void create_render_pass_();
    void create_graphics_pipeline_();
    vk::ShaderModule create_shader_module_(const std::string& name, const std::vector<char>& binary);

    void destroy_graphics_pipeline_();
    void destroy_render_pass_();
    void destroy_swapchain_image_views_();
    void destroy_swapchain_();
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

    swapchain_info swapchain_info_;

    vk::SwapchainKHR swapchain_{ nullptr };

    std::vector<swapchain_image> swapchain_images_;

    vk::RenderPass render_pass_{ nullptr };

    vk::PipelineLayout pipeline_layout_{ nullptr };

    vk::Pipeline graphics_pipeline_{ nullptr };

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
