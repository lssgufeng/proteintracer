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

#include <evaluator/Evaluator.h>

#include <sstream>

#include <FL/Fl.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Tile.H>

#include <evaluator/ChartParametersEditor.h>
#include <gui/AboutDialog.h>
#include <gui/common.h>
#include <io/AnalysisIO.h>
#include <io/export.h>

static const int WINDOW_WIDTH = 900;
static const int WINDOW_HEIGHT = 600;

Evaluator::Evaluator() : Fl_Double_Window(WINDOW_WIDTH, WINDOW_HEIGHT, "Protein Tracer:  Evaluator")
{
    // create user interface
    {
        static const int BUTTON_HEIGHT = 25;
        static const int BUTTON_WIDTH = 100;
        static const int IMAGE_SELECTOR_WIDTH = 240;
        static const int IMAGE_VIEWER_WIDTH = WINDOW_WIDTH / 2;
        static const int INPUT_HEIGHT = 25;
        static const int LABEL_HEIGHT = 14;
        static const int MEDIUM_MARGIN = 10;
        static const int MENU_BAR_HEIGHT = 25;
        static const int SETTINGS_GROUP_HEIGHT = 59;
        static const int SMALL_MARGIN = 5;
        static const int STATUSBAR_HEIGHT = 26;
        static const int STATUSBAR_SELECTION_BOX_WIDTH = 200;

        // called when the window is closed
        this->callback(fltk_member_cb<Evaluator, &Evaluator::exit>, this);

        // create menu bar
        Fl_Menu_Bar *menuBar = new Fl_Menu_Bar(
                0, 
                0, 
                this->w(), 
                MENU_BAR_HEIGHT);
        {
            menuBar->box(FL_FLAT_BOX);
            
            Fl_Menu_Item menuItems[] = 
            {
                { "&File", 0, 0, 0, FL_SUBMENU },
                { "&Open Analysis...", FL_CTRL + 'o', fltk_member_cb<Evaluator, &Evaluator::openAnalysis>, 0, FL_MENU_DIVIDER },
                { "&Export Selection...", 0, fltk_member_cb<Evaluator, &Evaluator::exportSelection>, 0, FL_MENU_DIVIDER },
                { "E&xit", FL_CTRL + 'q', fltk_member_cb<Evaluator, &Evaluator::exit>, 0 },
                { 0 },
                { "&Edit", 0, 0, 0, FL_SUBMENU },
                { "Select &All Cells", FL_CTRL + 'a', fltk_member_cb<Evaluator, &Evaluator::selectAllCells>, 0, 0 },
                { "&Clear Cell Selection", FL_CTRL + 'd', fltk_member_cb<Evaluator, &Evaluator::clearCellSelection>, 0, 0 },
                { "In&vert Cell Selection", FL_CTRL + 'i', fltk_member_cb<Evaluator, &Evaluator::invertCellSelection>, 0, 0 },
                { 0 },
                { "&View", 0, 0, 0, FL_SUBMENU },
                { "Show &First Image", FL_Home, fltk_member_cb<Evaluator, &Evaluator::showFirstImage>, 0, 0 },
                { "Show &Next Image", FL_Page_Down, fltk_member_cb<Evaluator, &Evaluator::showNextImage>, 0, 0 },
                { "Show &Previous Image", FL_Page_Up, fltk_member_cb<Evaluator, &Evaluator::showPreviousImage>, 0, 0 },
                { "Show &Last Image", FL_End, fltk_member_cb<Evaluator, &Evaluator::showLastImage>, 0, FL_MENU_DIVIDER },
                { "&Mark Cells",  FL_CTRL + 'm', fltk_member_cb_widget<Evaluator, &Evaluator::toggleMarkCells>, 0, FL_MENU_TOGGLE | FL_MENU_VALUE},
                { 0 },
                { "&Help", 0, 0, 0, FL_SUBMENU },
                { "&About",  0, fltk_member_cb<Evaluator, &Evaluator::showAboutDialog>, 0, 0 },
                { 0 },
                { 0 }
            };
            menuBar->copy(menuItems, this);
        }

        // create main group
        Fl_Tile* mainGroup = new Fl_Tile(
                menuBar->x(), 
                menuBar->y() + menuBar->h(), 
                menuBar->w(), 
                this->h() - menuBar->h() - SETTINGS_GROUP_HEIGHT - STATUSBAR_HEIGHT);
        {
            mainGroup->box(FL_FLAT_BOX);
            this->resizable(mainGroup);

            // create image viewer
            Fl_Group* imageViewerGroup = new Fl_Group(
                    mainGroup->x(), 
                    mainGroup->y(),
                    IMAGE_VIEWER_WIDTH,
                    mainGroup->h());
            imageViewerGroup->box(FL_DOWN_BOX);

            imageViewer_ = new AnalysisImageViewer(
                    imageViewerGroup->x() + 2,
                    imageViewerGroup->y() + 2,
                    imageViewerGroup->w() - 4,
                    imageViewerGroup->h() - 4);
            imageViewer_->PT::EventGenerator<AnalysisImageViewerEvent>::setEventHandler(this);
            imageViewerGroup->resizable(imageViewer_);

            imageViewerGroup->end();

            Fl_Group* chartGroup = new Fl_Group(
                    imageViewerGroup->x() + imageViewerGroup->w(), 
                    mainGroup->y(),
                    mainGroup->w() - IMAGE_VIEWER_WIDTH,
                    imageViewerGroup->h());
            chartGroup->box(FL_DOWN_BOX);

            chartViewer_ = new ChartViewer(
                    chartGroup->x() + 2,
                    chartGroup->y() + 2,
                    chartGroup->w() - 4,
                    chartGroup->h() - 4);
            chartViewer_->setEventHandler(this);
            chartGroup->resizable(chartViewer_);

            mainGroup->end();
        }

        // create settings group
        Fl_Group* settingsGroup = new Fl_Group(
                mainGroup->x(), 
                mainGroup->y() + mainGroup->h(), 
                mainGroup->w(), 
                SETTINGS_GROUP_HEIGHT);
        {
            settingsGroup->box(FL_FLAT_BOX);

            imageSelector_ = new ImageSelector(
                    settingsGroup->x() + MEDIUM_MARGIN,
                    settingsGroup->y() + MEDIUM_MARGIN,
                    IMAGE_SELECTOR_WIDTH,
                    LABEL_HEIGHT + INPUT_HEIGHT);
            imageSelector_->setEventHandler(this);

            Fl_Button* editChartParametersButton = new Fl_Button(
                    settingsGroup->x() + settingsGroup->w() - 2 * BUTTON_WIDTH - MEDIUM_MARGIN,
                    settingsGroup->y() + ((settingsGroup->h() - BUTTON_HEIGHT) / 2),
                    2 * BUTTON_WIDTH,
                    BUTTON_HEIGHT);
            editChartParametersButton->label("Edit Chart &Parameters");
            editChartParametersButton->callback(fltk_member_cb<Evaluator, &Evaluator::editChartParameters>, this);

            settingsGroup->end();
        }

        // create status bar
        Fl_Group* statusBar = new Fl_Group(
                settingsGroup->x() - 2, 
                settingsGroup->y() + settingsGroup->h(),
                settingsGroup->w() + 4,
                STATUSBAR_HEIGHT + 2);
        {
            statusBar->box(FL_ENGRAVED_BOX);

            statusBarMessageBox_ = new Fl_Box(
                    statusBar->x() + SMALL_MARGIN,
                    statusBar->y() + (statusBar->h() - LABEL_HEIGHT) / 2,
                    statusBar->w() - 2 * SMALL_MARGIN - STATUSBAR_SELECTION_BOX_WIDTH,
                    LABEL_HEIGHT);
            statusBarMessageBox_->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
            statusBar->resizable(statusBarMessageBox_);

            statusBarSelectionBox_ = new Fl_Box(
                    statusBarMessageBox_->x() + statusBarMessageBox_->w(),
                    statusBarMessageBox_->y(),
                    STATUSBAR_SELECTION_BOX_WIDTH,
                    LABEL_HEIGHT);
            statusBarSelectionBox_->align(FL_ALIGN_RIGHT | FL_ALIGN_INSIDE);
            statusBar->resizable(statusBarSelectionBox_);

            statusBar->end();
        }

        this->end();
    }
}

