cmake_minimum_required(VERSION 3.10)
project(FHEbackend CXX)

include(CheckIncludeFileCXX)

check_include_file_cxx(any HAS_ANY)
check_include_file_cxx(string_view HAS_STRING_VIEW)
check_include_file_cxx(coroutine HAS_COROUTINE)
if (NOT "${CMAKE_CXX_STANDARD}" STREQUAL "")
    # Do nothing
elseif (HAS_ANY AND HAS_STRING_VIEW AND HAS_COROUTINE)
    set(CMAKE_CXX_STANDARD 20)
elseif (HAS_ANY AND HAS_STRING_VIEW)
    set(CMAKE_CXX_STANDARD 17)
else ()
    set(CMAKE_CXX_STANDARD 14)
endif ()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ##############################################################################
# OpenMP on macOS (Homebrew)
if(APPLE)
    set(OpenMP_CXX_FLAGS "-Xpreprocessor -fopenmp -I/opt/homebrew/opt/libomp/include")
    set(OpenMP_CXX_LIB_NAMES "omp")
    set(OpenMP_omp_LIBRARY "/opt/homebrew/opt/libomp/lib/libomp.dylib")
    link_directories(/opt/homebrew/opt/libomp/lib)
endif()
# ##############################################################################

add_executable(${PROJECT_NAME} main.cc)

# ##############################################################################
# Drogon
find_package(Drogon CONFIG REQUIRED)

# ##############################################################################
# OpenFHE
find_package(OpenFHE REQUIRED PATHS /usr/local/lib/OpenFHE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenFHE_CXX_FLAGS}")
# ##############################################################################

if (CMAKE_CXX_STANDARD LESS 17)
    message(FATAL_ERROR "c++17 or higher is required")
elseif (CMAKE_CXX_STANDARD LESS 20)
    message(STATUS "use c++17")
else ()
    message(STATUS "use c++20")
endif ()

aux_source_directory(controllers CTL_SRC)
aux_source_directory(filters FILTER_SRC)
aux_source_directory(plugins PLUGIN_SRC)
aux_source_directory(models MODEL_SRC)
aux_source_directory(utils UTIL_SRC)

drogon_create_views(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/views
                    ${CMAKE_CURRENT_BINARY_DIR})

target_include_directories(${PROJECT_NAME}
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
                                   ${CMAKE_CURRENT_SOURCE_DIR}/models
                                   ${OpenFHE_INCLUDE}
                                   ${OpenFHE_INCLUDE}/core
                                   ${OpenFHE_INCLUDE}/pke
                                   ${OpenFHE_INCLUDE}/binfhe)

target_link_directories(${PROJECT_NAME} PRIVATE
    ${OpenFHE_LIBDIR}
)

target_sources(${PROJECT_NAME}
               PRIVATE
               ${SRC_DIR}
               ${CTL_SRC}
               ${FILTER_SRC}
               ${PLUGIN_SRC}
               ${MODEL_SRC}
               ${UTIL_SRC})

target_link_libraries(${PROJECT_NAME} PRIVATE Drogon::Drogon ${OpenFHE_LIBRARIES})

# ##############################################################################
add_subdirectory(test)
