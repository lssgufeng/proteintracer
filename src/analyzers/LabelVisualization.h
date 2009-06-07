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

#ifndef LabelVisualization_h
#define LabelVisualization_h

#include <sstream>

namespace PT 
{

template <class TLabelImage>
class LabelVisualization : public Visualization
{
public:

    LabelVisualization(
        typename TLabelImage::ConstPointer labelImage, 
        RGBImage::ConstPointer visualizationImage, 
        const ImageKey& imageKey) :
            Visualization(visualizationImage, imageKey),
            labelImage_(labelImage)
    {
        assert(labelImage->GetLargestPossibleRegion() == visualizationImage->GetLargestPossibleRegion());
    }

    std::string getPixelLabel(int x, int y)
    {
        ULongImage::IndexType index;
        index[0] = x;
        index[1] = y;

        typename TLabelImage::RegionType imageRegion = labelImage_->GetLargestPossibleRegion();

        if (imageRegion.IsInside(index))
        {
            std::stringstream text;

            typename TLabelImage::PixelType label = labelImage_->GetPixel(index);
            text << "Label: " << label;

            return text.str();
        }
        else
        {
            return "";
        }
    }

private:
    typename TLabelImage::ConstPointer labelImage_;

};

}

#endif
