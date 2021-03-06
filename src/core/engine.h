#ifndef ENGINE_H
#define ENGINE_H

#include "core/core.h"

struct physical_device_info
{
    vk::PhysicalDevice physical_device{ nullptr };

    vk::PhysicalDeviceProperties2 properties{};
    vk::PhysicalDeviceFeatures2 features{};
    std::vector<vk::QueueFamilyProperties2> queue_families;

    std::vector<uint32_t> graphics_family_queue_indices_;
    std::vector<uint32_t> transfer_family_queue_indices_;

    std::vector<uint32_t> present_family_queue_indices_;
};

struct swapchain_info
{
    vk::SurfaceCapabilities2KHR capabilities{};
    std::vector<vk::SurfaceFormat2KHR> surface_formats;
    std::vector<vk::PresentModeKHR> present_modes;

    vk::SurfaceFormat2KHR chosen_surface_format{};
    vk::PresentModeKHR chosen_present_mode{};
    vk::Extent2D chosen_extent{};
    std::uint32_t chosen_image_count{ 0 };
};

struct swapchain_image
{
    vk::Image image{ nullptr };
    vk::ImageView image_view{ nullptr };

    vk::Framebuffer framebuffer{ nullptr };
    vk::CommandPool command_pool{ nullptr };
};

struct frame_in_flight
{
    vk::CommandBuffer command_buffer{ nullptr };

    vk::Semaphore image_available_semaphore{ nullptr };
    vk::Semaphore render_finished_semaphore{ nullptr };

    vk::Fence fence{ nullptr };

    vk::CommandPool* command_pool_ptr{ nullptr };

    std::uint32_t swapchain_image_index{ 0 };

    //std::unique_ptr<std::binary_semaphore> frame_done;
};

class engine
{
public:
    constexpr static bool USE_DEBUG_LAYERS = true;
    constexpr static bool RENDER_THREAD_ENABLED = false;

    constexpr static std::uint64_t WAIT_FOR_FENCES_TIMEOUT_MS = 25; // milliseconds
    constexpr static std::uint64_t TOTAL_TIME_ABORT_LEVEL_MS = 1000; // milliseconds

    static constexpr int VULKAN_MAJOR{ 1 };
    static constexpr int VULKAN_MINOR{ 2 };
    static constexpr int VULKAN_PATCH{ 168 };

    vk::PhysicalDeviceType preferred_physical_device_type_{vk::PhysicalDeviceType::eDiscreteGpu };

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

    static constexpr std::uint64_t MAXIMUM_FRAMES_IN_FLIGHT{ 2 };

    engine();
    ~engine();

    void initialize();

    void destroy();

    int run();

    void stop();

    bool main_loop_running() const { return main_loop_running_; }

private:
    void main_loop_();

    void create_frame_in_flight_(frame_in_flight& p_frame_in_flight);
    void destroy_frame_in_flight_(frame_in_flight& p_frame_in_flight, bool final_destroy = false);

    void draw_frame_();

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
    void create_framebuffers_();
    void create_command_pools_();

    void reset_timeline_semaphore_(vk::Semaphore& timeline_semaphore, std::uint64_t initial_value);
    void record_command_buffer_(frame_in_flight& p_frame_in_flight);
    
    void destroy_command_pools_();
    void destroy_framebuffers_();
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

    void recreate_swapchain_();

    // render thread only begin

    void render_entrypoint_();

    void present_(frame_in_flight* our_frame_in_flight);

    // render thread only end

    void wait_on_fence(vk::Fence fence, const std::string& name);

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
    //std::vector<frame_in_flight> frames_in_flight_;

    vk::RenderPass render_pass_{ nullptr };

    vk::PipelineLayout pipeline_layout_{ nullptr };

    vk::Pipeline graphics_pipeline_{ nullptr };

    //std::uint64_t current_frame_{ 0 };

    using clock = std::chrono::system_clock;

    clock::time_point start_point_{};
    clock::time_point last_second_{};
    
    std::uint64_t second_counter_{ 0 };
    std::uint64_t fps_counter_{ 0 };

    bool out_of_date_{ false };

    std::thread render_thread_;
    std::atomic<bool> render_thread_active_{ true };

    frame_in_flight new_frame_in_flight;

    moodycamel::BlockingConcurrentQueue<frame_in_flight*> render_queue_;

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
