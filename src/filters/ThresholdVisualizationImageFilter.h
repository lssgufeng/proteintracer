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

#ifndef ThresholdVisualizationImageFilter_h
#define ThresholdVisualizationImageFilter_h

#include <filters/RescaleIntensityRGBImageFilter.h>

namespace PT 
{

template <class TInputImage>
class ThresholdVisualizationImageFilter : public RescaleIntensityRGBImageFilter<TInputImage>
{
    public:
        typedef ThresholdVisualizationImageFilter Self;
        typedef RescaleIntensityRGBImageFilter<TInputImage> Superclass;
        typedef itk::SmartPointer<Self> Pointer;
        typedef itk::SmartPointer<const Self> ConstPointer;

        itkNewMacro(Self);

        itkTypeMacro(ThresholdVisualizationImageFilter, RescaleIntensityRGBImageFilter);

        void setThreshold(double threshold)
        {
            threshold_ = threshold;
            this->Modified();
        }

    protected:

        ThresholdVisualizationImageFilter() : threshold_(0) {}

        void ThreadedGenerateData(const RGBImage::RegionType& outputRegionForThread, int threadId);  

        double threshold_;
};

}

// include template implementation
#include "ThresholdVisualizationImageFilter.txx"

#endif
