INCLUDE_DIRECTORIES( ${FLTK_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} )

ADD_LIBRARY(fltk_table STATIC 
    Fl_Table.cxx
    Fl_Table_Row.cxx
)

TARGET_LINK_LIBRARIES(fltk_table
    ${FLTK_LIBRARIES}
)
