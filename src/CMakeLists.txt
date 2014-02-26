find_package(Qt5Core REQUIRED)
find_package(Qt5DBus REQUIRED)
find_package(Qt5Xml REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(LibOTR REQUIRED)
find_package(Libgpg_error REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Core_EXECUTABLE_COMPILE_FLAGS}")

# Tell CMake to run moc when necessary:
set(CMAKE_AUTOMOC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_definitions(${Qt5Core_DEFINITIONS})
add_definitions(${Qt5DBus_DEFINITIONS})

include_directories(${Qt5Core_INCLUDE_DIRS})
include_directories(${Qt5DBus_INCLUDE_DIRS})
include_directories(${LIBOTR_INCLUDE_DIR})

find_path(TELEPATHY_QT5_INCLUDE_DIRS TelepathyQt PATHS /usr/include/telepathy-qt5)
find_library(TELEPATHY_QT5_LIBRARIES telepathy-qt5)
find_library(TELEPATHY_QT5_SERVICE_LIBRARIES telepathy-qt5-service)

include_directories(${TELEPATHY_QT5_INCLUDE_DIRS})

set(PIPE_INTERFACE_FILE ${OtrPipeTp_SOURCE_DIR}/interfaces/org.freedesktop.Telepathy.Pipe.xml)

#Temporary, could not find such variable
set(DBUS_INTERFACES_INSTALL_DIR share/dbus-1/interfaces)
install(FILES ${PIPE_INTERFACE_FILE} DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})

set(OtrPipe_SRCS
    otr_encryption.cpp
    pipe.cpp
    otr_pipe.cpp
    connection.cpp
)

qt5_add_dbus_adaptor(
    OtrPipe_SRCS
    ${PIPE_INTERFACE_FILE}
    pipe.hpp
    Pipe
)

add_library(OtrPipe STATIC ${OtrPipe_SRCS})

qt5_use_modules(OtrPipe Core DBus Xml Network)
target_link_libraries(OtrPipe ${TELEPATHY_QT5_LIBRARIES})
target_link_libraries(OtrPipe ${TELEPATHY_QT5_SERVICE_LIBRARIES})
target_link_libraries(OtrPipe ${LIBOTR_LIBRARY})
target_link_libraries(OtrPipe ${LIBGPG_ERROR_LIBRARY})

add_executable(telepathy-pipes-otr main.cpp)
target_link_libraries(telepathy-pipes-otr OtrPipe)

install(TARGETS telepathy-pipes-otr DESTINATION lib/telepathy)
