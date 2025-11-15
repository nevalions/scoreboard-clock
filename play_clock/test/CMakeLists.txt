# CMakeLists.txt for Test Components

cmake_minimum_required(VERSION 3.16)

# Include Unity test framework
set(UNITY_FETCH ON)
set(UNITY_INCLUDE_DEFAULT_ASSERT_MACROS OFF)

# Register test component
idf_component_register(
    SRCS "test_radio_comm.cpp"
         "test_display_driver.cpp"
         "test_integration.cpp"
    INCLUDE_DIRS "../include"
    REQUIRES driver esp_common unity
)

# Add test executable targets
idf_build_process(
    TARGET test_radio_comm
    SRCS "test_radio_comm.cpp"
    INCLUDE_DIRS "../include"
    REQUIRES driver esp_common unity
)

idf_build_process(
    TARGET test_display_driver
    SRCS "test_display_driver.cpp"
    INCLUDE_DIRS "../include"
    REQUIRES driver esp_common unity
)

idf_build_process(
    TARGET test_integration
    SRCS "test_integration.cpp"
    INCLUDE_DIRS "../include"
    REQUIRES driver esp_common unity
)