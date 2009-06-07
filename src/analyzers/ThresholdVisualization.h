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

#ifndef ThresholdVisualization_h
#define ThresholdVisualization_h

#include <sstream>

namespace PT 
{

template <class TIntensityImage>
class ThresholdVisualization : public Visualization
{
public:

    ThresholdVisualization(typename TIntensityImage::ConstPointer intensityImage,
            RGBImage::ConstPointer visualizationImage,
            const ImageKey& imageKey,
            typename TIntensityImage::PixelType maxIntensityValue) :
        Visualization(visualizationImage, imageKey),
        intensityImage_(intensityImage),
        maxIntensityValue_(maxIntensityValue)
    {
        assert(intensityImage->GetLargestPossibleRegion() == visualizationImage->GetLargestPossibleRegion());
    }

    std::string getPixelLabel(int x, int y)
    {
        typename TIntensityImage::RegionType imageRegion = intensityImage_->GetLargestPossibleRegion();

        typename TIntensityImage::IndexType index;
        index[0] = x;
        index[1] = y;

        if (imageRegion.IsInside(index))
        {
            typename TIntensityImage::PixelType relativeIntensity = intensityImage_->GetPixel(index) / maxIntensityValue_;

            std::stringstream text;
            text << "Relative Intensity: " << relativeIntensity;
            return text.str();
        }
        else
        {
            return "";
        }
    }

private:
    typename TIntensityImage::ConstPointer intensityImage_;
    typename TIntensityImage::PixelType maxIntensityValue_;

};

}

#endif
