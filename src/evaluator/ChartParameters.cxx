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

#include <evaluator/ChartParameters.h>

#include <itkNumericTraits.h>

std::string CHART_TYPE_NAME_ARRAY[CHART_TYPE_NUMBER] = 
{
    "Scatterplot",
    "Line Chart",
    "Histogram",
    "Boxplot"
};
const std::vector<std::string> CHART_TYPE_NAMES(CHART_TYPE_NAME_ARRAY, CHART_TYPE_NAME_ARRAY + CHART_TYPE_NUMBER);

std::auto_ptr<ChartParameters> ChartParameters::createParameters(ChartType chartType, const PT::Analysis* analysis)
{
    std::vector<PT::Parameter> parameters;

    if (chartType == CHART_TYPE_SCATTERPLOT)
    {
        PT::Parameter featureXParam(
            PARAMETER_SCATTERPLOT_FEATURE_X,
            "The feature plotted on the x-axis.",
            0, 
            analysis->getMetadata().featureNames);
        parameters.push_back(featureXParam);

        PT::Parameter minimumXParam(
            PARAMETER_SCATTERPLOT_MINIMUM_X,
            "The minimum value on the x-axis.",
            0.0, itk::NumericTraits<float>::NonpositiveMin(), itk::NumericTraits<float>::max());
        parameters.push_back(minimumXParam);

        PT::Parameter maximumXParam(
            PARAMETER_SCATTERPLOT_MAXIMUM_X,
            "The maximum value on the x-axis.",
            1.0, itk::NumericTraits<float>::NonpositiveMin(), itk::NumericTraits<float>::max());
        parameters.push_back(maximumXParam);

        PT::Parameter featureYParam(
            PARAMETER_SCATTERPLOT_FEATURE_Y,
            "The feature plotted on the y-axis.",
            0, 
            analysis->getMetadata().featureNames);
        parameters.push_back(featureYParam);

        PT::Parameter minimumYParam(
            PARAMETER_SCATTERPLOT_MINIMUM_Y,
            "The minimum value on the y-axis.",
            0.0, itk::NumericTraits<float>::NonpositiveMin(), itk::NumericTraits<float>::max());
        parameters.push_back(minimumYParam);

        PT::Parameter maximumYParam(
            PARAMETER_SCATTERPLOT_MAXIMUM_Y,
            "The maximum value on the y-axis.",
            1.0, itk::NumericTraits<float>::NonpositiveMin(), itk::NumericTraits<float>::max());
        parameters.push_back(maximumYParam);

        PT::Parameter autoParametersParam(
            PARAMETER_SCATTERPLOT_AUTO_PARAMETERS,
            "Determines whether the ranges of the axes are chosen automatically.",
            true);
        parameters.push_back(autoParametersParam);
    }
    else if (chartType == CHART_TYPE_LINE_CHART)
    {
        PT::Parameter featureParam(
            PARAMETER_LINE_CHART_FEATURE,
            "The feature to be visualized.",
            0, 
            analysis->getMetadata().featureNames);
        parameters.push_back(featureParam);

        PT::Parameter minimumTime(
            PARAMETER_LINE_CHART_MINIMUM_TIME,
            "The minimum time to be plotted on the x-axis",
            0.0, itk::NumericTraits<float>::NonpositiveMin(), itk::NumericTraits<float>::max());
        parameters.push_back(minimumTime);

        PT::Parameter maximumTime(
            PARAMETER_LINE_CHART_MAXIMUM_TIME,
            "The maximum time to be plotted on the x-axis",
            100.0, 0.0, itk::NumericTraits<float>::max());
        parameters.push_back(maximumTime);

        PT::Parameter minimumFeatureValueParam(
            PARAMETER_LINE_CHART_MINIMUM_VALUE,
            "The minimum feature value to be plotted on the y-axis",
            0.0, itk::NumericTraits<float>::NonpositiveMin(), itk::NumericTraits<float>::max());
        parameters.push_back(minimumFeatureValueParam);

        PT::Parameter maximumFeatureValueParam(
            PARAMETER_LINE_CHART_MAXIMUM_VALUE,
            "The maximum feature value to be plotted on the y-axis",
            1.0, itk::NumericTraits<float>::NonpositiveMin(), itk::NumericTraits<float>::max());
        parameters.push_back(maximumFeatureValueParam);

        PT::Parameter autoParametersParam(
            PARAMETER_LINE_CHART_AUTO_PARAMETERS,
            "Determines whether the ranges of the axes are chosen automatically.",
            true);
        parameters.push_back(autoParametersParam);
    }
    else if (chartType == CHART_TYPE_HISTOGRAM)
    {
        PT::Parameter featureParam(
            PARAMETER_HISTOGRAM_FEATURE,
            "The feature to be visualized.",
            0, 
            analysis->getMetadata().featureNames);
        parameters.push_back(featureParam);

        PT::Parameter intervalSizeParam(
            PARAMETER_HISTOGRAM_INTERVAL_SIZE,
            "The size of the value interval.",
            1.0, itk::NumericTraits<float>::min(), itk::NumericTraits<float>::max());
        parameters.push_back(intervalSizeParam);

        PT::Parameter minimumFeatureValueParam(
            PARAMETER_HISTOGRAM_MINIMUM_VALUE,
            "The minimum feature value to be plotted on the x-axis",
            0.0, itk::NumericTraits<float>::NonpositiveMin(), itk::NumericTraits<float>::max());
        parameters.push_back(minimumFeatureValueParam);

        PT::Parameter maximumFeatureValueParam(
            PARAMETER_HISTOGRAM_MAXIMUM_VALUE,
            "The maximum feature value to be plotted on the x-axis",
            1.0, itk::NumericTraits<float>::NonpositiveMin(), itk::NumericTraits<float>::max());
        parameters.push_back(maximumFeatureValueParam);

        PT::Parameter minimumFrequencyParam(
            PARAMETER_HISTOGRAM_MINIMUM_FREQUENCY,
            "The minimum frequency to be plotted on the y-axis",
            0.0, itk::NumericTraits<float>::NonpositiveMin(), itk::NumericTraits<float>::max());
        parameters.push_back(minimumFrequencyParam);

        PT::Parameter maximumFrequencyParam(
            PARAMETER_HISTOGRAM_MAXIMUM_FREQUENCY,
            "The maximum frequency to be plotted on the y-axis",
            200.0, itk::NumericTraits<float>::NonpositiveMin(), itk::NumericTraits<float>::max());
        parameters.push_back(maximumFrequencyParam);

        PT::Parameter autoParametersParam(
            PARAMETER_HISTOGRAM_AUTO_PARAMETERS,
            "Determines whether the ranges of the axis and the interval size are chosen automatically.",
            true);
        parameters.push_back(autoParametersParam);
    }
    else if (chartType == CHART_TYPE_BOXPLOT)
    {
        PT::Parameter featureParam(
            PARAMETER_BOXPLOT_FEATURE,
            "The feature to be visualized.",
            0, 
            analysis->getMetadata().featureNames);
        parameters.push_back(featureParam);

        PT::Parameter minimumTime(
            PARAMETER_BOXPLOT_MINIMUM_TIME,
            "The minimum time to be plotted on the x-axis",
            0.0, itk::NumericTraits<float>::NonpositiveMin(), itk::NumericTraits<float>::max());
        parameters.push_back(minimumTime);

        PT::Parameter maximumTime(
            PARAMETER_BOXPLOT_MAXIMUM_TIME,
            "The maximum time to be plotted on the x-axis",
            100.0, 0.0, itk::NumericTraits<float>::max());
        parameters.push_back(maximumTime);

        PT::Parameter minimumFeatureValueParam(
            PARAMETER_BOXPLOT_MINIMUM_VALUE,
            "The minimum feature value to be plotted on the y-axis",
            0.0, itk::NumericTraits<float>::NonpositiveMin(), itk::NumericTraits<float>::max());
        parameters.push_back(minimumFeatureValueParam);

        PT::Parameter maximumFeatureValueParam(
            PARAMETER_BOXPLOT_MAXIMUM_VALUE,
            "The maximum feature value to be plotted on the y-axis",
            1.0, itk::NumericTraits<float>::NonpositiveMin(), itk::NumericTraits<float>::max());
        parameters.push_back(maximumFeatureValueParam);

        PT::Parameter autoParametersParam(
            PARAMETER_BOXPLOT_AUTO_PARAMETERS,
            "Determines whether the ranges of the axes are chosen automatically.",
            true);
        parameters.push_back(autoParametersParam);
    }

    return std::auto_ptr<ChartParameters>(new ChartParameters(chartType, parameters));
}

