#include "engine.h"

#include "sdl_window.h"

engine::engine()
{

}

engine::~engine()
{
    if (messages_emitted_ == 0)
    {
        SPDLOG_DEBUG("No messages were emitted during the lifetime of the debug messenger callback.");
    }
    else
    {
        SPDLOG_WARN("{} messages were emitted during the lifetime of the debug messenger callback.", messages_emitted_);
    }
}

void engine::initialize()
{
    auto vk_layer_path_env = get_environment_variable("VK_LAYER_PATH");

    if (vk_layer_path_env)
    {
        SPDLOG_INFO("VK_LAYER_PATH = '{}'.", vk_layer_path_env.value());
    }

    layers_.emplace_back("VK_LAYER_KHRONOS_validation");

    instance_extensions_.emplace_back("VK_KHR_surface");
    instance_extensions_.emplace_back("VK_EXT_debug_report");
    instance_extensions_.emplace_back("VK_EXT_debug_utils");

#ifdef WIN32
    instance_extensions_.emplace_back("VK_KHR_win32_surface");
#else
    instance_extensions_.emplace_back("VK_KHR_xlib_surface");
    instance_extensions_.emplace_back("VK_KHR_wayland_surface");
#endif

    device_extensions_.push_back("VK_KHR_swapchain");

    create_sdl_window_();
    create_instance_();
    create_debug_utils_ext_();
    select_physical_device_();
}

void engine::destroy()
{
    destroy_debug_utils_ext_();
    destroy_instance_();
    destroy_sdl_window_();
}

int engine::run()
{
    SPDLOG_INFO("Engine is running.");

    while (main_loop_running_)
    {
        main_loop_();
    }

    SPDLOG_INFO("Engine has stopped.");

    return 0;
}

void engine::stop()
{
    main_loop_running_ = false;

    SPDLOG_WARN("Engine is set to stop.");
}

void engine::main_loop_()
{
    if (first_tick_)
    {
        SPDLOG_INFO("First tick started.");
    }

    sdl_window_->process_events();

    if (first_tick_)
    {
        SPDLOG_INFO("First tick done.");

        first_tick_ = false;
    }

    ticks_++;
}

void engine::create_sdl_window_()
{
    SPDLOG_INFO("Creating SDL window...");

    sdl_window_ = std::make_unique<sdl_window>(*this);

    SPDLOG_INFO("SDL window created.");
}

void engine::create_instance_()
{
    SPDLOG_DEBUG("Creating the Vulkan instance...");

    vk::ApplicationInfo application_info{};

    application_info
        .setApiVersion(VK_MAKE_VERSION(vulkan_major_, vulkan_minor_, vulkan_patch_))
        .setPEngineName("vulkantesting")
        .setPApplicationName("vulkantesting")
        .setEngineVersion(1)
        .setApplicationVersion(1);

    vk::InstanceCreateInfo create_info{};

    create_info
        .setEnabledExtensionCount(instance_extensions_.size())
        .setEnabledLayerCount(layers_.size())
        .setPApplicationInfo(&application_info)
        .setPpEnabledExtensionNames(instance_extensions_.data())
        .setPpEnabledLayerNames(layers_.data());

    vk::Result result = vk::createInstance(&create_info, nullptr, &instance_);

    EVK_ASSERT_RESULT(result, "Failed to create instance.");

    dispatch_.init(instance_, vkGetInstanceProcAddr);

    SPDLOG_DEBUG("Successfully created the Vulkan instance.");
}

void engine::create_debug_utils_ext_()
{
    SPDLOG_INFO("Creating debug utils messenger callback...");

    vk::DebugUtilsMessengerCreateInfoEXT create_info{};

    create_info
        .setMessageSeverity(
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eError |
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning)
        .setMessageType(
            vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
            vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
            vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);

    create_info.pUserData = reinterpret_cast<void*>(this);

    create_info.pfnUserCallback = &messenger_callback;

    debug_utils_messenger_ = instance_.createDebugUtilsMessengerEXT(create_info, nullptr, dispatch_);

    SPDLOG_INFO("Created debug utils messenger callback.");
}

