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

#ifndef images_h
#define images_h

#include <math.h>

#include <algorithm>
#include <memory>
#include <vector>

#include <itkImage.h>
#include <itkRGBPixel.h>
#include <itkRGBAPixel.h>

namespace PT
{

typedef itk::Size<2> ImageSize;

typedef itk::Offset<2> ImageOffset;

typedef itk::Index<2> ImageIndex;

typedef itk::ImageRegion<2> ImageRegion;

typedef itk::Image<unsigned char, 2> UCharImage;

typedef itk::Image<unsigned short, 2> UShortImage;

typedef itk::Image<unsigned int, 2> UIntImage;

typedef itk::Image<unsigned long, 2> ULongImage;

typedef itk::Image<float, 2> FloatImage;

typedef itk::RGBPixel<unsigned char> RGBPixel;

typedef itk::Image<RGBPixel, 2> RGBImage;

typedef itk::RGBAPixel<unsigned char> RGBAPixel;

typedef itk::Image<RGBAPixel, 2> RGBAImage;

}
#endif
