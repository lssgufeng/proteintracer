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

#include <evaluator/ChartParametersEditor.h>

#include <FL/Fl_Button.H>
#include <FL/Fl_Return_Button.H>
#include <FL/fl_ask.H>

#include <gui/common.h>

static const int WINDOW_WIDTH = 400;
static const int WINDOW_HEIGHT = 500;

ChartParametersEditor::ChartParametersEditor(const PT::Analysis* analysis) :
    analysis_(analysis),
    Fl_Double_Window(WINDOW_WIDTH, WINDOW_HEIGHT, "Edit Chart Parameters")
{
    static const int BIG_MARGIN = 15;
    static const int BUTTON_HEIGHT = 25;
    static const int BUTTON_WIDTH = 85;
    static const int INPUT_HEIGHT = 25;
    static const int LABEL_HEIGHT = 14;
    static const int MEDIUM_MARGIN = 10;

    static const int BUTTONS_HEIGHT = BUTTON_HEIGHT + MEDIUM_MARGIN * 2;

    this->set_modal();

    chartTypeChoice_ = new Fl_Choice(
            this->x() + MEDIUM_MARGIN,
            this->y() + BIG_MARGIN + LABEL_HEIGHT,
            this->w() - 2 * MEDIUM_MARGIN,
            INPUT_HEIGHT);
    chartTypeChoice_->label("Chart Type");
    chartTypeChoice_->align(FL_ALIGN_LEFT | FL_ALIGN_TOP);
    chartTypeChoice_->callback(fltk_member_cb<ChartParametersEditor, &ChartParametersEditor::selectChartType>, this);

    // fill chartTypeChoice_
    {
        Fl_Menu_Item* menuItems = new Fl_Menu_Item[CHART_TYPE_NAMES.size() + 1];

        std::vector<std::string>::const_iterator it = CHART_TYPE_NAMES.begin();
        std::vector<std::string>::const_iterator end = CHART_TYPE_NAMES.end();
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
        menuItems[CHART_TYPE_NAMES.size()].text = 0;

        chartTypeChoice_->menu(menuItems);
    }

    int pty = chartTypeChoice_->y() + chartTypeChoice_->h() + MEDIUM_MARGIN;
    parameterTable_ = new ParameterTable(
            chartTypeChoice_->x(),
            pty,
            chartTypeChoice_->w(),
            this->y() + this->h() - pty - BUTTONS_HEIGHT);
    this->resizable(parameterTable_);

    Fl_Return_Button* okButton = new Fl_Return_Button(
        parameterTable_->x() + parameterTable_->w() - MEDIUM_MARGIN - 2 * BUTTON_WIDTH,
        this->y() + this->h() - BUTTONS_HEIGHT + MEDIUM_MARGIN,
        BUTTON_WIDTH,
        BUTTON_HEIGHT);
    okButton->label("&OK");
    okButton->callback(fltk_member_cb<ChartParametersEditor, &ChartParametersEditor::handleOkButtonClick>, this);

    Fl_Button* cancelButton = new Fl_Button(
        okButton->x() + okButton->w() + MEDIUM_MARGIN,
        okButton->y(),
        BUTTON_WIDTH,
        BUTTON_HEIGHT);
    cancelButton->label("&Cancel");
    cancelButton->callback(fltk_member_cb<ChartParametersEditor, &ChartParametersEditor::handleCancelButtonClick>, this);

    this->end();
}

void ChartParametersEditor::selectChartType()
{
    ChartType chartTypeIndex = (ChartType) chartTypeChoice_->value();
    std::auto_ptr<ChartParameters> chartParameters = ChartParameters::createParameters(chartTypeIndex, analysis_);
    setChartParameters(chartParameters);
}

void ChartParametersEditor::setChartParameters(std::auto_ptr<ChartParameters> chartParameters)
{
    assert(chartParameters.get() != 0);

    parameters_ = chartParameters;

    int chartType = parameters_->getChartType();
    chartTypeChoice_->value(chartType);

    parameterTable_->setParameterSet(parameters_.get());
}

void ChartParametersEditor::handleOkButtonClick()
{
    std::string errorMessage = parameters_->validate();

    if (errorMessage.size() > 0)
        fl_message(errorMessage.c_str());
    else
        this->hide();
}

void ChartParametersEditor::handleCancelButtonClick()
{
    parameters_.reset();
    this->hide();
}

std::auto_ptr<ChartParameters> ChartParametersEditor::showEditor(
        const PT::Analysis* analysis, 
        const ChartParameters* chartParameters)
{
    assert(analysis != 0);

    ChartParametersEditor chartParametersEditor(analysis);
    if (chartParameters != 0)
    {
        std::auto_ptr<ChartParameters> newChartParamters(new ChartParameters(*chartParameters));
        chartParametersEditor.setChartParameters(newChartParamters);
    }
    else
    {
        std::auto_ptr<ChartParameters> newChartParamters =
            ChartParameters::createParameters(CHART_TYPE_SCATTERPLOT, analysis);
        chartParametersEditor.setChartParameters(newChartParamters);
    }

    chartParametersEditor.show();
    while (chartParametersEditor.shown())
        Fl::wait();

    return chartParametersEditor.parameters_;
}
