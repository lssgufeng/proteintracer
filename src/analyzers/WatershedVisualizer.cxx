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

#include <analyzers/WatershedVisualizer.h>

#include <sstream>

#include <analyzers/IntensityVisualization.h>
#include <analyzers/LabelVisualization.h>
#include <analyzers/SegmentVisualization.h>
#include <analyzers/ThresholdVisualization.h>

namespace PT 
{

static const std::string VISUALIZATION_SOURCE("Source");
static const std::string VISUALIZATION_DIFFUSION("Diffusion");
static const std::string VISUALIZATION_GRADIENT("Gradient");
static const std::string VISUALIZATION_SIGMOID_GRADIENT("Sigmoid Gradient");
static const std::string VISUALIZATION_THRESHOLD("Threshold");
static const std::string VISUALIZATION_WATERSHED("Watershed");
static const std::string VISUALIZATION_CELLS("Cells");
static const std::string VISUALIZATION_RINGS("Rings");
static const std::string VISUALIZATION_ANALYSIS("Analysis");

WatershedVisualizer::WatershedVisualizer() : filterPipeline_(
        AnalysisMetadata( FEATURE_NAMES, SUBREGION_NAMES, "") ) 
{
    rescaleIntensityRGBFilter_ = RescaleIntensityRGBImageFilter<FloatImage>::New();

    thresholdVisualizationFilter_ = ThresholdVisualizationImageFilter<FloatImage>::New();
    thresholdVisualizationFilter_->SetInput(filterPipeline_.sigmoidGradientFilter_->GetOutput());

    watershedVisualizationFilter_ = WatershedVisualizationFilter::New();
    watershedVisualizationFilter_->SetInput(filterPipeline_.watershedFilter_->GetOutput());

    cellsVisualizationFilter_ = CellsVisualizationFilter::New();
    cellsVisualizationFilter_->SetSourceInput(filterPipeline_.shrinkFilter_->GetOutput());
    cellsVisualizationFilter_->SetLabelInput(filterPipeline_.segmentSelectionAndMergingFilter_->GetOutput());

    ringsVisualizationFilter_ = RingsVisualizationFilter::New();
    ringsVisualizationFilter_->SetSourceInput(filterPipeline_.shrinkFilter_->GetOutput());
    ringsVisualizationFilter_->SetLabelInput(filterPipeline_.segmentRingsFilter_->GetOutput());

    analysisVisualizationFilter_ = AnalysisVisualizationFilter::New();
    analysisVisualizationFilter_->SetSourceInput(filterPipeline_.shrinkFilter_->GetOutput());
    analysisVisualizationFilter_->SetLabelInput(filterPipeline_.analysisFilter_->GetOutput());
}

const std::vector<std::string>& WatershedVisualizer::getVisualizationNames() const
{
    static std::vector<std::string> visualizations;
    if (visualizations.empty())
    {
        visualizations.push_back(VISUALIZATION_SOURCE);
        visualizations.push_back(VISUALIZATION_DIFFUSION);
        visualizations.push_back(VISUALIZATION_GRADIENT);
        visualizations.push_back(VISUALIZATION_SIGMOID_GRADIENT);
        visualizations.push_back(VISUALIZATION_THRESHOLD);
        visualizations.push_back(VISUALIZATION_WATERSHED);
        visualizations.push_back(VISUALIZATION_CELLS);
        visualizations.push_back(VISUALIZATION_RINGS);
        visualizations.push_back(VISUALIZATION_ANALYSIS);
    }
    return visualizations;
}

class CellsVisualization : public Visualization
{
public:

    CellsVisualization(
        ULongImage::ConstPointer labelImage, 
        RGBImage::ConstPointer visualizationImage, 
        const ImageKey& imageKey, 
        unsigned long segmentCount,
        const std::vector<unsigned long>* segmentSizeVector) :
            Visualization(visualizationImage, imageKey),
            labelImage_(labelImage),
            segmentCount_(segmentCount),
            segmentSizeVector_(segmentSizeVector)
    {
        assert(labelImage->GetLargestPossibleRegion() == visualizationImage->GetLargestPossibleRegion());
    }

    std::string getPixelLabel(int x, int y)
    {
        FloatImage::IndexType index;
        index[0] = x;
        index[1] = y;

        FloatImage::RegionType imageRegion = labelImage_->GetLargestPossibleRegion();

        if (imageRegion.IsInside(index))
        {
            std::stringstream text;

            unsigned long label = labelImage_->GetPixel(index);
            if (label > 0)
            {
                text << "Cell: " << label << " / " << segmentCount_;
            }
            else
            {
                text << "Background";
            }

            unsigned long segmentSize = (*segmentSizeVector_)[label];
            text << " (Size: " << segmentSize << ")";

            return text.str();
        }
        else
        {
            return "";
        }
    }

