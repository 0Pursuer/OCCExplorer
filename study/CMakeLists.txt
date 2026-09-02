cmake_minimum_required(VERSION 3.20)
project(OCCExplorerStudy VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
foreach(config DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${config} "${CMAKE_BINARY_DIR}/bin")
endforeach()

find_package(OpenCASCADE REQUIRED
    COMPONENTS
        FoundationClasses
        ModelingData
        ModelingAlgorithms
)

message(STATUS "OpenCASCADE version: ${OpenCASCADE_MAJOR_VERSION}.${OpenCASCADE_MINOR_VERSION}.${OpenCASCADE_MAINTENANCE_VERSION}")
message(STATUS "OpenCASCADE include: ${OpenCASCADE_INCLUDE_DIR}")

# OCCT exports imported targets by toolkit name, e.g. TKernel / TKBRep / TKBO.
add_library(occ_study_occt INTERFACE)
target_include_directories(occ_study_occt SYSTEM INTERFACE "${OpenCASCADE_INCLUDE_DIR}")
target_link_libraries(occ_study_occt INTERFACE
    TKernel
    TKMath
    TKG2d
    TKG3d
    TKGeomBase
    TKBRep
    TKTopAlgo
    TKGeomAlgo
    TKPrim
    TKBO
)

add_library(occ_study_warnings INTERFACE)
if(MSVC)
    target_compile_options(occ_study_warnings INTERFACE /W4)
else()
    target_compile_options(occ_study_warnings INTERFACE -Wall -Wextra -Wpedantic)
endif()

function(add_occ_example target source)
    add_executable(${target} ${source})
    target_link_libraries(${target} PRIVATE occ_study_occt occ_study_warnings)
endfunction()

add_occ_example(day1_hello_occ day1/01_hello_occ.cpp)
add_occ_example(day1_brep_basics day1/02_brep_basics.cpp)
add_occ_example(day2_edge_location day2/01_edge_location.cpp)
add_occ_example(day2_boolean_adjacency day2/02_boolean_adjacency.cpp)

add_library(occ_explorer_core STATIC final/ShapeInspector.cpp)
target_include_directories(occ_explorer_core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/final")
target_link_libraries(occ_explorer_core
    PUBLIC occ_study_occt
    PRIVATE occ_study_warnings
)

add_executable(occ_explorer final/main.cpp)
target_link_libraries(occ_explorer PRIVATE occ_explorer_core occ_study_warnings)

include(CTest)
if(BUILD_TESTING)
    add_test(NAME day1_hello_occ COMMAND day1_hello_occ)
    add_test(NAME day1_brep_basics COMMAND day1_brep_basics)
    add_test(NAME day2_edge_location COMMAND day2_edge_location)
    add_test(NAME day2_boolean_adjacency COMMAND day2_boolean_adjacency)
    add_test(NAME occ_explorer COMMAND occ_explorer)
endif()
