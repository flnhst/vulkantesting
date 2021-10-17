#########################################################################
# HEADER
#########################################################################

cmake_minimum_required(VERSION 3.17 FATAL_ERROR)

project(vulkantesting)

set(CMAKE_CXX_STANDARD 17)

if(NOT WIN32)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

cmake_policy(SET CMP0054 NEW)
cmake_policy(SET CMP0112 NEW)

#########################################################################
# EXTERNAL DEPENDENCY HEADERS
#########################################################################

# Headers


# External dependency 'GLM' begin
# User variables begin
set(GLM_INCLUDE_DIR CACHE PATH "")
if(GLM_INCLUDE_DIR STREQUAL "")
    message(FATAL_ERROR "Please specify 'PATH' variable 'GLM_INCLUDE_DIR'.")
endif()
if(NOT IS_DIRECTORY ${GLM_INCLUDE_DIR})
    message(FATAL_ERROR "Cannot find directory '${GLM_INCLUDE_DIR}' specified in variable 'GLM_INCLUDE_DIR'.")
endif()

# User variables end
add_library(GLM INTERFACE IMPORTED)
set_property(TARGET GLM PROPERTY INTERFACE_INCLUDE_DIRECTORIES
    "${GLM_INCLUDE_DIR}"
    )
# External dependency 'GLM' end

# External dependency 'SDL2' begin
# User variables begin
set(SDL2_INCLUDE_DIRECTORY CACHE PATH "")
if(SDL2_INCLUDE_DIRECTORY STREQUAL "")
    message(FATAL_ERROR "Please specify 'PATH' variable 'SDL2_INCLUDE_DIRECTORY'.")
endif()
if(NOT IS_DIRECTORY ${SDL2_INCLUDE_DIRECTORY})
    message(FATAL_ERROR "Cannot find directory '${SDL2_INCLUDE_DIRECTORY}' specified in variable 'SDL2_INCLUDE_DIRECTORY'.")
endif()

set(SDL2_RELEASE_BINARY_DIRECTORY CACHE PATH "")
if(SDL2_RELEASE_BINARY_DIRECTORY STREQUAL "")
    message(FATAL_ERROR "Please specify 'PATH' variable 'SDL2_RELEASE_BINARY_DIRECTORY'.")
endif()
if(NOT IS_DIRECTORY ${SDL2_RELEASE_BINARY_DIRECTORY})
    message(FATAL_ERROR "Cannot find directory '${SDL2_RELEASE_BINARY_DIRECTORY}' specified in variable 'SDL2_RELEASE_BINARY_DIRECTORY'.")
endif()

set(SDL2_DEBUG_BINARY_DIRECTORY CACHE PATH "")
if(SDL2_DEBUG_BINARY_DIRECTORY STREQUAL "")
    message(FATAL_ERROR "Please specify 'PATH' variable 'SDL2_DEBUG_BINARY_DIRECTORY'.")
endif()
if(NOT IS_DIRECTORY ${SDL2_DEBUG_BINARY_DIRECTORY})
    message(FATAL_ERROR "Cannot find directory '${SDL2_DEBUG_BINARY_DIRECTORY}' specified in variable 'SDL2_DEBUG_BINARY_DIRECTORY'.")
endif()

set(SDL2_RELEASE_LIBRARY CACHE FILEPATH "")
if(SDL2_RELEASE_LIBRARY STREQUAL "")
    message(FATAL_ERROR "Please specify 'FILEPATH' variable 'SDL2_RELEASE_LIBRARY'.")
endif()
if(NOT EXISTS ${SDL2_RELEASE_LIBRARY})
    message(FATAL_ERROR "Cannot find file '${SDL2_RELEASE_LIBRARY}' specified in variable 'SDL2_RELEASE_LIBRARY'.")
endif()

set(SDL2_DEBUG_LIBRARY CACHE FILEPATH "")
if(SDL2_DEBUG_LIBRARY STREQUAL "")
    message(FATAL_ERROR "Please specify 'FILEPATH' variable 'SDL2_DEBUG_LIBRARY'.")
endif()
if(NOT EXISTS ${SDL2_DEBUG_LIBRARY})
    message(FATAL_ERROR "Cannot find file '${SDL2_DEBUG_LIBRARY}' specified in variable 'SDL2_DEBUG_LIBRARY'.")
