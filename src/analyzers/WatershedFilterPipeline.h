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

#ifndef WatershedFilterPipeline_h
#define WatershedFilterPipeline_h

#include <memory>
#include <string>

#include <itkEventObject.h>
#include <itkGradientAnisotropicDiffusionImageFilter.h>
#include <itkGradientMagnitudeImageFilter.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkImageToImageFilter.h>
#include <itkShrinkImageFilter.h>
#include <itkSigmoidImageFilter.h>
#include <itkWatershedImageFilter.h>

#include <Analysis.h>
#include <Analyzer.h>
#include <Scan.h>
#include <filters/AnalysisImageFilter.h>
#include <filters/SegmentRingsImageFilter.h>
#include <filters/SegmentSelectionAndMergingImageFilter.h>
#include <images.h>

namespace PT 
{

const int FEATURE_NUMBER = 14;

const int FEATURE_RING_1_MEAN_INTENSITY = 0;
const int FEATURE_RING_2_MEAN_INTENSITY = 1;
const int FEATURE_CENTER_MEAN_INTENSITY = 2;
const int FEATURE_RING_1_INTENSITY_STANDARD_DEVIATION = 3;
const int FEATURE_RING_2_INTENSITY_STANDARD_DEVIATION = 4;
const int FEATURE_CENTER_INTENSITY_STANDARD_DEVIATION = 5;
const int FEATURE_RING_1_CENTER_MEAN_INTENSITY_RATIO = 6;
const int FEATURE_RING_2_CENTER_MEAN_INTENSITY_RATIO = 7;
const int FEATURE_RING_1_RING_2_MEAN_INTENSITY_RATIO = 8;
const int FEATURE_RING_1_AREA = 9;
const int FEATURE_RING_2_AREA = 10;
const int FEATURE_CENTER_AREA = 11;
const int FEATURE_CELL_MEAN_INTENSITY = 12;
const int FEATURE_CELL_AREA = 13;

extern const std::vector<std::string> FEATURE_NAMES;

static const int SUBREGION_NUMBER = 3;

static const int SUBREGION_RING_1 = 0;
static const int SUBREGION_RING_2 = 1;
static const int SUBREGION_CENTER = 2;

extern const std::vector<std::string> SUBREGION_NAMES;

static const std::string PARAMETER_SHRINK_FACTOR("Shrink Factor");
static const std::string PARAMETER_NUMBER_OF_ITERATIONS("Number Of Iterations");
static const std::string PARAMETER_CONDUCTANCE("Conductance");
static const std::string PARAMETER_SIGMOID_GRADIENT_ALPHA("Sigmoid Grad. Alpha");
static const std::string PARAMETER_SIGMOID_GRADIENT_BETA("Sigmoid Grad. Beta");
static const std::string PARAMETER_THRESHOLD("Threshold");
static const std::string PARAMETER_LEVEL("Level");
static const std::string PARAMETER_MINIMUM_CELL_AREA("Minimum Cell Area");
static const std::string PARAMETER_MAXIMUM_CELL_AREA("Maximum Cell Area");
static const std::string PARAMETER_RING_WIDTH_1("Ring Width 1");
static const std::string PARAMETER_RING_WIDTH_2("Ring Width 2");
static const std::string PARAMETER_MAX_MATCHING_OFFSET("Max. Matching Offset");
static const std::string PARAMETER_MATCHING_PERIOD("Matching Period");

class FilterProgressEvent { };

class WatershedFilterPipeline : public EventGenerator<FilterProgressEvent>
{
public:

    WatershedFilterPipeline(const AnalysisMetadata& analysisMetadata);
    
    void setAssay(const Assay& assay);

    void setImage(const ImageMetadata& imageMetadata);

    void cancel();

    Analysis* getAnalysis()
    {
        return analysis_.get();
    }

    void handleProgressEvent(const itk::EventObject& eventObject);

    // filter type definitions
    typedef itk::ImageFileReader<FloatImage> FileReader;
    typedef itk::ShrinkImageFilter<FloatImage, FloatImage> ShrinkFilter;
    typedef itk::GradientAnisotropicDiffusionImageFilter<FloatImage, FloatImage> DiffusionFilter;
    typedef itk::GradientMagnitudeImageFilter<FloatImage, FloatImage> GradientFilter;
    typedef itk::SigmoidImageFilter<FloatImage, FloatImage> SigmoidGradientFilter;
    typedef itk::WatershedImageFilter<FloatImage> WatershedFilter;
    typedef SegmentSelectionAndMergingImageFilter<ULongImage> SegmentSelectionAndMergingFilter;
    typedef SegmentRingsImageFilter<ULongImage, SegmentSelectionAndMergingFilter::LabelToSegmentKeyFunctor> SegmentRingsFilter;
    typedef itk::ImageFileWriter<RGBAImage> FileWriter;

    class AnalysisFilter : public AnalysisImageFilter<FloatImage, ULongImage, SegmentRingsFilter::LabelToSegmentKeyFunctor> 
    {
    public:
        typedef AnalysisFilter Self;
        typedef AnalysisImageFilter<FloatImage, ULongImage, SegmentRingsFilter::LabelToSegmentKeyFunctor> Superclass;
        typedef itk::SmartPointer<Self> Pointer;
        typedef itk::SmartPointer<const Self> ConstPointer;

        itkNewMacro(Self);
        itkTypeMacro(AnalysisFilter, AnalysisImageFilter);

    protected:

        virtual CellObservation* createCellObservation(short time);

        virtual void computeSubregionFeatures(
                const SegmentKey& segmentKey,
                const SegmentStatistics& segmentStatistics,
                CellObservation* cellObservation);

        virtual void computeCellFeatures(
                CellObservation* cellObservation);
    };

    FileReader::Pointer fileReader_;
    ShrinkFilter::Pointer shrinkFilter_;
    DiffusionFilter::Pointer diffusionFilter_;
    GradientFilter::Pointer gradientFilter_;
    SigmoidGradientFilter::Pointer sigmoidGradientFilter_;
    WatershedFilter::Pointer watershedFilter_;
    SegmentSelectionAndMergingFilter::Pointer segmentSelectionAndMergingFilter_;
    SegmentRingsFilter::Pointer segmentRingsFilter_;
    AnalysisFilter::Pointer analysisFilter_;
    FileWriter::Pointer fileWriter_;

    std::auto_ptr<Analysis> analysis_;

};

}
#endif
