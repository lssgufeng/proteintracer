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

#include <Scan.h>

#include <algorithm>
#include <iostream>
#include <iterator>
#include <stdexcept>

namespace PT
{

void Scan::addImageMetadata(const ImageMetadata& metadata)
{
    const ImageKey& imageKey = metadata.key;

    // make some assertions about metadata
    {
        assert(imageKey.isValid());
        assert(metadata.filepath != "");
    }

    // insert metadata
    {
        std::pair<ImageKey, ImageMetadata> p(imageKey, metadata);
        std::pair<ImageMetadataMap::iterator, bool> r = imageMetadataMap_.insert(p);
        if (! r.second)
        {
            throw DuplicateElementException("duplicate image key");
        }
    }

    // update image series
    {
        ImageSeriesMap::iterator it = imageSeriesMap_.find(imageKey.location);
        if (it == imageSeriesMap_.end()) 
        {
            ImageSeriesMap::value_type value(imageKey.location, ImageSeries(imageKey.location));
            std::pair<ImageSeriesMap::iterator, bool> result = imageSeriesMap_.insert(value);
            it = result.first;
        }
        ImageSeries& imageSeries = (*it).second;
        imageSeries.timeRange.update(imageKey.time);
    }
}

const ImageMetadata& Scan::getImageMetadata(const ImageKey& key) const
{
    ImageMetadataMap::const_iterator it = imageMetadataMap_.find(key);

    if (it != imageMetadataMap_.end()) 
        return constDerefImageMetadata(it);
    else 
        throw(NoSuchElementException("no such image key"));
}

}
