#include "core/core.h"

#include "log/spdlog/sinks/basic_file_sink.h"

#include "core/engine.h"

void initialize_logging()
{
    spdlog::init_thread_pool(8192, 1);

    auto default_logger = spdlog::default_logger();

    default_logger->set_level(spdlog::level::trace);

    auto file_sink_ptr = std::make_shared<spdlog::sinks::basic_file_sink_mt>("vulkantesting.log", true);
    default_logger->sinks().emplace_back(std::move(file_sink_ptr));

    spdlog::set_pattern("%T.%f %-7t %-20s %-8l : %^%v%$");
}

#ifdef WIN32
int wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
#else
int main(int argc, char* argv[])
#endif
{
#ifdef WIN32
    AllocConsole();
#endif

    initialize_logging();

    SPDLOG_INFO("Vulkan testing started.");

    SPDLOG_INFO("Initializing engine...");

    int result{ -1 };

    {
        engine engine_instance;

        engine_instance.initialize();

        SPDLOG_INFO("Engine initialized.");

        result = engine_instance.run();

        engine_instance.destroy();
    }

    SPDLOG_INFO("Vulkan testing exiting (result = {}).", result);

    if (has_environment_variable("VKT_KEEP_HANGING"))
    {
        SPDLOG_INFO("Hanging around (VKT_KEEP_HANGING is set).");

        while (true)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

    return result;
}
