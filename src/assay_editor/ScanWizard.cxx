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

#include <assay_editor/ScanWizard.h>

#include <ctype.h>
#include <stdlib.h>

#include <iostream>
#include <map>
#include <vector>
 
#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/filename.H>
#include <FL/fl_draw.H>

#include <io/ScanIO.h>
#include <gui/common.h>

using std::auto_ptr;
using std::list;
using std::string;
using std::stringstream;
using std::vector;

using PT::ImageMetadata;
using PT::Scan;

const char *ScanDialog::FileTable::COLUMN_CAPTIONS[] = {"Filename", "Well", "Position", "Slide", "Time"};

ScanDialog::FileTable::FileTable(int x, int y, int w, int h) : Fl_Table(x,y,w,h)
{
    col_header(1);            // enable col header
    col_resize(0);            // enable col resizing
    row_header(1);            // disable row header
    row_resize(0);            // enable row resizing

    cols(NUM_COLUMNS);
    rows(0);

    updateTable();

    end();
}

void ScanDialog::FileTable::updateTable(const list<FileItem>& fileList)
{
    fileVector_.clear();

    list<FileItem>::const_iterator it = fileList.begin();
    list<FileItem>::const_iterator end = fileList.end();

    for (;it != end; it++)
    {
        fileVector_.push_back(&(*it));
    }
    
    updateTable();
}

void ScanDialog::FileTable::updateTable()
{
    rows(fileVector_.size());

    // adjust column and row dimensions
    { 
        static const int MIN_NUMBER_COLUMN_WIDTH = 70;
        static const int MIN_FILEPATH_COLUMN_WIDTH = 300;
        static const int ROW_HEIGHT = 25;

        // set font so that measurement is performed correctly
        fl_font(FL_HELVETICA, 14);

        int rowHeaderWidth = 0;
        int columnWidths_[NUM_COLUMNS];
        columnWidths_[COLUMN_WELL] = MIN_NUMBER_COLUMN_WIDTH;
        columnWidths_[COLUMN_POSITION] = MIN_NUMBER_COLUMN_WIDTH;
        columnWidths_[COLUMN_SLIDE] = MIN_NUMBER_COLUMN_WIDTH;
        columnWidths_[COLUMN_TIME] = MIN_NUMBER_COLUMN_WIDTH;
        columnWidths_[COLUMN_FILEPATH] = MIN_FILEPATH_COLUMN_WIDTH;

        if (fileVector_.size() > 0)
        {
            int stringWidth, stringHeight;

            // measure row header width
            {
                stringWidth = 0;
                stringHeight = 0;
                string text(formatInt(fileVector_.size() + 1));
                fl_measure(text.c_str(), stringWidth, stringHeight, 0);
                rowHeaderWidth = stringWidth + 2 * COLUMN_PADDING;
            }

            vector<const FileItem*>::const_iterator it = fileVector_.begin();
            vector<const FileItem*>::const_iterator end = fileVector_.end();

            for (; it != end; it++)
            {
                const FileItem &fileItem = *(*it);

                for (int col = 0; col < NUM_COLUMNS; col++)
                {
                    string text;
                    formatColumnString(fileItem, (Column) col, text);

                    stringWidth = 0;
                    stringHeight = 0;
                    fl_measure(text.c_str(), stringWidth, stringHeight, 0);
                    stringWidth += 2 * COLUMN_PADDING;

                    if (stringWidth > columnWidths_[col])
                        columnWidths_[col] = stringWidth;
                }
            }
        }

        row_header_width(rowHeaderWidth);
        for (int col = 0; col < NUM_COLUMNS; col++)
        {
            col_width(col, columnWidths_[col]);
        }

        row_height_all(ROW_HEIGHT);
    }

    row_position(0);
    col_position(COLUMN_TIME);
}

