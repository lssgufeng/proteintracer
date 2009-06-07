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

#include <evaluator/ChartViewer.h>

#include <FL/Fl.H>
#include <FL/fl_draw.H>

const int COORDINATE_FONT_FACE = FL_HELVETICA;
const int COORDINATE_FONT_SIZE = 12;
const int COORDINATE_SYSTEM_MARGIN = 16;
const int MAX_LABEL_WIDTH = 76;
const int TEXT_MARGIN_X = 10;
const int TEXT_MARGIN_Y = 8;

PT::Range<float> computeFeatureRange(const PT::Analysis& constAnalysis, int featureIndex)
{
    PT::Analysis& analysis = const_cast<PT::Analysis&>(constAnalysis);

    PT::Range<float> featureRange;

    // it does not make sense to calculate the feature range 
    // of an empty set
    if (analysis.getNumberOfCells() == 0)
        return featureRange;

    PT::Analysis::CellIterator cellIt = analysis.getCellStart();
    PT::Analysis::CellIterator cellEnd = analysis.getCellEnd();
    for (;cellIt != cellEnd; ++cellIt)
    {
        PT::Cell* cell = *cellIt;

        PT::Cell::ObservationIterator observationIt = cell->getObservationStart();
        PT::Cell::ObservationIterator observationEnd = cell->getObservationEnd();
        for (;observationIt != observationEnd; ++observationIt)
        {
            PT::CellObservation* observation = *observationIt;

            if (observation->isFeatureAvailable(featureIndex))
            {
                float featureValue = observation->getFeature(featureIndex);
                featureRange.update(featureValue);
            }
        }
    }

    return featureRange;
}

PT::Range<float> computeTimeRange(const PT::Analysis& analysis)
{
    PT::Range<float> timeRange;

    PT::ImageSeriesSet::ImageSeriesConstIterator imageSeriesIt = 
        analysis.getImageSeriesStart(); 
    PT::ImageSeriesSet::ImageSeriesConstIterator imageSeriesEnd = 
        analysis.getImageSeriesEnd(); 
    for (;imageSeriesIt != imageSeriesEnd; ++imageSeriesIt)
    {
        const PT::ImageSeries& imageSeries = *imageSeriesIt;
        timeRange.update(imageSeries.timeRange);
    }

    return timeRange;
}

ChartViewer::ChartViewer(int x, int y, int width, int height) :
    Fl_Widget(x, y, width, height),
	data_(0),
    xAxis_(),
    yAxis_()
{
}

int ChartViewer::handle(int event)
{
    switch(event)
    {
    case FL_PUSH:
        return mouseClicked();
    case FL_RELEASE:
        return mouseReleased();
    case FL_DRAG:
        return mouseDragged();
    case FL_MOVE:
        return mouseMoved();
    case FL_ENTER:
        return 1;
    case FL_LEAVE:
        return mouseLeft();
    case FL_KEYDOWN:
        return keyDown();
    default:
        return Fl_Widget::handle(event);
    }
}

int ChartViewer::mouseClicked()
{
    Fl::focus(this);

    clickPos_.x = Fl::event_x();
    clickPos_.y = Fl::event_y();
    dragPos_ = clickPos_;

    return 1;
}

int ChartViewer::mouseReleased()
{
    if (data_ != 0 && isDrawable())
    {
        switch (data_->parameters->getChartType())
        {
        case CHART_TYPE_SCATTERPLOT:
            performScatterplotSelection();
            break;
        }
    }

    clickPos_ = PT::IntPoint();
    dragPos_ = clickPos_;

    redraw();

    return 1;
}

int ChartViewer::mouseDragged()
{
    dragPos_.x = Fl::event_x();
    dragPos_.y = Fl::event_y();

    redraw();

    return 1;
}

int ChartViewer::mouseMoved()
{
    int x = Fl::event_x();
    int y = Fl::event_y();

    std::stringstream statusMessageStream;

    if (data_ != 0 && isDrawable()
        && xAxis_.pixelRange.contains(x) 
        && yAxis_.pixelRange.contains(y))
    {
        float featureValueX = xAxis_.valueRange.map(x, xAxis_.pixelRange);
        float featureValueY = yAxis_.valueRange.mapReverse(y, yAxis_.pixelRange);

        // round feature value x if axis is discrete
        assert(data_->parameters.get() != 0);
        ChartType chartType = data_->parameters->getChartType();
        if (chartType == CHART_TYPE_LINE_CHART ||
            chartType == CHART_TYPE_BOXPLOT)
        {
			// floor(x + 0.5) is a workaround for round() 
			// since round() does not work with Visual C++
            featureValueX = floor(featureValueX + 0.5);
        }
        if (chartType == CHART_TYPE_HISTOGRAM)
        {
			// floor(x + 0.5) is a workaround for round() 
			// since round() does not work with Visual C++
            featureValueY = floor(featureValueY + 0.5);
        }

        statusMessageStream << featureValueX << ",  " << featureValueY;
    }

    // notify event handler
    ChartViewerEvent event;
    event.id = ChartViewerEvent::STATUS_MESSAGE;
    event.statusMessage = statusMessageStream.str();
    notifyEventHandler(event);

    return 1;
}

