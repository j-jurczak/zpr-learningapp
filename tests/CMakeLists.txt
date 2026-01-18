find_package(Catch2 3 REQUIRED)
include(Catch)
include_directories(${CMAKE_SOURCE_DIR}/src)

cmake_minimum_required(VERSION 3.16)

# Test executables
add_executable(CardTests src/core/learning/CardTests.cc)
add_executable(LearningSessionTests src/core/learning/LearningSessionTests.cc)
add_executable(StrategiesTests src/core/learning/StrategiesTests.cc)
add_executable(DatabaseManagerTests src/db/DatabaseManagerTests.cc)
add_executable(ImporterExporterTests src/core/utils/ImporterExporterTests.cc)
add_executable(LanguageManagerTests src/core/utils/LanguageManagerTests.cc)
add_executable(StyleLoaderTests src/core/utils/StyleLoaderTests.cc)

# Helper macro to setup tests
macro(setup_test_target target_name)
    target_link_libraries(${target_name} PRIVATE Catch2::Catch2WithMain CoreLib GuiLib Qt6::Core Qt6::Gui Qt6::Widgets)
    target_include_directories(${target_name} PRIVATE ${CMAKE_SOURCE_DIR}/src)
    catch_discover_tests(${target_name})
endmacro()

setup_test_target(CardTests)
setup_test_target(LearningSessionTests)
setup_test_target(StrategiesTests)
setup_test_target(DatabaseManagerTests)
setup_test_target(ImporterExporterTests)
setup_test_target(LanguageManagerTests)
setup_test_target(StyleLoaderTests)