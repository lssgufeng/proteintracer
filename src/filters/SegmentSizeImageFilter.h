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

#ifndef SegmentSizeImageFilter_h
#define SegmentSizeImageFilter_h

#include <itkImageToImageFilter.h>

#include <filters/SegmentKey.h>

namespace PT
{

/**
 * The SegmentSizeImageFilter sorts out segments, whose size is above or below
 * certain thresholds.  It takes a label image as input, where the pixel value
 * denotes the membership to an object.  Pixels of segments which are sorted
 * out are set to zero (denoting background).  All other segments are relabeled
 * so that their new label equals their rank in the list of all segments sorted
 * descendingly by size.
 */
template <class TInputImage>
class SegmentSizeImageFilter : public itk::ImageToImageFilter<TInputImage, TInputImage> 
{
public:
    typedef SegmentSizeImageFilter Self;
    typedef itk::ImageToImageFilter<TInputImage, TInputImage> Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;

    typedef typename TInputImage::PixelType InputPixelType;

    class LabelToSegmentKeyFunctor
    {
    public:
        SegmentKey operator()(InputPixelType label)
        {
            return SegmentKey(static_cast<int>(label), 0);
        }
    };

    itkNewMacro(Self);
    itkTypeMacro(SegmentSizeImageFilter, itk::ImageToImageFilter);

    unsigned long getNumberOfSegments() const
    {
        return numberOfSegments_;
    }

    const std::vector<unsigned long>& getSegmentSizesInPixels()
    {
        return segmentSizesInPixels_;
    }

    void setMaxNumberOfSegments(unsigned long maxNumberOfSegments)
    {
        if (maxNumberOfSegments != maxNumberOfSegments_)
        {
            maxNumberOfSegments_ = maxNumberOfSegments;
            this->Modified();
        }
    }

    void setMinimumSegmentSize(unsigned long minimumSegmentSize)
    {
        if (minimumSegmentSize != minimumSegmentSize_)
        {
            minimumSegmentSize_ = minimumSegmentSize;
            this->Modified();
        }
    }

    void setMaximumSegmentSize(unsigned long maximumSegmentSize)
    {
        if (maximumSegmentSize != maximumSegmentSize_)
        {
            maximumSegmentSize_ = maximumSegmentSize;
            this->Modified();
        }
    }

protected:

    SegmentSizeImageFilter() :
        maxNumberOfSegments_(itk::NumericTraits<unsigned long>::max() - 1),
        minimumSegmentSize_(0), 
        maximumSegmentSize_(0), 
        numberOfSegments_(0)
    {
    }

    struct Segment
    {
        InputPixelType label;
        unsigned long sizeInPixels;
    };

    class SegmentSizeComparator
    {
    public:
        bool operator()(const Segment& a, const Segment& b)
        {
            if (a.sizeInPixels > b.sizeInPixels)
            {
                return true;
            }
            else if (a.sizeInPixels < b.sizeInPixels)
            {
                return false;
            }
            // segments of equal size are sorted by original label
            else if (a.label < b.label)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
    };

    void GenerateData();

    /** 
     * The SegmentSizeImageFilter needs the entire input. Therefore it must
     * provide an implementation GenerateInputRequestedRegion().
     */
    void GenerateInputRequestedRegion();

private:
    unsigned long maxNumberOfSegments_;

    unsigned long minimumSegmentSize_;
    unsigned long maximumSegmentSize_;

    unsigned long numberOfSegments_;
    std::vector<unsigned long> segmentSizesInPixels_;

};

}

// include template implementation
#include "SegmentSizeImageFilter.txx"

#endif
