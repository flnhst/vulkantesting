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
    create_render_pass_();
    create_graphics_pipeline_();
    create_framebuffers_();
    create_command_pools_();
    allocate_command_buffers_();
    record_command_buffers_();
    create_frames_in_flight_();

    SPDLOG_INFO("Initialized.");
}

void engine::destroy()
{
    SPDLOG_TRACE("Destroying everything...");

    destroy_frames_in_flight_();
    free_command_buffers_();
    destroy_command_pools_();
    destroy_framebuffers_();
    destroy_graphics_pipeline_();
    destroy_render_pass_();
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

    start_point_ = clock::now();

    while (main_loop_running_)
    {
        main_loop_();
    }

    // NOT IN LOOP ONLY FOR EXIT
    // WAIT IDLE
    // WAIT IDLE
    // WAIT IDLE
    // WAIT IDLE
    device_.waitIdle(dispatch_);
    // WAIT IDLE
    // WAIT IDLE
    // WAIT IDLE
    // WAIT IDLE

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
    fps_counter_++;

    if ((clock::now() - last_second_) > std::chrono::seconds(1))
    {
        last_second_ = clock::now();

        second_counter_++;

        sdl_window_->set_title(fmt::format("Vulkan Testing: {} second(s) elapsed, {} FPS.", second_counter_, fps_counter_));
        SPDLOG_INFO("Tick {}: {} second(s) elapsed, {} FPS.", ticks_, second_counter_, fps_counter_);

        fps_counter_ = 0;
    }
    
    if (first_tick_)
    {
        SPDLOG_INFO("First tick started.");
    }

    sdl_window_->process_events();

    if (out_of_date_)
    {
        recreate_swapchain_();
    }

    draw_frame_();

    if (first_tick_)
    {
        SPDLOG_INFO("First tick done.");

        first_tick_ = false;
    }

    ticks_++;
}

