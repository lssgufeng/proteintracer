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

 #include <assay_editor/AssayDialog.h>

#include <sstream>
#include <vector>

#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Group.H>
#include <FL/fl_ask.H>

#include <itkComposeRGBImageFilter.h>
#include <itkFlipImageFilter.h>
#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkRescaleIntensityImageFilter.h>

#include <Scan.h>
#include <analyzers/AnalyzerRegistry.h>
#include <gui/common.h>
#include <images.h>
#include <io/AssayIO.h>

using std::auto_ptr;
using std::cout;
using std::endl;
using std::string;
using std::stringstream;
using std::vector;

using PT::AnalyzerRegistry;
using PT::Assay;
using PT::ImageIndex;
using PT::ImageKey;
using PT::ImageMetadata;
using PT::RGBImage;
using PT::Scan;
using PT::UCharImage;
using PT::UShortImage;
using PT::Visualization;

AssayDialog::AssayDialog(int x, int y, int width, int height) : 
    Fl_Group(x, y, width, height), 
    scan_(NULL), 
    assay_(0),
    visualizer_(0)
{
    // create user interface
    {
        static const int BIG_MARGIN = 15;
        static const int BUTTON_HEIGHT = 25;
        static const int INPUT_HEIGHT = 25;
        static const int LABEL_HEIGHT = 14;
        static const int MEDIUM_MARGIN = 10;
        static const int SETTINGS_WIDTH = 280;
        static const int SMALL_MARGIN = 5;
        static const int STATUSBAR_HEIGHT = 26;
        static const int STATUSBAR_LOCATION_BOX_WIDTH = 100;
        static const int STATUSBAR_ZOOM_BOX_WIDTH = 100;

        static const int VISUALIZATION_WIDGETS_HEIGHT = 
            BIG_MARGIN
            + BUTTON_HEIGHT
            + INPUT_HEIGHT * 2 
            + LABEL_HEIGHT * 2
            + MEDIUM_MARGIN * 3;

        // create settings group
        Fl_Group* settingsGroup;
        {
            settingsGroup = new Fl_Group(
                    x, 
                    y,
                    SETTINGS_WIDTH,
                    height - STATUSBAR_HEIGHT);
            settingsGroup->box(FL_FLAT_BOX);

            analyzerChoice_ = new Fl_Choice(
                    settingsGroup->x() + MEDIUM_MARGIN,
                    settingsGroup->y() + BIG_MARGIN + LABEL_HEIGHT,
                    settingsGroup->w() - 2 * MEDIUM_MARGIN,
                    INPUT_HEIGHT);
            analyzerChoice_->label("Analysis Method");
            analyzerChoice_->align(FL_ALIGN_LEFT | FL_ALIGN_TOP);
            analyzerChoice_->callback(fltk_member_cb<AssayDialog, &AssayDialog::selectAnalyzer>, this);

            // fill analyzer choice
            {
                const vector<string>& analyzerNames = AnalyzerRegistry::getAnalyzerNames();
                Fl_Menu_Item* menuItems = new Fl_Menu_Item[analyzerNames.size() + 1];

                vector<string>::const_iterator it = analyzerNames.begin();
                vector<string>::const_iterator end = analyzerNames.end();
                for (int i = 0; it != end; ++i, ++it)
                {
                    const std::string& name = *it;
                    menuItems[i].text = name.c_str();
                    menuItems[i].shortcut_ = 0;
                    menuItems[i].callback_ = 0;
                    menuItems[i].user_data_ = 0;
                    menuItems[i].flags = FL_MENU_VALUE;
                    menuItems[i].labeltype_ = FL_NORMAL_LABEL;
                    menuItems[i].labelfont_ = FL_HELVETICA;
                    menuItems[i].labelsize_ = 14;
                    menuItems[i].labelcolor_ = FL_BLACK;
                }

                // add closing item
                menuItems[analyzerNames.size()].text = 0;

                analyzerChoice_->menu(menuItems);
            }

            int pty = analyzerChoice_->y() + analyzerChoice_->h() + MEDIUM_MARGIN;
            parameterTable_ = new ParameterTable(
                    analyzerChoice_->x(),
                    pty,
                    analyzerChoice_->w(),
                    settingsGroup->y() + settingsGroup->h() - pty - VISUALIZATION_WIDGETS_HEIGHT);
            parameterTable_->setEventHandler(this);
            settingsGroup->resizable(parameterTable_);

            imageSelector_ = new ImageSelector(
                    parameterTable_->x(),
                    parameterTable_->y() + parameterTable_->h() + BIG_MARGIN,
                    parameterTable_->w(),
                    LABEL_HEIGHT + INPUT_HEIGHT);
            imageSelector_->setEventHandler(this);

            visualizationChoice_ = new Fl_Choice(
                    imageSelector_->x(),
                    imageSelector_->y() + imageSelector_->h() + MEDIUM_MARGIN + LABEL_HEIGHT,
                    imageSelector_->w(),
                    INPUT_HEIGHT);
            visualizationChoice_->label("Visualization");
            visualizationChoice_->align(FL_ALIGN_LEFT | FL_ALIGN_TOP);

            computeVisualizationButton_ = new Fl_Button(
                    visualizationChoice_->x(),
                    visualizationChoice_->y() + visualizationChoice_->h() + MEDIUM_MARGIN,
                    visualizationChoice_->w(),
                    BUTTON_HEIGHT);
            computeVisualizationButton_->label("Compute Visualization");
            computeVisualizationButton_->callback(fltk_member_cb<AssayDialog, &AssayDialog::computeVisualization>, this);

            settingsGroup->end();
        }

        // create image viewer
        {
            Fl_Group* imageViewerGroup = new Fl_Group(
                    settingsGroup->x() + settingsGroup->w(), 
                    settingsGroup->y(),
                    width - settingsGroup->w(),
                    settingsGroup->h());
            imageViewerGroup->box(FL_DOWN_BOX);
            this->resizable(imageViewerGroup);

            imageViewer_ = new ImageViewer(
                    imageViewerGroup->x() + 2,
                    imageViewerGroup->y() + 2,
                    imageViewerGroup->w() - 4,
                    imageViewerGroup->h() - 4);
            imageViewer_->setEventHandler(this);
            imageViewerGroup->resizable(imageViewer_);

            imageViewerGroup->end();
        }

        // create status bar
        {
            Fl_Group* statusBar = new Fl_Group(
                    x - 2, 
                    y + height - STATUSBAR_HEIGHT,
                    width + 4,
                    STATUSBAR_HEIGHT + 2);
            statusBar->box(FL_ENGRAVED_BOX);

            statusbarPixelLabelBox_ = new Fl_Box(
                    statusBar->x() + SMALL_MARGIN,
                    statusBar->y() + (statusBar->h() - LABEL_HEIGHT) / 2,
                    statusBar->w() 
                        - STATUSBAR_LOCATION_BOX_WIDTH 
                        - STATUSBAR_ZOOM_BOX_WIDTH 
                        - 2 * SMALL_MARGIN,
                    LABEL_HEIGHT);
            statusbarPixelLabelBox_->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
            statusBar->resizable(statusbarPixelLabelBox_);

            statusbarPixelLocationBox_ = new Fl_Box(
                    statusbarPixelLabelBox_->x() + statusbarPixelLabelBox_->w(),
                    statusbarPixelLabelBox_->y(),
                    STATUSBAR_LOCATION_BOX_WIDTH,
                    statusbarPixelLabelBox_->h());
            statusbarPixelLocationBox_->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE);

            statusbarZoomBox_ = new Fl_Box(
                    statusbarPixelLocationBox_->x() + statusbarPixelLocationBox_->w(),
                    statusbarPixelLocationBox_->y(),
                    STATUSBAR_ZOOM_BOX_WIDTH,
                    statusbarPixelLocationBox_->h());
            statusbarZoomBox_->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE);

            statusBar->end();
        }

        this->Fl_Group::end();
    }

    // create default assay
    const string& analyzerName = AnalyzerRegistry::getDefaultAnalyzerName();
    auto_ptr<Assay> assay = AnalyzerRegistry::createAssay(analyzerName);
    setAssay(assay);
}

