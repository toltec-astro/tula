cmake_minimum_required(VERSION 3.25)

project(
    tula
    VERSION 3.1.0
    DESCRIPTION "The C++ utility library for TolTEC"
    HOMEPAGE_URL "https://toltec-astro.github.io/tula"
    LANGUAGES CXX
)

include(CTest)
include(GNUInstallDirs)

find_package(TulaCMake 3 CONFIG REQUIRED)

option(TULA_ENABLE_LOGGING "Enable logging and formatting" OFF)
option(TULA_ENABLE_YAML "Enable YAML-backed configuration" OFF)
option(TULA_ENABLE_ECSV "Enable complete ECSV typed-table support" OFF)
option(TULA_ENABLE_EIGEN "Enable Eigen helpers" OFF)
option(TULA_ENABLE_NETCDF "Enable NetCDF C++ helpers" OFF)
option(TULA_ENABLE_ENUM "Enable reflected enums and typed flags" OFF)
option(TULA_ENABLE_CLI "Enable command-line construction" OFF)
option(TULA_ENABLE_PERFLIBS "Enable Threads and optional OpenMP" OFF)
option(TULA_ENABLE_OPENMP "Expose OpenMP through perflibs and GrPPI" OFF)
option(TULA_ENABLE_GRPPI "Enable parallel-pattern helpers" OFF)
option(TULA_ENABLE_FITTING "Enable Ceres-backed fitting helpers" OFF)

macro(tula_require_module parent child)
    if(TULA_ENABLE_${parent} AND NOT TULA_ENABLE_${child})
        message(
            FATAL_ERROR
            "TULA_ENABLE_${parent} requires TULA_ENABLE_${child}"
        )
    endif()
endmacro()

tula_require_module(ECSV LOGGING)
tula_require_module(ECSV YAML)
tula_require_module(ECSV EIGEN)
tula_require_module(YAML LOGGING)
tula_require_module(EIGEN LOGGING)
tula_require_module(NETCDF EIGEN)
tula_require_module(ENUM LOGGING)
tula_require_module(CLI LOGGING)
tula_require_module(CLI ENUM)
tula_require_module(OPENMP PERFLIBS)
tula_require_module(GRPPI LOGGING)
tula_require_module(GRPPI ENUM)
tula_require_module(GRPPI PERFLIBS)
tula_require_module(FITTING LOGGING)
tula_require_module(FITTING EIGEN)

set(tula_export_targets)