void engine::draw_frame_()
{
    auto& frame_in_flight = frames_in_flight_[current_frame_];

    auto result = device_.waitForFences(1, &frame_in_flight.fence, true, std::numeric_limits<std::uint64_t>::max());

    EVK_ASSERT_RESULT(result, "Failed to wait for fence.");

    std::uint32_t swapchain_image_index{ 0 };

    result = device_.acquireNextImageKHR(swapchain_, std::numeric_limits<std::uint64_t>::max(), frame_in_flight.image_available_semaphore, nullptr, &swapchain_image_index, dispatch_);

    EVK_ASSERT_RESULT(result, "Failed to acquire image.");

    auto& swapchain_image = swapchain_images_[swapchain_image_index];

    if (swapchain_image.fence != static_cast<vk::Fence>(nullptr))
    {
        auto result = device_.waitForFences(1, &swapchain_image.fence, true, std::numeric_limits<std::uint64_t>::max());

        EVK_ASSERT_RESULT(result, "Failed to wait for fence.");
    }

    swapchain_image.fence = frame_in_flight.fence;

    const vk::PipelineStageFlags wait_dst_stage_mask[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };

    result = device_.resetFences(1, &frame_in_flight.fence, dispatch_);

    EVK_ASSERT_RESULT(result, "Failed to reset fence.");

    const vk::Semaphore signal_semaphores[1] = { frame_in_flight.render_finished_semaphore };

    vk::StructureChain<vk::SubmitInfo> chain{};

    auto& submit_info = chain.get<vk::SubmitInfo>();

    submit_info
        .setWaitSemaphoreCount(1)
        .setPWaitSemaphores(&frame_in_flight.image_available_semaphore)
        .setPWaitDstStageMask(wait_dst_stage_mask)
        .setCommandBufferCount(1)
        .setPCommandBuffers(&swapchain_image.command_buffer)
        .setSignalSemaphoreCount(1)
        .setPSignalSemaphores(signal_semaphores);

    result = graphics_queue_.submit(1, &submit_info, frame_in_flight.fence, dispatch_);

    EVK_ASSERT_RESULT(result, "Failed to submit command buffer.");

    vk::PresentInfoKHR present_info{};

    present_info
        .setWaitSemaphoreCount(1)
        .setPWaitSemaphores(&frame_in_flight.render_finished_semaphore)
        .setSwapchainCount(1)
        .setPSwapchains(&swapchain_)
        .setPImageIndices(&swapchain_image_index);

    try
    {
        result = present_queue_.presentKHR(present_info, dispatch_);

        EVK_ASSERT_RESULT(result, "Failed to present.");
    }
    catch (vk::OutOfDateKHRError&)
    {
        SPDLOG_WARN("Surface is out of date.");

        out_of_date_ = true;
    }

    current_frame_ = (current_frame_ + 1) % MAXIMUM_FRAMES_IN_FLIGHT;
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

    vk::StructureChain<vk::DeviceCreateInfo, vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceTimelineSemaphoreFeatures> chain{};

    auto& create_info = chain.get<vk::DeviceCreateInfo>();

    chain.get<vk::PhysicalDeviceTimelineSemaphoreFeatures>()
        .setTimelineSemaphore(true);

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
    
    create_info
        .setQueueCreateInfoCount(queue_create_infos.size())
        .setPQueueCreateInfos(queue_create_infos.data())
        .setPEnabledFeatures(nullptr)
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

    vk::SwapchainKHR old_swapchain = swapchain_;

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
        .setClipped(true)
        .setOldSwapchain(old_swapchain);

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

void engine::create_render_pass_()
{
    SPDLOG_INFO("Creating render pass...");

    vk::AttachmentDescription color_attachment{};

    color_attachment
        .setFormat(swapchain_info_.chosen_surface_format.surfaceFormat.format)
        .setSamples(vk::SampleCountFlagBits::e1)
        .setLoadOp(vk::AttachmentLoadOp::eClear)
        .setStoreOp(vk::AttachmentStoreOp::eStore)
        .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
        .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
        .setInitialLayout(vk::ImageLayout::eUndefined)
        .setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

    vk::AttachmentReference color_attachment_reference{};

    color_attachment_reference
        .setAttachment(0)
        .setLayout(vk::ImageLayout::eColorAttachmentOptimal);

    vk::SubpassDescription subpass_description{};

    subpass_description
        .setColorAttachmentCount(1)
        .setPColorAttachments(&color_attachment_reference);

    vk::SubpassDependency dependency{};

    dependency
        .setSrcSubpass(VK_SUBPASS_EXTERNAL)
        .setDstSubpass(0)
        .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
        .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
        .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);

    vk::RenderPassCreateInfo create_info{};

    create_info
        .setAttachmentCount(1)
        .setPAttachments(&color_attachment)
        .setSubpassCount(1)
        .setPSubpasses(&subpass_description)
        .setDependencyCount(1)
        .setPDependencies(&dependency);

    const auto result = device_.createRenderPass(&create_info, nullptr, &render_pass_, dispatch_);

    EVK_ASSERT_RESULT(result, "Failed to create render pass.");

    SPDLOG_INFO("Created render pass.");
}

