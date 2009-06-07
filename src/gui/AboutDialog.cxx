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

#include <gui/AboutDialog.h>

#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Return_Button.H>

#include <gui/common.h>

static const int WINDOW_WIDTH = 400;
static const int WINDOW_HEIGHT = 300;

AboutDialog::AboutDialog(const std::string& applicationName) :
    Fl_Double_Window(WINDOW_WIDTH, WINDOW_HEIGHT, "About")
{
    static const int BIG_MARGIN = 15;
    static const int BUTTON_HEIGHT = 25;
    static const int BUTTON_WIDTH = 85;
    static const int MEDIUM_MARGIN = 10;

    static const int BUTTONS_HEIGHT = BUTTON_HEIGHT + MEDIUM_MARGIN * 2;

    this->set_modal();

    Fl_Text_Display* messageDisplay = new Fl_Text_Display(
            this->x() + MEDIUM_MARGIN,
            this->y() + BIG_MARGIN,
            this->w() - 2 * MEDIUM_MARGIN,
            this->h() - BIG_MARGIN - BUTTONS_HEIGHT);
    messageDisplay->color(FL_BACKGROUND_COLOR);
    messageDisplay->box(FL_NO_BOX);
    messageDisplay->align(FL_ALIGN_LEFT | FL_ALIGN_TOP);

    Fl_Return_Button* closeButton = new Fl_Return_Button(
            this->x() + this->w() - MEDIUM_MARGIN - BUTTON_WIDTH,
            this->y() + this->h() - BUTTONS_HEIGHT + MEDIUM_MARGIN,
            BUTTON_WIDTH,
            BUTTON_HEIGHT);
    closeButton->label("&Close");
    closeButton->callback(fltk_member_cb<Fl_Double_Window, &Fl_Double_Window::hide>, this);

    // set message
    Fl_Text_Buffer* textBuffer = new Fl_Text_Buffer();
    textBuffer->append("Protein Tracer:  ");
    textBuffer->append(applicationName.c_str());
    textBuffer->append("\n\nwritten by Andre Homeyer\n\nDeveloped in the context of a diploma thesis at the\nApplied Neuroinformatics Group of Bielefeld University.\n\nThis software is based in part on the work of several\nopen source projects:\n\n- Fast Light Toolkit,  http://www.fltk.org\n- Fl_Table,  http://seriss.com/people/erco/fltk/Fl_Table\n- Insight Toolkit,  http://www.itk.org\n- OpenGL,  http://www.opengl.org\n- tinyxml,  http://www.grinninglizard.com/tinyxml");
    messageDisplay->buffer(textBuffer);

    this->end();
}

void AboutDialog::showDialog(const std::string& applicationName)
{

    AboutDialog aboutDialog(applicationName);

    aboutDialog.show();
    while (aboutDialog.shown())
        Fl::wait();
}