endif()

# User variables end
add_library(SDL2 INTERFACE IMPORTED)
add_library(SDL2::lib UNKNOWN IMPORTED)
set_target_properties(SDL2::lib PROPERTIES
    IMPORTED_LOCATION_DEBUG ${SDL2_DEBUG_LIBRARY}
    )
set_target_properties(SDL2::lib PROPERTIES
    IMPORTED_LOCATION_RELWITHDEBINFO ${SDL2_DEBUG_LIBRARY}
    )
set_target_properties(SDL2::lib PROPERTIES
    IMPORTED_LOCATION_RELEASE ${SDL2_RELEASE_LIBRARY}
    )
set_target_properties(SDL2::lib PROPERTIES
    IMPORTED_LOCATION_MINSIZEREL ${SDL2_RELEASE_LIBRARY}
    )
set_target_properties(SDL2::lib PROPERTIES
    IMPORTED_LOCATION ${SDL2_RELEASE_LIBRARY}
    )
set(CURRENT_INTERFACE_LINK_LIBRARIES)
get_property(CURRENT_INTERFACE_LINK_LIBRARIES TARGET SDL2 PROPERTY INTERFACE_LINK_LIBRARIES)
set_property(TARGET SDL2 PROPERTY INTERFACE_LINK_LIBRARIES
    "${CURRENT_INTERFACE_LINK_LIBRARIES}"
    "SDL2::lib"
    )
set_property(TARGET SDL2 PROPERTY INTERFACE_INCLUDE_DIRECTORIES
    "${SDL2_INCLUDE_DIRECTORY}"
    )
# External dependency 'SDL2' end

# External dependency 'SDLPlusPlus' begin
# User variables begin
set(SDLPLUSPLUS_DEBUG_INSTALL_DIR CACHE PATH "")
if(SDLPLUSPLUS_DEBUG_INSTALL_DIR STREQUAL "")
    message(FATAL_ERROR "Please specify 'PATH' variable 'SDLPLUSPLUS_DEBUG_INSTALL_DIR'.")
endif()
if(NOT IS_DIRECTORY ${SDLPLUSPLUS_DEBUG_INSTALL_DIR})
    message(FATAL_ERROR "Cannot find directory '${SDLPLUSPLUS_DEBUG_INSTALL_DIR}' specified in variable 'SDLPLUSPLUS_DEBUG_INSTALL_DIR'.")
endif()

set(SDLPLUSPLUS_RELEASE_INSTALL_DIR CACHE PATH "")
if(SDLPLUSPLUS_RELEASE_INSTALL_DIR STREQUAL "")
    message(FATAL_ERROR "Please specify 'PATH' variable 'SDLPLUSPLUS_RELEASE_INSTALL_DIR'.")
endif()
if(NOT IS_DIRECTORY ${SDLPLUSPLUS_RELEASE_INSTALL_DIR})
    message(FATAL_ERROR "Cannot find directory '${SDLPLUSPLUS_RELEASE_INSTALL_DIR}' specified in variable 'SDLPLUSPLUS_RELEASE_INSTALL_DIR'.")
endif()

# User variables end
add_library(SDLPlusPlus INTERFACE IMPORTED)
if(WIN32)
    add_library(SDLPlusPlus::lib UNKNOWN IMPORTED)
    set_target_properties(SDLPlusPlus::lib PROPERTIES
        IMPORTED_LOCATION_DEBUG ${SDLPLUSPLUS_DEBUG_INSTALL_DIR}/lib/sdlplusplus.lib
        )
    set_target_properties(SDLPlusPlus::lib PROPERTIES
        IMPORTED_LOCATION_RELWITHDEBINFO ${SDLPLUSPLUS_DEBUG_INSTALL_DIR}/lib/sdlplusplus.lib
        )
    set_target_properties(SDLPlusPlus::lib PROPERTIES
        IMPORTED_LOCATION_RELEASE ${SDLPLUSPLUS_RELEASE_INSTALL_DIR}/lib/sdlplusplus.lib
        )
    set_target_properties(SDLPlusPlus::lib PROPERTIES
        IMPORTED_LOCATION_MINSIZEREL ${SDLPLUSPLUS_RELEASE_INSTALL_DIR}/lib/sdlplusplus.lib
        )
    set_target_properties(SDLPlusPlus::lib PROPERTIES
        IMPORTED_LOCATION ${SDLPLUSPLUS_RELEASE_INSTALL_DIR}/lib/sdlplusplus.lib
        )
    set(CURRENT_INTERFACE_LINK_LIBRARIES)
    get_property(CURRENT_INTERFACE_LINK_LIBRARIES TARGET SDLPlusPlus PROPERTY INTERFACE_LINK_LIBRARIES)
    set_property(TARGET SDLPlusPlus PROPERTY INTERFACE_LINK_LIBRARIES
        "${CURRENT_INTERFACE_LINK_LIBRARIES}"
        "SDLPlusPlus::lib"
        )
