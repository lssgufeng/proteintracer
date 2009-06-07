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

#ifndef SegmentRingsImageFilter_h
#define SegmentRingsImageFilter_h

#include <itkImageToImageFilter.h>

#include <filters/SegmentKey.h>

namespace PT
{

/**
 * The SegmentRingsImageFilter divides segments onion-like in rings.  This
 * filter expects a label image, where the membership to an object is denoted
 * by a common pixel value.  New labels are determined as follows.  The
 * original object label is shifted leftwards by 8 bit and the ring number is
 * stored in the  lowest eight bit.  Ring numbers are counted from the outside
 * to the inside, with the outermost ring having number one.  Pixels of the
 * background, i. e. the value zero, are left unchanged.
 */
template <class TInputImage, class TLabelToSegmentKeyFunctor>
class SegmentRingsImageFilter : public itk::ImageToImageFilter<TInputImage, TInputImage>
{
public:
    typedef SegmentRingsImageFilter Self;
    typedef itk::ImageToImageFilter<TInputImage, TInputImage> Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;

    typedef typename TInputImage::PixelType InputPixelType;

    class LabelToSegmentKeyFunctor
    {
    public:
        SegmentKey operator()(InputPixelType label)
        {
            return SegmentKey( label >> 8 , 0xff & label );
        }
    };

    itkNewMacro(Self);
    itkTypeMacro(SegmentRingsImageFilter, itk::ImageToImageFilter);

    void setRingWidth1(int ringWidth1)
    {
        assert(ringWidth1 <= 254);

        if (ringWidth1 != ringWidth1_)
        {
            ringWidth1_ = ringWidth1;
            this->Modified();
        }
    }

    void setRingWidth2(int ringWidth2)
    {
        assert(ringWidth2 <= 254);

        if (ringWidth2 != ringWidth2_)
        {
            ringWidth2_ = ringWidth2;
            this->Modified();
        }
    }

protected:

    SegmentRingsImageFilter() :
        ringWidth1_(0),
        ringWidth2_(0)
    {
    }

    void ThreadedGenerateData(const typename TInputImage::RegionType& outputRegionForThread, int threadId);  

    /**
     * SegmentRingsImageFilter needs the output requested region padded by the
     * maximum radius of the structuring element. So GenerateInputRequestedRegion
     * has to be overwritten.
     */
    void GenerateInputRequestedRegion();

private:

    TLabelToSegmentKeyFunctor labelToSegmentKeyFunctor_;

    int ringWidth1_;

    int ringWidth2_;

};

}

// include template implementation
#include "SegmentRingsImageFilter.txx"

#endif
