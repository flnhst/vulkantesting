#include "sdl_window.h"

#include "engine.h"

sdl_window::sdl_window(engine& p_engine)
    : engine_(&p_engine)
{
    sdl_context_ = sdl::context::init(sdl::init_flag::VIDEO);

    sdl_context_->on_quit(std::bind(&sdl_window::on_quit_, this));

    std::uint32_t x{ 200 };

#ifdef WIN32
    x = 1000;
#endif

    sdl_window_ = &sdl_context_->create_window("Vulkan Testing", x, 200, width_, height_,SDL_WINDOW_ALLOW_HIGHDPI);
}

sdl_window::~sdl_window()
{
    sdl_context_->destroy_window(*sdl_window_);

    sdl_context_.reset();
}

void sdl_window::process_events()
{
    sdl_context_->process_event_queue();
}

SDL_SysWMinfo sdl_window::get_system_wm_info()
{
    return sdl_window_->get_system_wm_info();
}

void sdl_window::on_quit_()
{
    SPDLOG_INFO("SDL is quitting.");

    engine_->stop();
}