void engine::select_physical_device_()
{
    auto physical_devices = instance_.enumeratePhysicalDevices(dispatch_);

    for (auto physical_device : physical_devices)
    {
        const auto properties = physical_device.getProperties2();

        SPDLOG_INFO("Found physical device: '{}'.", properties.properties.deviceName);

        if (is_physical_device_suitable_(physical_device))
        {
            physical_device_ = physical_device;

            SPDLOG_INFO("\tFound suitable device.");
        }
    }
}

void engine::destroy_debug_utils_ext_()
{
    SPDLOG_TRACE("Destroying debug utils messenger...");

    instance_.destroy(debug_utils_messenger_, nullptr, dispatch_);
}

void engine::destroy_instance_()
{
    SPDLOG_TRACE("Destroying Vulkan instance...");

    instance_.destroy(nullptr, dispatch_);

    SPDLOG_TRACE("Destroyed Vulkan instance.");
}

void engine::destroy_sdl_window_()
{
    SPDLOG_TRACE("Destroying SDL window...");

    sdl_window_.reset();

    SPDLOG_TRACE("Destroyed SDL window.");
}

bool engine::is_physical_device_suitable_(vk::PhysicalDevice physical_device)
{
    const auto properties = physical_device.getProperties2(dispatch_);
    const auto queue_family_properties = physical_device.getQueueFamilyProperties2();

    std::optional<std::uint32_t> graphics_queue_family_index;
    std::optional<std::uint32_t> transfer_queue_family_index;

    std::uint32_t queue_family_index{ 0 };

    for (auto queue_family : queue_family_properties)
    {
        if (queue_family.queueFamilyProperties.queueFlags& vk::QueueFlagBits::eGraphics)
        {
            graphics_queue_family_index = queue_family_index;

            SPDLOG_INFO("\tFound graphics family queue at index {}.", queue_family_index);
        }

        if (queue_family.queueFamilyProperties.queueFlags & vk::QueueFlagBits::eTransfer)
        {
            transfer_queue_family_index = queue_family_index;

            SPDLOG_INFO("\tFound transfer family queue at index {}.", queue_family_index);
        }

        queue_family_index++;
    }

    return properties.properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu && graphics_queue_family_index && transfer_queue_family_index;
}

VkBool32 messenger_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
    auto* thiz = reinterpret_cast<engine*>(pUserData);

    thiz->messages_emitted_++;

    bool trigger_debug_break = false;

    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
    {
        SPDLOG_ERROR("{}", pCallbackData->pMessageIdName);
        SPDLOG_ERROR("\t{}", pCallbackData->pMessage);

        trigger_debug_break = true;
    }
    else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
    {
        SPDLOG_WARN("{}", pCallbackData->pMessageIdName);
        SPDLOG_WARN("\t{}", pCallbackData->pMessage);

        trigger_debug_break = true;
    }
    else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
    {
        SPDLOG_INFO("{}", pCallbackData->pMessageIdName);
        SPDLOG_INFO("\t{}", pCallbackData->pMessage);
    }
    else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
    {
        SPDLOG_TRACE("{}", pCallbackData->pMessageIdName);
        SPDLOG_TRACE("\t{}", pCallbackData->pMessage);
    }

    for (std::uint64_t object_index = 0; object_index < pCallbackData->objectCount; ++object_index)
    {
        auto& object_name_info = pCallbackData->pObjects[object_index];

        vk::ObjectType object_type = static_cast<vk::ObjectType>(object_name_info.objectType);

        if (object_name_info.pObjectName == nullptr)
        {
            SPDLOG_DEBUG("\tPrevious message is associated with a '{}' object (no name): {:x}", vk::to_string(object_type), object_name_info.objectHandle);
        }
        else
        {
            SPDLOG_DEBUG("\tPrevious message is associated with a '{}' object: {:x}", vk::to_string(object_type), object_name_info.pObjectName);
        }
    }

    if (trigger_debug_break)
    {
        vulkantesting_debug_break();
    }

    return false;
}