endif()
if(UNIX)
    add_library(SDLPlusPlus::lib UNKNOWN IMPORTED)
    set_target_properties(SDLPlusPlus::lib PROPERTIES
        IMPORTED_LOCATION_DEBUG ${SDLPLUSPLUS_DEBUG_INSTALL_DIR}/lib/libsdlplusplus.a
        )
    set_target_properties(SDLPlusPlus::lib PROPERTIES
        IMPORTED_LOCATION_RELWITHDEBINFO ${SDLPLUSPLUS_DEBUG_INSTALL_DIR}/lib/libsdlplusplus.a
        )
    set_target_properties(SDLPlusPlus::lib PROPERTIES
        IMPORTED_LOCATION_RELEASE ${SDLPLUSPLUS_RELEASE_INSTALL_DIR}/lib/libsdlplusplus.a
        )
    set_target_properties(SDLPlusPlus::lib PROPERTIES
        IMPORTED_LOCATION_MINSIZEREL ${SDLPLUSPLUS_RELEASE_INSTALL_DIR}/lib/libsdlplusplus.a
        )
    set_target_properties(SDLPlusPlus::lib PROPERTIES
        IMPORTED_LOCATION ${SDLPLUSPLUS_RELEASE_INSTALL_DIR}/lib/libsdlplusplus.a
        )
    set(CURRENT_INTERFACE_LINK_LIBRARIES)
    get_property(CURRENT_INTERFACE_LINK_LIBRARIES TARGET SDLPlusPlus PROPERTY INTERFACE_LINK_LIBRARIES)
    set_property(TARGET SDLPlusPlus PROPERTY INTERFACE_LINK_LIBRARIES
        "${CURRENT_INTERFACE_LINK_LIBRARIES}"
        "SDLPlusPlus::lib"
        )
endif()
set_property(TARGET SDLPlusPlus PROPERTY INTERFACE_INCLUDE_DIRECTORIES
    "${SDLPLUSPLUS_DEBUG_INSTALL_DIR}/include"
    )
# External dependency 'SDLPlusPlus' end

# External dependency 'Vulkan' begin
# User variables begin
set(VULKAN_LOADER_DEBUG_INSTALL_DIR CACHE PATH "")
if(VULKAN_LOADER_DEBUG_INSTALL_DIR STREQUAL "")
    message(FATAL_ERROR "Please specify 'PATH' variable 'VULKAN_LOADER_DEBUG_INSTALL_DIR'.")
endif()
if(NOT IS_DIRECTORY ${VULKAN_LOADER_DEBUG_INSTALL_DIR})
    message(FATAL_ERROR "Cannot find directory '${VULKAN_LOADER_DEBUG_INSTALL_DIR}' specified in variable 'VULKAN_LOADER_DEBUG_INSTALL_DIR'.")
endif()

set(VULKAN_LOADER_RELEASE_INSTALL_DIR CACHE PATH "")
if(VULKAN_LOADER_RELEASE_INSTALL_DIR STREQUAL "")
    message(FATAL_ERROR "Please specify 'PATH' variable 'VULKAN_LOADER_RELEASE_INSTALL_DIR'.")
endif()
if(NOT IS_DIRECTORY ${VULKAN_LOADER_RELEASE_INSTALL_DIR})
    message(FATAL_ERROR "Cannot find directory '${VULKAN_LOADER_RELEASE_INSTALL_DIR}' specified in variable 'VULKAN_LOADER_RELEASE_INSTALL_DIR'.")
endif()

set(VULKAN_HEADERS_RELEASE_INSTALL_DIR CACHE PATH "")
if(VULKAN_HEADERS_RELEASE_INSTALL_DIR STREQUAL "")
    message(FATAL_ERROR "Please specify 'PATH' variable 'VULKAN_HEADERS_RELEASE_INSTALL_DIR'.")