add_library(tula_core INTERFACE)
add_library(tula::core ALIAS tula_core)
set_target_properties(tula_core PROPERTIES EXPORT_NAME core)
tula_cmake_target_defaults(TARGET tula_core)
target_include_directories(
    tula_core
    INTERFACE
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

set(TULA_HAS_LOGGING "${TULA_ENABLE_LOGGING}")
set(TULA_HAS_YAML "${TULA_ENABLE_YAML}")
set(TULA_HAS_ECSV "${TULA_ENABLE_ECSV}")
set(TULA_HAS_EIGEN "${TULA_ENABLE_EIGEN}")
set(TULA_HAS_NETCDF "${TULA_ENABLE_NETCDF}")
set(TULA_HAS_ENUM "${TULA_ENABLE_ENUM}")
set(TULA_HAS_CLI "${TULA_ENABLE_CLI}")
set(TULA_HAS_PERFLIBS "${TULA_ENABLE_PERFLIBS}")
set(TULA_HAS_OPENMP "${TULA_ENABLE_OPENMP}")
set(TULA_HAS_GRPPI "${TULA_ENABLE_GRPPI}")
set(TULA_HAS_FITTING "${TULA_ENABLE_FITTING}")

tula_cmake_generate_config_header(
    TARGET tula_core
    INPUT "${CMAKE_CURRENT_SOURCE_DIR}/include/tula/config.h.in"
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/generated/tula/config.h"
    BUILD_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated"
)
list(APPEND tula_export_targets tula_core)

macro(tula_add_module name)
    add_library("tula_${name}" INTERFACE)
    add_library("tula::${name}" ALIAS "tula_${name}")
    set_target_properties(
        "tula_${name}"
        PROPERTIES EXPORT_NAME "${name}"
    )
    target_link_libraries(
        "tula_${name}"
        INTERFACE
            tula::core
            ${ARGN}
    )
    list(APPEND tula_export_targets "tula_${name}")
endmacro()

if(TULA_ENABLE_LOGGING)
    find_package(TulaLogging CONFIG REQUIRED)
    tula_add_module(logging tula_deps::logging)
endif()

if(TULA_ENABLE_PERFLIBS)
    find_package(TulaPerflibs CONFIG REQUIRED)
    tula_add_module(perflibs tula_deps::perflibs)
endif()

if(TULA_ENABLE_EIGEN)
    find_package(TulaEigen3 CONFIG REQUIRED)
    tula_add_module(eigen tula::logging tula_deps::eigen3)
endif()

if(TULA_ENABLE_YAML)
    find_package(TulaYamlCpp CONFIG REQUIRED)
    tula_add_module(yaml tula::logging tula_deps::yaml_cpp)
endif()

if(TULA_ENABLE_ENUM)
    find_package(TulaBitmask CONFIG REQUIRED)
    find_package(TulaMetaEnum CONFIG REQUIRED)
    tula_add_module(
        enum
        tula::logging
        tula_deps::bitmask
        tula_deps::meta_enum
    )
endif()

if(TULA_ENABLE_ECSV)
    find_package(TulaCsvParser CONFIG REQUIRED)
    tula_add_module(
        ecsv
        tula::logging
        tula::yaml
        tula::eigen
        tula_deps::csv_parser
    )
endif()

if(TULA_ENABLE_NETCDF)
    find_package(TulaNetcdfCxx4 CONFIG REQUIRED)
    tula_add_module(
        netcdf
        tula::eigen
        tula_deps::netcdf_cxx4
    )
endif()

if(TULA_ENABLE_CLI)
    find_package(TulaClipp CONFIG REQUIRED)
    tula_add_module(
        cli
        tula::logging
        tula::enum
        tula_deps::clipp
    )
endif()

if(TULA_ENABLE_GRPPI)
    find_package(TulaGrppi CONFIG REQUIRED)
    tula_add_module(
        grppi
        tula::logging
        tula::enum
        tula::perflibs
        tula_deps::grppi
    )
    if(TULA_ENABLE_OPENMP)
        target_compile_definitions(tula_grppi INTERFACE GRPPI_OMP)
    endif()
endif()

if(TULA_ENABLE_FITTING)
    find_package(Ceres 2.2 CONFIG REQUIRED)
    tula_add_module(
        fitting
        tula::logging
        tula::eigen
        Ceres::ceres
    )
endif()

install(
    DIRECTORY include/
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    PATTERN ".DS_Store" EXCLUDE
    PATTERN "config.h.in" EXCLUDE
)
install(
    FILES "${CMAKE_CURRENT_BINARY_DIR}/generated/tula/config.h"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/tula"
)
tula_cmake_install_package(
    PACKAGE tula
    EXPORT tulaTargets
    NAMESPACE tula::
    VERSION "${PROJECT_VERSION}"
    TARGETS ${tula_export_targets}
    CONFIG_TEMPLATE "${CMAKE_CURRENT_SOURCE_DIR}/cmake/tulaConfig.cmake.in"
)

option(
    TULA_BUILD_TESTS
    "Compile and run the enabled Tula module surface"
    "${PROJECT_IS_TOP_LEVEL}"
)
if(TULA_BUILD_TESTS)
    set(
        TOLTECA_TEST_DATA_ROOT
        ""
        CACHE PATH
        "Path to the tolteca_test_data checkout used by real ECSV tests"
    )
    if(NOT TOLTECA_TEST_DATA_ROOT)
        set(_tolteca_test_data_candidate
            "${CMAKE_CURRENT_SOURCE_DIR}/../tolteca_test_data")
        if(IS_DIRECTORY "${_tolteca_test_data_candidate}")
            cmake_path(
                ABSOLUTE_PATH _tolteca_test_data_candidate
                NORMALIZE
                OUTPUT_VARIABLE TOLTECA_TEST_DATA_ROOT
            )
        endif()
    endif()
    add_subdirectory(tests)
endif()