void ScanDialog::FileTable::draw_cell(TableContext context, int R, int C, int X, int Y, int W, int H)
{
    switch ( context )
    {
    case CONTEXT_STARTPAGE:
        fl_font(FL_HELVETICA, 14);
        return;

    case CONTEXT_COL_HEADER:
    {
        fl_push_clip(X, Y, W, H);
        fl_draw_box(FL_THIN_UP_BOX, X, Y, W, H, color());

        fl_color(FL_BLACK);
        if (C == COLUMN_FILEPATH)
            fl_draw(COLUMN_CAPTIONS[C], X+COLUMN_PADDING, Y, W, H, FL_ALIGN_LEFT);
        else
            fl_draw(COLUMN_CAPTIONS[C], X, Y, W, H, FL_ALIGN_CENTER);

        fl_pop_clip();

        return;
    }

    case CONTEXT_ROW_HEADER:
    {
        fl_push_clip(X, Y, W, H);
        fl_draw_box(FL_THIN_UP_BOX, X, Y, W, H, color());

        string text(formatInt(R + 1));

        fl_color(FL_BLACK);
        fl_draw(text.c_str(), X - COLUMN_PADDING, Y, W, H, FL_ALIGN_RIGHT);

        fl_pop_clip();

        return;
    }

    case CONTEXT_CELL:
    {
        fl_push_clip(X, Y, W, H);

        string text;
        Fl_Color bgColor = FL_WHITE;
        if (R < (int)fileVector_.size() && C < NUM_COLUMNS)
        {
            const FileItem &fileItem = *(fileVector_[R]);
            formatColumnString(fileItem, (Column) C, text);
            if (! fileItem.valid) bgColor = FL_RED;
        }

        // draw background
        fl_color(bgColor);
        fl_rectf(X, Y, W, H);

        // draw text
        fl_color(FL_BLACK);
        if (C == COLUMN_FILEPATH)
            fl_draw(text.c_str(), X-COLUMN_PADDING, Y, W, H, FL_ALIGN_RIGHT);
        else
            fl_draw(text.c_str(), X, Y, W, H, FL_ALIGN_CENTER);

        // draw border
        fl_color(FL_LIGHT2);
        fl_rect(X, Y, W, H);

        fl_pop_clip();

        return;
    }

    case CONTEXT_ENDPAGE:
    case CONTEXT_RC_RESIZE:
    case CONTEXT_NONE:
    case CONTEXT_TABLE:
        return;
    }

}

inline void formatKeyInt(int value, string& text)
{
    if (value >= 0)
        text = formatInt(value);
    else
        text = "n/a";
}

inline void ScanDialog::FileTable::formatColumnString(const FileItem &fileItem, Column column, string& text)
{
    switch (column)
    {
    case COLUMN_FILEPATH:
        text = fileItem.filename;
        break;
    case COLUMN_WELL:
        formatKeyInt(fileItem.key.location.well, text);
        break;
    case COLUMN_POSITION:
        formatKeyInt(fileItem.key.location.position, text);
        break;
    case COLUMN_SLIDE:
        formatKeyInt(fileItem.key.location.slide, text);
        break;
    case COLUMN_TIME:
        formatKeyInt(fileItem.key.time, text);
    }
}

