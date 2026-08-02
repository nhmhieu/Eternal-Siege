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
    Audio
    Graphics
    Window
    System
)

target_link_libraries(my_game PRIVATE
    SFML::Audio
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

    set(CORE_LOGIC_SOURCES
        src/AssetLocator.cpp
        src/AudioManager.cpp
        src/Ally.cpp
        src/Arrow.cpp
        src/Boss.cpp
        src/BossEnrageNotice.cpp
        src/BossHealthBar.cpp
        src/Bow.cpp
        src/CombatManager.cpp
        src/Elite.cpp
        src/Entity.cpp
        src/EntityCollision.cpp
        src/EntityLifecycle.cpp
        src/Effects.cpp
        src/GameContext.cpp
        src/HUD.cpp
        src/IntermissionOverlay.cpp
        src/Map.cpp
        src/MagicBolt.cpp
        src/Monster.cpp
        src/Player.cpp
        src/Projectiles.cpp
        src/RadiantPulse.cpp
        src/SpiritBolt.cpp
        src/SpiritStaff.cpp
        src/Sword.cpp
        src/TextureManager.cpp
        src/TutorialOverlay.cpp
        src/UpgradeManager.cpp
        src/WaveManager.cpp
        src/Wand.cpp
    )

    add_executable(core_logic_tests
        tests/CoreLogicTests.cpp
        ${CORE_LOGIC_SOURCES}
    )

    add_executable(ally_combat_tests
        tests/AllyCombatTests.cpp
        ${CORE_LOGIC_SOURCES}
    )

    add_executable(combat_regression_tests
        tests/CombatRegressionTests.cpp
        ${CORE_LOGIC_SOURCES}
    )

    add_executable(ally_skill_tests
        tests/AllySkillTests.cpp
        ${CORE_LOGIC_SOURCES}
    )

    add_executable(presentation_tests
        tests/PresentationTests.cpp
        src/AssetLocator.cpp
        src/Effects.cpp
        src/EndScreenView.cpp
        src/TextureManager.cpp
    )

    add_executable(gameplay_balance_tests
        tests/GameplayBalanceTests.cpp
        ${CORE_LOGIC_SOURCES}
    )

    add_executable(support_presentation_tests
        tests/SupportPresentationTests.cpp
        ${CORE_LOGIC_SOURCES}
    )

    add_executable(tutorial_tests
        tests/TutorialTests.cpp
        ${CORE_LOGIC_SOURCES}
    )

    add_executable(intermission_tests
        tests/IntermissionTests.cpp
        ${CORE_LOGIC_SOURCES}
    )

    add_executable(boss_enrage_tests
        tests/BossEnrageTests.cpp
        ${CORE_LOGIC_SOURCES}
    )

    add_executable(full_game_integration_tests
        tests/FullGameIntegrationTests.cpp
        ${CORE_LOGIC_SOURCES}
    )

    target_include_directories(core_logic_tests PRIVATE
        "${CMAKE_SOURCE_DIR}/include"
    )

    target_include_directories(ally_combat_tests PRIVATE
        "${CMAKE_SOURCE_DIR}/include"
    )

    target_include_directories(combat_regression_tests PRIVATE
        "${CMAKE_SOURCE_DIR}/include"
    )

    target_include_directories(ally_skill_tests PRIVATE
        "${CMAKE_SOURCE_DIR}/include"
    )

    target_include_directories(presentation_tests PRIVATE
        "${CMAKE_SOURCE_DIR}/include"
    )

    target_include_directories(gameplay_balance_tests PRIVATE
        "${CMAKE_SOURCE_DIR}/include"
    )

    target_include_directories(support_presentation_tests PRIVATE
        "${CMAKE_SOURCE_DIR}/include"
    )

    target_include_directories(tutorial_tests PRIVATE
        "${CMAKE_SOURCE_DIR}/include"
    )

    target_include_directories(intermission_tests PRIVATE
        "${CMAKE_SOURCE_DIR}/include"
    )

    target_include_directories(boss_enrage_tests PRIVATE
        "${CMAKE_SOURCE_DIR}/include"
    )

    target_include_directories(full_game_integration_tests PRIVATE
        "${CMAKE_SOURCE_DIR}/include"
    )

    target_link_libraries(core_logic_tests PRIVATE
        SFML::Audio
        SFML::Graphics
        SFML::Window
        SFML::System
    )
    enable_project_asan(core_logic_tests)

    target_link_libraries(ally_combat_tests PRIVATE
        SFML::Audio
        SFML::Graphics
        SFML::Window
        SFML::System
    )
    enable_project_asan(ally_combat_tests)

    target_link_libraries(combat_regression_tests PRIVATE
        SFML::Audio
        SFML::Graphics
        SFML::Window
        SFML::System
    )
    enable_project_asan(combat_regression_tests)

    target_link_libraries(ally_skill_tests PRIVATE
        SFML::Audio
        SFML::Graphics
        SFML::Window
        SFML::System
    )
    enable_project_asan(ally_skill_tests)

    target_link_libraries(presentation_tests PRIVATE
        SFML::Graphics
        SFML::Window
        SFML::System
    )
    enable_project_asan(presentation_tests)

    target_link_libraries(gameplay_balance_tests PRIVATE
        SFML::Audio
        SFML::Graphics
        SFML::Window
        SFML::System
    )
    enable_project_asan(gameplay_balance_tests)

    target_link_libraries(support_presentation_tests PRIVATE
        SFML::Audio
        SFML::Graphics
        SFML::Window
        SFML::System
    )
    enable_project_asan(support_presentation_tests)

    target_link_libraries(tutorial_tests PRIVATE
        SFML::Audio
        SFML::Graphics
        SFML::Window
        SFML::System
    )
    enable_project_asan(tutorial_tests)

    target_link_libraries(intermission_tests PRIVATE
        SFML::Audio
        SFML::Graphics
        SFML::Window
        SFML::System
    )
    enable_project_asan(intermission_tests)

    target_link_libraries(boss_enrage_tests PRIVATE
        SFML::Audio
        SFML::Graphics
        SFML::Window
        SFML::System
    )
    enable_project_asan(boss_enrage_tests)

    target_link_libraries(full_game_integration_tests PRIVATE
        SFML::Audio
        SFML::Graphics
        SFML::Window
        SFML::System
    )
    enable_project_asan(full_game_integration_tests)

    add_test(NAME core_logic_tests COMMAND core_logic_tests)
    add_test(NAME ally_combat_tests COMMAND ally_combat_tests)
    add_test(NAME combat_regression_tests COMMAND combat_regression_tests)
    add_test(NAME ally_skill_tests COMMAND ally_skill_tests)
    add_test(NAME presentation_tests COMMAND presentation_tests)
    add_test(NAME gameplay_balance_tests COMMAND gameplay_balance_tests)
    add_test(NAME support_presentation_tests COMMAND support_presentation_tests)
    add_test(NAME tutorial_tests COMMAND tutorial_tests)
    add_test(NAME intermission_tests COMMAND intermission_tests)
    add_test(NAME boss_enrage_tests COMMAND boss_enrage_tests)
    add_test(NAME full_game_integration_tests COMMAND full_game_integration_tests)
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
