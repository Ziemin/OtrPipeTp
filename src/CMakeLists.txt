find_package(Qt5Core REQUIRED)
find_package(Qt5DBus REQUIRED)
find_package(Qt5Xml REQUIRED)
find_package(Qt5Network REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Core_EXECUTABLE_COMPILE_FLAGS}")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_definitions(${Qt5Core_DEFINITIONS})
add_definitions(${Qt5DBus_DEFINITIONS})

include_directories(${Qt5Core_INCLUDE_DIRS})
include_directories(${Qt5DBus_INCLUDE_DIRS})

find_path(TELEPATHY_QT5_INCLUDE_DIRS TelepathyQt PATHS /usr/include/telepathy-qt5)
find_library(TELEPATHY_QT5_LIBRARIES telepathy-qt5)
find_library(TELEPATHY_QT5_SERVICE_LIBRARIES telepathy-qt5-service)

include_directories(${TELEPATHY_QT5_INCLUDE_DIRS})

set(OtrPipeTp_SOURCE_DIR
    ${OtrPipeTp_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/org.freedesktop.Telepathy.Pipe.xml
    )

qt5_generate_dbus_interface(pipe.hpp)

set(PIPE_INTERFACE_FILE "org.freedesktop.Telepathy.Pipe.xml")
execute_process(COMMAND qdbuscpp2xml pipe.hpp 
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    OUTPUT_VARIABLE XML_FILE)
file(WRITE ${PIPE_INTERFACE_FILE} ${XML_FILE})

execute_process(COMMAND qdbusxml2cpp -a pipe_adaptor -c PipeAdaptor ${PIPE_INTERFACE_FILE}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

#Temporary, could not find such variable
set(DBUS_INTERFACES_INSTALL_DIR /usr/share/dbus-1/interfaces)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PIPE_INTERFACE_FILE} DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})

add_library(OtrPipe STATIC
    pipe.cpp
    otr_pipe.cpp
    pipe_adaptor.cpp
    )

qt5_use_modules(OtrPipe Core DBus Xml Network)
target_link_libraries(OtrPipe ${TELEPATHY_QT5_LIBRARIES})
target_link_libraries(OtrPipe ${TELEPATHY_QT5_SERVICE_LIBRARIES})

add_executable(otrPipe-telepathy main.cpp)
target_link_libraries(otrPipe-telepathy OtrPipe)

install(TARGETS otrPipe-telepathy DESTINATION ${EXEC_DIR})
