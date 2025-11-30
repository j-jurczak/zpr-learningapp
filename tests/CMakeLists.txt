find_package(Catch2 3 REQUIRED)

add_executable(CardTests src/CardTests.cc)
target_link_libraries(CardTests PRIVATE Catch2::Catch2WithMain CoreLib)

add_executable(LearningSessionTests src/LearningSessionTests.cc)
target_link_libraries(LearningSessionTests PRIVATE Catch2::Catch2WithMain CoreLib)

include(Catch)

catch_discover_tests(CardTests)
catch_discover_tests(LearningSessionTests)