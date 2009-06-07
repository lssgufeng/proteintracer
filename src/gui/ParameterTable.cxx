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

#include <gui/ParameterTable.h>

#include <string>

#include <FL/Fl.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Value_Input.H>
#include <FL/fl_draw.H>

#include <common.h>

using std::cout;
using std::endl;
using std::string;

using PT::Parameter;
using PT::ParameterSet;

ParameterTable::ParameterTable(int x, int y, int w, int h) : Fl_Table(x,y,w,h), parameterSet_(0)
{
    box(FL_NO_BOX);

    col_header(0);
    col_resize(1);
    row_header(0);
    row_resize(0);

    cols(2);
    fitColumnWidths(w);

    end();

    setParameterSet(0);
}

void ParameterTable::intParamCallback(Fl_Widget* widget, void* userData)
{
    Parameter* p = static_cast<Parameter*>(userData);
    Fl_Value_Input* valueInput = static_cast<Fl_Value_Input*>(widget);

    assert(p->getType() == Parameter::TYPE_INT);

    int newValue = (int)valueInput->value();
    if (newValue < p->getIntMin() || newValue > p->getIntMax())
    {
        valueInput->value(p->getIntValue());
    }
    else
    {
        p->setIntValue(newValue);
        ParameterTable* parameterTable = static_cast<ParameterTable*>(widget->parent()->parent());
        parameterTable->notifyEventHandler(ParameterTableEvent());
    }
}

void ParameterTable::doubleParamCallback(Fl_Widget* widget, void* userData)
{
    Parameter* p = static_cast<Parameter*>(userData);
    Fl_Value_Input* valueInput = static_cast<Fl_Value_Input*>(widget);

    assert(p->getType() == Parameter::TYPE_DOUBLE);

    double newValue = valueInput->value();
    if (newValue < p->getDoubleMin() || newValue > p->getDoubleMax())
    {
        valueInput->value(p->getDoubleValue());
    }
    else
    {
        p->setDoubleValue(newValue);
        ParameterTable* parameterTable = static_cast<ParameterTable*>(widget->parent()->parent());
        parameterTable->notifyEventHandler(ParameterTableEvent());
    }
}

void ParameterTable::boolParamCallback(Fl_Widget* widget, void* userData)
{
    Parameter* p = static_cast<Parameter*>(userData);
    Fl_Check_Button* checkButton = static_cast<Fl_Check_Button*>(widget);

    assert(p->getType() == Parameter::TYPE_BOOL);

    p->setBoolValue(checkButton->value());
    ParameterTable* parameterTable = static_cast<ParameterTable*>(widget->parent()->parent());
    parameterTable->notifyEventHandler(ParameterTableEvent());
}

void ParameterTable::listParamCallback(Fl_Widget* widget, void* userData)
{
    Parameter* p = static_cast<Parameter*>(userData);
    Fl_Choice* choice = static_cast<Fl_Choice*>(widget);

    assert(p->getType() == Parameter::TYPE_LIST);

    p->setListIndex(choice->value());
    ParameterTable* parameterTable = static_cast<ParameterTable*>(widget->parent()->parent());
    parameterTable->notifyEventHandler(ParameterTableEvent());
}

void ParameterTable::setParameterSet(ParameterSet* params)
{
    table->clear();

    if (params == 0)
    {
        rows(0);
        parameterSet_ = 0;
    }
    else
    {
        rows(params->getSize());

        // backup current group to set it again afterwards
        Fl_Group* currentGroup = Fl_Group::current();

        this->begin();

        ParameterSet::Iterator it = params->getParameterStart();
        ParameterSet::Iterator end = params->getParameterEnd();

        for (int row = 0; it != end; ++row, ++it)
        {
            Parameter& p = *it;

            int x, y, w, h; 

            // create label
            {
                find_cell(CONTEXT_TABLE, row, 0, x, y, w, h);
                const string& name = p.getName();

                Fl_Box* rowLabel = new Fl_Box(x, y, w, h);
                rowLabel->copy_label(name.c_str());
                rowLabel->box(FL_FLAT_BOX);
                rowLabel->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
                rowLabel->tooltip(p.getDescription().c_str());
            }

            // create parameter widget
            {
                find_cell(CONTEXT_TABLE, row, 1, x, y, w, h);
                createParameterWidget(p, x, y, w, h);
            }
        }

        this->end();

        // reset current group, since the parent group of the ParameterTable
        // was chosen by this->end() and this is not necessarily the actual
        // current Group
        Fl_Group::current(currentGroup);
    }

    parameterSet_ = params;
}