    ULongImage::ConstPointer labelImage_;

    unsigned long segmentCount_;
    const std::vector<unsigned long>* segmentSizeVector_;
};

class AnalysisVisualization : public Visualization
{
public:
    AnalysisVisualization(
        RGBAImage::ConstPointer analysisImage, 
        RGBImage::ConstPointer visualizationImage, 
        const ImageKey& imageKey,
        Analysis* analysis) :
            Visualization(visualizationImage, imageKey),
            analysisImage_(analysisImage),
            analysis_(analysis)
    {
        assert(analysisImage->GetLargestPossibleRegion() == visualizationImage->GetLargestPossibleRegion());
    }

    std::string getPixelLabel(int x, int y)
    {
        ImageIndex index;
        index[0] = x;
        index[1] = y;

        const ImageRegion& imageRegion = analysisImage_->GetLargestPossibleRegion();

        if (imageRegion.IsInside(index))
        {
            std::stringstream text;

            RGBAPixel pixelAnalysis = analysisImage_->GetPixel(index);
            int cellId = pixelAnalysis[0] | (pixelAnalysis[1] << 8);
            if (cellId > 0)
            {
                text << "Cell: " << cellId;

                int subregionIndex = pixelAnalysis[2];

                const AnalysisMetadata& metadata = analysis_->getMetadata();

                Cell* cell = analysis_->getCell(cellId);
                CellObservation* cellObservation = cell->getObservation(imageKey_.time);
                switch (subregionIndex)
                {
                    case SUBREGION_RING_1:
                        {
                            text << "  -  " << metadata.featureNames[FEATURE_RING_1_AREA];
                            text << ": " << cellObservation->getFeature(FEATURE_RING_1_AREA);

                            text << "  -  " << metadata.featureNames[FEATURE_RING_1_MEAN_INTENSITY];
                            text << ": " << cellObservation->getFeature(FEATURE_RING_1_MEAN_INTENSITY);

                            break;
                        }
                    case SUBREGION_RING_2:
                        {
                            text << "  -  " << metadata.featureNames[FEATURE_RING_2_AREA];
                            text << ": " << cellObservation->getFeature(FEATURE_RING_2_AREA);

                            text << "  -  " << metadata.featureNames[FEATURE_RING_2_MEAN_INTENSITY];
                            text << ": " << cellObservation->getFeature(FEATURE_RING_2_MEAN_INTENSITY);

                            break;
                        }
                    case SUBREGION_CENTER:
                        {
                            text << "  -  " << metadata.featureNames[FEATURE_CENTER_AREA];
                            text << ": " << cellObservation->getFeature(FEATURE_CENTER_AREA);

                            text << "  -  " << metadata.featureNames[FEATURE_CENTER_MEAN_INTENSITY];
                            text << ": " << cellObservation->getFeature(FEATURE_CENTER_MEAN_INTENSITY);

                            break;
                        }
                }
        }
        else
        {
            text << "Background";
        }

        return text.str();
    }
    else
    {
        return "";
    }
}

private:

    RGBAImage::ConstPointer analysisImage_;

