#include "engine.h"

#include "sdl_window.h"

engine::engine()
{

}

engine::~engine()
{

}

void engine::initialize()
{
    SPDLOG_INFO("Initializing...");

    auto vk_layer_path_env = get_environment_variable("VK_LAYER_PATH");

    if (vk_layer_path_env)
    {
        SPDLOG_INFO("VK_LAYER_PATH = '{}'.", vk_layer_path_env.value());
    }

    if (!has_environment_variable("VKT_DISABLE_VALIDATION"))
    {
        layers_.emplace_back("VK_LAYER_KHRONOS_validation");
    }
    else
    {
        SPDLOG_CRITICAL("VALIDATION IS DISABLED!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
        SPDLOG_CRITICAL("VALIDATION IS DISABLED!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
        SPDLOG_CRITICAL("VALIDATION IS DISABLED!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
        SPDLOG_CRITICAL("VALIDATION IS DISABLED!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
        SPDLOG_CRITICAL("VALIDATION IS DISABLED!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
        SPDLOG_CRITICAL("VALIDATION IS DISABLED!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
    }

    instance_extensions_.emplace_back(VK_KHR_SURFACE_EXTENSION_NAME);
    instance_extensions_.emplace_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    instance_extensions_.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    instance_extensions_.emplace_back(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);

#ifdef WIN32
    instance_extensions_.emplace_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#else
    instance_extensions_.emplace_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
    instance_extensions_.emplace_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
#endif

    device_extensions_.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    create_sdl_window_();
    create_instance_();
    create_debug_utils_ext_();
    create_surface_();
    enumerate_physical_devices_();
    select_physical_device_();
    create_device_();
    retrieve_queues_();
    query_swapchain_support_();
    create_swapchain_();
    retrieve_swapchain_images_();
    create_graphics_pipeline_();

    SPDLOG_INFO("Initialized.");
}

void engine::destroy()
{
    SPDLOG_TRACE("Destroying everything...");

    destroy_graphics_pipeline_();
    destroy_swapchain_image_views_();
    destroy_swapchain_();
    destroy_device_();
    destroy_surface_();
    destroy_debug_utils_ext_();
    destroy_instance_();
    destroy_sdl_window_();

    if (has_environment_variable("VKT_DISABLE_VALIDATION"))
    {
        SPDLOG_CRITICAL("VALIDATION IS DISABLED!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
        SPDLOG_CRITICAL("VALIDATION IS DISABLED!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
        SPDLOG_CRITICAL("VALIDATION IS DISABLED!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
        SPDLOG_CRITICAL("VALIDATION IS DISABLED!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
        SPDLOG_CRITICAL("VALIDATION IS DISABLED!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
        SPDLOG_CRITICAL("VALIDATION IS DISABLED!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
    }

    SPDLOG_TRACE("Destroyed everything.");
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
        .setApiVersion(VK_MAKE_VERSION(VULKAN_MAJOR, VULKAN_MINOR, VULKAN_PATCH))
        .setPEngineName("vulkantesting")
        .setPApplicationName("vulkantesting")
        .setEngineVersion(1)
        .setApplicationVersion(1);

    vk::InstanceCreateInfo create_info{};

    create_info
        .setEnabledExtensionCount(instance_extensions_.size())
        .setPpEnabledExtensionNames(instance_extensions_.data())
        .setEnabledLayerCount(layers_.size())
        .setPpEnabledLayerNames(layers_.data())
        .setPApplicationInfo(&application_info);

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

    const auto wm_info = sdl_window_->get_system_wm_info();

    const auto physical_devices = instance_.enumeratePhysicalDevices(dispatch_);

    for (auto physical_device : physical_devices)
    {
        physical_device_info& new_info = physical_device_infos_.emplace_back();

        new_info.physical_device = physical_device;

        new_info.properties = physical_device.getProperties2(dispatch_);
        new_info.features = physical_device.getFeatures2(dispatch_);
        new_info.queue_families = physical_device.getQueueFamilyProperties2(dispatch_);

        SPDLOG_INFO("Found physical device: '{}'.", new_info.properties.properties.deviceName);

        const auto present_modes = physical_device.getSurfacePresentModesKHR(surface_, dispatch_);

        for (auto present_mode : present_modes)
        {
            SPDLOG_INFO("Physical device and surface supports present mode '{}'.", vk::to_string(present_mode));
        }

        std::uint32_t queue_family_index{ 0 };

        for (auto queue_family : new_info.queue_families)
        {
            SPDLOG_INFO("Testing queue family index {}.", queue_family_index);

            if (queue_family.queueFamilyProperties.queueFlags & vk::QueueFlagBits::eGraphics)
            {
                new_info.graphics_family_queue_indices_.emplace_back(queue_family_index);

                SPDLOG_INFO("Found graphics family queue at index {}.", queue_family_index);
            }

            if (queue_family.queueFamilyProperties.queueFlags & vk::QueueFlagBits::eTransfer)
            {
                new_info.transfer_family_queue_indices_.emplace_back(queue_family_index);

                SPDLOG_INFO("Found transfer family queue at index {}.", queue_family_index);
            }

            SPDLOG_INFO("Querying surface support...");

            vk::Bool32 present_support{ false };

            const auto result = physical_device.getSurfaceSupportKHR(queue_family_index, surface_, &present_support, dispatch_);

            if (result != vk::Result::eSuccess)
            {
                SPDLOG_WARN("Querying surface support has failed: '{}'.", vk::to_string(result));
            }
            else if (present_support)
            {
                SPDLOG_INFO("Surface is supported by queue family index {}.", queue_family_index);

                new_info.present_family_queue_indices_.emplace_back(queue_family_index);
            }

#ifdef VK_USE_PLATFORM_WAYLAND_KHR
            if (get_environment_variable("SDL_VIDEODRIVER") == "wayland")
            {
                present_support = physical_device.getWaylandPresentationSupportKHR(queue_family_index, wm_info.info.wl.display, dispatch_);

                if (present_support)
                {
                    SPDLOG_INFO("Surface is supported for Wayland by queue family index {}.", queue_family_index);

                    new_info.present_family_queue_indices_.emplace_back(queue_family_index);
                }
            }
#endif

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
        throw_exception("Failed to select physical device.");
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

void engine::query_swapchain_support_()
{
    SPDLOG_INFO("Querying swapchain support...");

    swapchain_info_.capabilities = selected_physical_device_info_->physical_device.getSurfaceCapabilities2KHR(surface_, dispatch_);
    swapchain_info_.surface_formats = selected_physical_device_info_->physical_device.getSurfaceFormats2KHR(surface_, dispatch_);
    swapchain_info_.present_modes = selected_physical_device_info_->physical_device.getSurfacePresentModesKHR(surface_, dispatch_);

    bool preferred_format_found{ false };

    for (const auto format : swapchain_info_.surface_formats)
    {
        if (format.surfaceFormat.format == PREFERRED_FORMAT && format.surfaceFormat.colorSpace == PREFERRED_COLOR_SPACE)
        {
            swapchain_info_.chosen_surface_format = format;

            preferred_format_found = true;

            break;
        }
    }

    if (!preferred_format_found)
    {
        throw_exception(fmt::format("Could not find format '{}' and color space '{}'.", vk::to_string(PREFERRED_FORMAT), vk::to_string(PREFERRED_COLOR_SPACE)));
    }

    SPDLOG_INFO("Format '{}' and color space '{}' chosen.", vk::to_string(swapchain_info_.chosen_surface_format.surfaceFormat.format), vk::to_string(swapchain_info_.chosen_surface_format.surfaceFormat.colorSpace));

    bool preferred_present_mode_found{ false };

    for (const auto present_mode : swapchain_info_.present_modes)
    {
        if (present_mode == PREFERRED_PRESENT_MODE)
        {
            swapchain_info_.chosen_present_mode = present_mode;

            preferred_present_mode_found = true;

            break;
        }
    }

    if (!preferred_present_mode_found)
    {
        throw_exception(fmt::format("Could not find present mode '{}'.", vk::to_string(PREFERRED_PRESENT_MODE)));
    }

    SPDLOG_INFO("Present mode '{}' chosen.", vk::to_string(swapchain_info_.chosen_present_mode));

    swapchain_info_.chosen_extent = swapchain_info_.capabilities.surfaceCapabilities.currentExtent;

    SPDLOG_INFO("Extent chosen has width '{}' and height '{}'.", swapchain_info_.chosen_extent.width, swapchain_info_.chosen_extent.height);

    swapchain_info_.chosen_image_count = swapchain_info_.capabilities.surfaceCapabilities.minImageCount + PREFERRED_EXTRA_IMAGE_COUNT;

    if (swapchain_info_.capabilities.surfaceCapabilities.maxImageCount > 0 && swapchain_info_.chosen_image_count > swapchain_info_.capabilities.surfaceCapabilities.maxImageCount)
    {
        swapchain_info_.chosen_image_count = swapchain_info_.capabilities.surfaceCapabilities.maxImageCount;
    }

    SPDLOG_INFO("Chosen image count is '{}'.", swapchain_info_.chosen_image_count);

    SPDLOG_INFO("Queried swapchain support.");
}

void engine::create_swapchain_()
{
    SPDLOG_INFO("Creating swapchain...");

    vk::SwapchainCreateInfoKHR create_info{};

    create_info
        .setSurface(surface_)
        .setMinImageCount(swapchain_info_.chosen_image_count)
        .setImageFormat(swapchain_info_.chosen_surface_format.surfaceFormat.format)
        .setImageColorSpace(swapchain_info_.chosen_surface_format.surfaceFormat.colorSpace)
        .setImageExtent(swapchain_info_.chosen_extent)
        .setImageArrayLayers(1)
        .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
        .setImageSharingMode(vk::SharingMode::eExclusive)
        .setPreTransform(swapchain_info_.capabilities.surfaceCapabilities.currentTransform)
        .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
        .setPresentMode(swapchain_info_.chosen_present_mode)
        .setClipped(true);

    std::vector<std::uint32_t> queue_families;

    queue_families.emplace_back(graphics_queue_family_index_);
    queue_families.emplace_back(present_queue_family_index_);

    if (graphics_queue_family_index_ != present_queue_family_index_)
    {
        create_info
            .setImageSharingMode(vk::SharingMode::eConcurrent)
            .setQueueFamilyIndexCount(2)
            .setPQueueFamilyIndices(queue_families.data());
    }

    const auto result = device_.createSwapchainKHR(&create_info, nullptr, &swapchain_, dispatch_);

    EVK_ASSERT_RESULT(result, "Failed to create swapchain.");

    SPDLOG_INFO("Created swapchain.");
}

void engine::retrieve_swapchain_images_()
{
    SPDLOG_INFO("Retrieving swapchain images...");

    const auto images = device_.getSwapchainImagesKHR(swapchain_, dispatch_);

    for (const auto& image : images)
    {
        auto& new_swapchain_image = swapchain_images_.emplace_back();

        new_swapchain_image.image = image;

        vk::ImageViewCreateInfo create_info{};

        create_info
            .setImage(new_swapchain_image.image)
            .setViewType(vk::ImageViewType::e2D)
            .setFormat(swapchain_info_.chosen_surface_format.surfaceFormat.format);

        create_info.components
            .setR(vk::ComponentSwizzle::eIdentity)
            .setG(vk::ComponentSwizzle::eIdentity)
            .setB(vk::ComponentSwizzle::eIdentity)
            .setA(vk::ComponentSwizzle::eIdentity);

        create_info.subresourceRange
            .setAspectMask(vk::ImageAspectFlagBits::eColor)
            .setBaseMipLevel(0)
            .setLevelCount(1)
            .setBaseArrayLayer(0)
            .setLayerCount(1);

        auto result = device_.createImageView(&create_info, nullptr, &new_swapchain_image.image_view, dispatch_);

        EVK_ASSERT_RESULT(result, "Failed to create swapchain image view.");
    }

    SPDLOG_INFO("Retrieved swapchain images.");
}

void engine::create_graphics_pipeline_()
{
    SPDLOG_INFO("Creating graphics pipeline...");



    SPDLOG_INFO("Created graphics pipeline.");
}

void engine::destroy_graphics_pipeline_()
{
    SPDLOG_TRACE("Destroying graphics pipeline...");



    SPDLOG_TRACE("Destroyed graphics pipeline.");
}

void engine::destroy_swapchain_image_views_()
{
    SPDLOG_TRACE("Destroying swapchain image views...");

    for (const auto& swapchain_image : swapchain_images_)
    {
        device_.destroyImageView(swapchain_image.image_view, nullptr, dispatch_);
    }

    SPDLOG_TRACE("Destroyed swapchain image views.");
}

void engine::destroy_swapchain_()
{
    SPDLOG_TRACE("Destroying swapchain...");

    device_.destroySwapchainKHR(swapchain_, nullptr, dispatch_);

    SPDLOG_TRACE("Destroyed swapchain.");
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
    if (messages_emitted_ == 0)
    {
        SPDLOG_DEBUG("No messages were emitted during the lifetime of the debug messenger callback.");
    }
    else
    {
        SPDLOG_WARN("{} messages were emitted during the lifetime of the debug messenger callback.", messages_emitted_);
    }

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
