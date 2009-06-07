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

#include <evaluator/Histogram.h>

std::auto_ptr<Histogram> Histogram::compute(
        const PT::CellSelection& constCellSelection,
        int featureIndex, 
        short time,
        float intervalSize)
{
    // the cell selection is not changed in this method
    PT::CellSelection cellSelection = const_cast<PT::CellSelection&>(constCellSelection);
    
    std::auto_ptr<Histogram> histogram(new Histogram());
    histogram->featureIndex = featureIndex;
    histogram->frequencyRange = PT::Range<long>(0, 0);

    typedef std::map<int, Histogram::Interval> IntervalMap;
    IntervalMap intervalIndexMap;

    PT::CellSelection::CellIterator cellIt = cellSelection.getCellStart();
    PT::CellSelection::CellIterator cellEnd = cellSelection.getCellEnd();
    for (;cellIt != cellEnd; ++cellIt)
    {
        PT::Cell* cell = *cellIt;

        PT::CellObservation* observation = cell->getObservation(time);
        if (observation != 0 && observation->isFeatureAvailable(featureIndex))
        {
            float featureValue = observation->getFeature(featureIndex);

            int intervalIndex = (int)(featureValue / intervalSize);

            IntervalMap::iterator intervalIndexMapIt = intervalIndexMap.find(intervalIndex);
            if (intervalIndexMapIt == intervalIndexMap.end())
            {
                // create new interval
                Histogram::Interval interval;
                interval.valueRange = PT::Range<float>(intervalIndex * intervalSize, 
                        (intervalIndex + 1) * intervalSize);
                interval.frequency = 1;

                std::pair<int, Histogram::Interval> mapValue(intervalIndex, interval);
                intervalIndexMap.insert(mapValue);
            }
            else
            {
                // udate existing interval
                ((*intervalIndexMapIt).second).frequency++;
            }
        }
    }

    // initialize histogram->intervals and update frequency range
    IntervalMap::iterator intervalIndexMapIt = intervalIndexMap.begin();
    IntervalMap::iterator intervalIndexMapEnd = intervalIndexMap.end();
    for (;intervalIndexMapIt != intervalIndexMapEnd; ++intervalIndexMapIt)
    {
        const Histogram::Interval& interval = (*intervalIndexMapIt).second;
        histogram->intervals.push_back(interval);
        histogram->frequencyRange.update(interval.frequency);
    }

    return histogram;
}

std::auto_ptr<HistogramSeries> HistogramSeries::compute(
        const PT::CellSelection& constCellSelection,
        int featureIndex,
        float intervalSize)
{
    // the cell selection is not changed in this method
    PT::CellSelection& cellSelection = const_cast<PT::CellSelection&>(constCellSelection);
    
    // maps interval index to interval
    typedef std::map<int, Histogram::Interval> IntervalIndexMap;

    // maps time step to IntervalIndexMap
    typedef std::map<int, IntervalIndexMap> IntervalTimeMap;

    IntervalTimeMap intervalTimeMap;

    PT::CellSelection::CellIterator cellIt = cellSelection.getCellStart();
    PT::CellSelection::CellIterator cellEnd = cellSelection.getCellEnd();
    for (;cellIt != cellEnd; ++cellIt)
    {
        PT::Cell* cell = *cellIt;

        PT::Cell::ObservationIterator observationIt = cell->getObservationStart();
        PT::Cell::ObservationIterator observationEnd = cell->getObservationEnd();
        for (; observationIt != observationEnd; ++observationIt)
        {
            PT::CellObservation* observation = *observationIt;

            if (observation->isFeatureAvailable(featureIndex))
            {
                float featureValue = observation->getFeature(featureIndex);
                int time = observation->getTime();

                // find interval map or create on if none exists for the current time step
                IntervalTimeMap::iterator intervalTimeMapIt = intervalTimeMap.find(time);
                if (intervalTimeMapIt == intervalTimeMap.end())
                {
                    std::pair<int, IntervalIndexMap> mapValue(time, IntervalIndexMap());
                    std::pair<IntervalTimeMap::iterator, bool> insResult =
                        intervalTimeMap.insert(mapValue);

                    // assert that insertion took place
                    assert(insResult.second);
                    intervalTimeMapIt = insResult.first;
                }
                IntervalIndexMap& intervalIndexMap = (*intervalTimeMapIt).second;

                // update interval index map
                {
                    int intervalIndex = (int)(featureValue / intervalSize);

                    IntervalIndexMap::iterator intervalIndexMapIt = intervalIndexMap.find(intervalIndex);
                    if (intervalIndexMapIt == intervalIndexMap.end())
                    {
                        // create new interval
                        Histogram::Interval interval;
                        interval.valueRange = PT::Range<float>(intervalIndex * intervalSize, 
                                (intervalIndex + 1) * intervalSize);
                        interval.frequency = 1;

                        std::pair<int, Histogram::Interval> mapValue(intervalIndex, interval);
                        intervalIndexMap.insert(mapValue);
                    }
                    else
                    {
                        // udate existing interval
                        ((*intervalIndexMapIt).second).frequency++;
                    }
                }
            }
        }
    }

    std::auto_ptr<HistogramSeries> histogramSeries(new HistogramSeries());

    IntervalTimeMap::iterator intervalTimeMapIt = intervalTimeMap.begin();
    IntervalTimeMap::iterator intervalTimeMapEnd = intervalTimeMap.end();
    for (;intervalTimeMapIt != intervalTimeMapEnd; ++intervalTimeMapIt)
    {
        int time = (*intervalTimeMapIt).first;
        IntervalIndexMap& intervalIndexMap = (*intervalTimeMapIt).second;

        // create new histogram and add to histogram series
        std::pair<int, Histogram> histogramMapValue(time, Histogram());
        std::pair<HistogramSeries::HistogramMap::iterator, bool> insResult =
            histogramSeries->histogramMap.insert(histogramMapValue);

        // assert that histogram was inserted successfully
        assert(insResult.second);
        Histogram& histogram = (*insResult.first).second;

        // initialize histogram
        histogram.featureIndex = featureIndex;
        histogram.frequencyRange = PT::Range<long>(0, 0);

        // initialize histogram intervals
        IntervalIndexMap::iterator intervalIndexMapIt = intervalIndexMap.begin();
        IntervalIndexMap::iterator intervalIndexMapEnd = intervalIndexMap.end();
        for (;intervalIndexMapIt != intervalIndexMapEnd; ++intervalIndexMapIt)
        {
            const Histogram::Interval& interval = (*intervalIndexMapIt).second;

            histogram.intervals.push_back(interval);
            histogram.frequencyRange.update(interval.frequency);

            histogramSeries->valueRange.update(interval.valueRange);
        }

        // update histogram series ranges
        histogramSeries->timeRange.update(time);
        histogramSeries->frequencyRange.update(histogram.frequencyRange);
    }

    return histogramSeries;
}
