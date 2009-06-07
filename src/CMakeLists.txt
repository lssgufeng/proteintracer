INCLUDE_DIRECTORIES( 
    ${PROTEINTRACER_INCLUDE_DIR}
)

ADD_LIBRARY(proteintracer STATIC 
    Analysis.cxx
    ImageSeriesSet.cxx
    ParameterSet.cxx
    Scan.cxx
)
    
TARGET_LINK_LIBRARIES(proteintracer 
    ${ITK_LIBRARIES} 
)

ADD_SUBDIRECTORY(filters)

ADD_SUBDIRECTORY(analyzers)

ADD_SUBDIRECTORY(io)

ADD_SUBDIRECTORY(gui)

ADD_SUBDIRECTORY(assay_editor)

ADD_SUBDIRECTORY(assay_runner)

ADD_SUBDIRECTORY(evaluator)
