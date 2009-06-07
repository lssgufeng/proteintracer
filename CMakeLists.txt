CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT( ProteinTracer )

## FIND ITK ####################################################################

FIND_PACKAGE ( ITK REQUIRED )
INCLUDE( ${ITK_USE_FILE} ) 

## FIND FLTK ###################################################################

IF (UNIX)

    # Not using the FindFLTK module on Unix enables the use of a distribution
    # supplied FLTK development package instead of having to build FLTK
    # manually using CMake.

    FIND_PATH(FLTK_INCLUDE_DIR 
        NAMES FL/Fl.H Fl/Fl.H
        DOC "Path to the header files of the fltk library.")

    FIND_LIBRARY ( FLTK_BASE_LIBRARY
        NAMES fltk
        DOC "Path to the main FLTK library." )
    FIND_LIBRARY ( FLTK_IMAGES_LIBRARY
        NAMES fltk_images
        DOC "Path to the FLTK images library." )
    FIND_LIBRARY ( FLTK_FORMS_LIBRARY
        NAMES fltk_forms
        DOC "Path to the FLTK forms library." )
    FIND_LIBRARY ( FLTK_GL_LIBRARY
        NAMES fltk_gl
        DOC "Path to the FLTK gl library." )

    SET ( FLTK_LIBRARIES ${FLTK_BASE_LIBRARY} ${FLTK_IMAGES_LIBRARY} ${FLTK_FORMS_LIBRARY} ${FLTK_GL_LIBRARY} )

ELSEIF (WIN32)

    FIND_PACKAGE(FLTK REQUIRED)

    # The current implementation of the FindFLTK module does not respect the
    # parameter REQUIRED, so we have to check manually.
    IF (NOT FLTK_FOUND)
        MESSAGE ( FATAL_ERROR "Please set FLTK_DIR to the directory containing FLTKConfig.cmake, like root of the build tree, for example." )
    ENDIF (NOT FLTK_FOUND)

ENDIF (UNIX)

## BUILD FLTK TABLE ############################################################

# Fl_Table is an extension to FLTK for drawing tables.  Its sources are
# distributed with this project and therefore have to built prior to being
# used.  To do so a target "fltk_table" is defined.  Any target which uses
# Fl_Table is required to define a dependency to the fltk_target via the
# ADD_DEPENDENCIES(<target name> fltk_table ) command.
# Further information on Fl_Table can be found on
# http://seriss.com/people/erco/fltk/Fl_Table/

ADD_SUBDIRECTORY( lib/Fl_Table-3.12 )

SET ( FLTK_TABLE_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/lib/Fl_Table-3.12 )
SET ( FLTK_TABLE_LIBRARY fltk_table )

## FIND TINY XML ###############################################################

ADD_SUBDIRECTORY( lib/tinyxml )

SET ( TINYXML_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/lib/tinyxml )
SET ( TINYXML_LIBRARY tinyxml )

## APPLICATION SETTINGS ########################################################

SET ( PROTEINTRACER_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/src )

ADD_SUBDIRECTORY( src )
