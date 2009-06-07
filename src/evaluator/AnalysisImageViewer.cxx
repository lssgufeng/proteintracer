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

#include <evaluator/AnalysisImageViewer.h>

AnalysisImageViewer::AnalysisImageViewer(int x, int y, int width, int height) :
    ImageViewer(x, y, width, height),
    analysis_(0),
    selection_(0)
{
    ImageViewer::setEventHandler(this);

    fileReader_ = itk::ImageFileReader<PT::RGBAImage>::New();

    analysisVisualizationFilter_ = PT::AnalysisVisualizationImageFilter::New();
    analysisVisualizationFilter_->SetInput(fileReader_->GetOutput());
}

void AnalysisImageViewer::setAnalysis(PT::Analysis* analysis)
{
    this->analysis_ = analysis;
    this->selection_ = 0;
    this->imageKey_.invalidate();

    updateImage();
}

void AnalysisImageViewer::setImageKey(const PT::ImageKey& imageKey)
{
    bool retainFocus = (this->imageKey_.location == imageKey.location);
    this->imageKey_ = imageKey;
    updateImage(retainFocus);
}

void AnalysisImageViewer::setSelection(PT::CellSelection* selection)
{
    this->selection_ = selection;
    updateImage(true);
}

void AnalysisImageViewer::setMarkCells(bool markCells)
{
    analysisVisualizationFilter_->setMarkCells(markCells);
    updateImage(true);
}

void AnalysisImageViewer::handleEvent(const ImageViewerEvent &event)
{
    if (event.id == ImageViewerEvent::MOUSE_MOVE)
    {
        PT::ImageIndex loc = event.location;
        if (analysisImage_.IsNotNull() && loc[0] >= 0 && loc[1] >= 0)
        {
            PT::RGBAImage::PixelType pixel = analysisImage_->GetPixel(loc);
            int cellId = PT::Analysis::decodeCellId(pixel);
            int subregionIndex = PT::Analysis::decodeSubregionIndex(pixel);

            if (cellId > 0) // not background
            {
                AnalysisImageViewerEvent event;
                event.id = AnalysisImageViewerEvent::CELL_TOUCH;
                event.cellId = cellId;
                event.subregionIndex = subregionIndex;
                PT::EventGenerator<AnalysisImageViewerEvent>::notifyEventHandler(event);
            }
            else
            {
                AnalysisImageViewerEvent event;
                event.id = AnalysisImageViewerEvent::BACKGROUND_TOUCH;
                PT::EventGenerator<AnalysisImageViewerEvent>::notifyEventHandler(event);
            }
        }
        else
        {
            AnalysisImageViewerEvent event;
            event.id = AnalysisImageViewerEvent::VOID_TOUCH;
            PT::EventGenerator<AnalysisImageViewerEvent>::notifyEventHandler(event);
        }
    }
    else if (event.id == ImageViewerEvent::MOUSE_CLICK)
    {
        PT::ImageIndex loc = event.location;
        if (analysisImage_.IsNotNull() && loc[0] >= 0 && loc[1] >= 0)
        {
            PT::RGBAImage::PixelType pixel = analysisImage_->GetPixel(loc);
            int cellId = pixel[0] | (pixel[1] << 8);
            int subregionIndex = pixel[2];

            if (cellId > 0)
            {
                AnalysisImageViewerEvent event;
                event.id = AnalysisImageViewerEvent::CELL_CLICK;
                event.cellId = cellId;
                event.subregionIndex = subregionIndex;
                PT::EventGenerator<AnalysisImageViewerEvent>::notifyEventHandler(event);
            }
            else
            {
                AnalysisImageViewerEvent event;
                event.id = AnalysisImageViewerEvent::BACKGROUND_CLICK;
                PT::EventGenerator<AnalysisImageViewerEvent>::notifyEventHandler(event);
            }
        }
    }
}

void AnalysisImageViewer::updateImage(bool retainFocus)
{
    if (analysis_ != 0 && imageKey_.isValid())
    {
        std::string filepath = analysis_->getMetadata().getFilePath(this->imageKey_);
        fileReader_->SetFileName(filepath.c_str());

        analysisVisualizationFilter_->setSelection(this->selection_);

        // Update the largest possible region, because a call to Update() would
        // try to update the previously requested region, no matter whether the
        // largest possible region has changed (for example after changing the
        // analysis).  See documentation of itk::ProcessObject::Update() .
        analysisVisualizationFilter_->UpdateLargestPossibleRegion();

        this->analysisImage_ = fileReader_->GetOutput();
        PT::RGBImage::ConstPointer visualizationImage = analysisVisualizationFilter_->GetOutput();

        if (retainFocus)
            setImageAndRetainFocus(visualizationImage);
        else
            setImage(visualizationImage);
    }
    else
    {
        this->analysisImage_ = 0;
        setImage(0);
    }
}
