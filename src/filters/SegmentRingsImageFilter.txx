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

#ifndef SegmentRingsImageFilter_txx
#define SegmentRingsImageFilter_txx

#include <math.h>

#include <itkConstShapedNeighborhoodIterator.h>
#include <itkImageRegionIterator.h>
#include <itkNeighborhoodAlgorithm.h>
#include <itkProgressReporter.h>

namespace PT
{

template <class TInputImage, class TLabelToSegmentKeyFunctor>
void SegmentRingsImageFilter<TInputImage, TLabelToSegmentKeyFunctor>::ThreadedGenerateData(
        const typename TInputImage::RegionType& outputRegionForThread,
        int threadId)  
{
    typename TInputImage::ConstPointer input = this->GetInput();
    typename TInputImage::Pointer output = this->GetOutput();

    itk::ProgressReporter progress(this, 0, outputRegionForThread.GetNumberOfPixels());

    typedef itk::ConstShapedNeighborhoodIterator<TInputImage> ShapedNeighborhoodIteratorType;

    // init radii
    const int numRings = 2;
    int radii[] = {ringWidth1_, ringWidth2_};
    for (int ri = 1; ri < numRings; ++ri)
    {
        radii[ri] += radii[ri-1];
    }

    typename ShapedNeighborhoodIteratorType::RadiusType iteratorRadius;
    iteratorRadius.Fill(radii[numRings-1]);

    typedef typename itk::NeighborhoodAlgorithm::ImageBoundaryFacesCalculator<TInputImage> FaceCalculatorType;
    FaceCalculatorType faceCalculator;
    typename FaceCalculatorType::FaceListType faceList;
    faceList = faceCalculator(input, outputRegionForThread, iteratorRadius);
    typename FaceCalculatorType::FaceListType::iterator fit;
    for (fit = faceList.begin(); fit != faceList.end(); ++fit)
    {
        // initialize neighborhood iterators
        std::auto_ptr<ShapedNeighborhoodIteratorType> inputIts[numRings];
        {
            for (int ri = 0; ri < numRings; ++ri)
            {
                int currentRadius = radii[ri];
                int previousRadius = (ri > 0) ? radii[ri - 1] : 0;

                iteratorRadius.Fill(currentRadius);
                inputIts[ri] = std::auto_ptr<ShapedNeighborhoodIteratorType>(
                    new ShapedNeighborhoodIteratorType(iteratorRadius, input, *fit));

                for (int y = -currentRadius; y <= currentRadius; y++)
                {
                    for (int x = -currentRadius; x <= currentRadius; x++)
                    {     
                        float dist = ::sqrt((float)(x*x + y*y));
                        if (dist <= currentRadius && dist > previousRadius)
                        {
                            typename ShapedNeighborhoodIteratorType::OffsetType off;
                            off[0] = x;
                            off[1] = y;
                            inputIts[ri]->ActivateOffset(off);
                        }
                    }
                }

                inputIts[ri]->GoToBegin();
            }
        }

        typename ShapedNeighborhoodIteratorType::ConstIterator nhIt;
        typename itk::ImageRegionIterator<TInputImage> outputIt(output, *fit);
        for (outputIt.GoToBegin(); !outputIt.IsAtEnd(); ++outputIt)
        {
            InputPixelType inputPixel = inputIts[0]->GetCenterPixel();

            SegmentKey segmentKey = labelToSegmentKeyFunctor_(inputPixel);

            // check whether input pixel is not background
            if ( ! segmentKey.isBackground() )
            {
                int ringIndex = numRings;
                for (int ri = 0; ri < numRings; ++ri)
                {
                    for (nhIt = inputIts[ri]->Begin(); nhIt != inputIts[ri]->End(); ++nhIt)
                    {
                        int tempSegmentId = labelToSegmentKeyFunctor_(nhIt.Get()).mainId;
                        if (tempSegmentId != segmentKey.mainId)
                        {
                            ringIndex = ri;
                            break;
                        }
                    }

                    if (ringIndex < numRings) break;
                }
                
                outputIt.Set( (segmentKey.mainId << 8) | (0xff & ringIndex) ) ;
            }
            else
            {
                outputIt.Set(0);
            }

            // increment input iterators
            for (int ri = 0; ri < numRings; ++ri)
            {
                ++(*inputIts[ri]);
            }

            progress.CompletedPixel();
        }
    }
}

template <class TInputImage, class TLabelToSegmentKeyFunctor>
void SegmentRingsImageFilter<TInputImage, TLabelToSegmentKeyFunctor>::GenerateInputRequestedRegion()
{
    // call the superclass' implementation of this method. this should
    // copy the output requested region to the input requested region
    Superclass::GenerateInputRequestedRegion();

    typename TInputImage::Pointer input = const_cast< TInputImage* >( this->GetInput() );

    if ( !input ) return;

    // get a copy of the input requested region (should equal the output
    // requested region)
    typename TInputImage::RegionType inputRequestedRegion;
    inputRequestedRegion = input->GetRequestedRegion();

    // pad the input requested region by the radius of the maximum structuring
    // element
    int maxRadius = 0;
    {
        const int numRings = 2;
        int radii[] = {ringWidth1_, ringWidth2_};
        for (int ri = 0; ri < numRings; ++ri)
        {
            maxRadius += radii[ri];
        }
    }
    inputRequestedRegion.PadByRadius(maxRadius);

    // crop the input requested region at the input's largest possible region
    inputRequestedRegion.Crop( input->GetLargestPossibleRegion() );

    input->SetRequestedRegion( inputRequestedRegion );
}

}

#endif