ParameterSet* ParameterTable::getParameterSet()
{
    return parameterSet_;
}

void ParameterTable::createParameterWidget(Parameter& p, int x, int y, int w, int h)
{
    if (p.getType() == Parameter::TYPE_INT)
    {
        Fl_Value_Input* valueInput = new Fl_Value_Input(x, y, w, h);
        valueInput->when(FL_WHEN_RELEASE | FL_WHEN_ENTER_KEY);
        valueInput->step(1);
        valueInput->minimum(p.getIntMin());
        valueInput->maximum(p.getIntMax());
        valueInput->value(p.getIntValue());
        valueInput->callback(&ParameterTable::intParamCallback, &p);
    }
    else if (p.getType() == Parameter::TYPE_DOUBLE)
    {
        Fl_Value_Input* valueInput = new Fl_Value_Input(x, y, w, h);
        valueInput->when(FL_WHEN_RELEASE | FL_WHEN_ENTER_KEY);
        valueInput->minimum(p.getDoubleMin());
        valueInput->maximum(p.getDoubleMax());
        valueInput->precision(5);
        valueInput->value(p.getDoubleValue());
        valueInput->callback(&ParameterTable::doubleParamCallback, &p);
    }
    else if (p.getType() == Parameter::TYPE_BOOL)
    {
        Fl_Check_Button* checkButton = new Fl_Check_Button(x, y, w, h);
        checkButton->box(FL_FLAT_BOX);
        checkButton->value(p.getBoolValue());
        checkButton->callback(&ParameterTable::boolParamCallback, &p);
    }
    else if (p.getType() == Parameter::TYPE_LIST)
    {
        Fl_Choice* choice = new Fl_Choice(x, y, w, h);
        {
            const std::vector<std::string>& list = p.getList();
            Fl_Menu_Item* menuItems = new Fl_Menu_Item[list.size() + 1];

            std::vector<std::string>::const_iterator it = list.begin();
            std::vector<std::string>::const_iterator end = list.end();
            for (int i = 0; it != end; ++i, ++it)
            {
                const std::string& value = *it;
                menuItems[i].text = value.c_str();
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
            menuItems[list.size()].text = 0;

            choice->menu(menuItems);
        }

        assert(p.getListIndex() < p.getList().size());
        choice->value(p.getListIndex());
        choice->callback(&ParameterTable::listParamCallback, &p);
    }
}

void ParameterTable::fitColumnWidths(int tableWidth)
{
    double labelRatio = 3.0 / 5.0;
    int labelWidth = (int)(labelRatio * tableWidth);
    if (labelWidth > 200) labelWidth = 200;
    int valueWidth = tableWidth - labelWidth;

    col_width(0, labelWidth);
    col_width(1, valueWidth);
}

void ParameterTable::resize(int X, int Y, int W, int H)
{
    fitColumnWidths(W);
    Fl_Table::resize(X, Y, W, H);
}

void ParameterTable::draw_cell(TableContext context, int R, int C, int X, int Y, int W, int H)
{
    switch ( context )
    {
    case CONTEXT_STARTPAGE:
        fl_font(FL_HELVETICA, 14);
        return;

    case CONTEXT_RC_RESIZE:
    {
        int X, Y, W, H;
        int index = 0;
        for ( int r = 0; r < rows(); r++ )
        {
            for ( int c = 0; c < cols(); c++ )
            {
                if ( index >= children() ) break;
                find_cell(CONTEXT_TABLE, r, c, X, Y, W, H);
                child(index)->resize(X,Y,W,H);
                ++index;
            }
        }
        init_sizes(); // tell group children resized
        return;
    }

    case CONTEXT_ROW_HEADER:
    case CONTEXT_COL_HEADER:
    case CONTEXT_CELL:
    default:
        return;
    }
}