int ChartViewer::mouseLeft()
{
    return 0;
}

int ChartViewer::keyDown()
{
    return 0;
}

void ChartViewer::setData(ChartData* data)
{
    this->data_ = data;

    // reset members
    {
        featureStatisticsSeries_.reset();
        histogramSeries_.reset();

        xAxis_ = ChartAxis();
        yAxis_ = ChartAxis();
    }

    // initialize members
    if (this->data_ != 0)
    {
        assert(this->data_->analysis.get() != 0);
        assert(this->data_->cellSelection.get() != 0);
        assert(this->data_->parameters.get() != 0);
        assert(this->data_->time >= 0);

        ChartParameters* parameters = this->data_->parameters.get();

        switch (parameters->getChartType())
        {
            case CHART_TYPE_SCATTERPLOT:
                initScatterplot();
                break;
            case CHART_TYPE_LINE_CHART:
                initLineChart();
                break;
            case CHART_TYPE_HISTOGRAM:
                initHistogram();
                break;
            case CHART_TYPE_BOXPLOT:
                initBoxplot();
                break;
        }
    }

    redraw();
}

void ChartViewer::performScatterplotSelection()
{
    assert(data_ != 0);
    assert(data_->cellSelection.get() != 0);
    assert(data_->time >= 0);
    assert(isDrawable());

    if (clickPos_ == dragPos_)
        return;

    int x =(clickPos_.x < dragPos_.x) ? clickPos_.x : dragPos_.x;
    int y =(clickPos_.y < dragPos_.y) ? clickPos_.y : dragPos_.y;
    int a = abs(dragPos_.x - clickPos_.x) / 2;
    int b = abs(dragPos_.y - clickPos_.y) / 2;

    float centerX = x + a;
    float centerY = y + b;

    float aSquared = a * a;
    float bSquared = b * b;

    ChartParameters* parameters = data_->parameters.get();
    int featureIndexX = parameters->getParameter(
            PARAMETER_SCATTERPLOT_FEATURE_X).getListIndex();
    int featureIndexY = parameters->getParameter(
            PARAMETER_SCATTERPLOT_FEATURE_Y).getListIndex();

    PT::CellSelection* currentSelection = data_->cellSelection.get();
    std::auto_ptr<PT::CellSelection> newSelection =
        std::auto_ptr<PT::CellSelection>(new PT::CellSelection());

    PT::CellSelection::CellIterator cellIt = currentSelection->getCellStart();
    PT::CellSelection::CellIterator cellEnd = currentSelection->getCellEnd();
    for (;cellIt != cellEnd; ++cellIt)
    {
        PT::Cell* cell = *cellIt;

        PT::CellObservation* observation = cell->getObservation(data_->time);
        if (observation != 0 && 
            observation->isFeatureAvailable(featureIndexX) &&
            observation->isFeatureAvailable(featureIndexY))
        {
            float featureValueX = observation->getFeature(featureIndexX);
            float featureValueY = observation->getFeature(featureIndexY);

            PT::IntPoint dataPoint = PT::IntPoint(
                xAxis_.pixelRange.map(featureValueX, xAxis_.valueRange),
                yAxis_.pixelRange.mapReverse(featureValueY, yAxis_.valueRange));

            float dxSquared = dataPoint.x - centerX;
            dxSquared *= dxSquared;

            float dySquared = dataPoint.y - centerY;
            dySquared *= dySquared;

            if ( (dxSquared / aSquared) + (dySquared / bSquared) <= 1 )
                newSelection->addCell(cell);
        }
    }

    data_->cellSelection = newSelection;
    setData(data_);

    // notify event handler
    ChartViewerEvent event;
    event.id = ChartViewerEvent::CELL_SELECTION;
    notifyEventHandler(event);
}

float ChartViewer::computeAxisUnit(
        int maxLabelDimension,
        const PixelRange& pixelRange,
        const ValueRange& valueRange)
{
    // make some assertions
    assert(maxLabelDimension > 0);
    assert(pixelRange.size() > 0);
    assert(valueRange.size() > 0);

    float unit = 0;

    int maxNumberOfLabels = (int)(pixelRange.size() / (float) maxLabelDimension);

    // compute the greatest value which is an integer power of ten and
    // smaller than the value range
    // hint: log 10 x = ln x / ln 10
    float base =  (float) pow(10.0, floor( log(valueRange.size()) / log(10.0) ));

    float smallestPossibleUnit = valueRange.size();
    while (unit == 0) 
    {
        for (int i = 5; i >= 1; i--) 
        {
            // unit must be a multiple of a divisor of ten
            // only such units are intuitive to recognize
            if (10 % i != 0) continue;

            float currentUnit = base * i;

            int numLabels = (int)(valueRange.size() / currentUnit);
            if (numLabels <= maxNumberOfLabels) 
            {
                smallestPossibleUnit = currentUnit;
            }
            else
            {
                unit = smallestPossibleUnit;
                break;
            }
        }
        base /= 10.0;
    }

    return unit;
}

