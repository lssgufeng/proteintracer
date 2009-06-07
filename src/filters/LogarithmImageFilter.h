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

#ifndef LogarithmImageFilter_h
#define LogarithmImageFilter_h

#include <math.h>

#include <itkUnaryFunctorImageFilter.h>

namespace PT
{

template< typename TInput, typename  TOutput>
class LogarithmImageFunctor
{
public:
    bool operator!=(const LogarithmImageFunctor & other) const
    {
        return false;

    }

    bool operator==(const LogarithmImageFunctor & other) const
    {
        return true;
    }

    inline TOutput operator()(const TInput & inputValue)
    {
        double intensity  = static_cast<double>(inputValue);
        intensity += 1;
        if (intensity > 0)
            intensity = ::log10(intensity);
        return static_cast<TOutput>(intensity);
    }
}; 

template <typename  TInputImage, typename TOutputImage>
class LogarithmImageFilter :
    public itk::UnaryFunctorImageFilter<
        TInputImage,
        TOutputImage, 
        LogarithmImageFunctor<
             typename TInputImage::PixelType, 
             typename TOutputImage::PixelType> >
{
public:

    typedef LogarithmImageFilter Self;
    typedef itk::UnaryFunctorImageFilter<
        TInputImage,
        TOutputImage, 
        LogarithmImageFunctor<
            typename TInputImage::PixelType, 
            typename TOutputImage::PixelType> >  Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;

    itkNewMacro(Self);

};

} 
  
#endif
