find_package(Catch2 3 REQUIRED)

add_executable(UnitTests CoreTests.cc)

target_link_libraries(UnitTests PRIVATE Catch2::Catch2WithMain CoreLib)

include(Catch)
catch_discover_tests(UnitTests)