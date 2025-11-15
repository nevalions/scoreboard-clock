# CMakeLists.txt for Play Clock Module

cmake_minimum_required(VERSION 3.16)

# Include ESP-IDF build system
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# Set the project name
project(scoreboard_play_clock)

# Include test components
idf_component_register(
    SRCS "src/main.cpp"
         "src/display_driver.cpp"
         "src/radio_comm.cpp"
    INCLUDE_DIRS "include"
    REQUIRES driver esp_common
)

# Add test components
if(CONFIG_IDF_TARGET_ESP32)
    # Unity test framework
    set(UNITY_FETCH ON)
    set(UNITY_INCLUDE_DEFAULT_ASSERT_MACROS OFF)

    # Register test components
    idf_component_register(SRCS "test/test_radio_comm.cpp"
                                "test/test_display_driver.cpp"
                                "test/test_integration.cpp"
                          INCLUDE_DIRS "include"
                          REQUIRES driver esp_common unity)

    # Add test targets
    add_custom_target(test_radio
        COMMAND ${CMAKE_COMMAND} -E env IDF_TARGET=esp32 ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target flash
        DEPENDS ${CMAKE_BINARY_DIR}/flash_project_args
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Flashing radio communication tests"
    )

    add_custom_target(test_display
        COMMAND ${CMAKE_COMMAND} -E env IDF_TARGET=esp32 ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target flash
        DEPENDS ${CMAKE_BINARY_DIR}/flash_project_args
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Flashing display driver tests"
    )

    add_custom_target(test_integration
        COMMAND ${CMAKE_COMMAND} -E env IDF_TARGET=esp32 ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target flash
        DEPENDS ${CMAKE_BINARY_DIR}/flash_project_args
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Flashing integration tests"
    )
endif()