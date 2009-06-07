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

#ifndef AssayRunner_h
#define AssayRunner_h

#include <memory>
#include <string>

#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Progress.H>
#include <FL/Fl_Text_Display.H>

#include <Analyzer.h>
#include <common.h>
#include <gui/common.h>

class AssayRunner : public Fl_Double_Window, PT::EventHandler<PT::AnalyzerEvent>
{
public:

    AssayRunner();

    void handleEvent(const PT::AnalyzerEvent &event);

private:

    void browseScanFile();

    void browseAssayFile();

    void browseAnalysisDirectory();

    void exit();

    void start();

    void cancel();

    void showAboutDialog();

    void logMessage(const std::string& message);

    Fl_Input* scanFileInput_;

    Fl_Input* assayFileInput_;

    Fl_Input* analysisDirectoryInput_;

    Fl_Text_Display* messageDisplay_;

    Fl_Progress* progressBar_;

    std::auto_ptr<PT::Analyzer> analyzer_;

    bool running_;

};

#endif
