configure_file(OtrPipe-telepathy.service.in org.freedesktop.Telepathy.Pipe.Otr.service)

install (FILES ${CMAKE_CURRENT_BINARY_DIR}/org.freedesktop.Telepathy.Pipe.Otr.service DESTINATION share/dbus-1/services)