endif()
if(NOT IS_DIRECTORY ${VULKAN_HEADERS_RELEASE_INSTALL_DIR})
    message(FATAL_ERROR "Cannot find directory '${VULKAN_HEADERS_RELEASE_INSTALL_DIR}' specified in variable 'VULKAN_HEADERS_RELEASE_INSTALL_DIR'.")
endif()

# User variables end
add_library(Vulkan INTERFACE IMPORTED)
if(WIN32)
    add_library(loader UNKNOWN IMPORTED)
    set_target_properties(loader PROPERTIES
        IMPORTED_LOCATION_DEBUG ${VULKAN_LOADER_DEBUG_INSTALL_DIR}/lib/vulkan-1.lib
        )
    set_target_properties(loader PROPERTIES
        IMPORTED_LOCATION_RELWITHDEBINFO ${VULKAN_LOADER_DEBUG_INSTALL_DIR}/lib/vulkan-1.lib
        )
    set_target_properties(loader PROPERTIES
        IMPORTED_LOCATION_RELEASE ${VULKAN_LOADER_RELEASE_INSTALL_DIR}/lib/vulkan-1.lib
        )
    set_target_properties(loader PROPERTIES
        IMPORTED_LOCATION_MINSIZEREL ${VULKAN_LOADER_RELEASE_INSTALL_DIR}/lib/vulkan-1.lib
        )
    set_target_properties(loader PROPERTIES
        IMPORTED_LOCATION ${VULKAN_LOADER_RELEASE_INSTALL_DIR}/lib/vulkan-1.lib
        )
    set(CURRENT_INTERFACE_LINK_LIBRARIES)
    get_property(CURRENT_INTERFACE_LINK_LIBRARIES TARGET Vulkan PROPERTY INTERFACE_LINK_LIBRARIES)
    set_property(TARGET Vulkan PROPERTY INTERFACE_LINK_LIBRARIES
        "${CURRENT_INTERFACE_LINK_LIBRARIES}"
        "loader"
        )
endif()
if(UNIX)
    add_library(loader UNKNOWN IMPORTED)
    set_target_properties(loader PROPERTIES
        IMPORTED_LOCATION_DEBUG ${VULKAN_LOADER_DEBUG_INSTALL_DIR}/lib/libvulkan.so
        )
    set_target_properties(loader PROPERTIES
        IMPORTED_LOCATION_RELWITHDEBINFO ${VULKAN_LOADER_DEBUG_INSTALL_DIR}/lib/libvulkan.so
        )
    set_target_properties(loader PROPERTIES
        IMPORTED_LOCATION_RELEASE ${VULKAN_LOADER_RELEASE_INSTALL_DIR}/lib/libvulkan.so
        )
    set_target_properties(loader PROPERTIES
        IMPORTED_LOCATION_MINSIZEREL ${VULKAN_LOADER_RELEASE_INSTALL_DIR}/lib/libvulkan.so
        )
    set_target_properties(loader PROPERTIES
        IMPORTED_LOCATION ${VULKAN_LOADER_RELEASE_INSTALL_DIR}/lib/libvulkan.so
        )
    set(CURRENT_INTERFACE_LINK_LIBRARIES)
    get_property(CURRENT_INTERFACE_LINK_LIBRARIES TARGET Vulkan PROPERTY INTERFACE_LINK_LIBRARIES)
    set_property(TARGET Vulkan PROPERTY INTERFACE_LINK_LIBRARIES
        "${CURRENT_INTERFACE_LINK_LIBRARIES}"
        "loader"
        )
endif()
set_property(TARGET Vulkan PROPERTY INTERFACE_INCLUDE_DIRECTORIES
    "${VULKAN_HEADERS_RELEASE_INSTALL_DIR}/include"
    )
# External dependency 'Vulkan' end

# External dependency 'Threads' begin
find_package(Threads REQUIRED )
# External dependency 'Threads' end

#########################################################################
# SOURCES
#########################################################################

