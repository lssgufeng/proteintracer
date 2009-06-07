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

#ifndef AnalysisVisualizationImageFilter_h
#define AnalysisVisualizationImageFilter_h

#include <set>

#include <itkImageToImageFilter.h>
#include <itk_hash_map.h>

#include <Analysis.h>
#include <filters/SegmentKeyToColorFunctor.h>
#include <images.h>

namespace PT
{

class AnalysisVisualizationImageFilter : public itk::ImageToImageFilter<RGBAImage, RGBImage>
{
public:
    typedef AnalysisVisualizationImageFilter Self;
    typedef itk::ImageToImageFilter<RGBAImage, RGBImage> Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;

    itkNewMacro(Self);

    itkTypeMacro(AnalysisVisualizationImageFilter, itk::ImageToImageFilter);

    void setSelection(CellSelection* cellSelection)
    {
        this->cellSelection_ = cellSelection;
        this->Modified();
    }

    void setMarkCells(bool markCells)
    {
        if (markCells != this->markCells_)
        {
            this->markCells_ = markCells;
            this->Modified();
        }
    }

protected:

    AnalysisVisualizationImageFilter();

    void ThreadedGenerateData(const RGBImage::RegionType& outputRegionForThread, int threadId);

    /**
     * AnalysisVisualizationImageFilter needs the output requested region
     * padded by one. So GenerateInputRequestedRegion has to be overwritten.
     */
    void GenerateInputRequestedRegion();

    CellSelection* cellSelection_;

    bool markCells_;
};

}

#endif
