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

#ifndef Scan_h
#define Scan_h

#include <string>

#include <ImageSeriesSet.h>

namespace PT
{

struct ImageMetadata
{
    ImageKey key;

    std::string filepath;

    ImageMetadata()
    {};
};

class Scan : public ImageSeriesSet
{
private:

    typedef std::map<ImageKey, ImageMetadata> ImageMetadataMap;

    inline static const ImageMetadata& constDerefImageMetadata(ImageMetadataMap::const_iterator& it)
    {
        return (*it).second;
    }

public:

    typedef IteratorWrapper<ImageMetadataMap::const_iterator, const ImageMetadata, const ImageMetadata&, &constDerefImageMetadata> ImageMetadataConstIterator;

    Scan() {}

    void addImageMetadata(const ImageMetadata& metadata);

    const ImageMetadata& getImageMetadata(const ImageKey &key) const;

    ImageMetadataConstIterator getImageMetadataStart() const
    {
        return ImageMetadataConstIterator(imageMetadataMap_.begin());
    }

    ImageMetadataConstIterator getImageMetadataEnd() const
    {
        return ImageMetadataConstIterator(imageMetadataMap_.end());
    }

    int getNumberOfImages() const
    {
        return imageMetadataMap_.size();
    }

private:

    ImageMetadataMap imageMetadataMap_;

};

}

#endif