void ChartViewer::initScatterplot()
{
    // make some assertions
    assert(data_ != 0);
    assert(data_->analysis.get() != 0);
    assert(data_->cellSelection.get() != 0);
    assert(data_->parameters.get() != 0);

    PT::Analysis* analysis = data_->analysis.get();
    PT::CellSelection* cellSelection = data_->cellSelection.get();
    ChartParameters* parameters = data_->parameters.get();

    bool autoParameters = parameters->getParameter(
            PARAMETER_SCATTERPLOT_AUTO_PARAMETERS).getBoolValue();

    // init x-axis
    {
        int featureIndex = parameters->getParameter(
                PARAMETER_SCATTERPLOT_FEATURE_X).getListIndex();

        if (autoParameters)
        {
            xAxis_.valueRange = computeFeatureRange(*analysis, featureIndex);
            xAxis_.valueRange.extend(.05);
            if (xAxis_.valueRange.size() <= 0) xAxis_.valueRange = ValueRange(0, 1);
        }
        else
        {
            float min = parameters->getParameter(
                    PARAMETER_SCATTERPLOT_MINIMUM_X).getDoubleValue();
            float max = parameters->getParameter(
                    PARAMETER_SCATTERPLOT_MAXIMUM_X).getDoubleValue();

            assert(min < max);

            xAxis_.valueRange = ValueRange(min, max);
        }

        const std::vector<std::string>& featureList = parameters->getParameter(
                PARAMETER_SCATTERPLOT_FEATURE_X).getList();
        xAxis_.name = featureList[featureIndex];
    }

    // init y-axis
    {
        int featureIndex = parameters->getParameter(
                PARAMETER_SCATTERPLOT_FEATURE_Y).getListIndex();

        if (autoParameters)
        {
            yAxis_.valueRange = computeFeatureRange(*analysis, featureIndex);
            yAxis_.valueRange.extend(.05);
            if (yAxis_.valueRange.size() <= 0) yAxis_.valueRange = ValueRange(0, 1);
        }
        else
        {
            float min = parameters->getParameter(
                    PARAMETER_SCATTERPLOT_MINIMUM_Y).getDoubleValue();
            float max = parameters->getParameter(
                    PARAMETER_SCATTERPLOT_MAXIMUM_Y).getDoubleValue();

            assert(min < max);

            yAxis_.valueRange = ValueRange(min, max);
        }

        const std::vector<std::string>& featureList = parameters->getParameter(
                PARAMETER_SCATTERPLOT_FEATURE_Y).getList();
        yAxis_.name = featureList[featureIndex];
    }
}

void ChartViewer::initLineChart()
{
    // make some assertions
    assert(data_ != 0);
    assert(data_->analysis.get() != 0);
    assert(data_->cellSelection.get() != 0);
    assert(data_->parameters.get() != 0);

    PT::Analysis* analysis = data_->analysis.get();
    PT::CellSelection* cellSelection = data_->cellSelection.get();
    ChartParameters* parameters = data_->parameters.get();

    int featureIndex = parameters->getParameter(
            PARAMETER_LINE_CHART_FEATURE).getListIndex();
    bool autoParameters = parameters->getParameter(
            PARAMETER_LINE_CHART_AUTO_PARAMETERS).getBoolValue();

    // init feature statistics series
    featureStatisticsSeries_ = FeatureStatisticsSeries::compute(
            *cellSelection, featureIndex);

    // init x-axis
    {
        if (autoParameters)
        {
            xAxis_.valueRange = computeTimeRange(*analysis);
            xAxis_.valueRange.extend(.05);
            if (xAxis_.valueRange.size() <= 0) xAxis_.valueRange = ValueRange(0, 1);
        }
        else
        {
            float min = data_->parameters->getParameter(
                    PARAMETER_LINE_CHART_MINIMUM_TIME).getDoubleValue();
            float max = data_->parameters->getParameter(
                    PARAMETER_LINE_CHART_MAXIMUM_TIME).getDoubleValue();

            assert(min < max);

            xAxis_.valueRange = ValueRange(min, max);
        }

        xAxis_.name = "Time";
    }

    // init y-axis
    {
        if (autoParameters)
        {
            yAxis_.valueRange = computeFeatureRange(*analysis, featureIndex);
            yAxis_.valueRange.extend(.05);
            if (yAxis_.valueRange.size() <= 0) yAxis_.valueRange = ValueRange(0, 1);
        }
        else
        {
            float min = data_->parameters->getParameter(
                    PARAMETER_LINE_CHART_MINIMUM_VALUE).getDoubleValue();
            float max = data_->parameters->getParameter(
                    PARAMETER_LINE_CHART_MAXIMUM_VALUE).getDoubleValue();

            assert(min < max);

            yAxis_.valueRange = ValueRange(min, max);
        }

        const std::vector<std::string>& featureList = 
            parameters->getParameter(PARAMETER_LINE_CHART_FEATURE).getList();
        yAxis_.name = featureList[featureIndex];
    }
}

