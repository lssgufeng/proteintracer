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

#ifndef ChartViewer_h
#define ChartViewer_h

#include <FL/Fl_Widget.H>

#include <Analysis.h>
#include <evaluator/ChartParameters.h>
#include <evaluator/FeatureStatistics.h>
#include <evaluator/Histogram.h>

struct ChartData
{
    std::auto_ptr<PT::Analysis> analysis;

    std::auto_ptr<PT::CellSelection> cellSelection;

    short time; 

    std::auto_ptr<ChartParameters> parameters;
};

class ChartViewerEvent
{
public:
    enum Type 
    {
		STATUS_MESSAGE,
		CELL_SELECTION
    };
    
    Type id;

    std::string statusMessage;
};

class ChartViewer : public Fl_Widget, public PT::EventGenerator<ChartViewerEvent>
{
public:

    ChartViewer(int x, int y, int width, int height);

    int handle(int event);

    /**
     * General event handler methods.
     */
    int mouseClicked();
    int mouseReleased();
    int mouseDragged();
    int mouseMoved();
    int mouseLeft();
    int keyDown();

    void setData(ChartData* data);

    const ChartData* getData()
    {
        return data_;
    }

    void draw();

private:

    typedef PT::Range<int> PixelRange;

    typedef PT::Range<float> ValueRange;

    struct ChartAxis 
    {
        PixelRange pixelRange;

        ValueRange valueRange;

        float unit;

        std::string name;
    };

    bool isDrawable() const
    {
        return (xAxis_.pixelRange.size() > 0) 
            && (xAxis_.valueRange.size() > 0)
            && (yAxis_.pixelRange.size() > 0) 
            && (yAxis_.valueRange.size() > 0);
    }

    static float computeAxisUnit(
            int maxLabelDimension,
            const PixelRange& pixelRange,
            const ValueRange& valueRange);

    void initScatterplot();

    void initLineChart();

    void initHistogram();

    void initBoxplot();

    void performScatterplotSelection();

    void drawCoordinateSystem(bool drawGrid);

    void drawScatterplot();

    void drawLineChart();

    void drawHistogram();

    void drawBoxplot();

    ChartData* data_;

    std::auto_ptr<FeatureStatisticsSeries> featureStatisticsSeries_;

    std::auto_ptr<HistogramSeries> histogramSeries_;

    ChartAxis xAxis_;

    ChartAxis yAxis_;

    PT::IntPoint clickPos_;

    PT::IntPoint dragPos_;

};

#endif
