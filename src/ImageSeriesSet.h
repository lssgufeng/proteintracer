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

#ifndef ImageSeriesSet_h
#define ImageSeriesSet_h

#include <map>

#include <common.h>

namespace PT
{

struct ImageLocation
{
    short well;
    short position;
    short slide;

    ImageLocation() : well(-1), position(-1), slide(-1)
    {}

    ImageLocation(short w, short p, short s) : well(w), position(p), slide(s)
    {}

    bool operator<(const ImageLocation& location) const
    {
        short result = well - location.well;
        if (result == 0)
        {
            result = position - location.position;
            if (result == 0)
            {
                result = slide - location.slide;
            }
        }
        return result < 0;
    }

    bool operator!=(const ImageLocation& location) const
    {
        return well != location.well 
            || position != location.position 
            || slide != location.slide;
    }

    bool operator==(const ImageLocation& location) const
    {
        return well == location.well 
            && position == location.position 
            && slide == location.slide;
    }

    bool operator<=(const ImageLocation& location) const
    {
        return (operator==(location) || operator<(location));
    }

    void invalidate()
    {
        well = -1;
        position = -1;
        slide = -1;
    }

    bool isValid() const
    {
        return well > -1 && position > -1 && slide > -1;
    }
};

struct ImageKey
{
    ImageLocation location;

    short time;

    ImageKey() : time(-1)
    {}

    ImageKey(ImageLocation l, short t) : location(l), time(t)
    {}

    void operator=(const ImageKey& key) 
    {
        location = key.location;
        time = key.time;
    }

    bool operator<(const ImageKey& key) const
    {
        short result = location.well - key.location.well;
        if (result == 0)
        {
            result = location.position - key.location.position;
            if (result == 0)
            {
                result = location.slide - key.location.slide;
                if (result == 0)
                {
                    result = time - key.time;
                }
            }
        }
        return result < 0;
    }

    bool operator!=(const ImageKey& key) const
    {
        return location != key.location || time != key.time;
    }

    bool operator==(const ImageKey& key) const 
    {
        return location == key.location && time == key.time;
    }

    bool operator<=(const ImageKey& key) const
    {
        return (operator==(key) || operator<(key));
    }

    ImageKey previous()
    {
        return ImageKey(location, this->time - 1);
    }

    ImageKey next()
    {
        return ImageKey(location, this->time + 1);
    }

    void invalidate()
    {
        location.invalidate();
        time = -1;
    }

    bool isValid() const
    {
        return location.isValid() && time > -1;
    }
};

class ImageSeries
{
public:
    typedef Range<short> TimeRange;

    ImageSeries(const ImageLocation& loc, const TimeRange& tr = TimeRange()) :
        location(loc), timeRange(tr)
    {
    }

    ImageKey getImageKey(short time) const
    {
        return ImageKey(location, time);
    }

    ImageLocation location;

    TimeRange timeRange;

};

class ImageSeriesSet
{
protected:
    typedef std::map<ImageLocation, ImageSeries> ImageSeriesMap;

    inline static const ImageSeries& constDerefImageSeries(ImageSeriesMap::const_iterator& it)
    {
        return (*it).second;
    }

public:
    typedef IteratorWrapper<ImageSeriesMap::const_iterator, const ImageSeries, const ImageSeries&, &constDerefImageSeries> ImageSeriesConstIterator;

    virtual ~ImageSeriesSet() {}

    const ImageSeries& getImageSeries(const ImageLocation& location) const;

    ImageSeriesConstIterator getImageSeriesStart() const
    {
        return ImageSeriesConstIterator(imageSeriesMap_.begin());
    }

    ImageSeriesConstIterator getImageSeriesEnd() const
    {
        return ImageSeriesConstIterator(imageSeriesMap_.end());
    }

protected:
    ImageSeriesMap imageSeriesMap_;
};

}
#endif