void Evaluator::handleEvent(const AnalysisImageViewerEvent &event)
{
    if (chartData_.get() == 0) return;

    if (event.id == AnalysisImageViewerEvent::CELL_TOUCH) 
    {
        int cellId = event.cellId;
        int subregionIndex = event.subregionIndex;
        std::string subregionName =
            chartData_->analysis->getMetadata().subregionNames[subregionIndex];

        std::stringstream message;
        message << "Cell " << cellId << ", " << subregionName;
        showStatusBarMessage(message.str().c_str());
    }
    else if (event.id == AnalysisImageViewerEvent::BACKGROUND_TOUCH)
    {
        showStatusBarMessage("Background");
    }
    else if (event.id == AnalysisImageViewerEvent::VOID_TOUCH)
    {
        showStatusBarMessage("");
    }
    else if (event.id == AnalysisImageViewerEvent::CELL_CLICK) 
    {
        // add cell if it is not already contained in the set
        if (! chartData_->cellSelection->containsCell(event.cellId))
        {
            PT::Cell* cell = chartData_->analysis->getCell(event.cellId);
            assert(cell != 0);
            chartData_->cellSelection->addCell(cell);
        }
        // remove cell otherwise
        else
        {
            chartData_->cellSelection->removeCell(event.cellId);
        }

        handleCellSelectionChange();
    }
}

