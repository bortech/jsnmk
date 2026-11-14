add_executable(jsnmk_tests tests.c)

target_link_libraries(jsnmk_tests PRIVATE jsnmk)

add_test(
    NAME jsnmk_tests
    COMMAND jsnmk_tests -v
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)
