#ifndef SDL_WINDOW_H_
#define SDL_WINDOW_H_

#include "core/core.h"

class sdl_window
{
public:
    sdl_window() = delete;
    sdl_window(engine& p_engine);
    ~sdl_window();

    std::uint32_t width() { return width_; }
    std::uint32_t height() { return height_; }

    void process_events();

private:
    void on_quit_();

    engine* engine_{ nullptr };

    sdl::context_pointer_type sdl_context_;
    sdl::window* sdl_window_{ nullptr };

    std::uint32_t width_{ 512 };
    std::uint32_t height_{ 512 };
};

#endif
