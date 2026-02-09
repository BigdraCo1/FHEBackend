cmake_minimum_required(VERSION 3.10)
project(FHEbackend_test CXX)

add_executable(${PROJECT_NAME} test_main.cc)

# ##############################################################################
# Drogon
target_link_libraries(${PROJECT_NAME} PRIVATE Drogon::Drogon)

# ##############################################################################
# OpenFHE
target_include_directories(${PROJECT_NAME} PRIVATE
    ${OpenFHE_INCLUDE}
    ${OpenFHE_INCLUDE}/core
    ${OpenFHE_INCLUDE}/pke
    ${OpenFHE_INCLUDE}/binfhe
)

# OpenMP on macOS (Homebrew)
if(APPLE)
    target_link_directories(${PROJECT_NAME} PRIVATE
        /opt/homebrew/opt/libomp/lib
        ${OpenFHE_LIBDIR}
    )
else()
    target_link_directories(${PROJECT_NAME} PRIVATE ${OpenFHE_LIBDIR})
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE ${OpenFHE_LIBRARIES})

# ##############################################################################
ParseAndAddDrogonTests(${PROJECT_NAME})
