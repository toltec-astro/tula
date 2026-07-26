function(tula_add_behavior_test NAME SOURCE)
    set(_target "tula_test_${NAME}")
    add_executable("${_target}" "${SOURCE}")
    target_link_libraries("${_target}" PRIVATE tula::headers)
    target_compile_features("${_target}" PRIVATE cxx_std_23)
    add_test(NAME "tula::${NAME}" COMMAND "${_target}")
endfunction()

tula_add_behavior_test(header_smoke header_smoke.cpp)

if(TARGET tula::yaml_cpp AND TARGET tula::logging)
    tula_add_behavior_test(ecsv test_ecsv.cpp)
    tula_add_behavior_test(yamlconfig test_yamlconfig.cpp)
else()
    message(STATUS
        "(tula) ECSV and YAML config tests disabled: yaml_cpp and logging are required")
endif()

if(TARGET tula::logging)
    tula_add_behavior_test(flatconfig test_flatconfig.cpp)
    tula_add_behavior_test(filename test_filename.cpp)
else()
    message(STATUS
        "(tula) flat config and filename tests disabled: logging is required")
endif()

if(TARGET tula::eigen AND TARGET tula::logging)
    tula_add_behavior_test(eigen test_eigen.cpp)
    tula_add_behavior_test(nddata_eigen test_nddata_eigen.cpp)
else()
    message(STATUS
        "(tula) Eigen behavior tests disabled: eigen and logging are required")
endif()

if(TARGET tula::yaml_cpp AND TARGET tula::eigen AND TARGET tula::logging)
    tula_add_behavior_test(ecsv_table test_ecsv_table.cpp)
else()
    message(STATUS
        "(tula) ECSV table test disabled: yaml_cpp, eigen, and logging are required")
endif()

if(TARGET tula::csv_parser
   AND TARGET tula::yaml_cpp
   AND TARGET tula::eigen
   AND TARGET tula::logging)
    tula_add_behavior_test(ecsv_csv test_ecsv_csv.cpp)
else()
    message(STATUS
        "(tula) ECSV CSV test disabled: csv_parser, yaml_cpp, eigen, and logging are required")
endif()

if(TARGET tula::netcdf_cxx4 AND TARGET tula::eigen AND TARGET tula::logging)
    tula_add_behavior_test(nc test_nc.cpp)
else()
    message(STATUS
        "(tula) NetCDF test disabled: netcdf_cxx4, eigen, and logging are required")
endif()

if(TARGET tula::bitmask AND TARGET tula::meta_enum AND TARGET tula::logging)
    tula_add_behavior_test(enum test_enum.cpp)
else()
    message(STATUS
        "(tula) enum test disabled: bitmask, meta_enum, and logging are required")
endif()

if(TARGET tula::clipp
   AND TARGET tula::bitmask
   AND TARGET tula::meta_enum
   AND TARGET tula::logging)
    tula_add_behavior_test(cli test_cli.cpp)
else()
    message(STATUS
        "(tula) CLI test disabled: clipp, bitmask, meta_enum, and logging are required")
endif()

if(TARGET tula::grppi)
    tula_add_behavior_test(grppi test_grppi.cpp)
else()
    message(STATUS "(tula) GrPPI behavior test disabled: grppi is required")
endif()