void engine::create_graphics_pipeline_()
{
    SPDLOG_INFO("Creating graphics pipeline...");

    SPDLOG_INFO("Reading SPIRV files...");

    const auto vert_shader_code = read_file(VERT_SHADER_FILENAME);
    const auto frag_shader_code = read_file(FRAG_SHADER_FILENAME);

    SPDLOG_INFO("Vertex shader binary size is {} bytes.", vert_shader_code.size());
    SPDLOG_INFO("Fragment shader binary size is {} bytes.", frag_shader_code.size());

    SPDLOG_INFO("Read SPIRV files.");

    SPDLOG_INFO("Creating shader modules...");

    const auto vert_shader_module = create_shader_module_(VERT_SHADER_FILENAME, vert_shader_code);
    const auto frag_shader_module = create_shader_module_(FRAG_SHADER_FILENAME, frag_shader_code);

    SPDLOG_INFO("Creating shader modules.");

    vk::PipelineShaderStageCreateInfo vert_create_info{};

    vert_create_info
        .setStage(vk::ShaderStageFlagBits::eVertex)
        .setModule(vert_shader_module)
        .setPName("main");

    vk::PipelineShaderStageCreateInfo frag_create_info{};

    frag_create_info
        .setStage(vk::ShaderStageFlagBits::eFragment)
        .setModule(frag_shader_module)
        .setPName("main");

    vk::PipelineShaderStageCreateInfo shader_stages[] = { vert_create_info, frag_create_info };

    vk::PipelineVertexInputStateCreateInfo pipeline_vertex_input_state_create_info{};

    vk::PipelineInputAssemblyStateCreateInfo pipeline_input_assembly_state_create_info{};

    pipeline_input_assembly_state_create_info
        .setTopology(vk::PrimitiveTopology::eTriangleList);

    vk::Viewport viewport{};

    viewport
        .setX(0.0f)
        .setY(0.0f)
        .setWidth(swapchain_info_.chosen_extent.width)
        .setHeight(swapchain_info_.chosen_extent.height)
        .setMinDepth(0.0f)
        .setMaxDepth(1.0f);

    vk::Rect2D scissor{};

    scissor
        .setOffset(vk::Offset2D{ 0, 0 })
        .setExtent(swapchain_info_.chosen_extent);

    vk::PipelineViewportStateCreateInfo pipeline_viewport_state_create_info{};

    pipeline_viewport_state_create_info
        .setScissorCount(1)
        .setPScissors(&scissor)
        .setViewportCount(1)
        .setPViewports(&viewport);

    vk::PipelineRasterizationStateCreateInfo pipeline_rasterization_state_create_info{};

    pipeline_rasterization_state_create_info
        .setDepthClampEnable(false)
        .setRasterizerDiscardEnable(false)
        .setPolygonMode(vk::PolygonMode::eFill)
        .setLineWidth(1.0f)
        .setCullMode(vk::CullModeFlagBits::eBack)
        .setFrontFace(vk::FrontFace::eClockwise);

    vk::PipelineMultisampleStateCreateInfo pipeline_multisample_state_create_info{};

    pipeline_multisample_state_create_info
        .setSampleShadingEnable(false)
        .setRasterizationSamples(vk::SampleCountFlagBits::e1);

    vk::PipelineColorBlendAttachmentState pipeline_color_blend_attachment_state{};

    pipeline_color_blend_attachment_state
        .setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA)
        .setBlendEnable(false);

    vk::PipelineColorBlendStateCreateInfo pipeline_color_blend_state_create_info{};

    pipeline_color_blend_state_create_info
        .setLogicOpEnable(false)
        .setAttachmentCount(1)
        .setPAttachments(&pipeline_color_blend_attachment_state);

    vk::PipelineLayoutCreateInfo pipeline_layout_create_info{};

    auto result = device_.createPipelineLayout(&pipeline_layout_create_info, nullptr, &pipeline_layout_, dispatch_);

    EVK_ASSERT_RESULT(result, "Failed to create pipeline layout.");

    vk::GraphicsPipelineCreateInfo create_info{};

    create_info
        .setStageCount(2)
        .setPStages(shader_stages)
        .setPInputAssemblyState(&pipeline_input_assembly_state_create_info)
        .setPVertexInputState(&pipeline_vertex_input_state_create_info)
        .setPViewportState(&pipeline_viewport_state_create_info)
        .setPRasterizationState(&pipeline_rasterization_state_create_info)
        .setPMultisampleState(&pipeline_multisample_state_create_info)
        .setPColorBlendState(&pipeline_color_blend_state_create_info)
        .setLayout(pipeline_layout_)
        .setRenderPass(render_pass_);

    result = device_.createGraphicsPipelines(nullptr, 1, &create_info, nullptr, &graphics_pipeline_, dispatch_);

    EVK_ASSERT_RESULT(result, "Failed to create graphics pipeline.");

    SPDLOG_INFO("Created graphics pipeline.");

    SPDLOG_INFO("Destroying shader modules...");

    device_.destroyShaderModule(vert_shader_module, nullptr, dispatch_);
    device_.destroyShaderModule(frag_shader_module, nullptr, dispatch_);

    SPDLOG_INFO("Destroyed shader modules.");
}

vk::ShaderModule engine::create_shader_module_(const std::string& name, const std::vector<char>& binary)
{
    SPDLOG_INFO("Creating shader module for '{}'...", name);

    vk::ShaderModuleCreateInfo create_info{};

    create_info
        .setPCode(reinterpret_cast<const std::uint32_t*>(binary.data()))
        .setCodeSize(binary.size());

    vk::ShaderModule shader_module;

    const auto result = device_.createShaderModule(&create_info, nullptr, &shader_module, dispatch_);

    EVK_ASSERT_RESULT(result, fmt::format("Failed to create shader module for '{}'.", name));

    SPDLOG_INFO("Created shader module for '{}'.", name);

    return shader_module;
}