void ChartViewer::initHistogram()
{
    // make some assertions
    assert(data_ != 0);
    assert(data_->analysis.get() != 0);
    assert(data_->cellSelection.get() != 0);
    assert(data_->parameters.get() != 0);
    assert(data_->time >= 0);

    PT::Analysis* analysis = data_->analysis.get();
    PT::CellSelection* cellSelection = data_->cellSelection.get();
    ChartParameters* parameters = data_->parameters.get();

    int featureIndex = parameters->getParameter(
            PARAMETER_HISTOGRAM_FEATURE).getListIndex();
    bool autoParameters = parameters->getParameter(
            PARAMETER_HISTOGRAM_AUTO_PARAMETERS).getBoolValue();

    // init x-axis
    {
        if (autoParameters)
        {
            xAxis_.valueRange = computeFeatureRange(*analysis, featureIndex);
            xAxis_.valueRange.extend(.05);
            if (xAxis_.valueRange.size() <= 0) xAxis_.valueRange = ValueRange(0, 1);
        }
        else
        {
            float min = data_->parameters->getParameter(
                    PARAMETER_HISTOGRAM_MINIMUM_VALUE).getDoubleValue();
            float max = data_->parameters->getParameter(
                    PARAMETER_HISTOGRAM_MAXIMUM_VALUE).getDoubleValue();

            assert(min < max);

            xAxis_.valueRange = ValueRange(min, max);
        }

        const std::vector<std::string>& featureList = 
            parameters->getParameter(PARAMETER_HISTOGRAM_FEATURE).getList();
        xAxis_.name = featureList[featureIndex];
    }

    // init histogram series
    {
        float intervalSize;
        if (autoParameters)
            // apply a simple heuristic
            intervalSize = xAxis_.valueRange.size() / 50;
        else
            intervalSize = (float) parameters->getParameter(
                    PARAMETER_HISTOGRAM_INTERVAL_SIZE).getDoubleValue();

        // init histogram
        histogramSeries_ = HistogramSeries::compute(
                *cellSelection, featureIndex, intervalSize);
    }

    // init y-axis
    {
        if (autoParameters)
        {
            yAxis_.valueRange = histogramSeries_->frequencyRange;
            yAxis_.valueRange.max += yAxis_.valueRange.size() * 0.025;
            if (yAxis_.valueRange.size() <= 0) yAxis_.valueRange = ValueRange(0, 1);
        }
        else
        {
            float min = data_->parameters->getParameter(
                    PARAMETER_HISTOGRAM_MINIMUM_FREQUENCY).getDoubleValue();
            float max = data_->parameters->getParameter(
                    PARAMETER_HISTOGRAM_MAXIMUM_FREQUENCY).getDoubleValue();

            assert(min < max);

            yAxis_.valueRange = ValueRange(min, max);
        }

        yAxis_.name = "Frequency";
    }
}

void ChartViewer::initBoxplot()
{
    // make some assertions
    assert(data_ != 0);
    assert(data_->analysis.get() != 0);
    assert(data_->cellSelection.get() != 0);
    assert(data_->parameters.get() != 0);
    assert(data_->time >= 0);

    PT::Analysis* analysis = data_->analysis.get();
    PT::CellSelection* cellSelection = data_->cellSelection.get();
    ChartParameters* parameters = data_->parameters.get();

    int featureIndex = parameters->getParameter(
            PARAMETER_BOXPLOT_FEATURE).getListIndex();
    bool autoParameters = parameters->getParameter(
            PARAMETER_BOXPLOT_AUTO_PARAMETERS).getBoolValue();

    // init feature statistics series
    featureStatisticsSeries_ = FeatureStatisticsSeries::compute(
            *cellSelection, featureIndex);

    // init x-axis
    {
        if (autoParameters)
        {
            xAxis_.valueRange.min = (data_->time / 10) * 10 - 0.5;
            xAxis_.valueRange.max = xAxis_.valueRange.min + 10; 
        }
        else
        {
            float min = data_->parameters->getParameter(
                    PARAMETER_BOXPLOT_MINIMUM_TIME).getDoubleValue();
            float max = data_->parameters->getParameter(
                    PARAMETER_BOXPLOT_MAXIMUM_TIME).getDoubleValue();

            assert(min < max);

            xAxis_.valueRange = ValueRange(min, max);
        }

        xAxis_.name = "Time";
    }
    
    // init y-axis
    {
        if (autoParameters)
        {
            yAxis_.valueRange = computeFeatureRange(*analysis, featureIndex);
            yAxis_.valueRange.extend(.05);
            if (yAxis_.valueRange.size() <= 0) yAxis_.valueRange = ValueRange(0, 1);
        }
        else
        {
            float min = data_->parameters->getParameter(
                    PARAMETER_BOXPLOT_MINIMUM_VALUE).getDoubleValue();
            float max = data_->parameters->getParameter(
                    PARAMETER_BOXPLOT_MAXIMUM_VALUE).getDoubleValue();

            assert(min < max);

            yAxis_.valueRange = ValueRange(min, max);
        }

        const std::vector<std::string>& featureList = 
            parameters->getParameter(PARAMETER_BOXPLOT_FEATURE).getList();
        yAxis_.name = featureList[featureIndex];
    }
}

