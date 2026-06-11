cmake_minimum_required(VERSION 3.16)
project(ecosystem_sim)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable warnings
add_compile_options(-Wall -Wextra -Wpedantic)

# --------------------------------------------------------------------------
# SFML: tenta pacote do sistema primeiro; usa FetchContent como fallback
# --------------------------------------------------------------------------
find_package(SFML 2.6 COMPONENTS graphics window system QUIET)

if(NOT SFML_FOUND)
    message(STATUS "SFML nao encontrado no sistema -- usando FetchContent")
    include(FetchContent)
    FetchContent_Declare(
        SFML
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG        2.6.1
        GIT_SHALLOW    TRUE
    )
    set(SFML_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(SFML_BUILD_DOC      OFF CACHE BOOL "" FORCE)
    set(SFML_BUILD_TEST_SUITE OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(SFML)
endif()

# --------------------------------------------------------------------------
# Source files
# --------------------------------------------------------------------------
set(SOURCES
    src/main.cpp
    src/core/Organismo.cpp
    src/core/Animal.cpp
    src/core/WorldManager.cpp
    src/core/OrganismoFactory.cpp
    src/core/Renderer.cpp   # requer USE_SFML -- incluso aqui pois cmake sempre usa SFML
    src/entities/Planta.cpp
    src/entities/Presa.cpp
    src/entities/Predador.cpp
)

add_executable(ecosystem_sim ${SOURCES})

target_include_directories(ecosystem_sim PRIVATE src)

target_link_libraries(ecosystem_sim PRIVATE
    sfml-graphics
    sfml-window
    sfml-system
)

# Ativa o codigo SFML nos sources
target_compile_definitions(ecosystem_sim PRIVATE USE_SFML)

# Windows: flags necessarios para SFML estatico
if(WIN32)
    target_compile_definitions(ecosystem_sim PRIVATE UNICODE _UNICODE)
endif()

# --------------------------------------------------------------------------
# Copiar assets/ para o diretorio de build apos compilacao
# --------------------------------------------------------------------------
add_custom_command(TARGET ecosystem_sim POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:ecosystem_sim>/assets"
)

# Copiar DLLs do SFML para junto do executavel (necessario no Windows)
if(WIN32)
    add_custom_command(TARGET ecosystem_sim POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:sfml-graphics>
            $<TARGET_FILE:sfml-window>
            $<TARGET_FILE:sfml-system>
            $<TARGET_FILE_DIR:ecosystem_sim>
    )
endif()
