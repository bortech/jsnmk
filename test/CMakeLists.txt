add_executable(jsnmk_unit_tests tests.c)
target_link_libraries(jsnmk_unit_tests PRIVATE jsnmk)

add_test(NAME jsnmk_unit_tests COMMAND jsnmk_unit_tests -v)

add_executable(jsnmk_json_dump json_dump.c)
target_link_libraries(jsnmk_json_dump PRIVATE jsnmk)

find_package(Python3 COMPONENTS Interpreter QUIET)
if(Python3_Interpreter_FOUND)
    add_test(
        NAME jsnmk_json_valid
        COMMAND ${Python3_EXECUTABLE}
                ${CMAKE_CURRENT_SOURCE_DIR}/validate_json.py
                $<TARGET_FILE:jsnmk_json_dump>
                17
    )
else()
    message(STATUS "Python3 interpreter not found - JSON parser test disabled")
endif()

# AddressSanitizer / UBSan support. The flags are probed with
# check_c_compiler_flag() instead of being tied to a compiler id, so the
# option works with GCC, Clang and AppleClang; on unsupported compilers
# (e.g. MSVC) it degrades to a warning instead of breaking the build.
if(ENABLE_SANITIZERS)
    include(CheckCCompilerFlag)

    # Sanitizer runtimes must be linked too, so probe each flag with both
    # compile and link options (plain check_c_compiler_flag() only passes
    # the flag to the compiler, which breaks the ASan link check on macOS).
    function(jsnmk_check_sanitizer_flag flag var)
        set(CMAKE_REQUIRED_LINK_OPTIONS "${flag}")
        check_c_compiler_flag("${flag}" ${var})
        unset(CMAKE_REQUIRED_LINK_OPTIONS)
    endfunction()

    set(jsnmk_sanitizer_flags "")
    jsnmk_check_sanitizer_flag("-fsanitize=address" JSNMK_HAS_ASAN)
    jsnmk_check_sanitizer_flag("-fsanitize=undefined" JSNMK_HAS_UBSAN)

    if(JSNMK_HAS_ASAN)
        list(APPEND jsnmk_sanitizer_flags "-fsanitize=address")
    endif()
    if(JSNMK_HAS_UBSAN)
        list(APPEND jsnmk_sanitizer_flags "-fsanitize=undefined")
    endif()

    if(jsnmk_sanitizer_flags)
        jsnmk_check_sanitizer_flag("-fno-sanitize-recover=all" JSNMK_HAS_NO_RECOVER)
        if(JSNMK_HAS_NO_RECOVER)
            list(APPEND jsnmk_sanitizer_flags "-fno-sanitize-recover=all")
        endif()
        list(APPEND jsnmk_sanitizer_flags "-fno-omit-frame-pointer" "-g")

        foreach(target IN ITEMS jsnmk_unit_tests jsnmk_json_dump)
            target_compile_options(${target} PRIVATE ${jsnmk_sanitizer_flags})
            target_link_options(${target} PRIVATE ${jsnmk_sanitizer_flags})
        endforeach()

        message(STATUS "Sanitizers enabled for tests: ${jsnmk_sanitizer_flags}")
    else()
        message(WARNING
            "ENABLE_SANITIZERS=ON, but this compiler does not support "
            "-fsanitize=address or -fsanitize=undefined; sanitizers disabled")
    endif()
endif()