void ChartViewer::drawCoordinateSystem(bool drawGrid)
{
    // set font first, to ensure that font width/height measurements are
    // performed correctly
    fl_font(COORDINATE_FONT_FACE, COORDINATE_FONT_SIZE);

    // init x axis
    {
        int xPixelRangeMin = x() + MAX_LABEL_WIDTH + TEXT_MARGIN_X;
        int xPixelRangeMax = x() + w() - COORDINATE_SYSTEM_MARGIN;
        if (xPixelRangeMax < xPixelRangeMin) 
            xPixelRangeMin = xPixelRangeMax;
        xAxis_.pixelRange = PixelRange(xPixelRangeMin, xPixelRangeMax);

        if (xAxis_.pixelRange.size() > 0 && xAxis_.valueRange.size() > 0)
            xAxis_.unit = computeAxisUnit(
                    MAX_LABEL_WIDTH + TEXT_MARGIN_X, 
                    xAxis_.pixelRange, xAxis_.valueRange);
    }

    // init y axis
    {
        int yPixelRangeMin = y() + COORDINATE_SYSTEM_MARGIN;
        int yPixelRangeMax = y() + h() - TEXT_MARGIN_Y
            - fl_height() - TEXT_MARGIN_Y 
            - fl_height() - TEXT_MARGIN_Y;
        if (yPixelRangeMax < yPixelRangeMin) 
            yPixelRangeMin = yPixelRangeMax;
        yAxis_.pixelRange = PixelRange(yPixelRangeMin, yPixelRangeMax);

        if (yAxis_.pixelRange.size() > 0 && yAxis_.valueRange.size() > 0)
            yAxis_.unit = computeAxisUnit(
                    fl_height() + TEXT_MARGIN_Y,
                    yAxis_.pixelRange, yAxis_.valueRange);
    }

    // draw charts only if axes are initialized
    if (! isDrawable())
        return;

    // draw x axis lines and labels
    {
        // the value of the current line.
        float currentLineValue;

        // if the unit equals the value range then start with min value so
        // that the min value and max value are labelled
        if (xAxis_.unit == xAxis_.valueRange.size()) 
            currentLineValue = xAxis_.valueRange.min;
        else 
            currentLineValue = floor(xAxis_.valueRange.min / xAxis_.unit) * xAxis_.unit;

        // prevent -0 from being drawn
        if (currentLineValue == -0) currentLineValue = 0;

        // compute the value at which to stop drawing lines
        // add 1 / 100000 * unit to cope with numerical instabilities
        float maxLineValue = xAxis_.valueRange.max + xAxis_.unit * 0.00001;

        int y = yAxis_.pixelRange.max 
            + TEXT_MARGIN_Y + fl_height() - fl_descent();
        do 
        {
            int x = xAxis_.pixelRange.map(currentLineValue, xAxis_.valueRange);

            if (xAxis_.pixelRange.contains(x)) 
            {
                // draw line.
                if (drawGrid)
                {
                    fl_color(224, 224, 224);
                    fl_line(x, yAxis_.pixelRange.min, x, yAxis_.pixelRange.max);
                }

                // draw label
                {
                    // format label
                    std::stringstream labelstream;
                    labelstream << currentLineValue;
                    std::string label = labelstream.str();

                    // center label
                    x -= (int) ((int)fl_width(label.c_str()) / 2.0) - 1;

                    fl_color(0, 0, 0);
                    fl_draw(label.c_str(), x, y);
                }
            }

            // Increment the step of the current line by one unit.
            currentLineValue += xAxis_.unit;
        }
        while (currentLineValue <= maxLineValue);
    }

    // draw y axis lines and labels
    {
        // the value of the current line.
        float currentLineValue;

        // if the unit equals the value range then start with min value so that
        // the min value and max value are labelled
        if (yAxis_.unit == yAxis_.valueRange.size()) 
            currentLineValue = yAxis_.valueRange.min;
        else 
            currentLineValue = ceil(yAxis_.valueRange.min /  yAxis_.unit) * yAxis_.unit;

        // prevent -0 from being drawn
        if (currentLineValue == -0) currentLineValue = 0;

        // compute the value at which to stop drawing lines
        // add 1 / 100000 * unit to cope with numerical instabilities
        float maxLineValue = yAxis_.valueRange.max + yAxis_.unit * 0.00001;

        // the vertical distance from a grid line to the baseline of its label.
        int labelBaselineY = (int)(fl_height() / 2.0) - fl_descent();
        do 
        {
            int y = yAxis_.pixelRange.mapReverse(currentLineValue, yAxis_.valueRange);

            // draw only if the y coordinate is within the chart rectangle,
            if (yAxis_.pixelRange.contains(y)) 
            {
                // draw line
                if (drawGrid)
                {
                    fl_color(224, 224, 224);
                    fl_line(xAxis_.pixelRange.min, y, xAxis_.pixelRange.max, y);
                }

                // draw label
                {
                    std::stringstream labelstream;
                    labelstream << currentLineValue;
                    std::string label = labelstream.str();

                    int labelWidth = (int) fl_width(label.c_str());
                    int x = xAxis_.pixelRange.min - TEXT_MARGIN_X - labelWidth;

                    fl_color(0, 0, 0);
                    fl_draw(label.c_str(), x, y + labelBaselineY);
                }
            }
            else break;

            // Increment the value of the current line by one unit.
            currentLineValue += yAxis_.unit;
        }
        while (currentLineValue <= maxLineValue);
    }

    // draw frame
    {
        fl_color(0, 0, 0);
        fl_rect(xAxis_.pixelRange.min, 
                yAxis_.pixelRange.min, 
                xAxis_.pixelRange.size() + 1, 
                yAxis_.pixelRange.size() + 1);
    }

    // draw legend
    {
        std::stringstream legend1StringStream;
        legend1StringStream << "X-Axis:  " << xAxis_.name;
        std::string legend1String = legend1StringStream.str();
        const char* legend1 = legend1String.c_str();

        std::stringstream legend2StringStream;
        legend2StringStream << "Y-Axis:  " << yAxis_.name;
        std::string legend2String = legend2StringStream.str();
        const char* legend2 = legend2String.c_str();

        int legendWidth1 = (int) fl_width(legend1);
        int legendWidth2 = (int) fl_width(legend2);
        int gap = (w() - (legendWidth1 + legendWidth2)) / 3;

        int x1 = x() + gap;
        int x2 = x1 + legendWidth1 + gap;

        int y = this->y() + h() - TEXT_MARGIN_Y - fl_descent();

        fl_color(0, 0, 0);
        fl_draw(legend1, x1, y);
        fl_draw(legend2, x2, y);
    }
}

