/*==============================================================================
Copyright (c) 2009, André Homeyer
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
==============================================================================*/ 

#ifndef WatershedVisualizer_h
#define WatershedVisualizer_h

#include <memory>
#include <string>
#include <vector>

#include <Analyzer.h>
#include <analyzers/WatershedFilterPipeline.h>
#include <filters/LabelVisualizationImageFilter.h>
#include <filters/RescaleIntensityRGBImageFilter.h>
#include <filters/SegmentVisualizationImageFilter.h>
#include <filters/ThresholdVisualizationImageFilter.h>

namespace PT 
{

class WatershedVisualizer : public Visualizer
{
public:
    WatershedVisualizer();

    virtual const std::vector<std::string>& getVisualizationNames() const;

    virtual std::auto_ptr<Visualization> createVisualization(
        const std::string& visualizationName, 
        const Assay& assay, 
        const ImageMetadata& imageMetadata) throw(VisualizerException);

private:

    typedef LabelVisualizationImageFilter<ULongImage> WatershedVisualizationFilter;
    typedef SegmentVisualizationImageFilter<FloatImage, ULongImage, WatershedFilterPipeline::SegmentSelectionAndMergingFilter::LabelToSegmentKeyFunctor> CellsVisualizationFilter;
    typedef SegmentVisualizationImageFilter<FloatImage, ULongImage, WatershedFilterPipeline::SegmentRingsFilter::LabelToSegmentKeyFunctor> RingsVisualizationFilter;
    typedef SegmentVisualizationImageFilter<FloatImage, RGBAImage, WatershedFilterPipeline::AnalysisFilter::LabelToSegmentKeyFunctor> AnalysisVisualizationFilter;

    WatershedFilterPipeline filterPipeline_;

    RescaleIntensityRGBImageFilter<FloatImage>::Pointer rescaleIntensityRGBFilter_;
    ThresholdVisualizationImageFilter<FloatImage>::Pointer thresholdVisualizationFilter_;
    WatershedVisualizationFilter::Pointer watershedVisualizationFilter_;
    CellsVisualizationFilter::Pointer cellsVisualizationFilter_;
    RingsVisualizationFilter::Pointer ringsVisualizationFilter_;
    AnalysisVisualizationFilter::Pointer analysisVisualizationFilter_;

};

}
#endif
