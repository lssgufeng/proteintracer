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

#ifndef Analysis_h
#define Analysis_h

#include <memory>
#include <map>
#include <set>
#include <vector>

#include <images.h>
#include <ImageSeriesSet.h>

namespace PT 
{

class CellObservation
{
private:

    class FeatureValue
    {
    public:
        FeatureValue() : value_(0), available_(false) { }

        void set(float value)
        {
            value_ = value;
            available_ = true;
        }

        float get() const
        {
            assert(available_);
            return value_;
        }

        bool available() const
        {
            return available_;
        }

    private:
        float value_;
        bool available_;
    };

public:

    CellObservation(short time, int numFeatures) : 
        time_(time), 
        featureValues_(numFeatures)
    { 
    }

    short getTime() const
    {
        return time_;
    }

    void setRegion(const ImageRegion& region)
    {
        region_ = region;
    }

    const ImageRegion& getRegion() const
    {
        return region_;
    }

    int getNumberOfFeatures() const
    {
        return featureValues_.size();
    }

    void setFeature(int index, float value)
    {
        assert(index < featureValues_.size());
        featureValues_[index].set(value);
    }

    float getFeature(int index) const
    {
        assert(index < featureValues_.size());
        return featureValues_[index].get();
    }

    bool isFeatureAvailable(int index) const
    {
        assert(index < featureValues_.size());
        return featureValues_[index].available();
    }

private:

    int time_;

    ImageRegion region_;

    std::vector<FeatureValue> featureValues_;

};

class Cell
{
private:
    typedef std::map<short, CellObservation*> ObservationMap;

    inline static CellObservation* derefPointer(ObservationMap::iterator& it)
    {
        return (*it).second;
    }

public:
    typedef IteratorWrapper<ObservationMap::iterator, CellObservation*, CellObservation*, &derefPointer> ObservationIterator;

    Cell(int id, const ImageLocation& location) : id_(id), location_(location) { }

    ~Cell();

    int getId() const
    {
        return id_;
    }

    const ImageLocation& getLocation() const
    {
        return location_;
    }

    void addObservation(std::auto_ptr<CellObservation> observation);

    void removeObservation(short time);
    
    CellObservation* getObservation(short time);

    int getNumberOfObservations()
    {
        return observationMap_.size();
    }

    ObservationIterator getObservationStart()
    {
        return ObservationIterator(observationMap_.begin());
    }

    ObservationIterator getObservationEnd()
    {
        return ObservationIterator(observationMap_.end());
    }

    bool isObservedInTime(short time) const;

    bool isObservedInImage(const ImageKey& imageKey) const;

private:

    int id_;

    ImageLocation location_;

    ObservationMap observationMap_;
};

class CellSelection
{
private:

    typedef std::map<int, Cell*> CellMap;

    inline static Cell* derefCell(CellMap::iterator& it)
    {
        return (*it).second;
    }

public:

    typedef IteratorWrapper<CellMap::iterator, Cell*, Cell*, &derefCell> CellIterator;

    void addCell(Cell* cell);

    void removeCell(int id);

    void clear();

    Cell* getCell(int id);

    bool containsCell(int id)
    {
        return getCell(id) != 0;
    }

    int getNumberOfCells() const
    {
        return cellMap_.size();
    }

    CellIterator getCellStart()
    {
        return CellIterator(cellMap_.begin());
    }

    CellIterator getCellEnd()
    {
        return CellIterator(cellMap_.end());
    }

private:

    CellMap cellMap_;

};

class AnalysisMetadata
{
public:

    AnalysisMetadata(
        const std::vector<std::string>& featureNames_,
        const std::vector<std::string>& subregionNames_,
        const std::string& baseDirectory_) :
            featureNames(featureNames_),
            subregionNames(subregionNames_),
            baseDirectory(baseDirectory_)
    { }

    const std::vector<std::string> featureNames;
    const std::vector<std::string> subregionNames;
    const std::string baseDirectory;

    std::string getFilePath(const ImageKey& imageKey) const;

};

class Analysis : public ImageSeriesSet
{
private:

    typedef std::map<int, Cell*> CellMap;

    inline static Cell* derefCell(CellMap::iterator& it)
    {
        return (*it).second;
    }

public:

    typedef IteratorWrapper<CellMap::iterator, Cell*, Cell*, &derefCell> CellIterator;

    Analysis(const AnalysisMetadata& metadata);

    ~Analysis();

    const AnalysisMetadata& getMetadata() const
    {
        return metadata_;
    }

    void addImageSeries(const ImageSeries& imageSeries);

    void addCell(std::auto_ptr<Cell> cell);

    void removeCell(int id);

    Cell* getCell(int id);

    bool containsCell(int id)
    {
        return getCell(id) != 0;
    }

    int getNumberOfCells() const
    {
        return cellMap_.size();
    }

    CellIterator getCellStart()
    {
        return CellIterator(cellMap_.begin());
    }

    CellIterator getCellEnd()
    {
        return CellIterator(cellMap_.end());
    }

    std::auto_ptr<CellSelection> selectAllCells();

    std::auto_ptr<CellSelection> selectCellsInImage(const ImageKey& imageKey);

    std::auto_ptr<CellSelection> invertCellSelection(CellSelection* cellSelection);

    static inline unsigned short decodeCellId(const RGBAPixel& pixel)
    {
        return pixel[0] | (pixel[1] << 8);
    }

    static inline unsigned char decodeSubregionIndex(const RGBAPixel& pixel)
    {
        return pixel[2];
    }

    static inline unsigned char decodeIntensity(const RGBAPixel& pixel)
    {
        return pixel[3];
    }

private:

    AnalysisMetadata metadata_;

    CellMap cellMap_;

};

}
#endif