void ChartViewer::drawScatterplot()
{
    // make some assertions
    assert(data_ != 0);
    assert(data_->analysis.get() != 0);
    assert(data_->cellSelection.get() != 0);
    assert(data_->parameters.get() != 0);

    // init and draw coordate system
    drawCoordinateSystem(true);

    // draw charts only if coordinate system are initialized
    if (! isDrawable())
        return;

    PT::Analysis* analysis = data_->analysis.get();
    PT::CellSelection* cellSelection = data_->cellSelection.get();
    ChartParameters* parameters = data_->parameters.get();

    int featureIndexX = parameters->getParameter(
            PARAMETER_SCATTERPLOT_FEATURE_X).getListIndex();
    int featureIndexY = parameters->getParameter(
            PARAMETER_SCATTERPLOT_FEATURE_Y).getListIndex();

    // draw data points
    {
        fl_push_clip(
                xAxis_.pixelRange.min, 
                yAxis_.pixelRange.min, 
                xAxis_.pixelRange.size() + 1, 
                yAxis_.pixelRange.size() + 1);

        // set color to black
        fl_color(0, 0, 0);

        // draw data point of other time steps
        PT::CellSelection::CellIterator cellIt = cellSelection->getCellStart();
        PT::CellSelection::CellIterator cellEnd = cellSelection->getCellEnd();
        for (;cellIt != cellEnd; ++cellIt)
        {
            PT::Cell* cell = *cellIt;

            PT::CellObservation* observation = cell->getObservation(data_->time);
            if (observation != 0 && 
                observation->isFeatureAvailable(featureIndexX) &&
                observation->isFeatureAvailable(featureIndexY))
            {
                float featureValueX = observation->getFeature(featureIndexX);
                float featureValueY = observation->getFeature(featureIndexY);

                int x = xAxis_.pixelRange.map(featureValueX, xAxis_.valueRange);
                int y = yAxis_.pixelRange.mapReverse(featureValueY, yAxis_.valueRange);

                fl_rectf(x-2, y-2, 5, 5);
            }

        }

        fl_pop_clip();
    }

    // draw selection
    if (dragPos_ != clickPos_)
    {
        fl_color(255, 0, 0);
        fl_line_style(FL_SOLID, 0);

        int x =(clickPos_.x < dragPos_.x) ? clickPos_.x : dragPos_.x;
        int y =(clickPos_.y < dragPos_.y) ? clickPos_.y : dragPos_.y;
        int w = abs(dragPos_.x - clickPos_.x);
        int h = abs(dragPos_.y - clickPos_.y);

        fl_arc(x, y, w, h, 0, 360);
    }
}

