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

#ifndef RescaleIntensityRGBImageFilter_h
#define RescaleIntensityRGBImageFilter_h

#include <math.h>

#include <itkImageToImageFilter.h>

#include <images.h>

namespace PT
{

template <class TInputImage>
class RescaleIntensityRGBImageFilter : public itk::ImageToImageFilter<TInputImage, RGBImage>
{
public:
    typedef RescaleIntensityRGBImageFilter Self;
    typedef itk::ImageToImageFilter<TInputImage, RGBImage> Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;

    itkNewMacro(Self);

    itkTypeMacro(RescaleIntensityRGBImageFilter, itk::ImageToImageFilter);

    typename TInputImage::PixelType getMinIntensity() { return minIntensity_; }

    typename TInputImage::PixelType getMaxIntensity() { return maxIntensity_; }

protected:

    RescaleIntensityRGBImageFilter() :
        minIntensity_( itk::NumericTraits< typename TInputImage::PixelType >::max() ),
        maxIntensity_( itk::NumericTraits< typename TInputImage::PixelType >::min() )
    {
    }

    virtual ~RescaleIntensityRGBImageFilter() {}

    void BeforeThreadedGenerateData();

    void ThreadedGenerateData(const RGBImage::RegionType& outputRegionForThread, int threadId);  

    typename TInputImage::PixelType minIntensity_;

    typename TInputImage::PixelType maxIntensity_;

};

}

// include template implementation
#include "RescaleIntensityRGBImageFilter.txx"

#endif
