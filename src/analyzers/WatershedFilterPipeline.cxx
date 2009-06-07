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

#include <analyzers/WatershedFilterPipeline.h>

#include <sstream>
#include <vector>

#include <itkCommand.h>

namespace PT 
{

std::string FEATURE_NAME_ARRAY[FEATURE_NUMBER] = 
{
"Ring 1 Mean Intensity",
"Ring 2 Mean Intensity",
"Center Mean Intensity",
"Ring 1 Intensity Standard Deviation",
"Ring 2 Intensity Standard Deviation",
"Center Intensity Standard Deviation",
"Ring 1 / Center Mean Intensity Ratio",
"Ring 2 / Center Mean Intensity Ratio",
"Ring 1 / Ring 2 Mean Intensity Ratio",
"Ring 1 Area",
"Ring 2 Area",
"Center Area",
"Cell Mean Intensity",
"Cell Area"
};
const std::vector<std::string> FEATURE_NAMES(FEATURE_NAME_ARRAY, FEATURE_NAME_ARRAY + FEATURE_NUMBER);

std::string SUBREGION_NAME_ARRAY[SUBREGION_NUMBER] = 
{ 
    "Ring 1", 
    "Ring 2", 
    "Center" 
};
const std::vector<std::string> SUBREGION_NAMES(SUBREGION_NAME_ARRAY, SUBREGION_NAME_ARRAY + SUBREGION_NUMBER);

CellObservation* WatershedFilterPipeline::AnalysisFilter::createCellObservation(short time)
{
    CellObservation* cellObservation = new CellObservation(time, FEATURE_NUMBER);
    
    cellObservation->setFeature(FEATURE_RING_1_AREA, 0);
    cellObservation->setFeature(FEATURE_RING_2_AREA, 0);
    cellObservation->setFeature(FEATURE_CENTER_AREA, 0);

    cellObservation->setFeature(FEATURE_RING_1_MEAN_INTENSITY, 0);
    cellObservation->setFeature(FEATURE_RING_2_MEAN_INTENSITY, 0);
    cellObservation->setFeature(FEATURE_CENTER_MEAN_INTENSITY, 0);

    cellObservation->setFeature(FEATURE_CELL_AREA, 0);
    cellObservation->setFeature(FEATURE_CELL_MEAN_INTENSITY, 0);

    return cellObservation;
}

void WatershedFilterPipeline::AnalysisFilter::computeSubregionFeatures(
        const SegmentKey& segmentKey,
        const SegmentStatistics& segmentStatistics,
        CellObservation* cellObservation)
{
    float area = segmentStatistics.pixelCount;
    float sum = segmentStatistics.intensitySum;
    float sumOfSquares = segmentStatistics.intensitySumOfSquares;

    assert(area > 0);

    float mean = sum / area;
    float standardDeviation = sqrt((sumOfSquares - (sum * sum) / area) / area);

    switch (segmentKey.subId)
    {
        case SUBREGION_RING_1:
        {
            cellObservation->setFeature(FEATURE_RING_1_MEAN_INTENSITY, mean); 
            cellObservation->setFeature(FEATURE_RING_1_INTENSITY_STANDARD_DEVIATION,
                    standardDeviation); 
            cellObservation->setFeature(FEATURE_RING_1_AREA, area); 
            break;
        }
        case SUBREGION_RING_2:
        {
            cellObservation->setFeature(FEATURE_RING_2_MEAN_INTENSITY, mean); 
            cellObservation->setFeature(FEATURE_RING_2_INTENSITY_STANDARD_DEVIATION,
                    standardDeviation); 
            cellObservation->setFeature(FEATURE_RING_2_AREA, area); 
            break;
        }
        case SUBREGION_CENTER:
        {
            cellObservation->setFeature(FEATURE_CENTER_MEAN_INTENSITY, mean); 
            cellObservation->setFeature(FEATURE_CENTER_INTENSITY_STANDARD_DEVIATION,
                    standardDeviation); 
            cellObservation->setFeature(FEATURE_CENTER_AREA, area); 
            break;
        }
    }

    float cellArea = cellObservation->getFeature(FEATURE_CELL_AREA) + area;
    cellObservation->setFeature(FEATURE_CELL_AREA, cellArea); 

    float cellMean = cellObservation->getFeature(FEATURE_CELL_MEAN_INTENSITY) + sum;
    cellObservation->setFeature(FEATURE_CELL_MEAN_INTENSITY, cellMean); 
}

void WatershedFilterPipeline::AnalysisFilter::computeCellFeatures(
        CellObservation* cellObservation)
{
    float cellArea = cellObservation->getFeature(FEATURE_CELL_AREA);
    float cellMean = cellObservation->getFeature(FEATURE_CELL_MEAN_INTENSITY) / cellArea;
    cellObservation->setFeature(FEATURE_CELL_MEAN_INTENSITY, cellMean); 

    float ring1Mean = cellObservation->getFeature(FEATURE_RING_1_MEAN_INTENSITY);
    float ring2Mean = cellObservation->getFeature(FEATURE_RING_2_MEAN_INTENSITY);
    float centerMean = cellObservation->getFeature(FEATURE_CENTER_MEAN_INTENSITY);

    if (centerMean > 0)
    {
        cellObservation->setFeature(FEATURE_RING_1_CENTER_MEAN_INTENSITY_RATIO,
                ring1Mean / centerMean);
        cellObservation->setFeature(FEATURE_RING_2_CENTER_MEAN_INTENSITY_RATIO,
                ring2Mean / centerMean);
    }

    if (ring2Mean > 0)
    {
        cellObservation->setFeature(FEATURE_RING_1_RING_2_MEAN_INTENSITY_RATIO,
                ring1Mean / ring2Mean);
    }
}

void addObserver(WatershedFilterPipeline* filterPipeline, itk::ProcessObject* processObject)
{
    typedef itk::ReceptorMemberCommand<WatershedFilterPipeline> ProgressCommand;
    ProgressCommand::Pointer progressCommand = ProgressCommand::New();
    progressCommand->SetCallbackFunction(filterPipeline, &PT::WatershedFilterPipeline::handleProgressEvent);
    processObject->AddObserver(itk::ProgressEvent(), progressCommand);
}

WatershedFilterPipeline::WatershedFilterPipeline(const AnalysisMetadata& analysisMetadata)
{
    // create and initialize filters
    fileReader_ = FileReader::New();

    shrinkFilter_ = ShrinkFilter::New();
    shrinkFilter_->SetInput(fileReader_->GetOutput());

    diffusionFilter_ = DiffusionFilter::New();
    diffusionFilter_->SetInput(shrinkFilter_->GetOutput());

    gradientFilter_ = GradientFilter::New();
    gradientFilter_->SetInput(diffusionFilter_->GetOutput());

    sigmoidGradientFilter_ = SigmoidGradientFilter::New();
    sigmoidGradientFilter_->SetOutputMinimum(0);
    sigmoidGradientFilter_->SetOutputMaximum(1);
    sigmoidGradientFilter_->SetInput(gradientFilter_->GetOutput());

    watershedFilter_ = WatershedFilter::New();
    watershedFilter_->SetInput(sigmoidGradientFilter_->GetOutput());

    segmentSelectionAndMergingFilter_ = SegmentSelectionAndMergingFilter::New();
    segmentSelectionAndMergingFilter_->setMaxNumberOfSegments(256 * 256 - 1);
    segmentSelectionAndMergingFilter_->SetInput(watershedFilter_->GetOutput());

    segmentRingsFilter_ = SegmentRingsFilter::New();
    segmentRingsFilter_->SetInput(segmentSelectionAndMergingFilter_->GetOutput());

    analysisFilter_ = AnalysisFilter::New();
    analysisFilter_->setIntensityInput(shrinkFilter_->GetOutput());
    analysisFilter_->setLabelInput(segmentRingsFilter_->GetOutput());

    fileWriter_ = FileWriter::New();
    fileWriter_->SetInput(analysisFilter_->GetOutput());

    // add observer to filters
    {
        addObserver(this, fileReader_);
        addObserver(this, shrinkFilter_);
        addObserver(this, diffusionFilter_);
        addObserver(this, gradientFilter_);
        addObserver(this, sigmoidGradientFilter_);
        addObserver(this, watershedFilter_);
        addObserver(this, segmentSelectionAndMergingFilter_);
        addObserver(this, segmentRingsFilter_);
        addObserver(this, analysisFilter_);
        addObserver(this, fileWriter_);
    }

    // create analysis
    analysis_ = std::auto_ptr<Analysis>( new Analysis(analysisMetadata) );
    analysisFilter_->setAnalysis( analysis_.get() );
}

void WatershedFilterPipeline::setAssay(const Assay& assay)
{
    // collect parameters
    int shrinkFactor = assay.getParameter(PARAMETER_SHRINK_FACTOR).getIntValue();
    int diffusionNumberOfIterations = assay.getParameter(PARAMETER_NUMBER_OF_ITERATIONS).getIntValue();
    double diffusionConductance = assay.getParameter(PARAMETER_CONDUCTANCE).getDoubleValue();
    double sigmoidAlpha = assay.getParameter(PARAMETER_SIGMOID_GRADIENT_ALPHA).getDoubleValue();
    double sigmoidBeta = assay.getParameter(PARAMETER_SIGMOID_GRADIENT_BETA).getDoubleValue();
    double threshold = assay.getParameter(PARAMETER_THRESHOLD).getDoubleValue();
    double level = assay.getParameter(PARAMETER_LEVEL).getDoubleValue();
    int minimumCellArea = assay.getParameter(PARAMETER_MINIMUM_CELL_AREA).getIntValue();
    int maximumCellArea = assay.getParameter(PARAMETER_MAXIMUM_CELL_AREA).getIntValue();
    int ringWidth1 = assay.getParameter(PARAMETER_RING_WIDTH_1).getIntValue();
    int ringWidth2 = assay.getParameter(PARAMETER_RING_WIDTH_2).getIntValue();
    double maximumMatchingOffset = assay.getParameter(PARAMETER_MAX_MATCHING_OFFSET).getDoubleValue();
    int matchingPeriod = assay.getParameter(PARAMETER_MATCHING_PERIOD).getIntValue();

    // update shrink filter
    shrinkFilter_->SetShrinkFactors(shrinkFactor);

    // update diffusion filter
    diffusionFilter_->SetNumberOfIterations(diffusionNumberOfIterations);
    diffusionFilter_->SetTimeStep(0.125); // standard value
    diffusionFilter_->SetConductanceParameter(diffusionConductance);

    // update sigmoid gradient filter
    sigmoidGradientFilter_->SetAlpha(sigmoidAlpha);
    sigmoidGradientFilter_->SetBeta(sigmoidBeta);

    // update watershed filter
    watershedFilter_->SetThreshold(threshold);
    watershedFilter_->SetLevel(level);

    // update cell extraction filter
    segmentSelectionAndMergingFilter_->setMinimumSegmentSize(minimumCellArea);
    segmentSelectionAndMergingFilter_->setMaximumSegmentSize(maximumCellArea);

    // update segment rings filter
    segmentRingsFilter_->setRingWidth1(ringWidth1);
    segmentRingsFilter_->setRingWidth2(ringWidth2);

    // update analysis filter
    analysisFilter_->setMaxMatchingOffset((float)maximumMatchingOffset);
}

void WatershedFilterPipeline::setImage(const ImageMetadata& imageMetadata)
{
    const std::string& filepath = imageMetadata.filepath;
    fileReader_->SetFileName(filepath.c_str());

    analysisFilter_->setImage(imageMetadata.key);

    std::string filePath = analysis_->getMetadata().getFilePath(imageMetadata.key);
    fileWriter_->SetFileName(filePath);
}

void WatershedFilterPipeline::cancel()
{
    fileWriter_->AbortGenerateDataOn();
    analysisFilter_->AbortGenerateDataOn();
    segmentRingsFilter_->AbortGenerateDataOn();
    segmentSelectionAndMergingFilter_->AbortGenerateDataOn();
    watershedFilter_->AbortGenerateDataOn();
    sigmoidGradientFilter_->AbortGenerateDataOn();
    gradientFilter_->AbortGenerateDataOn();
    diffusionFilter_->AbortGenerateDataOn();
    shrinkFilter_->AbortGenerateDataOn();
    fileReader_->AbortGenerateDataOn();
}

void WatershedFilterPipeline::handleProgressEvent(const itk::EventObject& eventObject)
{
    notifyEventHandler(FilterProgressEvent());
}

}
