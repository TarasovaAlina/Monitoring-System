cmake_minimum_required(VERSION 3.19)

# настраиваем стандарт языка для компиляции
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Прописываем директории, куда будут сохранятся файлы компиляции
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/agents)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/bin)

# для работы Qt
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

project(System-Monitoring)

# включение заголовочных файлов
include_directories(include)

# Подключаем папки с исходным кодом и тестами
add_subdirectory(src)
add_subdirectory(tests)

find_package(Doxygen)

if(DOXYGEN_FOUND)
    add_custom_target(dvi
            COMMAND doxygen ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Генерация документации..."
            VERBATIM
    )
endif()

add_custom_target(dist
        COMMAND ${CMAKE_COMMAND} -E tar "czvf" "${CMAKE_BINARY_DIR}/Team_CPP9_SystemMonitoring.tar.gz"
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/materials
        ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
        ${CMAKE_CURRENT_SOURCE_DIR}/README.md
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Упаковка дистрибутива в архив..."
        VERBATIM
)

add_custom_target(uninstall
        COMMAND rm -rf ${CMAKE_BINARY_DIR}/bin ${CMAKE_BINARY_DIR}/lib
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Удаление программы..."
        VERBATIM
)