void Evaluator::handleEvent(const ChartViewerEvent& event)
{
    switch (event.id)
    {
    case ChartViewerEvent::STATUS_MESSAGE:
        showStatusBarMessage(event.statusMessage);
        break;
    case ChartViewerEvent::CELL_SELECTION:
        handleCellSelectionChange(true, false);
        break;
    }
}

void Evaluator::handleEvent(const ImageSelectorEvent &event)
{
    if (chartData_.get() != 0)
    {
        imageViewer_->setImageKey(event.key);
        chartData_->time = event.key.time;
        chartViewer_->setData(chartData_.get());
    }
}

void Evaluator::openAnalysis()
{
    // show file chooser
    const char* filepath = fl_file_chooser("Open Analysis",
            "XML Files (*.xml)", ".", 0);

    if (filepath != NULL) 
    {
        try
        {
            std::auto_ptr<PT::Analysis> analysis = PT::loadAnalysis(filepath);
            std::auto_ptr<PT::CellSelection> cellSelection =
                std::auto_ptr<PT::CellSelection>(new PT::CellSelection());
            std::auto_ptr<ChartParameters> parameters =
                ChartParameters::createParameters(CHART_TYPE_SCATTERPLOT, analysis.get());

            imageSelector_->setImageSeriesSet(analysis.get());

            imageViewer_->setAnalysis(analysis.get());
            imageViewer_->setImageKey(imageSelector_->getSelection());

            chartData_ = std::auto_ptr<ChartData>( new ChartData() ); 
            chartData_->analysis = analysis;
            chartData_->cellSelection = cellSelection;
            chartData_->time = imageSelector_->getSelection().time;
            chartData_->parameters = parameters;

            handleCellSelectionChange();
        } 
        catch (PT::IOException ex)
        {
            std::stringstream messageStream;
            messageStream << "Cannot load analysis." << std::endl << std::endl << ex.what();
            std::string message = messageStream.str();
            fl_message(message.c_str());
        }
    }
}

void Evaluator::exportSelection()
{
    if (chartData_.get() != 0)
    {
        assert(chartData_->analysis.get() != NULL);
        assert(chartData_->cellSelection.get() != NULL);

        // show file chooser
        const char* filepath = fl_file_chooser("Export Selection",
                "CSV (*.csv)", ".", 0);

        if (filepath != NULL) 
        {
            try
            {
                PT::exportCSV(*chartData_->analysis, *chartData_->cellSelection, filepath);
            } 
            catch (PT::IOException ex)
            {
                std::stringstream messageStream;
                messageStream << "Cannot export selection." << std::endl << std::endl << ex.what();
                std::string message = messageStream.str();
                fl_message(message.c_str());
            }
        }
    }
}

void Evaluator::clearCellSelection()
{
    if (chartData_.get() != 0)
    {
        chartData_->cellSelection->clear();
        handleCellSelectionChange();
    }
}

