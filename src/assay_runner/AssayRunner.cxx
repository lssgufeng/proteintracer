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

#include <assay_runner/AssayRunner.h>

#include <FL/Fl.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Return_Button.H>
#include <FL/filename.H>

#include <analyzers/AnalyzerRegistry.h>
#include <gui/AboutDialog.h>
#include <io/AssayIO.h>
#include <io/ScanIO.h>

AssayRunner::AssayRunner() : Fl_Double_Window(600, 500, "Protein Tracer:  Assay Runner"), running_(false)
{
    // create user interface
    {
        static const int BIG_MARGIN = 15;
        static const int BUTTON_HEIGHT = 25;
        static const int BUTTON_WIDTH = 85;
        static const int COUNTER_HEIGHT = 25;
        static const int COUNTER_WIDTH = 50;
        static const int INPUT_HEIGHT = 25;
        static const int LABEL_HEIGHT = 14;
        static const int MEDIUM_MARGIN = 10;
        static const int MESSAGE_HEIGHT = 60;
        static const int PROGRESS_BAR_HEIGHT = 25;
        static const int SMALL_MARGIN = 5;
        static const int VALUE_INPUT_WIDTH = 100;

        this->box(FL_FLAT_BOX);

        // called when the window is closed
        this->callback(fltk_member_cb<AssayRunner, &AssayRunner::exit>, this);

        Fl_Group *scanFileGroup = new Fl_Group(
            this->x(),
            this->y(),
            this->w(),
            INPUT_HEIGHT + LABEL_HEIGHT + BIG_MARGIN);

        scanFileInput_ = new Fl_Input(
            scanFileGroup->x() + MEDIUM_MARGIN,
            scanFileGroup->y() + LABEL_HEIGHT + BIG_MARGIN,
            scanFileGroup->w() - 3 * MEDIUM_MARGIN - BUTTON_WIDTH,
            INPUT_HEIGHT);
        scanFileInput_->label("Scan File");
        scanFileInput_->align(FL_ALIGN_LEFT | FL_ALIGN_TOP);
        scanFileGroup->resizable(scanFileInput_);

        Fl_Button* browseScanFileButton = new Fl_Button(
            scanFileInput_->x() + scanFileInput_->w() + MEDIUM_MARGIN,
            scanFileInput_->y(),
            BUTTON_WIDTH, 
            scanFileInput_->h());
        browseScanFileButton->label("Browse");
        browseScanFileButton->callback(fltk_member_cb<AssayRunner, &AssayRunner::browseScanFile>, this);

        scanFileGroup->end();

        Fl_Group *assayFileGroup = new Fl_Group(
            scanFileGroup->x(),
            scanFileGroup->y() + scanFileGroup->h(),
            scanFileGroup->w(),
            scanFileGroup->h());

        assayFileInput_ = new Fl_Input(
            assayFileGroup->x() + MEDIUM_MARGIN,
            assayFileGroup->y() + LABEL_HEIGHT + BIG_MARGIN,
            assayFileGroup->w() - 3 * MEDIUM_MARGIN - BUTTON_WIDTH,
            INPUT_HEIGHT);
        assayFileInput_->label("Assay File");
        assayFileInput_->align(FL_ALIGN_LEFT | FL_ALIGN_TOP);
        assayFileGroup->resizable(assayFileInput_);

        Fl_Button* browseAssayFileButton = new Fl_Button(
            assayFileInput_->x() + assayFileInput_->w() + MEDIUM_MARGIN,
            assayFileInput_->y(),
            BUTTON_WIDTH, 
            assayFileInput_->h());
        browseAssayFileButton->label("Browse");
        browseAssayFileButton->callback(fltk_member_cb<AssayRunner, &AssayRunner::browseAssayFile>, this);

        assayFileGroup->end();

        Fl_Group *analysisDirectoryGroup = new Fl_Group(
            assayFileGroup->x(),
            assayFileGroup->y() + assayFileGroup->h(),
            assayFileGroup->w(),
            assayFileGroup->h());

        analysisDirectoryInput_ = new Fl_Input(
            analysisDirectoryGroup->x() + MEDIUM_MARGIN,
            analysisDirectoryGroup->y() + LABEL_HEIGHT + BIG_MARGIN,
            analysisDirectoryGroup->w() - 3 * MEDIUM_MARGIN - BUTTON_WIDTH,
            INPUT_HEIGHT);
        analysisDirectoryInput_->label("Analysis Directory");
        analysisDirectoryInput_->align(FL_ALIGN_LEFT | FL_ALIGN_TOP);
        analysisDirectoryGroup->resizable(analysisDirectoryInput_);

        Fl_Button* browseAnalysisDirectoryButton = new Fl_Button(
            analysisDirectoryInput_->x() + analysisDirectoryInput_->w() + MEDIUM_MARGIN,
            analysisDirectoryInput_->y(),
            BUTTON_WIDTH, 
            analysisDirectoryInput_->h());
        browseAnalysisDirectoryButton->label("Browse");
        browseAnalysisDirectoryButton->callback(fltk_member_cb<AssayRunner, &AssayRunner::browseAnalysisDirectory>, this);

        analysisDirectoryGroup->end();
        
        Fl_Group *buttonGroup = new Fl_Group(
            analysisDirectoryGroup->x(),
            analysisDirectoryGroup->y() + analysisDirectoryGroup->h(),
            analysisDirectoryGroup->w(),
            INPUT_HEIGHT + BIG_MARGIN);

        Fl_Return_Button* startButton = new Fl_Return_Button(
            MEDIUM_MARGIN,
            buttonGroup->y() + BIG_MARGIN,
            BUTTON_WIDTH, 
            BUTTON_HEIGHT);
        startButton->label("&Start");
        startButton->callback(fltk_member_cb<AssayRunner, &AssayRunner::start>, this);

        Fl_Button* cancelButton = new Fl_Button(
            startButton->x() + startButton->w() + MEDIUM_MARGIN,
            buttonGroup->y() + BIG_MARGIN,
            BUTTON_WIDTH, 
            BUTTON_HEIGHT);
        cancelButton->label("&Cancel");
        cancelButton->callback(fltk_member_cb<AssayRunner, &AssayRunner::cancel>, this);

        Fl_Box* spacerBox = new Fl_Box(
            cancelButton->x() + cancelButton->w(),
            cancelButton->y(),
            buttonGroup->w() - 3 * BUTTON_WIDTH - 3 * MEDIUM_MARGIN,
            cancelButton->h());
        buttonGroup->resizable(spacerBox);

        Fl_Button* aboutButton = new Fl_Button(
            spacerBox->x() + spacerBox->w(),
            spacerBox->y(),
            BUTTON_WIDTH, 
            spacerBox->h());
        aboutButton->label("&About");
        aboutButton->callback(fltk_member_cb<AssayRunner, &AssayRunner::showAboutDialog>, this);

        buttonGroup->end();

        messageDisplay_ = new Fl_Text_Display(
                this->x() + SMALL_MARGIN,
                buttonGroup->y() + buttonGroup->h() + BIG_MARGIN,
                this->w() - 2 * SMALL_MARGIN,
                this->h() - 3 * scanFileGroup->h() - buttonGroup->h() - PROGRESS_BAR_HEIGHT - BIG_MARGIN - 2 * SMALL_MARGIN);
        this->resizable(messageDisplay_);
        messageDisplay_->buffer(new Fl_Text_Buffer());

        progressBar_ = new Fl_Progress(
                messageDisplay_->x(),
                messageDisplay_->y() + messageDisplay_->h() + SMALL_MARGIN,
                messageDisplay_->w(),
                PROGRESS_BAR_HEIGHT);
        progressBar_->minimum(0);
        progressBar_->maximum(1);

        this->end();
    }
}

