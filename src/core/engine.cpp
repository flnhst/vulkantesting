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
    create_surface_();
    enumerate_physical_devices_();
    select_physical_device_();
    create_device_();
    retrieve_queues_();

}

void engine::destroy()
{
    destroy_device_();
    destroy_surface_();
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
            //vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
            vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
            vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);

    create_info.pUserData = reinterpret_cast<void*>(this);

    create_info.pfnUserCallback = &messenger_callback;

    debug_utils_messenger_ = instance_.createDebugUtilsMessengerEXT(create_info, nullptr, dispatch_);

    SPDLOG_INFO("Created debug utils messenger callback.");
}

void engine::create_surface_()
{
    SPDLOG_INFO("Creating surface...");

    const auto wm_info = sdl_window_->get_system_wm_info();

    vk::Result result;

#ifdef WIN32
    SPDLOG_INFO("Using Win32 surface.");

    vk::Win32SurfaceCreateInfoKHR create_info{};

    create_info
        .setHwnd(wm_info.info.win.window)
        .setHinstance(GetModuleHandle(nullptr));

    result = instance_.createWin32SurfaceKHR(&create_info, nullptr, &surface_, dispatch_);
#else
    auto display = get_environment_variable("DISPLAY");
    auto sdl_videodriver = get_environment_variable("SDL_VIDEODRIVER");

    if (!display || display.value().empty() || sdl_videodriver == "wayland")
    {
        SPDLOG_INFO("Using Wayland surface.");

        vk::WaylandSurfaceCreateInfoKHR create_info{};

        create_info
            .setDisplay(wm_info.info.wl.display)
            .setSurface(wm_info.info.wl.surface);

        result = instance_.createWaylandSurfaceKHR(&create_info, nullptr, &surface_, dispatch_);
    }
    else
    {
        SPDLOG_INFO("Using X11 surface.");

        vk::XlibSurfaceCreateInfoKHR create_info{};

        create_info
            .setDpy(wm_info.info.x11.display)
            .setWindow(wm_info.info.x11.window);

        result = instance_.createXlibSurfaceKHR(&create_info, nullptr, &surface_, dispatch_);
    }
#endif

    EVK_ASSERT_RESULT(result, "Failed to create surface.");

    SPDLOG_INFO("Created surface.");
}

void engine::enumerate_physical_devices_()
{
    SPDLOG_INFO("Enumerating physical devices...");

    const auto physical_devices = instance_.enumeratePhysicalDevices(dispatch_);

    for (auto physical_device : physical_devices)
    {
        physical_device_info& new_info = physical_device_infos_.emplace_back();

        new_info.physical_device = physical_device;

        new_info.properties = physical_device.getProperties2(dispatch_);
        new_info.features = physical_device.getFeatures2(dispatch_);
        new_info.queue_families = physical_device.getQueueFamilyProperties2(dispatch_);

        SPDLOG_INFO("Found physical device: '{}'.", new_info.properties.properties.deviceName);

        std::uint32_t queue_family_index{ 0 };

        for (auto queue_family : new_info.queue_families)
        {
            if (queue_family.queueFamilyProperties.queueFlags & vk::QueueFlagBits::eGraphics)
            {
                new_info.graphics_family_queue_indices_.emplace_back(queue_family_index);

                SPDLOG_INFO("\tFound graphics family queue at index {}.", queue_family_index);
            }

            if (queue_family.queueFamilyProperties.queueFlags & vk::QueueFlagBits::eTransfer)
            {
                new_info.transfer_family_queue_indices_.emplace_back(queue_family_index);

                SPDLOG_INFO("\tFound transfer family queue at index {}.", queue_family_index);
            }

            vk::Bool32 present_support{ false };

            auto result = physical_device.getSurfaceSupportKHR(queue_family_index, surface_, &present_support, dispatch_);

            EVK_ASSERT_RESULT(result, "Failed to query surface support.");

            if (present_support)
            {
                new_info.present_family_queue_indices_.emplace_back(queue_family_index);
            }

            queue_family_index++;
        }
    }

    SPDLOG_INFO("Enumerated physical devices.");
}

void engine::select_physical_device_()
{
    for (const auto& physical_device_info : physical_device_infos_)
    {
        if (is_physical_device_suitable_(physical_device_info))
        {
            selected_physical_device_info_ = &physical_device_info;

            SPDLOG_INFO("Selected device '{}'.", selected_physical_device_info_->properties.properties.deviceName);

            graphics_queue_family_index_ = selected_physical_device_info_->graphics_family_queue_indices_[0];
            present_queue_family_index_ = selected_physical_device_info_->present_family_queue_indices_[0];

            SPDLOG_INFO("Selected queue family index {} for graphics.", graphics_queue_family_index_);
            SPDLOG_INFO("Selected queue family index {} for presentation.", present_queue_family_index_);

            return;
        }
    }

    if (selected_physical_device_info_ == nullptr)
    {
        throw std::runtime_error("Failed to select physical device.");
    }
}

void engine::create_device_()
{
    SPDLOG_INFO("Creating device...");

    const auto physical_device = selected_physical_device_info_->physical_device;

    vk::PhysicalDeviceFeatures features{};

    float queue_priority{ 1.0f };

    std::vector<vk::DeviceQueueCreateInfo> queue_create_infos;
    std::set<uint32_t> unique_queue_families = { graphics_queue_family_index_, present_queue_family_index_ };

    for (auto queue_family : unique_queue_families)
    {
        auto& queue_create_info = queue_create_infos.emplace_back();

        queue_create_info
            .setQueueFamilyIndex(queue_family)
            .setQueueCount(1)
            .setPQueuePriorities(&queue_priority);
    }

    vk::DeviceCreateInfo create_info{};

    create_info
        .setQueueCreateInfoCount(queue_create_infos.size())
        .setPQueueCreateInfos(queue_create_infos.data())
        .setPEnabledFeatures(&features)
        .setEnabledExtensionCount(device_extensions_.size())
        .setPpEnabledExtensionNames(device_extensions_.data())
        .setEnabledLayerCount(layers_.size())
        .setPpEnabledLayerNames(layers_.data());

    const auto result = physical_device.createDevice(&create_info, nullptr, &device_, dispatch_);

    EVK_ASSERT_RESULT(result, "Failed to create device.");

    SPDLOG_INFO("Creating device.");
}

void engine::retrieve_queues_()
{
    SPDLOG_INFO("Retrieving queues...");

    device_.getQueue(graphics_queue_family_index_, 0, &graphics_queue_, dispatch_);
    device_.getQueue(present_queue_family_index_, 0, &present_queue_, dispatch_);

    SPDLOG_INFO("Retrieved queues.");
}

void engine::destroy_device_()
{
    SPDLOG_TRACE("Destroying device...");

    device_.destroy(nullptr, dispatch_);

    SPDLOG_TRACE("Destroyed device.");
}

void engine::destroy_surface_()
{
    SPDLOG_TRACE("Destroying surface.");

    instance_.destroySurfaceKHR(surface_, nullptr, dispatch_);

    SPDLOG_TRACE("Destroyed surface.");
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

bool engine::is_physical_device_suitable_(const physical_device_info& p_physical_device_info)
{
    return p_physical_device_info.properties.properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu
        && !p_physical_device_info.graphics_family_queue_indices_.empty()
        && !p_physical_device_info.transfer_family_queue_indices_.empty()
        && !p_physical_device_info.present_family_queue_indices_.empty();
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
