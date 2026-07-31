cmake_minimum_required(VERSION 3.20)
project(EternalSiege LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