void AssayRunner::handleEvent(const PT::AnalyzerEvent &event)
{
    if (event.getId() == PT::AnalyzerEvent::MESSAGE)
    {
        logMessage(event.getMessage());
    }
    else if (event.getId() == PT::AnalyzerEvent::PROGRESS)
    {
        progressBar_->value(event.getProgress());
    }
    else if (event.getId() == PT::AnalyzerEvent::CHECK_GUI)
    {
        // just call Fl::check()
    }

    Fl::check();
}

void AssayRunner::browseScanFile()
{
    const char* file = fl_file_chooser("Scan File",
            "XML Files (*.xml)", ".", 0);
    if (file != NULL)
    {
        scanFileInput_->value(file);
    }
}

void AssayRunner::browseAssayFile()
{
    const char* file = fl_file_chooser("Assay File",
            "XML Files (*.xml)", ".", 0);
    if (file != NULL)
    {
        assayFileInput_->value(file);
    }
}

void AssayRunner::browseAnalysisDirectory()
{
    const char* directory = fl_dir_chooser("Analysis Directory", ".", 0);
    if (directory != NULL)
    {
        analysisDirectoryInput_->value(directory);
    }
}

void AssayRunner::exit()
{
    ::exit(0);
}

void AssayRunner::start()
{
    if (running_) 
        return;
    running_ = true;

    // clear message buffer and progress bar
    Fl_Text_Buffer* textBuffer = messageDisplay_->buffer();
    textBuffer->remove(0, textBuffer->length());
    progressBar_->value(0);

    // load scan
    std::auto_ptr<PT::Scan> scan;
    {
        try
        {
            const char* scanFile = scanFileInput_->value();
            scan = PT::loadScan(scanFile);
        } 
        catch (PT::IOException ex)
        {
            std::stringstream messageStream;
            messageStream << "Cannot load scan:" << std::endl << ex.what();
            logMessage(messageStream.str());
            running_ = false;
            return;
        }
    }

    // load assay
    std::auto_ptr<PT::Assay> assay;
    {
        try
        {
            const char* assayFile = assayFileInput_->value();
            assay = PT::loadAssay(assayFile);
        } 
        catch (PT::IOException ex)
        {
            std::stringstream messageStream;
            messageStream << "Cannot load assay:" << std::endl << ex.what();
            logMessage(messageStream.str());
            running_ = false;
            return;
        }
    }

    // check analysis directory
    std::string analysisDirectory;
    {
        analysisDirectory = analysisDirectoryInput_->value();

        // fl_filename_isdir works only if the last character of the
        // filename is no separator - so strip it
        {
            int lastIndex = analysisDirectory.size() - 1;
            const char& lastChar = analysisDirectory[lastIndex];
#ifdef WIN32
            if (lastChar == '\\')
                analysisDirectory.erase(lastIndex);
#else
            if (lastChar == '/')
                analysisDirectory.erase(lastIndex);
#endif
        }

        if ( fl_filename_isdir(analysisDirectory.c_str()) == 0)
        {
            logMessage("Invalid analysis directory");
            running_ = false;
            return;
        }

        // analysisDirectory is required to end with a separator
        // so append it again
        {
#ifdef WIN32
            analysisDirectory.append(1, '\\');
#else
            analysisDirectory.append(1, '/');
#endif
        }
    }

    // perform analysis
    {
        analyzer_ = PT::AnalyzerRegistry::createAnalyzer(*assay, scan.get(), analysisDirectory);
        analyzer_->setEventHandler(this);

        analyzer_->process();

        analyzer_.reset();
    }

    running_ = false;
}

void AssayRunner::cancel()
{
    if (analyzer_.get() != 0)
    {
        analyzer_->cancel();
    }
}

void AssayRunner::showAboutDialog()
{
    AboutDialog::showDialog("Assay Runner");
}

void AssayRunner::logMessage(const std::string& message)
{
    Fl_Text_Buffer* textBuffer = messageDisplay_->buffer();
    textBuffer->append(message.c_str());
}

int main( int argc, char ** argv )
{
    Fl::get_system_colors();

    AssayRunner assayRunner;
    assayRunner.show();

    Fl::run();

    return 0;
}
