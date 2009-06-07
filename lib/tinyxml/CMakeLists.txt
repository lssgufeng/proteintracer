INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} )

ADD_LIBRARY(tinyxml STATIC 
    tinystr.cpp
    tinyxml.cpp
    tinyxmlerror.cpp
    tinyxmlparser.cpp
)
