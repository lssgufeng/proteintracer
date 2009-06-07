INCLUDE_DIRECTORIES( 
    ${TINYXML_INCLUDE_DIR}
    ${PROTEINTRACER_INCLUDE_DIR}
)

ADD_LIBRARY(proteintracer_io STATIC
    AnalysisIO.cxx 
    AssayIO.cxx 
    ScanIO.cxx 
    export.cxx 
)
TARGET_LINK_LIBRARIES(proteintracer_io
    proteintracer
    proteintracer_analyzers
    tinyxml
)