void Evaluator::editChartParameters()
{
    if (chartData_.get() != 0)
    {
        std::auto_ptr<ChartParameters> parameters = 
            ChartParametersEditor::showEditor(
                chartData_->analysis.get(),
                chartData_->parameters.get());

        if (parameters.get() != 0)
            chartData_->parameters = parameters;

        chartViewer_->setData(chartData_.get());
    }
}

void Evaluator::invertCellSelection()
{
    if (chartData_.get() != 0)
    {
        PT::CellSelection* oldCellSelection = chartData_->cellSelection.get();
        chartData_->cellSelection =
            chartData_->analysis->invertCellSelection(oldCellSelection);
        handleCellSelectionChange();
    }
}

void Evaluator::selectAllCells()
{
    if (chartData_.get() != 0)
    {
        chartData_->cellSelection = chartData_->analysis->selectAllCells();
        handleCellSelectionChange();
    }
}

void Evaluator::showFirstImage()
{
    if (chartData_.get() != 0)
    {
        assert(chartData_->analysis.get());

        imageSelector_->selectFirstImage();

        PT::ImageKey imageKey = imageSelector_->getSelection();
        imageViewer_->setImageKey(imageKey);
        chartData_->time = imageKey.time;
        chartViewer_->setData(chartData_.get());
    }
}

void Evaluator::showNextImage()
{
    if (chartData_.get() != 0)
    {
        assert(chartData_->analysis.get());

        imageSelector_->selectNextImage();

        PT::ImageKey imageKey = imageSelector_->getSelection();
        imageViewer_->setImageKey(imageKey);
        chartData_->time = imageKey.time;
        chartViewer_->setData(chartData_.get());
    }
}

void Evaluator::showPreviousImage()
{
    if (chartData_.get() != 0)
    {
        assert(chartData_->analysis.get());

        imageSelector_->selectPreviousImage();

        PT::ImageKey imageKey = imageSelector_->getSelection();
        imageViewer_->setImageKey(imageKey);
        chartData_->time = imageKey.time;
        chartViewer_->setData(chartData_.get());
    }
}

void Evaluator::showLastImage()
{
    if (chartData_.get() != 0)
    {
        assert(chartData_->analysis.get());

        imageSelector_->selectLastImage();

        PT::ImageKey imageKey = imageSelector_->getSelection();
        imageViewer_->setImageKey(imageKey);
        chartData_->time = imageKey.time;
        chartViewer_->setData(chartData_.get());
    }
}

void Evaluator::toggleMarkCells(Fl_Widget* widget)
{
    Fl_Menu_* menu = (Fl_Menu_*)widget;
    const Fl_Menu_Item* menuItem = menu->mvalue();

    // assert that menu item is a checkbox
    assert(menuItem->checkbox());

    imageViewer_->setMarkCells(menuItem->value() != 0);
}

void Evaluator::showStatusBarMessage(const std::string& message)
{
    statusBarMessageBox_->copy_label(message.c_str());
}

void Evaluator::handleCellSelectionChange(bool updateImageViewer, bool updateChartViewer)
{
    if (chartData_.get() == 0)
    {
        imageViewer_->setSelection(0);

        // update status bar selection box
        statusBarSelectionBox_->copy_label("");
    }
    else
    {
        if (updateImageViewer)
            imageViewer_->setSelection(chartData_->cellSelection.get());

        if (updateChartViewer)
            chartViewer_->setData(chartData_.get());


        // update status bar selection box
        std::stringstream selectionStringStream;
        selectionStringStream << chartData_->cellSelection->getNumberOfCells() 
            << " of " << chartData_->analysis->getNumberOfCells()
            << " cells selected";
        std::string selectionString = selectionStringStream.str();
        statusBarSelectionBox_->copy_label(selectionString.c_str());
    }
}

void Evaluator::exit()
{
    ::exit(0);
}

void Evaluator::showAboutDialog()
{
    AboutDialog::showDialog("Evaluator");
}

int main( int argc, char ** argv )
{
    Fl::get_system_colors();

    Evaluator evaluator;
    evaluator.show();

    Fl::run();

    return 0;
}