ScanDialog::ScanDialog(int x, int y, int width, int height) : Fl_Group(x, y, width, height), status_(STATUS_START), fileListStatus_(STATUS_NO_FILES), propertyStatus_(STATUS_ADOPTABLE)
{
    // create user interface
    {
        const static int BUTTON_WIDTH = 85;
        const static int COUNTER_WIDTH = 50;
        const static int VALUE_INPUT_WIDTH = 100;
        const static int INPUT_HEIGHT = 25;
        const static int COUNTER_HEIGHT = 25;
        const static int LABEL_HEIGHT = 14;
        const static int BIG_MARGIN = 15;
        const static int MEDIUM_MARGIN = 10;
        const static int SMALL_MARGIN = 5;

        this->box(FL_FLAT_BOX);

        Fl_Group *settingsGroup = new Fl_Group(
            x, 
            y, 
            width, 
            INPUT_HEIGHT + LABEL_HEIGHT + 2 * BIG_MARGIN);

        directoryInput_ = new Fl_Input(
            settingsGroup->x() + MEDIUM_MARGIN,
            settingsGroup->y() + LABEL_HEIGHT + BIG_MARGIN,
            settingsGroup->w() - 6 * MEDIUM_MARGIN - BIG_MARGIN - BUTTON_WIDTH - 4 * COUNTER_WIDTH,
            INPUT_HEIGHT);
        directoryInput_->label("Directory");
        directoryInput_->align(FL_ALIGN_LEFT | FL_ALIGN_TOP);
        directoryInput_->callback(fltk_member_cb_widget<ScanDialog, &ScanDialog::updateStatus>, this);
        settingsGroup->resizable(directoryInput_);

        Fl_Button* browseDirectoryButton_ = new Fl_Button(
            directoryInput_->x() + directoryInput_->w() + MEDIUM_MARGIN,
            directoryInput_->y(),
            BUTTON_WIDTH, 
            directoryInput_->h());
        browseDirectoryButton_->label("Browse");
        browseDirectoryButton_->callback(fltk_member_cb<ScanDialog, &ScanDialog::browseDirectory>, this);

        wellCounter_ = new Fl_Counter(
            browseDirectoryButton_->x() + browseDirectoryButton_->w() + BIG_MARGIN,
            browseDirectoryButton_->y(),
            COUNTER_WIDTH,
            COUNTER_HEIGHT);
        wellCounter_->type(FL_SIMPLE_COUNTER);
        wellCounter_->step(1);
        wellCounter_->minimum(0);
        wellCounter_->maximum(10);
        wellCounter_->label("Well");
        wellCounter_->align(FL_ALIGN_CENTER | FL_ALIGN_TOP);
        wellCounter_->value(0);
        wellCounter_->callback(fltk_member_cb_widget<ScanDialog, &ScanDialog::updateStatus>, this);
        wellCounter_->when(FL_WHEN_RELEASE);

        positionCounter_ = new Fl_Counter(
            wellCounter_->x() + wellCounter_->w() + MEDIUM_MARGIN,
            wellCounter_->y(),
            wellCounter_->w(),
            wellCounter_->h());
        positionCounter_->type(FL_SIMPLE_COUNTER);
        positionCounter_->step(1);
        positionCounter_->minimum(0);
        positionCounter_->maximum(10);
        positionCounter_->label("Position");
        positionCounter_->align(FL_ALIGN_CENTER | FL_ALIGN_TOP);
        positionCounter_->value(0);
        positionCounter_->callback(fltk_member_cb_widget<ScanDialog, &ScanDialog::updateStatus>, this);
        positionCounter_->when(FL_WHEN_RELEASE);

        slideCounter_ = new Fl_Counter(
            positionCounter_->x() + positionCounter_->w() + MEDIUM_MARGIN,
            positionCounter_->y(),
            positionCounter_->w(),
            positionCounter_->h());
        slideCounter_->type(FL_SIMPLE_COUNTER);
        slideCounter_->step(1);
        slideCounter_->minimum(0);
        slideCounter_->maximum(10);
        slideCounter_->label("Slide");
        slideCounter_->align(FL_ALIGN_CENTER | FL_ALIGN_TOP);
        slideCounter_->value(0);
        slideCounter_->callback(fltk_member_cb_widget<ScanDialog, &ScanDialog::updateStatus>, this);
        slideCounter_->when(FL_WHEN_RELEASE);

        timeCounter_ = new Fl_Counter(
            slideCounter_->x() + slideCounter_->w() + MEDIUM_MARGIN,
            slideCounter_->y(),
            slideCounter_->w(),
            slideCounter_->h());
        timeCounter_->type(FL_SIMPLE_COUNTER);
        timeCounter_->step(1);
        timeCounter_->minimum(1);
        timeCounter_->maximum(10);
        timeCounter_->label("Time");
        timeCounter_->align(FL_ALIGN_CENTER | FL_ALIGN_TOP);
        timeCounter_->value(1);
        timeCounter_->callback(fltk_member_cb_widget<ScanDialog, &ScanDialog::updateStatus>, this);
        timeCounter_->when(FL_WHEN_RELEASE);

        settingsGroup->end();

        fileTable_ = new FileTable(
                x + SMALL_MARGIN,
                settingsGroup->y() + settingsGroup->h(),
                width - 2 * SMALL_MARGIN,
                height - settingsGroup->h());
        this->resizable(fileTable_);

        this->end();
    }
}

