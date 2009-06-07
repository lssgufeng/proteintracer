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

#ifndef SegmentVisualizationImageFilter_txx
#define SegmentVisualizationImageFilter_txx

#include <itkImageRegionConstIterator.h>
#include <itkImageRegionIterator.h>
#include <itkProgressReporter.h>

namespace PT 
{

template <class TIntensityImage, class TLabelImage, class TLabelToSegmentKeyFunctor>
void SegmentVisualizationImageFilter<TIntensityImage, TLabelImage, TLabelToSegmentKeyFunctor>::ThreadedGenerateData(const RGBImage::RegionType& outputRegionForThread, int threadId)
{
    itk::ProgressReporter progress(this, threadId, outputRegionForThread.GetNumberOfPixels());

    const TIntensityImage* gradientImage = this->GetInput(0);
    const TLabelImage* labelImage = static_cast<const TLabelImage*>(this->itk::ProcessObject::GetInput(1));
    RGBImage::Pointer outputImage = this->GetOutput();

    assert(gradientImage->GetLargestPossibleRegion() == labelImage->GetLargestPossibleRegion());

    itk::ImageRegionConstIterator<TIntensityImage> sourceIt(gradientImage, outputRegionForThread);
    itk::ImageRegionConstIterator<TLabelImage> labelIt(labelImage, outputRegionForThread);
    itk::ImageRegionIterator<RGBImage> outputIt(outputImage, outputRegionForThread);

    typename TIntensityImage::PixelType maxIntensity = RescaleIntensityRGBImageFilter<TIntensityImage>::getMaxIntensity();
    typename TIntensityImage::PixelType minIntensity = RescaleIntensityRGBImageFilter<TIntensityImage>::getMinIntensity();
    float intensityRange = (float)(maxIntensity - minIntensity);

    for (; !outputIt.IsAtEnd(); ++sourceIt, ++labelIt, ++outputIt)
    {
        typename TLabelImage::PixelType label = labelIt.Get();
        SegmentKey segmentKey = labelToSegmentKeyFunctor_(label);

        RGBPixel rgbPixel;
        if (segmentKey.isBackground())
        {
            typename TIntensityImage::PixelType intensity = sourceIt.Get();
            unsigned char channelValue = (unsigned char)(((intensity - minIntensity) / intensityRange) * 255.0);
            rgbPixel[0] = channelValue;
            rgbPixel[1] = channelValue;
            rgbPixel[2] = channelValue;
        }
        else
        {
            rgbPixel = SegmentKeyToColorFunctor::INSTANCE(segmentKey);
        }

        outputIt.Set(rgbPixel);

        progress.CompletedPixel();
    }
}

}

#endif