void AssayDialog::setScan(const Scan* scan)
{
    scan_ = scan;

    imageSelector_->setImageSeriesSet(scan_);

    showStandardVisualization();
}

void initMenuItem(Fl_Menu_Item* menuItem, const char* name)
{
    // text values of menu items are not changed internally so this
    // const cast is safe
    menuItem->text = const_cast<char*>(name);
    menuItem->shortcut_ = 0;
    menuItem->callback_ = 0;
    menuItem->user_data_ = 0;
    menuItem->flags = FL_MENU_VALUE;
    menuItem->labeltype_ = FL_NORMAL_LABEL;
    menuItem->labelfont_ = FL_HELVETICA;
    menuItem->labelsize_ = 14;
    menuItem->labelcolor_ = FL_BLACK;
}

void AssayDialog::setAssay(std::auto_ptr<Assay> assay)
{
    assert(assay.get() != 0);

    assay_ = assay;

    // set analyzer choice
    const string& name = assay_->getAnalyzerName();
    const Fl_Menu_Item* menuItem = analyzerChoice_->find_item(name.c_str());
    analyzerChoice_->value(menuItem);

    // set parameter table
    parameterTable_->setParameterSet(assay_.get());

    // create new visualizer and replace old one
    const string& analyzerName = assay_->getAnalyzerName();
    visualizer_ = AnalyzerRegistry::createVisualizer(analyzerName);

    // fill visualizer choice widgets 
    {
        const vector<string>& visualizationNames = visualizer_->getVisualizationNames();
        assert(visualizationNames.size() > 0);
        Fl_Menu_Item* menuItems = new Fl_Menu_Item[visualizationNames.size() + 1];

        for (int i = 0; i < visualizationNames.size(); ++i)
        {
            initMenuItem(&menuItems[i], visualizationNames[i].c_str());
        }
        initMenuItem(&menuItems[visualizationNames.size()], 0);

        visualizationChoice_->copy(menuItems);

        delete[] menuItems;

        // just updating the menu items does not redraw the Fl_Choice
        visualizationChoice_->redraw();
    }

    showStandardVisualization();
}

