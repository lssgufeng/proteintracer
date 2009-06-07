INCLUDE_DIRECTORIES( 
    ${FLTK_INCLUDE_DIR}
    ${FLTK_TABLE_INCLUDE_DIR}
    ${PROTEINTRACER_INCLUDE_DIR}
)

# The parameter WIN32 causes Visual C++ to set the subsystem to "WINDOWS"
# instead of "CONSOLE", which prevents the showing of a console window when the
# applications is started.
ADD_EXECUTABLE(Evaluator WIN32 
    AnalysisImageViewer.cxx
    ChartParameters.cxx
    ChartParametersEditor.cxx
    ChartViewer.cxx
    Evaluator.cxx
    FeatureStatistics.cxx
    Histogram.cxx
)
TARGET_LINK_LIBRARIES(Evaluator 
    proteintracer
    proteintracer_io
    proteintracer_gui
)