set(ALL_SOURCES)
set(CORE_SOURCE_DIR src/core)
set(CORE_SOURCES 
    ${CORE_SOURCE_DIR}/core.cpp
    ${CORE_SOURCE_DIR}/core.h
    ${CORE_SOURCE_DIR}/engine.cpp
    ${CORE_SOURCE_DIR}/engine.h
    ${CORE_SOURCE_DIR}/sdl_window.cpp
    ${CORE_SOURCE_DIR}/sdl_window.h
    Project.bgp)

source_group(core FILES ${CORE_SOURCE_DIR})

set(LOG_SOURCE_DIR src/log)
set(LOG_SOURCES 
    ${LOG_SOURCE_DIR}/async.cpp
    ${LOG_SOURCE_DIR}/async_logger.cpp
    ${LOG_SOURCE_DIR}/common.cpp
    ${LOG_SOURCE_DIR}/details/file_helper.cpp
    ${LOG_SOURCE_DIR}/details/helpers.cpp
    ${LOG_SOURCE_DIR}/details/log_msg.cpp
    ${LOG_SOURCE_DIR}/details/log_msg_buffer.cpp
    ${LOG_SOURCE_DIR}/details/os.cpp
    ${LOG_SOURCE_DIR}/details/periodic_worker.cpp
    ${LOG_SOURCE_DIR}/details/registry.cpp
    ${LOG_SOURCE_DIR}/details/thread_pool.cpp
    ${LOG_SOURCE_DIR}/fmt.cpp
    ${LOG_SOURCE_DIR}/log.cpp
    ${LOG_SOURCE_DIR}/log.h
    ${LOG_SOURCE_DIR}/logger.cpp
    ${LOG_SOURCE_DIR}/pattern_formatter.cpp
    ${LOG_SOURCE_DIR}/sinks/ansicolor_sink.cpp
    ${LOG_SOURCE_DIR}/sinks/base_sink.cpp
    ${LOG_SOURCE_DIR}/sinks/basic_file_sink.cpp
    ${LOG_SOURCE_DIR}/sinks/rotating_file_sink.cpp
    ${LOG_SOURCE_DIR}/sinks/sink.cpp
    ${LOG_SOURCE_DIR}/sinks/stdout_color_sinks.cpp
    ${LOG_SOURCE_DIR}/sinks/stdout_sinks.cpp
    ${LOG_SOURCE_DIR}/sinks/wincolor_sink.cpp
    ${LOG_SOURCE_DIR}/spdlog/async.h
    ${LOG_SOURCE_DIR}/spdlog/async_logger.h
    ${LOG_SOURCE_DIR}/spdlog/cfg/argv.h
    ${LOG_SOURCE_DIR}/spdlog/cfg/env.h
    ${LOG_SOURCE_DIR}/spdlog/cfg/helpers.h
    ${LOG_SOURCE_DIR}/spdlog/cfg/log_levels.h
    ${LOG_SOURCE_DIR}/spdlog/common.h
    ${LOG_SOURCE_DIR}/spdlog/details/circular_q.h
    ${LOG_SOURCE_DIR}/spdlog/details/console_globals.h
    ${LOG_SOURCE_DIR}/spdlog/details/file_helper.h
    ${LOG_SOURCE_DIR}/spdlog/details/fmt_helper.h
    ${LOG_SOURCE_DIR}/spdlog/details/log_msg.h
    ${LOG_SOURCE_DIR}/spdlog/details/log_msg_buffer.h
    ${LOG_SOURCE_DIR}/spdlog/details/mpmc_blocking_q.h
    ${LOG_SOURCE_DIR}/spdlog/details/null_mutex.h
    ${LOG_SOURCE_DIR}/spdlog/details/os.h
    ${LOG_SOURCE_DIR}/spdlog/details/periodic_worker.h
    ${LOG_SOURCE_DIR}/spdlog/details/registry.h
    ${LOG_SOURCE_DIR}/spdlog/details/synchronous_factory.h
    ${LOG_SOURCE_DIR}/spdlog/details/tcp_client-windows.h
    ${LOG_SOURCE_DIR}/spdlog/details/tcp_client.h
    ${LOG_SOURCE_DIR}/spdlog/details/thread_pool.h
    ${LOG_SOURCE_DIR}/spdlog/details/windows_include.h
    ${LOG_SOURCE_DIR}/spdlog/fmt/bin_to_hex.h
    ${LOG_SOURCE_DIR}/spdlog/fmt/bundled/chrono.h
    ${LOG_SOURCE_DIR}/spdlog/fmt/bundled/color.h
    ${LOG_SOURCE_DIR}/spdlog/fmt/bundled/compile.h
    ${LOG_SOURCE_DIR}/spdlog/fmt/bundled/core.h
    ${LOG_SOURCE_DIR}/spdlog/fmt/bundled/format-inl.h
    ${LOG_SOURCE_DIR}/spdlog/fmt/bundled/format.h
    ${LOG_SOURCE_DIR}/spdlog/fmt/bundled/locale.h
    ${LOG_SOURCE_DIR}/spdlog/fmt/bundled/ostream.h
    ${LOG_SOURCE_DIR}/spdlog/fmt/bundled/posix.h
    ${LOG_SOURCE_DIR}/spdlog/fmt/bundled/printf.h
    ${LOG_SOURCE_DIR}/spdlog/fmt/bundled/ranges.h
    ${LOG_SOURCE_DIR}/spdlog/fmt/fmt.h
    ${LOG_SOURCE_DIR}/spdlog/fmt/ostr.h
    ${LOG_SOURCE_DIR}/spdlog/formatter.h
    ${LOG_SOURCE_DIR}/spdlog/fwd.h
    ${LOG_SOURCE_DIR}/spdlog/logger.h
    ${LOG_SOURCE_DIR}/spdlog/pattern_formatter.h
    ${LOG_SOURCE_DIR}/spdlog/sinks/android_sink.h
    ${LOG_SOURCE_DIR}/spdlog/sinks/ansicolor_sink.h
    ${LOG_SOURCE_DIR}/spdlog/sinks/base_sink.h
    ${LOG_SOURCE_DIR}/spdlog/sinks/basic_file_sink.h
    ${LOG_SOURCE_DIR}/spdlog/sinks/daily_file_sink.h
    ${LOG_SOURCE_DIR}/spdlog/sinks/dist_sink.h
    ${LOG_SOURCE_DIR}/spdlog/sinks/dup_filter_sink.h
    ${LOG_SOURCE_DIR}/spdlog/sinks/msvc_sink.h
    ${LOG_SOURCE_DIR}/spdlog/sinks/null_sink.h
    ${LOG_SOURCE_DIR}/spdlog/sinks/ostream_sink.h
    ${LOG_SOURCE_DIR}/spdlog/sinks/ringbuffer_sink.h
    ${LOG_SOURCE_DIR}/spdlog/sinks/rotating_file_sink.h
    ${LOG_SOURCE_DIR}/spdlog/sinks/sink.h
    ${LOG_SOURCE_DIR}/spdlog/sinks/stdout_color_sinks.h
    ${LOG_SOURCE_DIR}/spdlog/sinks/stdout_sinks.h
    ${LOG_SOURCE_DIR}/spdlog/sinks/syslog_sink.h
    ${LOG_SOURCE_DIR}/spdlog/sinks/systemd_sink.h
    ${LOG_SOURCE_DIR}/spdlog/sinks/tcp_sink.h
    ${LOG_SOURCE_DIR}/spdlog/sinks/wincolor_sink.h
    ${LOG_SOURCE_DIR}/spdlog/sinks/win_eventlog_sink.h
    ${LOG_SOURCE_DIR}/spdlog/spdlog-inl.h
    ${LOG_SOURCE_DIR}/spdlog/spdlog.h
    ${LOG_SOURCE_DIR}/spdlog/stopwatch.h
    ${LOG_SOURCE_DIR}/spdlog/tweakme.h
    ${LOG_SOURCE_DIR}/spdlog/version.h
    ${LOG_SOURCE_DIR}/spdlog.cpp
    Project.bgp)

