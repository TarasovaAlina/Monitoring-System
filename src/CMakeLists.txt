# смотрим, чтобы были установлены необходимые пакеты
find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(CURL QUIET)

#if(NOT CURL_FOUND)
#    message(STATUS "Библиотека curl не установлена в системе, установка...")
#
#    include(FetchContent)
#
#    # Настраиваем скачивание
#    FetchContent_Declare(
#            curl
#            GIT_REPOSITORY https://github.com/curl/curl.git
#            GIT_TAG curl-8_22_0
#    )
#
#    FetchContent_MakeAvailable(curl)
#endif()

file(GLOB cpu_monitoring_agent_src_files "agent/cpu/*.cpp")
file(GLOB memory_monitoring_agent_src_files "agent/memory/*.cpp")
file(GLOB network_monitoring_agent_src_files "agent/network/*.cpp")
file(GLOB core_src_files "core/*.cpp")
file(GLOB gui_files "gui/*.cpp" "../include/gui/*.h")

# Динамические библиотеки агентов
add_library(cpu_monitoring_agent SHARED ${cpu_monitoring_agent_src_files})
add_library(memory_monitoring_agent SHARED ${memory_monitoring_agent_src_files})
target_link_libraries(memory_monitoring_agent PRIVATE libcurl)
target_include_directories(memory_monitoring_agent PRIVATE ${curl_SOURCE_DIR}/include)
add_library(network_monitoring_agent SHARED ${network_monitoring_agent_src_files})

add_executable(SystemMonitoring
        ${gui_files}
        ${core_src_files}
)
target_link_libraries(SystemMonitoring PRIVATE Qt6::Widgets)