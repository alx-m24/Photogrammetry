cmake_minimum_required(VERSION 3.5)

project(Photogrammetry LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the library
add_executable(${PROJECT_NAME}
    src/main.cpp
    src/thirdParty/stb.cpp
)

# Include directories for this library
target_include_directories(${PROJECT_NAME} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# ---- FetchContent for GLFW and GLAD ----
include(FetchContent)

# GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE) # Disabling docs
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
)
FetchContent_MakeAvailable(glfw)

# GLAD
FetchContent_Declare(
    glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
    GIT_TAG v0.1.36
)
FetchContent_MakeAvailable(glad)

# GLM
FetchContent_Declare(
	glm
	GIT_REPOSITORY https://github.com/g-truc/glm.git
	GIT_TAG 1.0.2
)
FetchContent_MakeAvailable(glm)

# ---- Link the libraries ----
target_link_libraries(${PROJECT_NAME} PRIVATE glfw glad glm)

# STB
FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
)
FetchContent_MakeAvailable(stb)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${glm_SOURCE_DIR}
        ${glfw_SOURCE_DIR}/include
        ${stb_SOURCE_DIR}
)

# ---- Automatically copy MinGW runtime DLLs on Windows ----
if(WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # Use the compiler to locate runtime DLLs
    execute_process(
        COMMAND ${CMAKE_CXX_COMPILER} -print-file-name=libstdc++-6.dll
        OUTPUT_VARIABLE LIBSTDCPP_DLL
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    execute_process(
        COMMAND ${CMAKE_CXX_COMPILER} -print-file-name=libgcc_s_seh-1.dll
        OUTPUT_VARIABLE LIBGCC_DLL
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    set(MINGW_DLLS ${LIBSTDCPP_DLL} ${LIBGCC_DLL})

    foreach(DLL ${MINGW_DLLS})
        if(EXISTS ${DLL})
            add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${DLL}"
                $<TARGET_FILE_DIR:${PROJECT_NAME}>
            )
        endif()
    endforeach()
endif()
