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

#include <io/export.h>

#include <fstream>

#include <common.h>

namespace PT
{

void exportCSV(const Analysis& analysis, const CellSelection& cellSelection, const char* filepath)
{
    assert(filepath != 0);

    const std::vector<std::string>& featureNames = analysis.getMetadata().featureNames;

    std::ofstream file;
    file.open(filepath);           

    file << "CellID;Well;Position;Slide;Time;";       

    // write feature names
    std::vector<std::string>::const_iterator featureNameIt = featureNames.begin();
    std::vector<std::string>::const_iterator featureNameEnd = featureNames.end();
    for (; featureNameIt != featureNameEnd; ++featureNameIt)
    {
        file << (*featureNameIt).c_str() << ";";
    }

    // write cell observations
    PT::CellSelection::CellIterator cellIt = const_cast<CellSelection&>(cellSelection).getCellStart();
    PT::CellSelection::CellIterator cellEnd = const_cast<CellSelection&>(cellSelection).getCellEnd();
    for (;cellIt != cellEnd; ++cellIt)
    {
        PT::Cell* cell = *cellIt;

        const ImageLocation& location = cell->getLocation();
        const ImageSeries& imageSeries = analysis.getImageSeries(location);
        ImageSeries::TimeRange timeRange = imageSeries.timeRange;

        // iterate over all time steps
        for (short time = timeRange.min; time <= timeRange.max; ++time)
        {
            PT::CellObservation* observation = cell->getObservation(time);
            if (observation != 0)
            { 
                file << "\n";
                file << cell->getId() << ";";
                file << location.well << ";" << location.position << ";" << location.slide << ";";
                file << time << ";";

                for (int featureIndex = 0; featureIndex < featureNames.size(); ++ featureIndex)
                {
                    if (observation->isFeatureAvailable(featureIndex))
                    {
                        file << observation->getFeature(featureIndex);                    
                    }
                    file << ";";
                }
            }
        }
    }

    file << "\n";       

    file.close();

    // throw an exception if an error occured
    if( !file ) {
        throw IOException();
    }
}

}
