# смотрим, чтобы были установлены необходимые пакеты
find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(cpr CONFIG QUIET) # Для работы NetworkCollector
find_package(nlohmann_json QUIET) # Для работы ConfigService (сохранение конфигов в json формате)

include(FetchContent)

if(NOT cpr_FOUND)
    message(STATUS "CPR не найден в системе. Загрузка через FetchContent...")

    set(BUILD_CPR_TESTS OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(
        cpr
        GIT_REPOSITORY https://github.com/libcpr/cpr.git
        GIT_TAG 1.10.5
    )

    FetchContent_MakeAvailable(cpr)
endif()

if(NOT nlohmann_json_FOUND)
    message(STATUS "Установка библиотеки для работы с json-форматом")

    include(FetchContent)

    FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.12.0
    )
    
    FetchContent_MakeAvailable(nlohmann_json)

endif()

file(GLOB agent_src_files "agent/*.cpp")
file(GLOB core_src_files "core/*.cpp")
file(GLOB gui_files "gui/*.cpp" "../include/gui/*.h")
file(GLOB tools_src_files "tools/*.cpp")

# Сборка динамической библиотеки агентов
add_library(agent SHARED ${agent_src_files})
target_link_libraries(agent PRIVATE cpr::cpr)

add_executable(SystemMonitoring
        main.cpp
        ${gui_files}
        ${core_src_files}
        ${tools_src_files}
)
target_link_libraries(SystemMonitoring PRIVATE Qt6::Widgets nlohmann_json::nlohmann_json)