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

#include <filters/AnalysisVisualizationImageFilter.h>

#include <itkConstShapedNeighborhoodIterator.h>
#include <itkImageRegionIterator.h>
#include <itkNeighborhoodAlgorithm.h>
#include <itkProgressReporter.h>

namespace PT
{

AnalysisVisualizationImageFilter::AnalysisVisualizationImageFilter() :
    cellSelection_(0),
    markCells_(true)
{
}

void AnalysisVisualizationImageFilter::ThreadedGenerateData(
        const RGBImage::RegionType& outputRegionForThread,
        int threadId)
{
    itk::ProgressReporter progress(this, threadId, outputRegionForThread.GetNumberOfPixels());

    RGBAImage::ConstPointer analysisImage = this->GetInput();
    RGBImage::Pointer outputImage = this->GetOutput();

    typedef itk::ConstShapedNeighborhoodIterator<RGBAImage> ShapedNeighborhoodIterator;

    ShapedNeighborhoodIterator::RadiusType iteratorRadius;
    iteratorRadius.Fill(1);

    typedef itk::NeighborhoodAlgorithm::ImageBoundaryFacesCalculator<RGBAImage> FaceCalculator;
    typedef FaceCalculator::FaceListType FaceList;
    typedef FaceList::iterator FaceIterator;

    FaceCalculator faceCalculator;
    FaceList faceList = faceCalculator(analysisImage, outputRegionForThread, iteratorRadius);
    for (FaceIterator fit = faceList.begin(); fit != faceList.end(); ++fit)
    {
        ShapedNeighborhoodIterator analysisImageIt(iteratorRadius, analysisImage, *fit);

        // initialize neighborhood iterator
        for (int y = -1; y <= 1; y++)
        {
            for (int x = -1; x <= 1; x++)
            {     
                if (x*x + y*y <= 1) // don't activate diagonal offsets
                {
                    ShapedNeighborhoodIterator::OffsetType off;
                    off[0] = x;
                    off[1] = y;
                    analysisImageIt.ActivateOffset(off);
                }
            }
        }

        itk::ImageRegionIterator<RGBImage> outputImageIt(outputImage, *fit);
        for (outputImageIt.GoToBegin(); !outputImageIt.IsAtEnd(); ++analysisImageIt, ++outputImageIt)
        {
            RGBAPixel analysisPixel = analysisImageIt.GetCenterPixel();

            unsigned short cellId = Analysis::decodeCellId(analysisPixel);
            unsigned char subregionIndex = Analysis::decodeSubregionIndex(analysisPixel);
            unsigned char intensity = Analysis::decodeIntensity(analysisPixel);

            // mark cells only if marking cells is enabled
            // ensure that current pixel doesn't belong to background
            if (this->markCells_ && cellId > 0)
            {
                // find out whether pixel is selected
                bool isSelected = (cellSelection_ != 0 && cellSelection_->containsCell(cellId));

                // find out whether pixel is in cell border or subregion border
                bool isCellBorder = false;
                bool isSubregionBorder = false;
                ShapedNeighborhoodIterator::ConstIterator neighborhoodIt;
                for (neighborhoodIt = analysisImageIt.Begin();
                        neighborhoodIt != analysisImageIt.End();
                        ++neighborhoodIt)
                {
                    RGBAPixel neighborhoodPixel = neighborhoodIt.Get();
                    unsigned short neighborhoodCellId = Analysis::decodeCellId(neighborhoodPixel);
                    if (cellId != neighborhoodCellId)
                    {
                        isCellBorder = true;
                        break;
                    }
                    else
                    {
                        unsigned char neighborhoodSubregionIndex = 
                            Analysis::decodeSubregionIndex(neighborhoodPixel);
                        // use > operator to prevent that subregion borders are
                        // marked from both sides
                        if (subregionIndex > neighborhoodSubregionIndex) 
                        {
                            isSubregionBorder = true;
                        }
                    }
                }

                // set output value
                RGBPixel outputPixel;
                if (isCellBorder)
                {
                    if (isSelected)
                    {
                        outputPixel[0] = 0;
                        outputPixel[1] = 255;
                        outputPixel[2] = 0;
                    }
                    else
                    {
                        outputPixel[0] = 160;
                        outputPixel[1] = 0;
                        outputPixel[2] = 0;
                    }
                }
                else if (isSubregionBorder)
                {
                    if (isSelected)
                    {
                        outputPixel[0] = 0;
                        outputPixel[1] = 160;
                        outputPixel[2] = 0;
                    }
                    else
                    {
                        outputPixel[0] = intensity;
                        outputPixel[1] = intensity;
                        outputPixel[2] = intensity;
                    }
                }
                else
                {
                    outputPixel[0] = intensity;
                    outputPixel[1] = intensity;
                    outputPixel[2] = intensity;
                }
                outputImageIt.Set(outputPixel);
            }
            else // background
            {
                RGBPixel outputPixel;
                outputPixel[0] = intensity;
                outputPixel[1] = intensity;
                outputPixel[2] = intensity;
                outputImageIt.Set(outputPixel);
            }

            progress.CompletedPixel();
        }
    }
}

void AnalysisVisualizationImageFilter::GenerateInputRequestedRegion()
{
    // call the superclass' implementation of this method. this should
    // copy the output requested region to the input requested region
    Superclass::GenerateInputRequestedRegion();

    RGBAImage::Pointer input = const_cast<RGBAImage*>( this->GetInput() );

    if ( !input ) return;

    // get a copy of the input requested region (should equal the output
    // requested region)
    RGBAImage::RegionType inputRequestedRegion;
    inputRequestedRegion = input->GetRequestedRegion();

    // pad the input requested region by one
    inputRequestedRegion.PadByRadius(1);

    // crop the input requested region at the input's largest possible region
    inputRequestedRegion.Crop( input->GetLargestPossibleRegion() );

    input->SetRequestedRegion( inputRequestedRegion );
}

}
