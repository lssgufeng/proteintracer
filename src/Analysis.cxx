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

#include <Analysis.h>

#include <math.h>
#include <sstream>

namespace PT
{

Cell::~Cell()
{
    ObservationMap::iterator it = observationMap_.begin();
    ObservationMap::iterator end = observationMap_.end();

    while (it != end)
    {
        CellObservation* observation = (*it).second; 
        delete observation;

        ++it;
    }
}

void Cell::addObservation(std::auto_ptr<CellObservation> observation)
{
    std::pair<short, CellObservation*> mapVal(observation->getTime(), observation.get());
    std::pair<Cell::ObservationMap::iterator, bool> insResult = observationMap_.insert(mapVal);

    if (insResult.second)
        observation.release();
    else
        throw DuplicateElementException("duplicate observation");
}

void Cell::removeObservation(short time)
{
    ObservationMap::iterator observationMapIt = observationMap_.find(time);
    if (observationMapIt != observationMap_.end())
    {
        CellObservation* cellObservation = (*observationMapIt).second;
        delete cellObservation;
        observationMap_.erase(observationMapIt); 

        assert(observationMap_.find(time) == observationMap_.end());
    }
}

CellObservation* Cell::getObservation(short time)
{
    Cell::ObservationMap::iterator it = observationMap_.find(time);

    if (it != observationMap_.end()) 
        return (*it).second;
    else 
        return 0;
}

bool Cell::isObservedInTime(short time) const
{
    ObservationMap::const_iterator it = observationMap_.find(time); 
    return (it != observationMap_.end());
}

bool Cell::isObservedInImage(const ImageKey& imageKey) const
{
    return (location_ == imageKey.location) && isObservedInTime(imageKey.time);
}

Cell* CellSelection::getCell(int id)
{
    CellMap::iterator it = cellMap_.find(id);

    if (it != cellMap_.end()) 
        return (*it).second;
    else 
        return 0;
}

void CellSelection::addCell(Cell* cell)
{
    assert(cell != 0);
    std::pair<int, Cell*> p(cell->getId(), cell);
    std::pair<CellSelection::CellMap::iterator, bool> r = cellMap_.insert(p);
}

void CellSelection::removeCell(int id)
{
    CellMap::iterator cellMapIt = cellMap_.find(id);
    if (cellMapIt != cellMap_.end())
    {
        Cell* cell = (*cellMapIt).second;
        cellMap_.erase(cellMapIt);
    }
}

void CellSelection::clear()
{
    cellMap_.clear();
}

std::string AnalysisMetadata::getFilePath(const ImageKey& imageKey) const
{
    std::stringstream filePath;
    filePath << baseDirectory;

    filePath << "analysis_w" << imageKey.location.well 
             << "_p" << imageKey.location.position
             << "_s" << imageKey.location.slide
             << "_t" << imageKey.time
             << ".png";
    return filePath.str();
}

Analysis::Analysis(const AnalysisMetadata& metadata) :
        metadata_(metadata)
{
}

Analysis::~Analysis()
{
    Analysis::CellMap::iterator it = cellMap_.begin();
    Analysis::CellMap::iterator end = cellMap_.end();

    for (; it != end; ++it)
    {
        delete (*it).second;
    }
}

void Analysis::addImageSeries(const ImageSeries& imageSeries)
{
    std::pair<ImageLocation, ImageSeries> p(imageSeries.location, imageSeries);
    std::pair<Analysis::ImageSeriesMap::iterator, bool> r = imageSeriesMap_.insert(p);

    if (! r.second)
    {
        throw DuplicateElementException("duplicate image series");
    }
}

void Analysis::addCell(std::auto_ptr<Cell> cell)
{
    std::pair<int, Cell*> p(cell->getId(), cell.get());
    std::pair<Analysis::CellMap::iterator, bool> r = cellMap_.insert(p);

    if (r.second)
    {
        cell.release();
    }
    else
    {
        throw DuplicateElementException("duplicate cell");
    }
}

void Analysis::removeCell(int id)
{
    CellMap::iterator cellMapIt = cellMap_.find(id);
    if (cellMapIt != cellMap_.end())
    {
        Cell* cell = (*cellMapIt).second;
        cellMap_.erase(cellMapIt);
        delete cell;
    }
}

Cell* Analysis::getCell(int id)
{
    CellMap::iterator it = cellMap_.find(id);

    if (it != cellMap_.end()) 
        return (*it).second;
    else 
        return 0;
}

std::auto_ptr<CellSelection> Analysis::selectAllCells()
{
    std::auto_ptr<CellSelection> cellSet(new CellSelection());

    Analysis::CellIterator it = this->getCellStart();
    Analysis::CellIterator end = this->getCellEnd();
    for (;it != end; ++it)
    {
        cellSet->addCell(*it);
    }

    return cellSet;
}

std::auto_ptr<CellSelection> Analysis::selectCellsInImage(const ImageKey& imageKey)
{
    std::auto_ptr<CellSelection> cellSet(new CellSelection());

    Analysis::CellIterator it = this->getCellStart();
    Analysis::CellIterator end = this->getCellEnd();

    for (;it != end; ++it)
    {
        Cell* cell = *it;
        if (cell->isObservedInImage(imageKey))
        {
            cellSet->addCell(cell);
        }
    }

    return cellSet;
}

std::auto_ptr<CellSelection> Analysis::invertCellSelection(CellSelection* cellSelection)
{
    assert(cellSelection != 0);

    std::auto_ptr<CellSelection> cellSet(new CellSelection());

    Analysis::CellIterator it = this->getCellStart();
    Analysis::CellIterator end = this->getCellEnd();

    for (;it != end; ++it)
    {
        Cell* cell = *it;
        if (! cellSelection->containsCell(cell->getId()))
        {
            cellSet->addCell(cell);
        }
    }

    return cellSet;
}

}
