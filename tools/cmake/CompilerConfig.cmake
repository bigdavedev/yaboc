if (PROJECT_IS_TOP_LEVEL)
    set (CMAKE_COMPILE_WARNING_AS_ERROR TRUE)
endif ()

add_library (yaboc_compiler_options INTERFACE)
add_library (Yaboc::CompilerOptions ALIAS yaboc_compiler_options)

foreach (OPTION ${YABOC_CXX_FLAGS})
    target_compile_options (yaboc_compiler_options INTERFACE ${OPTION})
endforeach ()

if (YABOC_ENABLE_SANITZERS)
    target_compile_options (yaboc_compiler_options INTERFACE $<$<CXX_COMPILER_ID:Clang>:-fsanitize=address,undefined>)
    target_link_options (yaboc_compiler_options INTERFACE $<$<CXX_COMPILER_ID:Clang>:-fsanitize=address,undefined>)
endif ()

target_compile_features (yaboc_compiler_options INTERFACE cxx_std_23)

set_target_properties (
    yaboc_compiler_options

    PROPERTIES
    CMAKE_CXX_STANDARD_REQUIRED YES
    CMAKE_CXX_EXTENSIONS OFF
)

target_compile_definitions (yaboc_compiler_options INTERFACE ${YABOC_PLATFORM})