void ChartViewer::drawLineChart()
{
    // make some assertions
    assert(data_ != 0);
    assert(data_->cellSelection.get() != 0);
    assert(data_->parameters.get() != 0);
    assert(this->featureStatisticsSeries_.get() != 0);

    // init and draw coordate system
    drawCoordinateSystem(true);

    // draw chart only if coordinate system are initialized
    if (! isDrawable())
        return;

    PT::Analysis* analysis = data_->analysis.get();
    PT::CellSelection* cellSelection = data_->cellSelection.get();
    ChartParameters* parameters = data_->parameters.get();
    FeatureStatisticsSeries* series = this->featureStatisticsSeries_.get();

    int featureIndex = parameters->getParameter(
            PARAMETER_LINE_CHART_FEATURE).getListIndex();

    // draw line
    {
        fl_push_clip(
                xAxis_.pixelRange.min + 1, 
                yAxis_.pixelRange.min + 1, 
                xAxis_.pixelRange.size() - 1, 
                yAxis_.pixelRange.size() - 1);

        // draw marking of current time
        {
            fl_color(224, 224, 224);
            fl_line_style(FL_SOLID, 5);

            int x = xAxis_.pixelRange.map((float)data_->time, xAxis_.valueRange);
            fl_line(x, yAxis_.pixelRange.min, x, yAxis_.pixelRange.max);
        }

        // set color to black and width to 0 (results in a nicer 1 pixel thin line)
        fl_color(0, 0, 0);
        fl_line_style(FL_SOLID, 0);

        // iterator over all time steps
        int lastX, lastY;
        for (int time = series->timeRange.min;
             time <= series->timeRange.max; 
             ++time)
        {
            typedef FeatureStatisticsSeries::FeatureStatisticsMap FeatureStatisticsMap;

            FeatureStatisticsMap::iterator featureStatisticsIt =
                series->featureStatisticsMap.find(time);

            // continue if there are no statistics for the current time step
            if (featureStatisticsIt == series->featureStatisticsMap.end())
                continue;

            // retrieve mean and standard deviation of current time step
            const FeatureStatistics& featureStatistics = (*featureStatisticsIt).second;

            int x = xAxis_.pixelRange.map((float)time, xAxis_.valueRange);
            int y = yAxis_.pixelRange.mapReverse(featureStatistics.mean, 
                    yAxis_.valueRange);

            // connect last data point and current data point with line
            if (time > series->timeRange.min)
                fl_line(lastX, lastY, x, y);

            // draw box
            fl_rectf(x-2, y-2, 5, 5);

            lastX = x;
            lastY = y;
        }

        fl_pop_clip();
    }
}

void ChartViewer::drawHistogram()
{
    // make some assertions
    assert(data_ != 0);
    assert(data_->cellSelection.get() != 0);
    assert(data_->parameters.get() != 0);
    assert(data_->time >= 0);
    assert(this->histogramSeries_.get() != 0);

    // init and draw coordate system
    drawCoordinateSystem(true);

    // draw chart only if coordinate system are initialized
    if (! isDrawable())
        return;

    HistogramSeries::HistogramMap::iterator histogramIt = 
        histogramSeries_->histogramMap.find(data_->time);

    // return if there's no histogram for the current time step
    if (histogramIt == histogramSeries_->histogramMap.end())
        return;

    Histogram& histogram = (*histogramIt).second;

    PT::CellSelection* cellSelection = data_->cellSelection.get();
    ChartParameters* parameters = data_->parameters.get();

    int featureIndex = parameters->getParameter(
            PARAMETER_HISTOGRAM_FEATURE).getListIndex();

    // draw histogram
    {
        fl_push_clip(
                xAxis_.pixelRange.min + 1, 
                yAxis_.pixelRange.min + 1, 
                xAxis_.pixelRange.size() - 1, 
                yAxis_.pixelRange.size() - 1);

        // set color to black
        fl_color(0, 0, 0);

        Histogram::IntervalVector::iterator intervalsIt = histogram.intervals.begin();
        Histogram::IntervalVector::iterator intervalsEnd = histogram.intervals.end();
        for (;intervalsIt != intervalsEnd; ++intervalsIt)
        {
            const Histogram::Interval& interval = *intervalsIt;

            int xMin = xAxis_.pixelRange.map(interval.valueRange.min, xAxis_.valueRange);
            int xMax = xAxis_.pixelRange.map(interval.valueRange.max, xAxis_.valueRange);

            int yMin = yAxis_.pixelRange.mapReverse((float) interval.frequency, yAxis_.valueRange);
            int yMax = yAxis_.pixelRange.mapReverse((float) 0.0, yAxis_.valueRange);

            if (xMax > xAxis_.pixelRange.min || xMin < xAxis_.pixelRange.max)
                fl_rectf(xMin, yMin, xMax - xMin + 1, yMax - yMin + 1);
        }

        fl_pop_clip();
    }
}