std::string ChartParameters::validate()
{
    if (chartType_ == CHART_TYPE_SCATTERPLOT)
    {
        float xMin = getParameter(PARAMETER_SCATTERPLOT_MINIMUM_X).getDoubleValue();
        float xMax = getParameter(PARAMETER_SCATTERPLOT_MAXIMUM_X).getDoubleValue();
        if (xMax <= xMin)
        {
            std::stringstream messageStream;
            messageStream << "Invalid values: \"" << PARAMETER_SCATTERPLOT_MINIMUM_X 
                << "\" must be less than \"" << PARAMETER_SCATTERPLOT_MAXIMUM_X << "\"";
            return messageStream.str();
        }

        float yMin = getParameter(PARAMETER_SCATTERPLOT_MINIMUM_Y).getDoubleValue();
        float yMax = getParameter(PARAMETER_SCATTERPLOT_MAXIMUM_Y).getDoubleValue();
        if (yMax <= yMin)
        {
            std::stringstream messageStream;
            messageStream << "Invalid values: \"" << PARAMETER_SCATTERPLOT_MINIMUM_Y 
                << "\" must be less than \"" << PARAMETER_SCATTERPLOT_MAXIMUM_Y << "\"";
            return messageStream.str();
        }
    }
    else if (chartType_ == CHART_TYPE_LINE_CHART)
    {
        float minTime = getParameter(PARAMETER_LINE_CHART_MINIMUM_TIME).getDoubleValue();
        float maxTime = getParameter(PARAMETER_LINE_CHART_MAXIMUM_TIME).getDoubleValue();
        if (maxTime <= minTime)
        {
            std::stringstream messageStream;
            messageStream << "Invalid values: \"" << PARAMETER_LINE_CHART_MINIMUM_TIME 
                << "\" must be less than \"" << PARAMETER_LINE_CHART_MAXIMUM_TIME << "\"";
            return messageStream.str();
        }

        float minValue = getParameter(PARAMETER_LINE_CHART_MINIMUM_VALUE).getDoubleValue();
        float maxValue = getParameter(PARAMETER_LINE_CHART_MAXIMUM_VALUE).getDoubleValue();
        if (maxValue <= minValue)
        {
            std::stringstream messageStream;
            messageStream << "Invalid values: \"" << PARAMETER_LINE_CHART_MINIMUM_VALUE 
                << "\" must be less than \"" << PARAMETER_LINE_CHART_MAXIMUM_VALUE << "\"";
            return messageStream.str();
        }
    }
    else if (chartType_ == CHART_TYPE_HISTOGRAM)
    {
        float minValue = getParameter(PARAMETER_HISTOGRAM_MINIMUM_VALUE).getDoubleValue();
        float maxValue = getParameter(PARAMETER_HISTOGRAM_MAXIMUM_VALUE).getDoubleValue();
        if (maxValue <= minValue)
        {
            std::stringstream messageStream;
            messageStream << "Invalid values: \"" << PARAMETER_HISTOGRAM_MINIMUM_VALUE 
                << "\" must be less than \"" << PARAMETER_HISTOGRAM_MAXIMUM_VALUE << "\"";
            return messageStream.str();
        }

        float minFrequency = getParameter(PARAMETER_HISTOGRAM_MINIMUM_FREQUENCY).getDoubleValue();
        float maxFrequency = getParameter(PARAMETER_HISTOGRAM_MAXIMUM_FREQUENCY).getDoubleValue();
        if (maxFrequency <= minFrequency)
        {
            std::stringstream messageStream;
            messageStream << "Invalid values: \"" << PARAMETER_HISTOGRAM_MINIMUM_FREQUENCY 
                << "\" must be less than \"" << PARAMETER_HISTOGRAM_MAXIMUM_FREQUENCY << "\"";
            return messageStream.str();
        }
    }
    else if (chartType_ == CHART_TYPE_BOXPLOT)
    {
        float minTime = getParameter(PARAMETER_BOXPLOT_MINIMUM_TIME).getDoubleValue();
        float maxTime = getParameter(PARAMETER_BOXPLOT_MAXIMUM_TIME).getDoubleValue();
        if (maxTime <= minTime)
        {
            std::stringstream messageStream;
            messageStream << "Invalid values: \"" << PARAMETER_BOXPLOT_MINIMUM_TIME 
                << "\" must be less than \"" << PARAMETER_BOXPLOT_MAXIMUM_TIME << "\"";
            return messageStream.str();
        }

        float minValue = getParameter(PARAMETER_BOXPLOT_MINIMUM_VALUE).getDoubleValue();
        float maxValue = getParameter(PARAMETER_BOXPLOT_MAXIMUM_VALUE).getDoubleValue();
        if (maxValue <= minValue)
        {
            std::stringstream messageStream;
            messageStream << "Invalid values: \"" << PARAMETER_BOXPLOT_MINIMUM_VALUE 
                << "\" must be less than \"" << PARAMETER_BOXPLOT_MAXIMUM_VALUE << "\"";
            return messageStream.str();
        }
    }

    return "";
}
