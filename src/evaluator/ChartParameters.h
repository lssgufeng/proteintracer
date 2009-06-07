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

#ifndef ChartParameters_h
#define ChartParameters_h

#include <Analysis.h>
#include <ParameterSet.h>

static const std::string PARAMETER_SCATTERPLOT_FEATURE_X("Feature X");
static const std::string PARAMETER_SCATTERPLOT_MINIMUM_X("Minimum X");
static const std::string PARAMETER_SCATTERPLOT_MAXIMUM_X("Maximum X");
static const std::string PARAMETER_SCATTERPLOT_FEATURE_Y("Feature Y");
static const std::string PARAMETER_SCATTERPLOT_MINIMUM_Y("Minimum Y");
static const std::string PARAMETER_SCATTERPLOT_MAXIMUM_Y("Maximum Y");
static const std::string PARAMETER_SCATTERPLOT_AUTO_PARAMETERS("Auto Parameters");

static const std::string PARAMETER_LINE_CHART_FEATURE("Feature");
static const std::string PARAMETER_LINE_CHART_MINIMUM_TIME("Min. Time");
static const std::string PARAMETER_LINE_CHART_MAXIMUM_TIME("Max. Time");
static const std::string PARAMETER_LINE_CHART_MINIMUM_VALUE("Min. Feature Value");
static const std::string PARAMETER_LINE_CHART_MAXIMUM_VALUE("Max. Feature Value");
static const std::string PARAMETER_LINE_CHART_AUTO_PARAMETERS("Auto Parameters");

static const std::string PARAMETER_HISTOGRAM_FEATURE("Feature");
static const std::string PARAMETER_HISTOGRAM_INTERVAL_SIZE("Interval Size");
static const std::string PARAMETER_HISTOGRAM_MINIMUM_VALUE("Min. Feature Value");
static const std::string PARAMETER_HISTOGRAM_MAXIMUM_VALUE("Max. Feature Value");
static const std::string PARAMETER_HISTOGRAM_MINIMUM_FREQUENCY("Min. Frequency");
static const std::string PARAMETER_HISTOGRAM_MAXIMUM_FREQUENCY("Max. Frequency");
static const std::string PARAMETER_HISTOGRAM_AUTO_PARAMETERS("Auto Parameters");

static const std::string PARAMETER_BOXPLOT_FEATURE("Feature");
static const std::string PARAMETER_BOXPLOT_MINIMUM_TIME("Min. Time");
static const std::string PARAMETER_BOXPLOT_MAXIMUM_TIME("Max. Time");
static const std::string PARAMETER_BOXPLOT_MINIMUM_VALUE("Min. Feature Value");
static const std::string PARAMETER_BOXPLOT_MAXIMUM_VALUE("Max. Feature Value");
static const std::string PARAMETER_BOXPLOT_AUTO_PARAMETERS("Auto Parameters");

const int CHART_TYPE_NUMBER = 4;

extern const std::vector<std::string> CHART_TYPE_NAMES;

enum ChartType
{
    CHART_TYPE_SCATTERPLOT = 0,
    CHART_TYPE_LINE_CHART = 1,
    CHART_TYPE_HISTOGRAM = 2,
    CHART_TYPE_BOXPLOT = 3
};

class ChartParameters : public PT::ParameterSet
{
public:
    static std::auto_ptr<ChartParameters> createParameters(ChartType chartType, const PT::Analysis* analysis);

    ChartParameters(const ChartParameters& parameters) :
        chartType_(parameters.chartType_),
        PT::ParameterSet(parameters)
    {
    }

    ChartType getChartType()
    {
        return chartType_;
    }

    std::string validate();

private:

    ChartParameters(ChartType chartType, const std::vector<PT::Parameter>& params) :
        chartType_(chartType),
        PT::ParameterSet(params) 
    {
    }

    ChartType chartType_;
};

#endif