bool ScanDialog::isAdoptable()
{
    return status_ == STATUS_ADOPTABLE;    
}

const char* ScanDialog::getErrorMessage()
{
    static const char *ERROR_MESSAGES[] =
    {
        "",
        "",
        "The chosen directory contains no image files.",
        "Some filenames could not be parsed correctly.",
        "Some image keys are not unique.",
    };

    return ERROR_MESSAGES[status_];
}

void ScanDialog::browseDirectory()
{
    const char* directory = fl_dir_chooser("Scan Directory", ".", 0);
    if (directory != NULL)
    {
        directoryInput_->value(directory);
        updateStatus(directoryInput_);
    }
}

void ScanDialog::initializeScan(Scan& scan)
{
    std::string directory = std::string(directoryInput_->value());

    // ensure that directory ends with a slash or backslash
    {
        int lastIndex = directory.size() - 1;
        const char& lastChar = directory[lastIndex];

#ifdef WIN32
        if (lastChar != '\\')
            directory.append(1, '\\');
#else
        if (lastChar != '/')
            directory.append(1, '/');
#endif
    }

    list<FileItem>::iterator it = fileList_.begin();
    list<FileItem>::iterator end = fileList_.end();

    for (;it != end; ++it)
    {
        FileItem& fileItem = *it;

        ImageMetadata imageMetadata;
        imageMetadata.key = fileItem.key;
        imageMetadata.filepath = directory + fileItem.filename;

        scan.addImageMetadata(imageMetadata);
    }
}

