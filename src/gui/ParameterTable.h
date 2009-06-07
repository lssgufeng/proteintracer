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

#ifndef ParameterTable_h
#define ParameterTable_h

#include <FL/Fl_Table.H>

#include <ParameterSet.h>
#include <common.h>

class ParameterTableEvent
{
public:
    enum Id {
        PARAMETER_CHANGE
    } id;

    ParameterTableEvent() : id(PARAMETER_CHANGE) {}
};

class ParameterTable : public Fl_Table, public PT::EventGenerator<ParameterTableEvent>
{
public:

    ParameterTable(int x, int y, int w, int h);

    void setParameterSet(PT::ParameterSet* params);

    PT::ParameterSet* getParameterSet();

    void resize(int X, int Y, int W, int H);

protected:

    void draw_cell(TableContext context, int R=0, int C=0, int X=0, int Y=0, int W=0, int H=0);

private:

    static void intParamCallback(Fl_Widget* widget, void* userData);
    static void doubleParamCallback(Fl_Widget* widget, void* userData);
    static void boolParamCallback(Fl_Widget* widget, void* userData);
    static void listParamCallback(Fl_Widget* widget, void* userData);

    void createParameterWidget(PT::Parameter& p, int x, int y, int w, int h);

    void fitColumnWidths(int tableWidth);

    PT::ParameterSet* parameterSet_;
};

#endif
