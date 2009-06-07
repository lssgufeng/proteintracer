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

#ifndef Histogram_h
#define Histogram_h

#include <common.h>
#include <Analysis.h>

struct Histogram
{
    struct Interval
    {
        // minimum value is inclusive, maximum value is exclusive
        PT::Range<float> valueRange;

        long frequency;
    };

    typedef std::vector<Interval> IntervalVector;

    int featureIndex;

    PT::Range<long> frequencyRange;

    IntervalVector intervals;

    static std::auto_ptr<Histogram> compute(
            const PT::CellSelection& cellSelection,
            int featureIndex, 
            short time,
            float intervalSize);

};

struct HistogramSeries
{
    typedef std::map<int, Histogram> HistogramMap;

    HistogramMap histogramMap;

    PT::Range<short> timeRange;

    PT::Range<float> valueRange;

    PT::Range<float> frequencyRange;

    static std::auto_ptr<HistogramSeries> compute(
            const PT::CellSelection& cellSelection,
            int featureIndex,
            float intervalSize);
};

#endif
