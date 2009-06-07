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

#include <assay_editor/MainWindow.h>

#include <sstream>

#include <itkImage.h>
#include <itkImageFileReader.h>

#include <FL/Fl.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Menu_Bar.H>

#include <Scan.h>
#include <assay_editor/ScanWizard.h>
#include <gui/AboutDialog.h>
#include <gui/common.h>
#include <io/AssayIO.h>
#include <io/ScanIO.h>

using std::auto_ptr;
using std::cout;
using std::endl;
using std::string;
using std::stringstream;

using PT::Assay;
using PT::ImageIndex;
using PT::ImageKey;
using PT::ImageMetadata;
using PT::Scan;

MainWindow::MainWindow() : 
    Fl_Window( 900, 600, "Protein Tracer:  Assay Editor"), 
    scan_(NULL)
{
    // create user interface
    {
        static const int MENU_BAR_HEIGHT = 25;

        // called when the window is closed
        this->callback(fltk_member_cb<MainWindow, &MainWindow::exit>, this);

        Fl_Menu_Bar *menuBar = new Fl_Menu_Bar(
                0, 
                0, 
                w(), 
                MENU_BAR_HEIGHT);
        menuBar->box(FL_FLAT_BOX);
        Fl_Menu_Item menuItems[] = 
        {
            { "&File", 0, 0, 0, FL_SUBMENU },
            { "&Open Assay...", FL_CTRL + 'o', fltk_member_cb<MainWindow, &MainWindow::openAssay>, 0, 0 },
            { "&Save Assay...", FL_CTRL + 's', fltk_member_cb<MainWindow, &MainWindow::saveAssay>, 0, FL_MENU_DIVIDER },
            { "E&xit", FL_CTRL + 'q', fltk_member_cb<MainWindow, &MainWindow::exit>, 0 },
            { 0 },
            { "&Scan", 0, 0, 0, FL_SUBMENU },
            { "&New Scan...", FL_CTRL + FL_SHIFT + 'n', fltk_member_cb<MainWindow, &MainWindow::createScan>, 0, 0 },
            { "&Open Scan...", FL_CTRL + FL_SHIFT + 'o', fltk_member_cb<MainWindow, &MainWindow::openScan>, 0, 0 },
            { 0 },
            { "&Help", 0, 0, 0, FL_SUBMENU },
            { "&About",  0, fltk_member_cb<MainWindow, &MainWindow::showAboutDialog>, 0, 0 },
            { 0 },
            { 0 }
        };
        menuBar->copy(menuItems, this);

        Fl_Box* separatorBox = new Fl_Box(
                menuBar->x() - 2,
                menuBar->y() + menuBar->h(),
                menuBar->w() + 4,
                2);
        separatorBox->box(FL_ENGRAVED_BOX);

        assayDialog_ = new AssayDialog(
                menuBar->x(), 
                separatorBox->y() + separatorBox->h(), 
                menuBar->w(), 
                h() - menuBar->h() - separatorBox->h());

        this->resizable(assayDialog_);

        this->end();
    }
}

void MainWindow::exit()
{
    ::exit(0);
}

void MainWindow::openAssay()
{
    // show file chooser
    const char* filepath = fl_file_chooser("Open Assay",
            "XML Files (*.xml)", ".", 0);
    if (filepath != NULL) 
    {
        try
        {
            auto_ptr<Assay> assay = PT::loadAssay(filepath);
            assayDialog_->setAssay(assay);
        } 
        catch (PT::IOException ex)
        {
            std::stringstream messageStream;
            messageStream << "Cannot load assay." << std::endl << std::endl << ex.what();
            std::string message = messageStream.str();
            fl_message(message.c_str());
        }
    }
}

void MainWindow::saveAssay()
{
    const char* filepath = fl_file_chooser("Save Assay", "XML Files (*.xml)", ".", 0);
    if (filepath != NULL)
    {
        const PT::Assay& assay = assayDialog_->getAssay();

        try
        {
            PT::saveAssay(assay, filepath);
        }
        catch (PT::IOException&)
        {
            fl_alert("Could not save file.");
        }
    }
}

void MainWindow::createScan()
{
    std::string filepath = ScanWizard::showWizard();

    if (! filepath.empty())
    {
        openScan(filepath);
    }
}

void MainWindow::openScan()
{
    const char* filepath = fl_file_chooser("Open Scan",
            "XML Files (*.xml)", ".", 0);

    if (filepath != NULL) 
    {
        string filepathStr(filepath);
        openScan(filepathStr);
    }
}

void MainWindow::openScan(const string& filepath)
{
    try
    {
        scan_ = PT::loadScan(filepath.c_str());
        assayDialog_->setScan(scan_.get());
    }
    catch (PT::IOException ex)
    {
        std::stringstream messageStream;
        messageStream << "Cannot load scan." << std::endl << std::endl << ex.what();
        std::string message = messageStream.str();
        fl_message(message.c_str());
    }
}

void MainWindow::showAboutDialog()
{
    AboutDialog::showDialog("Assay Editor");
}

int main( int argc, char ** argv )
{
    Fl::get_system_colors();

    MainWindow p;
    p.show();

    Fl::run();

    return 0;
}
