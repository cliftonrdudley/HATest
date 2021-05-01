cmake_minimum_required( VERSION 3.1 )
project( HATest )

set( CMAKE_CXX_STANDARD 17 )
add_definitions( -g -O0 )

add_library( HATest Mesh.cpp Parser.cpp )

add_executable( TestMesh TestMesh.cpp )

target_link_libraries( TestMesh HATest )
enable_testing()
add_test( Mesh TestMesh )
