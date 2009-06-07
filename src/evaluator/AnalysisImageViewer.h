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

#ifndef AnalysisImageViewer_h
#define AnalysisImageViewer_h

#include <itkImageFileReader.h>

#include <Analysis.h>
#include <filters/AnalysisVisualizationImageFilter.h>
#include <gui/ImageViewer.h>

class AnalysisImageViewerEvent
{
public:
    enum Id {
        CELL_TOUCH,
        BACKGROUND_TOUCH,
        VOID_TOUCH,
		CELL_CLICK,
		BACKGROUND_CLICK,
    } id;

    short cellId;
    short subregionIndex;
};

class AnalysisImageViewer : public ImageViewer, public PT::EventGenerator<AnalysisImageViewerEvent>, PT::EventHandler<ImageViewerEvent>
{
public:

    AnalysisImageViewer(int x, int y, int width, int height);

    void setAnalysis(PT::Analysis* analysis);

    void setImageKey(const PT::ImageKey& imageKey);

    const PT::ImageKey& getImageKey() const
    {
        return imageKey_;
    }

    void setSelection(PT::CellSelection* selection);

    void setMarkCells(bool markCells);

    void handleEvent(const ImageViewerEvent &event);

private:

    void updateImage(bool retainFocus = false);

    itk::ImageFileReader<PT::RGBAImage>::Pointer fileReader_;

    PT::AnalysisVisualizationImageFilter::Pointer analysisVisualizationFilter_;

    PT::RGBAImage::ConstPointer analysisImage_;

    PT::Analysis* analysis_;

    PT::ImageKey imageKey_;

    PT::CellSelection* selection_;

};

#endif
