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

#ifndef ThresholdVisualizationImageFilter_txx
#define ThresholdVisualizationImageFilter_txx

#include <sstream>

#include <itkImageRegionConstIterator.h>
#include <itkImageRegionIterator.h>
#include <itkProgressReporter.h>

namespace PT 
{

template <class TInputImage>
void ThresholdVisualizationImageFilter<TInputImage>::ThreadedGenerateData(const RGBImage::RegionType& outputRegionForThread, int threadId)
{
    itk::ProgressReporter progress(this, threadId, outputRegionForThread.GetNumberOfPixels());

    typename TInputImage::ConstPointer inputPtr = this->GetInput();
    RGBImage::Pointer outputPtr = this->GetOutput();

    typename itk::ImageRegionConstIterator<TInputImage> inIt(inputPtr, outputRegionForThread);
    itk::ImageRegionIterator<RGBImage> outIt(outputPtr, outputRegionForThread);

    typename TInputImage::PixelType maxIntensity = RescaleIntensityRGBImageFilter<TInputImage>::getMaxIntensity();
    typename TInputImage::PixelType minIntensity = RescaleIntensityRGBImageFilter<TInputImage>::getMinIntensity();
    float intensityRange = (float)(maxIntensity - minIntensity);

    static const RGBPixel::ValueType maxRGBChannelValue = itk::NumericTraits<RGBPixel::ValueType>::max();
    typename TInputImage::PixelType threshold = threshold_ * maxIntensity;
    RGBPixel rgbPixel;
    for (; !outIt.IsAtEnd(); ++inIt, ++outIt)
    {
        typename TInputImage::PixelType intensity = inIt.Get();

        if (intensity < threshold)
        {
            rgbPixel[0] = 192;
            rgbPixel[1] = 0;
            rgbPixel[2] = 0;
        }
        else
        {
            float relIntensity = ((intensity - minIntensity) / intensityRange);
            RGBPixel::ValueType channelValue = (RGBPixel::ValueType)(relIntensity * maxRGBChannelValue);

            rgbPixel[0] = channelValue;
            rgbPixel[1] = channelValue;
            rgbPixel[2] = channelValue;
        }

        outIt.Set(rgbPixel);

        progress.CompletedPixel();
    }
}

}
#endif