source_group(log FILES ${LOG_SOURCE_DIR})

set(VULKANTESTING_SOURCE_DIR src/main)
set(VULKANTESTING_SOURCES 
    ${VULKANTESTING_SOURCE_DIR}/main.cpp
    Project.bgp)

source_group(vulkantesting FILES ${VULKANTESTING_SOURCE_DIR})

set(ALL_SOURCES ${ALL_SOURCES} ${CORE_SOURCES})
set(ALL_SOURCES ${ALL_SOURCES} ${LOG_SOURCES})
set(ALL_SOURCES ${ALL_SOURCES} ${VULKANTESTING_SOURCES})

#########################################################################
# TARGETS
#########################################################################

add_library(core ${CORE_SOURCES})

add_library(log ${LOG_SOURCES})

add_executable(vulkantesting ${VULKANTESTING_SOURCES})
if(WIN32)
    target_link_options(vulkantesting PRIVATE "/SUBSYSTEM:WINDOWS")
endif()
if(UNIX)
    if(CMAKE_COMPILER_IS_GNUCXX
        OR
        "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        set_property(TARGET vulkantesting PROPERTY LINK_OPTIONS
            -fuse-ld=lld
            )
    endif()
endif()

#########################################################################
# INCLUDES
#########################################################################

target_include_directories(core PUBLIC src src/log)
target_include_directories(log PUBLIC src src/log)
target_include_directories(vulkantesting PUBLIC src src/log)

