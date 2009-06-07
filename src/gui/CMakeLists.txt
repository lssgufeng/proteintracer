INCLUDE_DIRECTORIES( 
    ${FLTK_INCLUDE_DIR}
    ${FLTK_TABLE_INCLUDE_DIR}
    ${PROTEINTRACER_INCLUDE_DIR}
)

ADD_LIBRARY(proteintracer_gui STATIC
    AboutDialog.cxx 
    ImageViewer.cxx 
    ImageSelector.cxx
    ParameterTable.cxx
)
TARGET_LINK_LIBRARIES(proteintracer_gui
    proteintracer
    ${FLTK_LIBRARIES} 
    fltk_table
)
