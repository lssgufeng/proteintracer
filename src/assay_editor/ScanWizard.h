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

#ifndef ScanWizard_h
#define ScanWizard_h

#include <FL/Fl_Button.H>
#include <FL/Fl_Counter.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Return_Button.H>
#include <FL/Fl_Table_Row.H>
#include <FL/Fl_Value_Input.H>
#include <FL/Fl_Wizard.H>

#include <list>

#include <common.h>
#include <Scan.h>

struct ScanDialogEvent
{
    enum Id {
        STATUS_CHANGE
    } id;
};

class ScanDialog : public Fl_Group, public PT::EventGenerator<ScanDialogEvent>
{
public:

    ScanDialog(int x, int y, int width, int height);

    bool isAdoptable();

	const char* getErrorMessage();

    void initializeScan(PT::Scan& scan);

private:

    enum Status
    {
        STATUS_ADOPTABLE = 0,
        STATUS_START = 1,
        STATUS_NO_FILES = 2,
        STATUS_INVALID_INDEX = 3,
        STATUS_DUPLICATE_KEY = 4
    };

    struct FileItem
    {
        std::string filename;
        PT::ImageKey key;
        bool valid;

        FileItem() : valid(true) { }

        bool operator<(const FileItem &item) const
        {
            return (!valid && item.valid) || (key < item.key);
        }
    };

    class FileTable : public Fl_Table
    {
    public:

        FileTable(int x, int y, int w, int h);

        void updateTable(const std::list<FileItem>& fileList);

    protected:

        enum Column {
            COLUMN_FILEPATH = 0,
            COLUMN_WELL = 1,
            COLUMN_POSITION = 2,
            COLUMN_SLIDE = 3,
            COLUMN_TIME = 4
        };

        static const int NUM_COLUMNS =  5;
        static const char *COLUMN_CAPTIONS[];
        static const int COLUMN_PADDING = 3;

        std::vector<const FileItem*> fileVector_;

        void updateTable();

        void draw_cell(TableContext context, int R=0, int C=0, int X=0, int Y=0, int W=0, int H=0);

        void formatColumnString(const FileItem &fileItem, Column column, std::string& s);
    };

    std::list<FileItem> fileList_;

    Status fileListStatus_;
    Status propertyStatus_;
    Status status_;

    Fl_Input *directoryInput_;
    Fl_Counter *wellCounter_;
    Fl_Counter *positionCounter_;
    Fl_Counter *slideCounter_;
    Fl_Counter *timeCounter_;

    FileTable *fileTable_;

    void createUserInterface(int x, int y, int width, int height);
    void browseDirectory();
    void updateStatus(Fl_Widget *widget);
};

class ScanWizard : public Fl_Double_Window, public PT::EventHandler<ScanDialogEvent>
{
public:

    static std::string showWizard();

private:

    Fl_Box *messageDisplay_;
    Fl_Box *errorDisplay_;

    Fl_Wizard *wizard_;
    ScanDialog *scanDialog_;

    Fl_Return_Button *nextButton_;
    Fl_Button *cancelButton_;

    std::auto_ptr<PT::Scan> scan_;
    std::string filepath_;

    ScanWizard();

    void updateState();
    void handleEvent(const ScanDialogEvent &event);
    void handleCancelButtonClick();
    void handleNextButtonClick();
    bool saveScan();
};

#endif