#########################################################################
# DEFINITIONS
#########################################################################

target_compile_definitions(core PUBLIC SDL_MAIN_HANDLED)
target_compile_definitions(core PUBLIC _SCL_SECURE_NO_WARNINGS)
target_compile_definitions(core PUBLIC NOMINMAX)
target_compile_definitions(core PUBLIC _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS)
target_compile_definitions(core PUBLIC _HAS_AUTO_PTR_ETC=1)
set(DEFINITION_0_CMAKE_BUILD_TYPE "$<CONFIG>")
string(REPLACE "\\" "\\\\" DEFINITION_0_CMAKE_BUILD_TYPE ${DEFINITION_0_CMAKE_BUILD_TYPE})
target_compile_definitions(core PUBLIC CMAKE_BUILD_TYPE="${DEFINITION_0_CMAKE_BUILD_TYPE}")
target_compile_definitions(core PUBLIC CMAKE_BUILD_TYPE_DEBUG=$<CONFIG:Debug>)
target_compile_definitions(core PUBLIC CMAKE_BUILD_TYPE_RELEASE=$<CONFIG:Release>)
target_compile_definitions(core PUBLIC CMAKE_BUILD_TYPE_RELWITHDEBINFO=$<CONFIG:RelWithDebInfo>)
target_compile_definitions(core PUBLIC CMAKE_BUILD_TYPE_MINSIZEREL=$<CONFIG:MinSizeRel>)
target_compile_definitions(core PUBLIC WINVER=_WIN32_WINNT_WIN10)
target_compile_definitions(core PUBLIC _WIN32_WINNT=_WIN32_WINNT_WIN10)
target_compile_definitions(log PUBLIC SDL_MAIN_HANDLED)
target_compile_definitions(log PUBLIC _SCL_SECURE_NO_WARNINGS)
target_compile_definitions(log PUBLIC NOMINMAX)
target_compile_definitions(log PUBLIC _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS)
target_compile_definitions(log PUBLIC _HAS_AUTO_PTR_ETC=1)
set(DEFINITION_1_CMAKE_BUILD_TYPE "$<CONFIG>")
string(REPLACE "\\" "\\\\" DEFINITION_1_CMAKE_BUILD_TYPE ${DEFINITION_1_CMAKE_BUILD_TYPE})
target_compile_definitions(log PUBLIC CMAKE_BUILD_TYPE="${DEFINITION_1_CMAKE_BUILD_TYPE}")
target_compile_definitions(log PUBLIC CMAKE_BUILD_TYPE_DEBUG=$<CONFIG:Debug>)
target_compile_definitions(log PUBLIC CMAKE_BUILD_TYPE_RELEASE=$<CONFIG:Release>)
target_compile_definitions(log PUBLIC CMAKE_BUILD_TYPE_RELWITHDEBINFO=$<CONFIG:RelWithDebInfo>)
target_compile_definitions(log PUBLIC CMAKE_BUILD_TYPE_MINSIZEREL=$<CONFIG:MinSizeRel>)
target_compile_definitions(log PUBLIC WINVER=_WIN32_WINNT_WIN10)
target_compile_definitions(log PUBLIC _WIN32_WINNT=_WIN32_WINNT_WIN10)
target_compile_definitions(vulkantesting PUBLIC SDL_MAIN_HANDLED)
target_compile_definitions(vulkantesting PUBLIC _SCL_SECURE_NO_WARNINGS)
target_compile_definitions(vulkantesting PUBLIC NOMINMAX)
target_compile_definitions(vulkantesting PUBLIC _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS)
target_compile_definitions(vulkantesting PUBLIC _HAS_AUTO_PTR_ETC=1)
set(DEFINITION_2_CMAKE_BUILD_TYPE "$<CONFIG>")
string(REPLACE "\\" "\\\\" DEFINITION_2_CMAKE_BUILD_TYPE ${DEFINITION_2_CMAKE_BUILD_TYPE})
target_compile_definitions(vulkantesting PUBLIC CMAKE_BUILD_TYPE="${DEFINITION_2_CMAKE_BUILD_TYPE}")
target_compile_definitions(vulkantesting PUBLIC CMAKE_BUILD_TYPE_DEBUG=$<CONFIG:Debug>)
target_compile_definitions(vulkantesting PUBLIC CMAKE_BUILD_TYPE_RELEASE=$<CONFIG:Release>)
target_compile_definitions(vulkantesting PUBLIC CMAKE_BUILD_TYPE_RELWITHDEBINFO=$<CONFIG:RelWithDebInfo>)
target_compile_definitions(vulkantesting PUBLIC CMAKE_BUILD_TYPE_MINSIZEREL=$<CONFIG:MinSizeRel>)
target_compile_definitions(vulkantesting PUBLIC WINVER=_WIN32_WINNT_WIN10)
target_compile_definitions(vulkantesting PUBLIC _WIN32_WINNT=_WIN32_WINNT_WIN10)

