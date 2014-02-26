# Taken from emeraldviewer
# -*- cmake -*-

# - Find libgpg-error
# Find the libgpg-error includes and library
# This module defines
# LIBGPG_ERROR_INCLUDE_DIR, where to find gpg-error.h, etc.
# LIBGPG_ERROR_LIBRARIES, the libraries needed to use libgpg-error.
# LIBGPG_ERROR_FOUND, If false, do not try to use libgpg-error.
# also defined, but not for general use are
# LIBGPG_ERROR_LIBRARY, where to find the libgpg-error library.

FIND_PATH(LIBGPG_ERROR_INCLUDE_DIR gpg-error.h)

SET(LIBGPG_ERROR_NAMES ${LIBGPG_ERROR_NAMES} gpg-error)
FIND_LIBRARY(LIBGPG_ERROR_LIBRARY
    NAMES ${LIBGPG_ERROR_NAMES}
    )

IF (LIBGPG_ERROR_LIBRARY AND LIBGPG_ERROR_INCLUDE_DIR)
    SET(LIBGPG_ERROR_LIBRARIES ${LIBGPG_ERROR_LIBRARY})
    SET(LIBGPG_ERROR_FOUND "YES")
ELSE (LIBGPG_ERROR_LIBRARY AND LIBGPG_ERROR_INCLUDE_DIR)
    SET(LIBGPG_ERROR_FOUND "NO")
ENDIF (LIBGPG_ERROR_LIBRARY AND LIBGPG_ERROR_INCLUDE_DIR)


IF (LIBGPG_ERROR_FOUND)
    IF (NOT LIBGPG_ERROR_FIND_QUIETLY)
        MESSAGE(STATUS "Found libgpg-error: '${LIBGPG_ERROR_LIBRARIES}' and header in '${LIBGPG_ERROR_INCLUDE_DIR}'")
    ENDIF (NOT LIBGPG_ERROR_FIND_QUIETLY)
ELSE (LIBGPG_ERROR_FOUND)
    IF (LIBGPG_ERROR_FIND_REQUIRED)
        MESSAGE(FATAL_ERROR "Could not find libgpg-error library")
    ENDIF (LIBGPG_ERROR_FIND_REQUIRED)
ENDIF (LIBGPG_ERROR_FOUND)

# Deprecated declarations.
SET (NATIVE_LIBGPG_ERROR_INCLUDE_PATH ${LIBGPG_ERROR_INCLUDE_DIR} )
GET_FILENAME_COMPONENT (NATIVE_LIBGPG_ERROR_LIB_PATH ${LIBGPG_ERROR_LIBRARY} PATH)

MARK_AS_ADVANCED(
    LIBGPG_ERROR_LIBRARY
    LIBGPG_ERROR_INCLUDE_DIR
    )