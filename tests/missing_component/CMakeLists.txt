cmake_minimum_required(VERSION 3.25)

project(tula_missing_component LANGUAGES CXX)

find_package(
    tula 3.1 CONFIG REQUIRED
    COMPONENTS component_that_does_not_exist
)
