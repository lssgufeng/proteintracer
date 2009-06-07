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

#ifndef SegmentVisualizationImageFilter_h
#define SegmentVisualizationImageFilter_h

#include <filters/SegmentKeyToColorFunctor.h>
#include <filters/RescaleIntensityRGBImageFilter.h>

namespace PT
{

template <class TIntensityImage, class TLabelImage, class TLabelToSegmentKeyFunctor>
class SegmentVisualizationImageFilter : public RescaleIntensityRGBImageFilter<TIntensityImage>
{
public:
    typedef SegmentVisualizationImageFilter Self;
    typedef RescaleIntensityRGBImageFilter<TIntensityImage> Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;

    itkNewMacro(Self);

    itkTypeMacro(SegmentVisualizationImageFilter, RescaleIntensityRGBImageFilter);

    void SetSourceInput(const TIntensityImage* sourceImage) 
    {
        // Process object is not const-correct so the const casting is required.
        SetNthInput(0, const_cast<TIntensityImage*>(sourceImage));
    }

    void SetLabelInput(const TLabelImage* labelImage) 
    {
        // Process object is not const-correct so the const casting is required.
        SetNthInput(1, const_cast<TLabelImage*>(labelImage));
    }

protected:
    void ThreadedGenerateData(const RGBImage::RegionType& outputRegionForThread, int threadId);

private:
    TLabelToSegmentKeyFunctor labelToSegmentKeyFunctor_;
};

}

// include template implementation
#include "SegmentVisualizationImageFilter.txx"

#endif
