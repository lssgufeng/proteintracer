INCLUDE_DIRECTORIES( 
    ${FLTK_INCLUDE_DIR}
    ${PROTEINTRACER_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# The parameter WIN32 causes Visual C++ to set the subsystem to "WINDOWS"
# instead of "CONSOLE", which prevents the showing of a console window when the
# applications is started.
ADD_EXECUTABLE( AssayRunner WIN32
    AssayRunner.cxx 
)
TARGET_LINK_LIBRARIES( AssayRunner
    proteintracer
    proteintracer_analyzers
    proteintracer_gui
    proteintracer_io
    ${FLTK_LIBRARIES} 
) 
