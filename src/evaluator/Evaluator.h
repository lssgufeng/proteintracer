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

#ifndef Evaluator_h
#define Evaluator_h

#include <FL/Fl_Box.H>
#include <FL/Fl_Double_Window.H>

#include <evaluator/AnalysisImageViewer.h>
#include <evaluator/ChartViewer.h>
#include <gui/ImageSelector.h>

#include <Analysis.h>

class Evaluator : public Fl_Double_Window,
    public PT::EventHandler<AnalysisImageViewerEvent>,
    public PT::EventHandler<ImageSelectorEvent>,
    public PT::EventHandler<ChartViewerEvent>
{
public:
    Evaluator();
    ~Evaluator() {}

    void handleEvent(const AnalysisImageViewerEvent &event);

    void handleEvent(const ChartViewerEvent& event);

    void handleEvent(const ImageSelectorEvent &event);

private:

    void openAnalysis();

    void exportSelection();

    void clearCellSelection();

    void editChartParameters();

    void invertCellSelection();

    void selectAllCells();

    void showFirstImage();

    void showNextImage();

    void showPreviousImage();

    void showLastImage();

    void toggleMarkCells(Fl_Widget* widget);

    void showStatusBarMessage(const std::string& message);

    void handleCellSelectionChange(
            bool updateImageViewer = true, 
            bool updateChartViewer = true);

    void exit();

    void showAboutDialog();

    AnalysisImageViewer* imageViewer_;

    ImageSelector* imageSelector_;

    ChartViewer* chartViewer_;

    Fl_Box* statusBarMessageBox_;

    Fl_Box* statusBarSelectionBox_;

    std::auto_ptr<ChartData> chartData_;

};
#endif
