INCLUDE_DIRECTORIES( 
    ${PROTEINTRACER_INCLUDE_DIR}
)

ADD_LIBRARY(proteintracer_filters STATIC
    AnalysisVisualizationImageFilter.cxx 
    SegmentKeyToColorFunctor.cxx 
)
TARGET_LINK_LIBRARIES(proteintracer_filters
    proteintracer
)