    Analysis* analysis_;
};

std::auto_ptr<Visualization> WatershedVisualizer::createVisualization(
        const std::string& visualizationName, 
        const Assay& assay, 
        const ImageMetadata& imageMetadata) throw(VisualizerException)
{
    try 
    {
        // update filter parameters
        {

            filterPipeline_.setAssay(assay);
            filterPipeline_.setImage(imageMetadata);
            
            // remove all cell observations from this image and all following images
            {
                const ImageKey& imageKey = imageMetadata.key;

                Analysis* analysis = filterPipeline_.analysis_.get();

                Analysis::CellIterator cellIt = analysis->getCellStart();
                Analysis::CellIterator cellEnd = analysis->getCellEnd();
                while (cellIt != cellEnd)
                {
                    Cell* cell = *cellIt;
                    
                    Cell::ObservationIterator observationIt = cell->getObservationStart();
                    Cell::ObservationIterator observationEnd = cell->getObservationEnd();
                    while (observationIt != observationEnd)
                    {
                        CellObservation* observation = *observationIt;

                        // it is important to increment the iterator before the erasure of the element
                        ++observationIt;

                        short tempTime = observation->getTime();
                        if (imageKey.time <= tempTime)
                        {
                            cell->removeObservation(tempTime);
                        }
                    }

                    // it is important to increment the iterator before the erasure of the element
                    ++cellIt;

                    // remove cell if it doesn't contain any observations any more
                    if (cell->getNumberOfObservations() == 0)
                    {
                        analysis->removeCell(cell->getId());
                    }
                }

                filterPipeline_.analysisFilter_->Modified();
            }

            // update threshold visualization filter
            double threshold = assay.getParameter(PARAMETER_THRESHOLD).getDoubleValue();
            thresholdVisualizationFilter_->setThreshold(threshold);
        }

        if (visualizationName == VISUALIZATION_SOURCE)
        {
            rescaleIntensityRGBFilter_->SetInput(filterPipeline_.fileReader_->GetOutput());

            // Update the largest possible region, because a call to
            // Update() would try to update the previously requested
            // region, no matter whether the largest possible region
            // has changed (for example by setting a different shrink
            // factor).  See documentation of
            // itk::ProcessObject::Update() .
            rescaleIntensityRGBFilter_->UpdateLargestPossibleRegion();

            FloatImage::ConstPointer intensityImage = filterPipeline_.fileReader_->GetOutput();
            RGBImage::ConstPointer visualizationImage = rescaleIntensityRGBFilter_->GetOutput();

            return std::auto_ptr<Visualization>(new IntensityVisualization<FloatImage>(intensityImage, visualizationImage, imageMetadata.key));
        }
        else if (visualizationName == VISUALIZATION_DIFFUSION)
        {
            rescaleIntensityRGBFilter_->SetInput(filterPipeline_.diffusionFilter_->GetOutput());

            // Update the largest possible region, because a call to
            // Update() would try to update the previously requested
            // region, no matter whether the largest possible region
            // has changed (for example by setting a different shrink
            // factor).  See documentation of
            // itk::ProcessObject::Update() .
            rescaleIntensityRGBFilter_->UpdateLargestPossibleRegion();

            FloatImage::ConstPointer intensityImage = filterPipeline_.diffusionFilter_->GetOutput();
            RGBImage::ConstPointer visualizationImage = rescaleIntensityRGBFilter_->GetOutput();

            return std::auto_ptr<Visualization>(new IntensityVisualization<FloatImage>(intensityImage, visualizationImage, imageMetadata.key));
        }
        else if (visualizationName == VISUALIZATION_GRADIENT)
        {
            rescaleIntensityRGBFilter_->SetInput(filterPipeline_.gradientFilter_->GetOutput());

            // Update the largest possible region, because a call to
            // Update() would try to update the previously requested
            // region, no matter whether the largest possible region
            // has changed (for example by setting a different shrink
            // factor).  See documentation of
            // itk::ProcessObject::Update() .
            rescaleIntensityRGBFilter_->UpdateLargestPossibleRegion();

            FloatImage::ConstPointer intensityImage = filterPipeline_.gradientFilter_->GetOutput();
            RGBImage::ConstPointer visualizationImage = rescaleIntensityRGBFilter_->GetOutput();

            return std::auto_ptr<Visualization>(new IntensityVisualization<FloatImage>(intensityImage, visualizationImage, imageMetadata.key));
        }
        else if (visualizationName == VISUALIZATION_SIGMOID_GRADIENT)
        {
            rescaleIntensityRGBFilter_->SetInput(filterPipeline_.sigmoidGradientFilter_->GetOutput());

            // Update the largest possible region, because a call to
            // Update() would try to update the previously requested
            // region, no matter whether the largest possible region
            // has changed (for example by setting a different shrink
            // factor).  See documentation of
            // itk::ProcessObject::Update() .
            rescaleIntensityRGBFilter_->UpdateLargestPossibleRegion();

            FloatImage::ConstPointer intensityImage = filterPipeline_.sigmoidGradientFilter_->GetOutput();
            RGBImage::ConstPointer visualizationImage = rescaleIntensityRGBFilter_->GetOutput();

            return std::auto_ptr<Visualization>(new IntensityVisualization<FloatImage>(intensityImage, visualizationImage, imageMetadata.key));
        }
        else if (visualizationName == VISUALIZATION_THRESHOLD)
        {
            // Update the largest possible region, because a call to
            // Update() would try to update the previously requested
            // region, no matter whether the largest possible region
            // has changed (for example by setting a different shrink
            // factor).  See documentation of
            // itk::ProcessObject::Update() .
            thresholdVisualizationFilter_->UpdateLargestPossibleRegion();

            FloatImage::ConstPointer sourceImage = filterPipeline_.sigmoidGradientFilter_->GetOutput();
            RGBImage::ConstPointer thresholdImage = thresholdVisualizationFilter_->GetOutput();

            float maxIntensity = thresholdVisualizationFilter_->getMaxIntensity();
            return std::auto_ptr<Visualization>(new ThresholdVisualization<FloatImage>(sourceImage, thresholdImage, imageMetadata.key, maxIntensity));
        }
        else if (visualizationName == VISUALIZATION_WATERSHED)
        {
            // Update the largest possible region, because a call to
            // Update() would try to update the previously requested
            // region, no matter whether the largest possible region
            // has changed (for example by setting a different shrink
            // factor).  See documentation of
            // itk::ProcessObject::Update() .
            watershedVisualizationFilter_->UpdateLargestPossibleRegion();

            ULongImage::ConstPointer labelImage = filterPipeline_.watershedFilter_->GetOutput();
            RGBImage::ConstPointer visualizationImage = watershedVisualizationFilter_->GetOutput();

            return std::auto_ptr<Visualization>(
                new LabelVisualization<ULongImage>(
                    labelImage, 
                    visualizationImage, 
                    imageMetadata.key));
        }
        else if (visualizationName == VISUALIZATION_CELLS)
        {
            // Update the largest possible region, because a call to
            // Update() would try to update the previously requested
            // region, no matter whether the largest possible region
            // has changed (for example by setting a different shrink
            // factor).  See documentation of
            // itk::ProcessObject::Update() .
            cellsVisualizationFilter_->UpdateLargestPossibleRegion();

            ULongImage::ConstPointer labelImage = filterPipeline_.segmentSelectionAndMergingFilter_->GetOutput();
            RGBImage::ConstPointer visualizationImage = cellsVisualizationFilter_->GetOutput();

            unsigned long segmentCount = filterPipeline_.segmentSelectionAndMergingFilter_->getNumberOfSegments();
            const std::vector<unsigned long>* segmentSizeVector = &filterPipeline_.segmentSelectionAndMergingFilter_->getSegmentSizesInPixels();

            return std::auto_ptr<Visualization>(
                new CellsVisualization(
                    labelImage, 
                    visualizationImage, 
                    imageMetadata.key, 
                    segmentCount,
                    segmentSizeVector));
        }
        else if (visualizationName == VISUALIZATION_RINGS)
        {
            // Update the largest possible region, because a call to
            // Update() would try to update the previously requested
            // region, no matter whether the largest possible region
            // has changed (for example by setting a different shrink
            // factor).  See documentation of
            // itk::ProcessObject::Update() .
            ringsVisualizationFilter_->UpdateLargestPossibleRegion();

            ULongImage::ConstPointer labelImage = filterPipeline_.segmentRingsFilter_->GetOutput();
            RGBImage::ConstPointer visualizationImage = ringsVisualizationFilter_->GetOutput();

            unsigned long segmentCount = filterPipeline_.segmentSelectionAndMergingFilter_->getNumberOfSegments();

            return std::auto_ptr<Visualization>(
                new SegmentVisualization<
                    ULongImage, 
                    WatershedFilterPipeline::SegmentRingsFilter::LabelToSegmentKeyFunctor>(
                        labelImage, 
                        visualizationImage, 
                        imageMetadata.key, 
                        segmentCount) );
        }
        else if (visualizationName == VISUALIZATION_ANALYSIS)
        {
            // Update the largest possible region, because a call to
            // Update() would try to update the previously requested
            // region, no matter whether the largest possible region
            // has changed (for example by setting a different shrink
            // factor).  See documentation of
            // itk::ProcessObject::Update() .
            analysisVisualizationFilter_->UpdateLargestPossibleRegion();

            RGBAImage::ConstPointer analysisImage = filterPipeline_.analysisFilter_->GetOutput();
            RGBImage::ConstPointer visualizationImage = analysisVisualizationFilter_->GetOutput();

            Analysis* analysis = filterPipeline_.getAnalysis();

            return std::auto_ptr<Visualization>(
                new AnalysisVisualization(
                    analysisImage, 
                    visualizationImage, 
                    imageMetadata.key, 
                    analysis));
        }
        else
        {
            throw InvalidArgumentException("invalid visualization name");
        }
    }
    catch (itk::ExceptionObject &e)
    {
        // TODO reset individual visualization filter
        filterPipeline_.analysisFilter_->ResetPipeline();
        throw VisualizerException(e);
    }
}

}
