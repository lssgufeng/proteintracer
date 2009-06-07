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

#include <analyzers/WatershedAnalyzer.h>

#include <itkExceptionObject.h>

#include <sstream>

#include <io/AnalysisIO.h>

namespace PT 
{

static const std::string ANALYZER_NAME("Watershed");

WatershedAnalyzer::WatershedAnalyzer(
        const Assay& assay, 
        const Scan* scan,
        const std::string& analysisDirectory) :
            scan_(scan), 
            filterPipeline_(
                    AnalysisMetadata( 
                        FEATURE_NAMES, 
                        SUBREGION_NAMES, 
                        analysisDirectory) )
{
    filterPipeline_.setEventHandler(this);
    filterPipeline_.setAssay(assay);
}

void WatershedAnalyzer::process()
{
    int imageCounter = 0;

    try 
    {
        // notify event handler
        {
            std::stringstream message;
            message << "start analysis" << std::endl;
            notifyEventHandler( AnalyzerEvent(message.str()) );
            notifyEventHandler( AnalyzerEvent(0.0) );
        }

        // iterator over image series
        Scan::ImageSeriesConstIterator imageSeriesIt = scan_->getImageSeriesStart();
        Scan::ImageSeriesConstIterator imageSeriesEnd = scan_->getImageSeriesEnd();
        for (;imageSeriesIt != imageSeriesEnd; ++imageSeriesIt)
        {
            const ImageSeries& imageSeries = *imageSeriesIt;

            // add image series to analysis
            filterPipeline_.analysis_->addImageSeries(imageSeries);

            // iterator over time range
            const ImageSeries::TimeRange& timeRange = imageSeries.timeRange;
            for (short time = timeRange.min; time <= timeRange.max; ++time)
            {
                ImageKey imageKey = imageSeries.getImageKey(time);
                const ImageMetadata& imageMetadata = scan_->getImageMetadata(imageKey);

                // generate message event
                std::stringstream message;
                message << "processing image " << imageMetadata.filepath << std::endl;
                notifyEventHandler( AnalyzerEvent(message.str()) );

                // process image
                filterPipeline_.setImage(imageMetadata);
                filterPipeline_.fileWriter_->Update();

                // generate progress event
                float progress = (++imageCounter / (float) scan_->getNumberOfImages());
                notifyEventHandler(AnalyzerEvent(progress));
            }
        }

        // save analysis
        saveAnalysis(*filterPipeline_.analysis_.get());

        filterPipeline_.analysisFilter_->ResetPipeline();

        // notify event handler
        {
            std::stringstream message;
            message << "analysis finished successfully" << std::endl;
            notifyEventHandler( AnalyzerEvent(message.str()) );
            notifyEventHandler(AnalyzerEvent(1.0));
        }
    }
    catch(PT::Exception& err)
    {
        std::stringstream message;
        message << "an error occurred: " << err.what() << std::endl;
        notifyEventHandler( AnalyzerEvent(message.str()) );
    }
    catch(itk::ProcessAborted&)
    {
        filterPipeline_.fileWriter_->ResetPipeline();
        filterPipeline_.analysis_.reset();

        // notify event handler
        std::stringstream message;
        message << "analysis cancelled" << std::endl;
        notifyEventHandler( AnalyzerEvent(message.str()) );
    }
    catch(itk::ExceptionObject& err)
    {
        std::stringstream message;
        message << "an error occurred: " << err.what() << std::endl;
        notifyEventHandler( AnalyzerEvent(message.str()) );
    }
}

const std::string& WatershedAnalyzer::getName()
{
    return ANALYZER_NAME;
}

std::auto_ptr<Assay> WatershedAnalyzer::createAssay()
{
    std::vector<Parameter> vec;

    Parameter shrinkFactorParam(
        PARAMETER_SHRINK_FACTOR, 
        "The image size is reduced by the factor in each dimension before any further processing is done.",
        2, 1, 4);
    vec.push_back(shrinkFactorParam);

    Parameter diffusionNumberOfIterationsParam(
        PARAMETER_NUMBER_OF_ITERATIONS,
        "The number of iterations performed by the anisotropic diffusion filter.  More iteration steps will result in stronger diffusion.",
        5, 1, 10);
    vec.push_back(diffusionNumberOfIterationsParam);

    Parameter diffusionConductanceParam(
        PARAMETER_CONDUCTANCE,
        "Controls the conductance term of the anisotropic diffusion equation.  A lower value will preserve edges, a higher value will result in stronger diffusion. Too little diffusion might lead to over-segmentation and increase the effort of the watershed transformation.",
        2.0, 0.1, 10.0);
    vec.push_back(diffusionConductanceParam);

    Parameter sigmoidGradientAlphaParam(
        PARAMETER_SIGMOID_GRADIENT_ALPHA,
        "Controls the steepness of the sigmoid function.  Lower values act like a threshold while higher values cause a nearly linear mapping.  Thus, the alpha parameter determines the strength of the effect the sigmoid filter has on the image."
        , 10.0, 0.0, itk::NumericTraits<float>::max());
    vec.push_back(sigmoidGradientAlphaParam);

    Parameter sigmoidGradientBetaParam(
        PARAMETER_SIGMOID_GRADIENT_BETA,
        "Controls the offset of the sigmoid function, measured in intensity units.  The sigmoid will be centered at the intensity with the same value as the beta parameter.  Thus, the beta parameter determines which intensity values are attenuated or accentuated.",
        20.0, itk::NumericTraits<float>::NonpositiveMin(), itk::NumericTraits<float>::max());
    vec.push_back(sigmoidGradientBetaParam);

    Parameter thresholdParam(
        PARAMETER_THRESHOLD,
        "Sets the minimum intensity value for watershed processing, measured relative to the maximum intensity of the image.  Raising this value will reduce over-segmentation.",
        0.3, 0.0, 1.0);
    vec.push_back(thresholdParam);

    Parameter levelParam(
        PARAMETER_LEVEL,
        "Sets the flood level of the watershed segmentation, measured relative to the maximum intensity of the image.  Segments with watershed depths lower than this level are merged.  Raising the level parameter will reduce over-segmentation, but might also result in under-segmentation.",
        4E-1, 0.0, 10000.0);
    vec.push_back(levelParam);

    Parameter minimumCellAreaParam(
        PARAMETER_MINIMUM_CELL_AREA,
        "Segments smaller than this area will be discarded. The area is measured in pixels.",
        10, 0, itk::NumericTraits<int>::max());
    vec.push_back(minimumCellAreaParam);

    Parameter maximumCellAreaParam(
        PARAMETER_MAXIMUM_CELL_AREA,
        "Segments greater than this area will be discarded. The area is measured in pixels.",
        10000, 0, itk::NumericTraits<int>::max());
    vec.push_back(maximumCellAreaParam);

    Parameter ringWidth1Param(
        PARAMETER_RING_WIDTH_1,
        "The width of the first ring, measured in pixels.",
        4, 0, 50);
    vec.push_back(ringWidth1Param);

    Parameter ringWidth2Param(
        PARAMETER_RING_WIDTH_2,
        "The width of the second ring, measured in pixels",
        4, 0, 50);
    vec.push_back(ringWidth2Param);

    Parameter maximumMatchingOffsetParam(
        PARAMETER_MAX_MATCHING_OFFSET,
        "The maximum offset between two successive cell observations to be identified as one cell, measured in pixels.",
        5.0, 0.0, 100.0);
    vec.push_back(maximumMatchingOffsetParam);

    Parameter matchingPeriodParam(
        PARAMETER_MATCHING_PERIOD,
        "The number of previous time steps that are examined for matching cell observations. A value of 0 causes a new cell to be created for every single cell observation.",
        2, 0, 10);
    vec.push_back(matchingPeriodParam);

    std::auto_ptr<Assay> assay = std::auto_ptr<Assay>(new Assay(getName(), vec));
    return assay;
}

void WatershedAnalyzer::handleEvent(const FilterProgressEvent &event)
{
    notifyEventHandler(AnalyzerEvent());
}

}
