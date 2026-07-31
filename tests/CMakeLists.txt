function(tula_add_behavior_test NAME SOURCE)
    set(_target "tula_test_${NAME}")
    add_executable("${_target}" "${SOURCE}")
    target_link_libraries(
        "${_target}"
        PRIVATE
            tula::core
            ${ARGN}
    )
    target_compile_features("${_target}" PRIVATE cxx_std_23)
    add_test(NAME "tula::${NAME}" COMMAND "${_target}")
endfunction()

set(_tula_header_smoke_components)
if(TARGET tula::logging)
    list(APPEND _tula_header_smoke_components tula::logging)
endif()
if(TARGET tula::perflibs)
    list(APPEND _tula_header_smoke_components tula::perflibs)
endif()
tula_add_behavior_test(
    header_smoke
    header_smoke.cpp
    ${_tula_header_smoke_components}
)

if(TARGET tula::ecsv)
    tula_add_behavior_test(ecsv test_ecsv.cpp tula::ecsv)
    tula_add_behavior_test(ecsv_table test_ecsv_table.cpp tula::ecsv)
    tula_add_behavior_test(ecsv_csv test_ecsv_csv.cpp tula::ecsv)

    add_executable(
        tula_ecsv_reader
        "${CMAKE_CURRENT_SOURCE_DIR}/../examples/ecsv_reader/main.cpp"
    )
    target_link_libraries(tula_ecsv_reader PRIVATE tula::ecsv)
    target_compile_features(tula_ecsv_reader PRIVATE cxx_std_23)

    set(
        _tula_workdir_tune_report_dir
        "${TOLTECA_TEST_DATA_ROOT}/tolteca_workdir/data"
    )
    if(IS_DIRECTORY "${_tula_workdir_tune_report_dir}")
        file(
            GLOB _tula_tune_report_candidates
            CONFIGURE_DEPENDS
            "${_tula_workdir_tune_report_dir}/*_tune.txt"
        )
        foreach(_tula_tune_report IN LISTS _tula_tune_report_candidates)
            if(EXISTS "${_tula_tune_report}")
                list(APPEND _tula_tune_reports "${_tula_tune_report}")
            endif()
        endforeach()
    endif()
    if(NOT _tula_tune_reports)
        file(
            GLOB _tula_tune_reports
            CONFIGURE_DEPENDS
            "${TOLTECA_TEST_DATA_ROOT}/data_lmt/toltec/reduced/*_tune.txt"
        )
    endif()
    if(_tula_tune_reports)
        add_test(
            NAME tula::ecsv_reader_real_tunes
            COMMAND tula_ecsv_reader ${_tula_tune_reports}
        )
    else()
        message(
            STATUS
            "(tula) real ECSV reader test disabled: tune reports not found"
        )
    endif()
else()
    message(STATUS "(tula) ECSV behavior tests disabled")
endif()

if(TARGET tula::yaml)
    tula_add_behavior_test(
        yamlconfig
        test_yamlconfig.cpp
        tula::yaml
    )
else()
    message(STATUS "(tula) YAML config test disabled")
endif()

if(TARGET tula::logging)
    tula_add_behavior_test(flatconfig test_flatconfig.cpp tula::logging)
    tula_add_behavior_test(filename test_filename.cpp tula::logging)
else()
    message(STATUS "(tula) logging behavior tests disabled")
endif()

if(TARGET tula::eigen)
    tula_add_behavior_test(
        eigen
        test_eigen.cpp
        tula::eigen
    )
    tula_add_behavior_test(
        nddata_eigen
        test_nddata_eigen.cpp
        tula::eigen
    )
else()
    message(STATUS "(tula) Eigen helper tests disabled")
endif()

if(TARGET tula::perflibs)
    tula_add_behavior_test(
        perflibs
        test_perflibs.cpp
        tula::perflibs
    )
else()
    message(STATUS "(tula) perflibs test disabled")
endif()

if(TARGET tula::netcdf)
    tula_add_behavior_test(nc test_nc.cpp tula::netcdf)
else()
    message(STATUS "(tula) NetCDF test disabled")
endif()

if(TARGET tula::enum)
    tula_add_behavior_test(enum test_enum.cpp tula::enum)
else()
    message(STATUS "(tula) enum test disabled")
endif()

if(TARGET tula::cli)
    tula_add_behavior_test(cli test_cli.cpp tula::cli)
else()
    message(STATUS "(tula) CLI test disabled")
endif()

if(TARGET tula::grppi)
    tula_add_behavior_test(grppi test_grppi.cpp tula::grppi)
else()
    message(STATUS "(tula) GrPPI test disabled")
endif()

if(TARGET tula::fitting)
    tula_add_behavior_test(fitting test_fitting.cpp tula::fitting)
else()
    message(STATUS "(tula) fitting test disabled")
endif()
