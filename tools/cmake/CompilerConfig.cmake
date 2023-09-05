if (PROJECT_IS_TOP_LEVEL)
    set (CMAKE_COMPILE_WARNING_AS_ERROR TRUE)
endif ()

foreach (OPTION ${YABOC_CXX_FLAGS})
    add_compile_options (${OPTION})
endforeach ()

set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED YES)
set (CMAKE_CXX_EXTENSIONS OFF)

add_compile_definitions (${YABOC_PLATFORM})