void engine::create_framebuffers_()
{
    SPDLOG_INFO("Creating framebuffers...");

    for (auto& swapchain_image : swapchain_images_)
    {
        vk::FramebufferCreateInfo create_info{};

        create_info
            .setRenderPass(render_pass_)
            .setAttachmentCount(1)
            .setPAttachments(&swapchain_image.image_view)
            .setWidth(swapchain_info_.chosen_extent.width)
            .setHeight(swapchain_info_.chosen_extent.height)
            .setLayers(1);

        const auto result = device_.createFramebuffer(&create_info, nullptr, &swapchain_image.framebuffer, dispatch_);

        EVK_ASSERT_RESULT(result, "Failed to create framebuffer.");
    }

    SPDLOG_INFO("Creating framebuffers.");
}

void engine::create_command_pools_()
{
    SPDLOG_INFO("Creating command pools...");

    for (auto& swapchain_image : swapchain_images_)
    {
        vk::CommandPoolCreateInfo create_info{};

        create_info
            .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer | vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
            .setQueueFamilyIndex(graphics_queue_family_index_);

        const auto result = device_.createCommandPool(&create_info, nullptr, &swapchain_image.command_pool, dispatch_);

        EVK_ASSERT_RESULT(result, "Failed to create command pool.");
    }

    SPDLOG_INFO("Created command pools.");
}

void engine::allocate_command_buffers_()
{
    SPDLOG_INFO("Allocating command buffers...");

    for (auto& swapchain_image : swapchain_images_)
    {
        vk::CommandBufferAllocateInfo allocate_info{};

        allocate_info
            .setCommandBufferCount(1)
            .setCommandPool(swapchain_image.command_pool)
            .setLevel(vk::CommandBufferLevel::ePrimary);

        const auto result = device_.allocateCommandBuffers(&allocate_info, &swapchain_image.command_buffer, dispatch_);

        EVK_ASSERT_RESULT(result, "Failed to allocate command buffer.");
    }

    SPDLOG_INFO("Allocated command buffers.");
}

void engine::record_command_buffers_()
{
    SPDLOG_INFO("Recording command buffers...");

    for (std::size_t i = 0; i < swapchain_images_.size(); i++)
    {
        record_command_buffer_(i);
    }

    SPDLOG_INFO("Recorded command buffers.");
}

void engine::create_frames_in_flight_()
{
    SPDLOG_INFO("Creating frames in flight synchronization objects...");

    frames_in_flight_.resize(MAXIMUM_FRAMES_IN_FLIGHT);

    for (auto& frame_in_flight : frames_in_flight_)
    {
        vk::SemaphoreCreateInfo create_info{};

        auto result = device_.createSemaphore(&create_info, nullptr, &frame_in_flight.image_available_semaphore, dispatch_);

        EVK_ASSERT_RESULT(result, "Failed to create semaphore.");

        result = device_.createSemaphore(&create_info, nullptr, &frame_in_flight.render_finished_semaphore, dispatch_);

        EVK_ASSERT_RESULT(result, "Failed to create semaphore.");

        vk::FenceCreateInfo fence_create_info{};

        fence_create_info.setFlags(vk::FenceCreateFlagBits::eSignaled);

        result = device_.createFence(&fence_create_info, nullptr, &frame_in_flight.fence, dispatch_);

        EVK_ASSERT_RESULT(result, "Failed to create fence.");
    }
    
    SPDLOG_INFO("Created frames in flight synchronization objects.");
}

void engine::reset_timeline_semaphore_(vk::Semaphore& timeline_semaphore, std::uint64_t initial_value)
{
    if (timeline_semaphore != static_cast<vk::Semaphore>(nullptr))
    {
        device_.destroySemaphore(timeline_semaphore, nullptr, dispatch_);

        timeline_semaphore = nullptr;
    }

    vk::StructureChain<vk::SemaphoreCreateInfo, vk::SemaphoreTypeCreateInfo> chain{};

    chain.get<vk::SemaphoreTypeCreateInfo>()
        .setSemaphoreType(vk::SemaphoreType::eTimeline)
        .setInitialValue(initial_value);

    const auto result = device_.createSemaphore(&chain.get<vk::SemaphoreCreateInfo>(), nullptr, &timeline_semaphore, dispatch_);

    EVK_ASSERT_RESULT(result, "Failed to create timeline semaphore.");
}

