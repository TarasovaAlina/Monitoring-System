# Ищем и добавляем GTest для компиляции тестов
find_package(GTest QUIET)
if(NOT GTest_FOUND)
    message(STATUS "GTest не установлен в системе, скачиваем его...")

    include(FetchContent)
    FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG v1.16.0
    )

    # Команда скачивает GTest и делает его цели доступными для нашего проекта
    FetchContent_MakeAvailable(googletest)
endif()

include(GoogleTest)

enable_testing() # чтобы можно было запускать через ctest

file(GLOB gtest_src_files "*.cpp")
add_executable(Testing_backend_modules ${gtest_src_files})

target_include_directories(Testing_backend_modules PUBLIC)
target_link_libraries(Testing_backend_modules
        backend
        GTest::gtest_main
)

gtest_discover_tests(Testing_backend_modules)