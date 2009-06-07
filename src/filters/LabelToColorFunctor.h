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

#ifndef LabelToColorFunctor_h
#define LabelToColorFunctor_h

#include <vector>

#include <images.h>

namespace PT
{

/**
 * The itk::ScalarToRGBPixelFunctor sometimes assigns very similar colors to
 * adjacent segments. That's why a custom implementation is provided here.  In
 * contrast to the itk::ScalarToRGBPixelFunctor which relies on the least
 * significant bit of label values to generate distinct colors, this
 * implementation uses a randomly generated color map.
 */
template <class TLabel>
class LabelToColorFunctor
{
private:
    static const int NUM_COLORS = 117;

public:
    LabelToColorFunctor();

    inline const RGBPixel& operator()(const TLabel& label)
    {
        return colors_[label % NUM_COLORS];
    }

    bool operator != (const LabelToColorFunctor& other) const
    { 
        return colors_ != other.colors_;
    }

    bool operator==(const LabelToColorFunctor& other ) const
    {
        return !(*this != other);
    }

private:

    RGBPixel black_;
    RGBPixel colors_[NUM_COLORS];

};

}

// include template implementation
#include "LabelToColorFunctor.txx"

#endif