void engine::destroy_frames_in_flight_()
{
    SPDLOG_TRACE("Destroying frames in flight synchronization objects...");

    for (const auto& frame_in_flight : frames_in_flight_)
    {
        device_.destroySemaphore(frame_in_flight.image_available_semaphore, nullptr, dispatch_);
        device_.destroySemaphore(frame_in_flight.render_finished_semaphore, nullptr, dispatch_);
        device_.destroyFence(frame_in_flight.fence, nullptr, dispatch_);
    }

    frames_in_flight_.clear();

    SPDLOG_TRACE("Destroyed frames in flight synchronization objects.");
}

void engine::record_command_buffer_(std::uint32_t swapchain_image_index)
{
    const auto& swapchain_image = swapchain_images_[swapchain_image_index];

    const auto& cmd_buffer = swapchain_image.command_buffer;

    vk::CommandBufferBeginInfo begin_info{};

    cmd_buffer.begin(begin_info, dispatch_);

    vk::RenderPassBeginInfo render_pass_begin_info{};

    vk::ClearValue clear_color{};

    clear_color.color.setFloat32({ 0.0f, 0.0f, 0.0f, 1.0f });

    render_pass_begin_info
        .setRenderPass(render_pass_)
        .setFramebuffer(swapchain_image.framebuffer)
        .setClearValueCount(1)
        .setClearValues(clear_color);

    render_pass_begin_info.renderArea
        .setOffset(vk::Offset2D{ 0, 0 })
        .setExtent(swapchain_info_.chosen_extent);

    cmd_buffer.beginRenderPass(render_pass_begin_info, vk::SubpassContents::eInline, dispatch_);

    cmd_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, graphics_pipeline_, dispatch_);

    cmd_buffer.draw(3, 1, 0, 0, dispatch_);

    cmd_buffer.endRenderPass(dispatch_);

    cmd_buffer.end(dispatch_);
}

void engine::free_command_buffers_()
{
    SPDLOG_TRACE("Freeing command buffers...");

    for (const auto& swapchain_image : swapchain_images_)
    {
        device_.freeCommandBuffers(swapchain_image.command_pool, 1, &swapchain_image.command_buffer, dispatch_);
    }

    SPDLOG_TRACE("Freed command buffers.");
}

void engine::destroy_command_pools_()
{
    SPDLOG_TRACE("Destroying command pools...");

    for (const auto& swapchain_image : swapchain_images_)
    {
        device_.destroyCommandPool(swapchain_image.command_pool, nullptr, dispatch_);
    }

    SPDLOG_TRACE("Destroyed command pools.");
}

void engine::destroy_framebuffers_()
{
    SPDLOG_TRACE("Destroying framebuffers...");

    for (const auto& swapchain_image : swapchain_images_)
    {
        device_.destroyFramebuffer(swapchain_image.framebuffer, nullptr, dispatch_);
    }

    SPDLOG_TRACE("Destroyed framebuffers.");
}

void engine::destroy_graphics_pipeline_()
{
    SPDLOG_TRACE("Destroying graphics pipeline...");

    device_.destroyPipelineLayout(pipeline_layout_, nullptr, dispatch_);

    device_.destroyPipeline(graphics_pipeline_, nullptr, dispatch_);

    SPDLOG_TRACE("Destroyed graphics pipeline.");
}

void engine::destroy_render_pass_()
{
    SPDLOG_TRACE("Destroying render pass...");

    device_.destroyRenderPass(render_pass_, nullptr, dispatch_);

    SPDLOG_TRACE("Destroyed render pass.");
}

void engine::destroy_swapchain_image_views_()
{
    SPDLOG_TRACE("Destroying swapchain image views...");

    for (const auto& swapchain_image : swapchain_images_)
    {
        device_.destroyImageView(swapchain_image.image_view, nullptr, dispatch_);
    }

    swapchain_images_.clear();

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
        //vulkantesting_debug_break();
    }

    return false;
}

void engine::recreate_swapchain_()
{
    device_.waitIdle(dispatch_);

    destroy_frames_in_flight_();
    free_command_buffers_();
    destroy_command_pools_();
    destroy_framebuffers_();
    destroy_render_pass_();
    destroy_swapchain_image_views_();
    //destroy_surface_();

    current_frame_ = 0;

    //create_surface_();
    query_swapchain_support_();
    create_swapchain_();
    retrieve_swapchain_images_();
    create_render_pass_();
    create_framebuffers_();
    create_command_pools_();
    allocate_command_buffers_();
    record_command_buffers_();
    create_frames_in_flight_();

    out_of_date_ = false;
}