void ChartViewer::drawBoxplot()
{
    // make some assertions
    assert(data_ != 0);
    assert(data_->cellSelection.get() != 0);
    assert(data_->parameters.get() != 0);
    assert(this->featureStatisticsSeries_.get() != 0);

    // init and draw coordate system
    drawCoordinateSystem(false);

    // draw chart only if coordinate system are initialized
    if (! isDrawable())
        return;

    PT::CellSelection* cellSelection = data_->cellSelection.get();
    ChartParameters* parameters = data_->parameters.get();
    FeatureStatisticsSeries* series = this->featureStatisticsSeries_.get();

    int featureIndex = parameters->getParameter(
            PARAMETER_BOXPLOT_FEATURE).getListIndex();

    // draw boxplot
    {
        fl_push_clip(
                xAxis_.pixelRange.min + 1, 
                yAxis_.pixelRange.min + 1, 
                xAxis_.pixelRange.size() - 1, 
                yAxis_.pixelRange.size() - 1);

        int numBoxes = (int)xAxis_.valueRange.size() + 1;
        int boxGap = (int)(xAxis_.pixelRange.size() / numBoxes * 0.4);
        int boxWidth = (xAxis_.pixelRange.size() - (numBoxes - 1) * boxGap) / numBoxes;
        int halfBoxWidth = (boxWidth - 1) / 2;

        // draw marking of current time
        {
            fl_color(224, 224, 224);

            int x = xAxis_.pixelRange.map((float)data_->time, xAxis_.valueRange);
            fl_rectf(x - halfBoxWidth - boxGap / 2, yAxis_.pixelRange.min,
                    boxWidth + boxGap, yAxis_.pixelRange.max);
        }

        // iterator over all time steps
        for (int time = series->timeRange.min;
             time <= series->timeRange.max; 
             ++time)
        {
            typedef FeatureStatisticsSeries::FeatureStatisticsMap FeatureStatisticsMap;

            FeatureStatisticsMap::iterator featureStatisticsIt =
                series->featureStatisticsMap.find(time);

            // continue if there are no statistics for the current time step
            if (featureStatisticsIt == series->featureStatisticsMap.end())
                continue;

            // retrieve mean and standard deviation of current time step
            const FeatureStatistics& featureStatistics = (*featureStatisticsIt).second;

            int x = xAxis_.pixelRange.map((float)time, xAxis_.valueRange);

            // draw whiskers
            {
                int maxValueY = yAxis_.pixelRange.mapReverse(
                        featureStatistics.valueRange.max,
                        yAxis_.valueRange);
                int minValueY = yAxis_.pixelRange.mapReverse(
                        featureStatistics.valueRange.min,
                        yAxis_.valueRange);

                // set color to black and width to 0 (results in a nicer 1
                // pixel thin line)
                fl_color(0, 0, 0);

                fl_line(x, maxValueY, x, minValueY);

                fl_line(x-halfBoxWidth, maxValueY, x + halfBoxWidth, maxValueY);
                fl_line(x-halfBoxWidth, minValueY, x + halfBoxWidth, minValueY);
            }

            // draw quartile box
            if (featureStatistics.count >= 4)
            {
                int upperQuartileY = yAxis_.pixelRange.mapReverse(
                        featureStatistics.upperQuartile,
                        yAxis_.valueRange);
                int lowerQuartileY = yAxis_.pixelRange.mapReverse(
                        featureStatistics.lowerQuartile,
                        yAxis_.valueRange);

                int boxHeight = lowerQuartileY - upperQuartileY + 1;

                fl_color(255, 255, 255);

                fl_rectf(x - halfBoxWidth, upperQuartileY, boxWidth, boxHeight);
                fl_color(0, 0, 0);
                fl_rect(x - halfBoxWidth, upperQuartileY, boxWidth, boxHeight);
            }

            // draw median line
            {
                int y = yAxis_.pixelRange.mapReverse(featureStatistics.median, 
                        yAxis_.valueRange);

                fl_line(x - halfBoxWidth, y, x + halfBoxWidth, y);
            }
        }

        fl_pop_clip();
    }
}

void ChartViewer::draw()
{
    // fill background
    fl_draw_box(FL_FLAT_BOX, x(), y(), w(), h(), FL_WHITE);

    if (data_ == 0)
        return;

    switch (data_->parameters->getChartType())
    {
    case CHART_TYPE_SCATTERPLOT:
        drawScatterplot();
        break;
    case CHART_TYPE_LINE_CHART:
        drawLineChart();
        break;
    case CHART_TYPE_HISTOGRAM:
        drawHistogram();
        break;
    case CHART_TYPE_BOXPLOT:
        drawBoxplot();
        break;
    }

}
