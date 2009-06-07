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

#ifndef AssayDialog_h
#define AssayDialog_h

#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Group.H>

#include <Analyzer.h>
#include <Scan.h>
#include <common.h>
#include <gui/ImageSelector.h>
#include <gui/ImageViewer.h>
#include <gui/ParameterTable.h>

class AssayDialog : public Fl_Group, public PT::EventHandler<ParameterTableEvent>, public PT::EventHandler<ImageSelectorEvent>, public PT::EventHandler<ImageViewerEvent>
{
public:
    AssayDialog(int x, int y, int width, int height);

    void setScan(const PT::Scan *scan);

    void setAssay(std::auto_ptr<PT::Assay> assay);
    const PT::Assay& getAssay();

    int handle(int event);
    void handleEvent(const ImageViewerEvent &event);
    void handleEvent(const ImageSelectorEvent &event);

private:

    Fl_Choice* analyzerChoice_;

    ParameterTable* parameterTable_;

    ImageSelector* imageSelector_;

    Fl_Choice* visualizationChoice_;

    Fl_Button* computeVisualizationButton_;

    ImageViewer* imageViewer_;

    Fl_Box *statusbarPixelLabelBox_;
    Fl_Box *statusbarPixelLocationBox_;
    Fl_Box *statusbarZoomBox_;

    const PT::Scan *scan_;

    std::auto_ptr<PT::Assay> assay_;

    std::auto_ptr<PT::Visualizer> visualizer_;

    std::auto_ptr<PT::Visualization> visualization_;

    void selectAnalyzer();

    void showStandardVisualization();

    void updateStatusbar();

    void computeVisualization();

};

#endif
