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

#include <evaluator/FeatureStatistics.h>

#include <algorithm>

std::auto_ptr<FeatureStatisticsSeries> FeatureStatisticsSeries::compute(
        const PT::CellSelection& constCellSelection,
        int featureIndex)
{
    // the cell selection is not changed in this method
    PT::CellSelection cellSelection = const_cast<PT::CellSelection&>(constCellSelection);

    std::auto_ptr<FeatureStatisticsSeries> series = 
        std::auto_ptr<FeatureStatisticsSeries>(new FeatureStatisticsSeries());

    typedef FeatureStatisticsSeries::FeatureStatisticsMap FeatureStatisticsMap;
    FeatureStatisticsMap& featureStatisticsMap = series->featureStatisticsMap;

    // maps time step to feature value vector
    typedef std::map<int, std::vector<float> > FeatureValueVectorMap;
    FeatureValueVectorMap featureValueVectorMap;

    // iterate over all cells and observations to initialize feature statistics
    PT::CellSelection::CellIterator cellIt = cellSelection.getCellStart();
    PT::CellSelection::CellIterator cellEnd = cellSelection.getCellEnd();
    for (; cellIt != cellEnd; ++cellIt)
    {
        PT::Cell* cell = *cellIt;

        PT::Cell::ObservationIterator observationIt = cell->getObservationStart();
        PT::Cell::ObservationIterator observationEnd = cell->getObservationEnd();
        for (; observationIt != observationEnd; ++observationIt)
        {
            PT::CellObservation* observation = *observationIt;

            // proceed only if feature is available
            if (! observation->isFeatureAvailable(featureIndex))
                continue;

            int time = observation->getTime();
            float featureValue = observation->getFeature(featureIndex);

            // update time course ranges
            series->timeRange.update(time);
            series->valueRange.update(featureValue);

            FeatureStatisticsMap::iterator featureStatisticsIt = featureStatisticsMap.find(time);
            // add feature statistics for time if not already existing
            if (featureStatisticsIt == featureStatisticsMap.end())
            {
                FeatureStatistics featureStatistics;

                featureStatistics.valueRange.update(featureValue);
                featureStatistics.count = 1;
                featureStatistics.mean = featureValue;
                featureStatistics.standardDeviation = featureValue * featureValue;

                std::pair<short, FeatureStatistics> mapValue(time, featureStatistics);
                featureStatisticsMap.insert(mapValue);
            }
            // update feature statistics for time if already existing
            else
            {
                FeatureStatistics& featureStatistics = (*featureStatisticsIt).second;

                featureStatistics.valueRange.update(featureValue);
                featureStatistics.count++;
                featureStatistics.mean += featureValue;
                featureStatistics.standardDeviation += featureValue * featureValue;
            }

            // add feature value to feature value vector for computing the
            // quartiles and median later
            FeatureValueVectorMap::iterator featureValueVectorIt =
                featureValueVectorMap.find(time);
            if (featureValueVectorIt == featureValueVectorMap.end())
            {
                std::pair<short, std::vector<float> > mapValue(time, std::vector<float>());
                std::pair<FeatureValueVectorMap::iterator, bool> insResult =
                    featureValueVectorMap.insert(mapValue);
                assert(insResult.second);
                featureValueVectorIt = insResult.first;
            }
            (*featureValueVectorIt).second.push_back(featureValue);

        }
    }

    // compute mean, standard deviation, quartiles and median
    FeatureStatisticsMap::iterator featureStatisticsIt = featureStatisticsMap.begin();
    FeatureStatisticsMap::iterator featureStatisticsEnd = featureStatisticsMap.end();
    for (; featureStatisticsIt != featureStatisticsEnd; ++featureStatisticsIt)
    {
        int time = (*featureStatisticsIt).first;
        FeatureStatistics& featureStatistics = (*featureStatisticsIt).second;

        // compute mean and standard deviation
        {
            long count = featureStatistics.count;
            float sum = featureStatistics.mean;
            float sumOfSquares = featureStatistics.standardDeviation;

            featureStatistics.mean = sum / count;
            featureStatistics.standardDeviation = 
                sqrt((sumOfSquares - (sum * sum / count)) / count);
        }

        // compute lower quartile, median and upper quartile
        {
            // retrieve feature value vector
            FeatureValueVectorMap::iterator featureValueVectorIt =
                featureValueVectorMap.find(time);
            assert (featureValueVectorIt != featureValueVectorMap.end());
            std::vector<float>& featureValueVector = (*featureValueVectorIt).second;
            assert(featureValueVector.size() == featureStatistics.count);

            // sort feature value vector
            sort(featureValueVector.begin(), featureValueVector.end());

            int medianIndex = featureValueVector.size() / 2;
            if ((featureValueVector.size() % 2) == 1) // odd size
                featureStatistics.median = featureValueVector[medianIndex];
            else // even size
                featureStatistics.median = (featureValueVector[medianIndex] +
                        featureValueVector[medianIndex-1]) / 2.0;

            if (featureValueVector.size() >=4)
            {
                int lowerQuartileIndex = medianIndex / 2;
                int upperQuartileIndex = featureValueVector.size() - (medianIndex -lowerQuartileIndex);
                if ((medianIndex % 2) == 1) // odd median index
                {
                    featureStatistics.lowerQuartile = featureValueVector[lowerQuartileIndex];
                    featureStatistics.upperQuartile = featureValueVector[upperQuartileIndex];
                }
                else // even median index
                {
                    featureStatistics.lowerQuartile = (featureValueVector[lowerQuartileIndex] +
                            featureValueVector[lowerQuartileIndex-1]) / 2.0;
                    featureStatistics.upperQuartile = (featureValueVector[upperQuartileIndex] +
                            featureValueVector[upperQuartileIndex-1]) / 2.0;
                }
            }
        }
    }


    return series;
}

