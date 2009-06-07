INCLUDE_DIRECTORIES( 
    ${PROTEINTRACER_INCLUDE_DIR}
)

ADD_LIBRARY(proteintracer_analyzers STATIC
    AnalyzerRegistry.cxx
    TestAnalyzer.cxx 
    TestVisualizer.cxx
    WatershedAnalyzer.cxx 
    WatershedFilterPipeline.cxx
    WatershedVisualizer.cxx
)
TARGET_LINK_LIBRARIES(proteintracer_analyzers
    proteintracer
    proteintracer_filters
    ${ITK_LIBRARIES} 
)
