cmake_minimum_required(VERSION 3.20)
project(EternalSiege LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ENABLE_ASAN "Enable AddressSanitizer for game and tests" OFF)

if(ENABLE_ASAN)
    include(CheckCXXSourceCompiles)
    if(MSVC)
        set(CMAKE_REQUIRED_FLAGS "/fsanitize=address")
        set(CMAKE_REQUIRED_LINK_OPTIONS "/fsanitize=address")
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        set(CMAKE_REQUIRED_FLAGS "-fsanitize=address")
        set(CMAKE_REQUIRED_LINK_OPTIONS "-fsanitize=address")
    else()
        message(FATAL_ERROR
            "ENABLE_ASAN is not supported by ${CMAKE_CXX_COMPILER_ID}")
    endif()

    check_cxx_source_compiles(
        "int main() { return 0; }"
        PROJECT_ASAN_SUPPORTED
    )
    unset(CMAKE_REQUIRED_FLAGS)
    unset(CMAKE_REQUIRED_LINK_OPTIONS)
    if(NOT PROJECT_ASAN_SUPPORTED)
        message(FATAL_ERROR
            "ENABLE_ASAN was requested, but the compiler/runtime cannot "
            "compile and link an AddressSanitizer executable")
    endif()
endif()

function(enable_project_asan target_name)
    if(NOT ENABLE_ASAN)
        return()
    endif()

    if(MSVC)
        target_compile_options(${target_name} PRIVATE /fsanitize=address)
        target_link_options(${target_name} PRIVATE /fsanitize=address)
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(
            ${target_name} PRIVATE
            -fsanitize=address
            -fno-omit-frame-pointer
        )
        target_link_options(${target_name} PRIVATE -fsanitize=address)
    endif()
endfunction()

file(GLOB SOURCES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
)

add_executable(my_game ${SOURCES})

target_include_directories(my_game PRIVATE
    "${CMAKE_SOURCE_DIR}/include"
)

find_package(SFML 3 REQUIRED COMPONENTS
    Graphics
    Window
    System
)

target_link_libraries(my_game PRIVATE
    SFML::Graphics
    SFML::Window
    SFML::System
)
enable_project_asan(my_game)

include(CTest)

if(BUILD_TESTING)
    add_executable(state_machine_tests
        tests/StateMachineTests.cpp
        src/StateMachine.cpp
    )

    target_include_directories(state_machine_tests PRIVATE
        "${CMAKE_SOURCE_DIR}/include"
    )

    target_link_libraries(state_machine_tests PRIVATE
        SFML::Graphics
        SFML::Window
        SFML::System
    )
    enable_project_asan(state_machine_tests)

    add_test(NAME state_machine_tests COMMAND state_machine_tests)

    add_executable(core_logic_tests
        tests/CoreLogicTests.cpp
        src/Ally.cpp
        src/Arrow.cpp
        src/Boss.cpp
        src/Bow.cpp
        src/CombatManager.cpp
        src/Elite.cpp
        src/Entity.cpp
        src/EntityCollision.cpp
        src/EntityLifecycle.cpp
        src/GameContext.cpp
        src/Map.cpp
        src/Monster.cpp
        src/Player.cpp
        src/Projectiles.cpp
        src/Sword.cpp
        src/TextureManager.cpp
        src/UpgradeManager.cpp
        src/WaveManager.cpp
    )

    target_include_directories(core_logic_tests PRIVATE
        "${CMAKE_SOURCE_DIR}/include"
    )

    target_link_libraries(core_logic_tests PRIVATE
        SFML::Graphics
        SFML::Window
        SFML::System
    )
    enable_project_asan(core_logic_tests)

    add_test(NAME core_logic_tests COMMAND core_logic_tests)
endif()

add_custom_command(
    TARGET my_game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/assets"
            "$<TARGET_FILE_DIR:my_game>/assets"
)

if(WIN32)
    add_custom_command(
        TARGET my_game POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_RUNTIME_DLLS:my_game>
                "$<TARGET_FILE_DIR:my_game>"
        COMMAND_EXPAND_LISTS
    )
endif()