void ScanDialog::updateStatus(Fl_Widget *widget)
{
    // update file list
    if (widget == directoryInput_ || 
        widget == wellCounter_ || 
        widget == positionCounter_ || 
        widget == slideCounter_ ||
        widget == timeCounter_)
    {
        fileListStatus_ = STATUS_ADOPTABLE;

        static const char *IMAGE_FILE_PATTERN = "{*.tif|*.png}";

        const char *directory = directoryInput_->value();
        int wellIndex = (int)wellCounter_->value();
        int positionIndex = (int)positionCounter_->value();
        int slideIndex = (int)slideCounter_->value();
        int timeIndex = (int)timeCounter_->value();

        fileList_.clear();

        // read image files
        {
            // create file items
            dirent **dirEntries;
            int numDirEntries = fl_filename_list(directory, &dirEntries);
            FileItem emptyItem;
            for (int i = 0; i < numDirEntries; i++) 
            {
                char *filename = dirEntries[i]->d_name;
                if (fl_filename_isdir(filename) == 0 &&
                        fl_filename_match(filename, IMAGE_FILE_PATTERN) != 0)
                {
                    FileItem& insertedItem = *fileList_.insert(fileList_.end(), emptyItem);
                    insertedItem.filename = filename;
                }
            }

            // free dynamically allocated directory entry structures
            if (numDirEntries > 0)
            {
                for (int i = numDirEntries; i > 0;) 
                {
                    free(dirEntries[--i]);
                }
                free(dirEntries);
            }

            // check validity
            if (fileList_.empty())
            {
                fileListStatus_ = STATUS_NO_FILES;
            }
        }

        // parse indices
        if (fileListStatus_ == STATUS_ADOPTABLE)
        {
            list<FileItem>::iterator it = fileList_.begin();
            list<FileItem>::iterator end = fileList_.end();

            for (;it != end; it++)
            {
                FileItem& fileItem = *it;

                // parse tokens
                vector<int> tokens;
                {
                    string token;
                    string::const_iterator it = fileItem.filename.begin();
                    string::const_iterator end = fileItem.filename.end();
                    for(;it != end; it++)
                    {
                        char c = *it;
                        if (c >= '0' && c <= '9') {
                            token += *it;
                        }
                        else
                        {
                            if (token.size() > 0)
                            {
                                int value = atoi(token.c_str()); 
                                tokens.push_back(value);
                                token.clear();
                            }
                        }
                    }
                }

                if (wellIndex == 0)
                    fileItem.key.location.well = 0;
                else if (wellIndex >= 1 && wellIndex <= tokens.size())
                    fileItem.key.location.well = tokens[wellIndex-1];
                else
                    fileItem.valid = false;

                if (positionIndex == 0)
                    fileItem.key.location.position = 0;
                else if (positionIndex >= 1 && positionIndex <= tokens.size())
                    fileItem.key.location.position = tokens[positionIndex-1];
                else
                    fileItem.valid = false;

                if (slideIndex == 0)
                    fileItem.key.location.slide = 0;
                else if (slideIndex >= 1 && slideIndex <= tokens.size())
                    fileItem.key.location.slide = tokens[slideIndex-1];
                else
                    fileItem.valid = false;

                if (timeIndex >= 1 && timeIndex <= tokens.size())
                    fileItem.key.time = tokens[timeIndex-1];
                else
                    fileItem.valid = false;

                if (! fileItem.valid)
                    fileListStatus_ = STATUS_INVALID_INDEX;
            }

            fileList_.sort();
        }

        // check uniqueness of keys
        if (fileListStatus_ == STATUS_ADOPTABLE)
        {
            list<FileItem>::iterator before = fileList_.begin();
            list<FileItem>::iterator current = ++fileList_.begin();
            list<FileItem>::iterator end = fileList_.end();

            for (;current != end; ++before, ++current) {

                FileItem& beforeItem = *before;
                FileItem& currentItem = *current;
                if (beforeItem.key == currentItem.key)
                {
                    beforeItem.valid = false; 
                    currentItem.valid = false; 
                    fileListStatus_ = STATUS_DUPLICATE_KEY;
                }
            }

            if (!fileListStatus_ == STATUS_ADOPTABLE)
            {
                fileList_.sort();
            }
        }

        fileTable_ ->updateTable(fileList_);
    }

    // set new status
    Status oldStatus = status_;
    if (fileListStatus_ != STATUS_ADOPTABLE)
        status_ = fileListStatus_;
    else if (propertyStatus_ != STATUS_ADOPTABLE)
        status_ = propertyStatus_;
    else
        status_ = STATUS_ADOPTABLE;

    // notify event handler
    if (oldStatus != status_)
    {
        ScanDialogEvent event;
        event.id = ScanDialogEvent::STATUS_CHANGE;
        notifyEventHandler(event);
    }
}

