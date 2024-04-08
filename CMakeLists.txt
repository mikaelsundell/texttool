# Copyright 2023-present Contributors to the overlaytool project.
# SPDX-License-Identifier: BSD-3-Clause
# https://github.com/mikaelsundell/overlaytool

cmake_minimum_required(VERSION 3.23)
set( project_name "titletool" )

project( "${project_name} project" )

# modules
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/modules" )

# imath
find_package( Imath CONFIG REQUIRED )
find_package( OpenImageIO CONFIG REQUIRED )

# font
configure_file( 
    "${PROJECT_SOURCE_DIR}/fonts/Roboto.ttf" 
    "${CMAKE_BINARY_DIR}/Roboto.ttf"
    COPYONLY
)

# package
add_executable( ${project_name} "titletool.cpp" )
set_property( TARGET ${project_name} PROPERTY CXX_STANDARD 14)

target_link_libraries( ${project_name}
    PRIVATE
        Imath::Imath
        OpenImageIO::OpenImageIO
)

install(TARGETS ${project_name}
    RUNTIME DESTINATION bin
)