#########################################################################
# DEPENDENCIES
#########################################################################

target_link_libraries(core
    log
    GLM
    SDL2
    SDLPlusPlus
    Vulkan
    Threads::Threads
    )

target_link_libraries(log
    Threads::Threads
    )

target_link_libraries(vulkantesting
    core
    log
    GLM
    SDL2
    SDLPlusPlus
    Vulkan
    Threads::Threads
    )


#########################################################################
# EXTERNAL DEPENDENCY HEADERS
#########################################################################

# Footers


# External dependency 'GLM' begin
# External dependency 'GLM' end

# External dependency 'SDL2' begin
if(WIN32)
    add_custom_command(TARGET vulkantesting POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<$<CONFIG:Debug>:\"${SDL2_DEBUG_BINARY_DIRECTORY}\\SDL2.dll\">$<$<CONFIG:Release>:\"${SDL2_RELEASE_BINARY_DIRECTORY}\\SDL2.dll\">$<$<CONFIG:RelWithDebInfo>:\"${SDL2_DEBUG_BINARY_DIRECTORY}\\SDL2.dll\">$<$<CONFIG:MinSizeRel>:\"${SDL2_RELEASE_BINARY_DIRECTORY}\\SDL2.dll\">  \"$<TARGET_FILE_DIR:vulkantesting>\" >>copies.log 2>&1 || $ENV{COMSPEC} /c \"exit /b 0\")
endif()
# External dependency 'SDL2' end

# External dependency 'SDLPlusPlus' begin
# External dependency 'SDLPlusPlus' end

# External dependency 'Vulkan' begin
if(WIN32)
    add_custom_command(TARGET vulkantesting POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<$<CONFIG:Debug>:\"${VULKAN_LOADER_DEBUG_INSTALL_DIR}\\bin\\vulkan-1.dll\">$<$<CONFIG:Release>:\"${VULKAN_LOADER_RELEASE_INSTALL_DIR}\\bin\\vulkan-1.dll\">$<$<CONFIG:RelWithDebInfo>:\"${VULKAN_LOADER_DEBUG_INSTALL_DIR}\\bin\\vulkan-1.dll\">$<$<CONFIG:MinSizeRel>:\"${VULKAN_LOADER_RELEASE_INSTALL_DIR}\\bin\\vulkan-1.dll\">  \"$<TARGET_FILE_DIR:vulkantesting>\" >>copies.log 2>&1 || $ENV{COMSPEC} /c \"exit /b 0\")
endif()
# External dependency 'Vulkan' end

# External dependency 'Threads' begin
# External dependency 'Threads' end

#########################################################################
# INSTALL
#########################################################################

include(GNUInstallDirs)


#########################################################################
# COMPILER FLAGS
#########################################################################

if(CMAKE_COMPILER_IS_GNUCXX
    OR
    "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -Wno-invalid-offsetof -Wno-comment")
    
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fPIC")
endif()

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
    
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /ignore:4217 /ignore:4049")
endif()