ScanWizard::ScanWizard() : Fl_Double_Window(800, 600, "New Scan"), filepath_("")
{
    // create user interface
    {
        static const int BUTTON_HEIGHT = 25;
        static const int BUTTON_WIDTH = 85;
        static const int LABEL_HEIGHT = 14;
        static const int MEDIUM_MARGIN = 10;
        static const int MESSAGE_HEIGHT = 60;
        static const int WINDOW_WIDTH = 800;
        static const int WINDOW_HEIGHT = 600;

        static const int BUTTONS_HEIGHT = 2 * MEDIUM_MARGIN + BUTTON_HEIGHT;
        static const int PAGE_HEIGHT = WINDOW_HEIGHT - MESSAGE_HEIGHT - BUTTONS_HEIGHT;

        this->set_modal();

        this->begin();

        Fl_Group *messageGroup = new Fl_Group(
            0 - 2,
            0 - 2,
            WINDOW_WIDTH + 4,
            MESSAGE_HEIGHT + 4);
        messageGroup->box(FL_ENGRAVED_BOX);

        messageGroup->begin();

        Fl_Box *spaceBox = new Fl_Box(
            messageGroup->w(),
            messageGroup->y(),
            1,
            messageGroup->h());
        messageGroup->resizable(spaceBox);

        messageDisplay_ = new Fl_Box(
            messageGroup->x() + MEDIUM_MARGIN, 
            messageGroup->y() + (messageGroup->h() - 2 * LABEL_HEIGHT - MEDIUM_MARGIN) / 2, 
            messageGroup->w() - MEDIUM_MARGIN, 
            LABEL_HEIGHT);
        messageDisplay_->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
        messageDisplay_->labelfont(FL_HELVETICA_BOLD);

        errorDisplay_ = new Fl_Box(
            messageDisplay_->x(), 
            messageDisplay_->y() + messageDisplay_->h() + MEDIUM_MARGIN, 
            messageDisplay_->w(), 
            LABEL_HEIGHT);
        errorDisplay_->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
        errorDisplay_->labelcolor(FL_RED);

        messageGroup->end();

        wizard_ = new Fl_Wizard(
            messageGroup->x(),
            messageGroup->y() + messageGroup->h(),
            messageGroup->w(),
            PAGE_HEIGHT);
        this->resizable(wizard_);

        scanDialog_ = new ScanDialog(
            wizard_->x(),
            wizard_->y(),
            wizard_->w(),
            wizard_->h());
        scanDialog_->setEventHandler(this);
        wizard_->add(scanDialog_);

        wizard_->end();

        Fl_Group *buttonGroup = new Fl_Group(
            wizard_->x(),
            wizard_->y() + wizard_->h(),
            wizard_->w(), 
            BUTTONS_HEIGHT);

        buttonGroup->begin();

        Fl_Box *spacerBox = new Fl_Box(
            buttonGroup->x(),
            buttonGroup->y(),
            buttonGroup->w() - 2 * (BUTTON_WIDTH + MEDIUM_MARGIN),
            buttonGroup->h());
        buttonGroup->resizable(spacerBox);

        cancelButton_ = new Fl_Button(
            spacerBox->x() + spacerBox->w(),
            buttonGroup->y() + MEDIUM_MARGIN,
            BUTTON_WIDTH,
            BUTTON_HEIGHT);
        cancelButton_->label("Cancel");
        cancelButton_->callback(fltk_member_cb<ScanWizard, &ScanWizard::handleCancelButtonClick>, this);

        nextButton_ = new Fl_Return_Button(
            cancelButton_->x() + cancelButton_->w() + MEDIUM_MARGIN,
            cancelButton_->y(),
            cancelButton_->w(),
            cancelButton_->h());
        nextButton_->label("Next");
        nextButton_->callback(fltk_member_cb<ScanWizard, &ScanWizard::handleNextButtonClick>, this);

        buttonGroup->end();

        this->end();

        updateState();
    }
}

std::string ScanWizard::showWizard()
{
    ScanWizard scanWizard;

    scanWizard.show();
    while (scanWizard.shown())
        Fl::wait();

    return scanWizard.filepath_;
}

void ScanWizard::handleEvent(const ScanDialogEvent &event)
{
    updateState();
}

void ScanWizard::updateState()
{
    if (wizard_->value() == scanDialog_)
    {
        messageDisplay_->label("Please specify the scan directory and how to parse the image filenames.");

        const char *errorMessage = scanDialog_->getErrorMessage();
        errorDisplay_->label(errorMessage);

        nextButton_->label("Save");
        if (scanDialog_->isAdoptable())
            nextButton_->activate();
        else
            nextButton_->deactivate();
    }
}

void ScanWizard::handleCancelButtonClick()
{
    this->hide();
}

void ScanWizard::handleNextButtonClick()
{
    if (wizard_->value() == scanDialog_ && scanDialog_->isAdoptable())
    {
        if (saveScan())
            this->hide();
    }
}

bool ScanWizard::saveScan()
{
    assert(scanDialog_->isAdoptable());

    const char* filepath = fl_file_chooser("Save Scan",
            "XML Files (*.xml)", ".", 0);
    if (filepath != NULL)
    {
        auto_ptr<Scan> scan(new Scan());
        scanDialog_->initializeScan(*scan);


        try
        {
            PT::saveScan(*scan, filepath);

            scan_ = scan;
            filepath_ = filepath;

            return true;
        }
        catch (PT::IOException&)
        {
            fl_alert("Could not save file.");
        }
    }

    return false;
}
