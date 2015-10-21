project(pc-whiteboard)


find_package(PkgConfig)
pkg_check_modules(PC_XML tinyxml)

add_definitions(${PC_XML_CFLAGS} -std=c++11)
link_libraries(boost_system boost_thread ${PC_XML_LIBRARIES})

add_executable(pc-whiteboard serial.cpp main.cpp)