const Assay& AssayDialog::getAssay()
{
    return *assay_;
}

void AssayDialog::showStandardVisualization()
{
    if (visualizer_.get() != NULL)
    {
        visualizationChoice_->value(0);
        computeVisualization();
    }
}

int AssayDialog::handle(int event)
{
    switch(event)
    {
    case FL_SHORTCUT:
    {
        int key = Fl::event_key();
        if (key == FL_F + 5)
        {
            computeVisualization();
        }
        return 1;
    }
    default:
        return Fl_Group::handle(event);
    }
}

void AssayDialog::handleEvent(const ImageViewerEvent &event)
{
    updateStatusbar();
}

void AssayDialog::handleEvent(const ImageSelectorEvent &event)
{
    showStandardVisualization();
}

void AssayDialog::selectAnalyzer()
{
    const string& selection = analyzerChoice_->text();
    auto_ptr<Assay> assay = AnalyzerRegistry::createAssay(selection);
    setAssay(assay);
}

void AssayDialog::updateStatusbar()
{
    ImageIndex mousePos = imageViewer_->getMousePosition();
    int x = mousePos[0];
    int y = mousePos[1];

    // update label box
    if (visualization_.get() != 0)
    {
        string label = visualization_->getPixelLabel(x, y);
        statusbarPixelLabelBox_->copy_label(label.c_str());
    } 
    else
    {
        statusbarPixelLabelBox_->copy_label("");
    }

    // update location box
    if (x >= 0 && y >= 0)
    {

        stringstream text;
        text << x << ", " << y;
        statusbarPixelLocationBox_->copy_label(text.str().c_str());
    }
    else
    {
        statusbarPixelLocationBox_->copy_label("");
    }

    // update zoom box
    stringstream zoomText;
    zoomText << imageViewer_->getZoom() << " %";
    statusbarZoomBox_->copy_label(zoomText.str().c_str());
}

void AssayDialog::computeVisualization()
{
    assert(visualizer_.get() != 0);

    if (scan_ != 0)
    {
        if (visualizationChoice_->text()) 
        {
            try 
            {
                string visualizationName(visualizationChoice_->text()); 
                ImageKey imageKey = imageSelector_->getSelection();
                const ImageMetadata& imageMetadata = scan_->getImageMetadata(imageKey);

                auto_ptr<Visualization> visualization = visualizer_->createVisualization(visualizationName, *assay_.get(), imageMetadata);

                assert(visualization.get() != 0);
                assert(visualization->getImage().IsNotNull());

                if (visualization_.get() != 0 && imageKey.location == visualization_->getImageKey().location)
                {
                    imageViewer_->setImageAndRetainFocus(visualization->getImage());
                }
                else
                {
                    imageViewer_->setImage(visualization->getImage());
                }

                this->visualization_ = visualization;
            }
            catch (PT::VisualizerException& e)
            {
                stringstream messageStream;
                messageStream << "Visualization failed..." << endl << endl << e.what();
                string message = messageStream.str();
                fl_message(message.c_str());
            }
        }
    }
}
