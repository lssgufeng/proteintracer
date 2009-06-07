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

#include <filters/AnalysisImageFilter.h>

#include <algorithm>
#include <sstream>
#include <vector>

#include <itkImageRegionConstIterator.h>
#include <itkImageRegionConstIteratorWithIndex.h>
#include <itkImageRegionIterator.h>
#include <itkProgressReporter.h>

namespace PT 
{

template <class TIntensityImage, class TLabelImage, class TLabelToSegmentKeyFunctor>
void AnalysisImageFilter<TIntensityImage, TLabelImage, TLabelToSegmentKeyFunctor>::computeStatistics(
        const TIntensityImage* intensityImage,
        const TLabelImage* labelImage,
        SegmentStatisticsMap& segmentStatisticsMap, 
        itk::ProgressReporter& progressReporter)
{
    ImageRegion intensityRegion = intensityImage->GetLargestPossibleRegion();

    itk::ImageRegionConstIteratorWithIndex<TIntensityImage> intensityIt(intensityImage, intensityRegion);
    itk::ImageRegionConstIterator<TLabelImage> labelIt(labelImage, intensityRegion);
    while (!intensityIt.IsAtEnd())
    {
        ImageIndex index = intensityIt.GetIndex();
        typename TIntensityImage::PixelType intensity = intensityIt.Get();
        typename TLabelImage::PixelType label = labelIt.Get();

        // if label not background
        if (label != 0)
        {
            typename SegmentStatisticsMap::iterator labelMapIt = segmentStatisticsMap.find(label);
            if (labelMapIt == segmentStatisticsMap.end())
            {
                typename SegmentStatisticsMap::value_type mapValue(label, SegmentStatistics());
                labelMapIt = segmentStatisticsMap.insert(mapValue).first;
            }
            SegmentStatistics& segmentStatistics = (*labelMapIt).second;

            segmentStatistics.includePixel(index, intensity);
        }

        // update value range of intensity image
        if (intensity < minIntensity_) minIntensity_ = intensity;
        if (intensity > maxIntensity_) maxIntensity_ = intensity;

        ++intensityIt;
        ++labelIt;

        progressReporter.CompletedPixel();
    }
}

template <class TIntensityImage, class TLabelImage, class TLabelToSegmentKeyFunctor>
void AnalysisImageFilter<TIntensityImage, TLabelImage, TLabelToSegmentKeyFunctor>::createCellObservations(
        const SegmentStatisticsMap& segmentStatisticsMap,
        CellObservationMap& cellObservationMap)
{
    // create and initialize CellObservation objects from labels
    typename SegmentStatisticsMap::const_iterator segmentStatisticsIt = segmentStatisticsMap.begin();
    typename SegmentStatisticsMap::const_iterator segmentStatisticsEnd = segmentStatisticsMap.end();
    for (;segmentStatisticsIt != segmentStatisticsEnd; ++segmentStatisticsIt)
    {
        typename TLabelImage::PixelType label = (*segmentStatisticsIt).first;
        const SegmentStatistics& segmentStatistics = (*segmentStatisticsIt).second;

        SegmentKey segmentKey = labelToSegmentKeyFunctor_(label);
        int cellId = segmentKey.mainId;

        // find CellObservation object of cell id or create on if none
        // exists
        CellObservationMap::iterator cellObservationIt = cellObservationMap.find(cellId);
        if (cellObservationIt == cellObservationMap.end())
        {
            // create new CellObservation object
            CellObservation* cellObservation = createCellObservation(imageKey_.time);
            CellObservationMap::value_type value(cellId, cellObservation);
            cellObservationIt = cellObservationMap.insert(value).first;
        }
        CellObservation* cellObservation = (*cellObservationIt).second;

        // increase cell bounds
        {
            ImageIndex cellIndex = cellObservation->getRegion().GetIndex();
            ImageSize cellSize = cellObservation->getRegion().GetSize();
            // check whether cell region is still uninitialized
            if (cellSize[0] == 0 && cellSize[1] == 0)
            {
                cellIndex[0] = segmentStatistics.regionMin[0];
                cellIndex[1] = segmentStatistics.regionMin[1];
                cellSize[0] = segmentStatistics.regionMax[0] - segmentStatistics.regionMin[0] + 1;
                cellSize[1] = segmentStatistics.regionMax[1] - segmentStatistics.regionMin[1] + 1;
            }
            else
            {
                // adjust index
                if (segmentStatistics.regionMin[0] < cellIndex[0])
                {
                    cellSize[0] += cellIndex[0] - segmentStatistics.regionMin[0];
                    cellIndex[0] = segmentStatistics.regionMin[0];
                }
                if (segmentStatistics.regionMin[1] < cellIndex[1])
                {
                    cellSize[1] += cellIndex[1] - segmentStatistics.regionMin[1];
                    cellIndex[1] = segmentStatistics.regionMin[1];
                }

                // adjust size
                if ((cellIndex[0] + cellSize[0] - 1) < segmentStatistics.regionMax[0])
                {
                    cellSize[0] = segmentStatistics.regionMax[0] - cellIndex[0] + 1;
                }
                if ((cellIndex[1] + cellSize[1] - 1) < segmentStatistics.regionMax[1])
                {
                    cellSize[1] = segmentStatistics.regionMax[1] - cellIndex[1] + 1;
                }
            }
            ImageRegion newRegion(cellIndex, cellSize);
            cellObservation->setRegion(newRegion);
        }

        // compute subregion features
        computeSubregionFeatures(segmentKey, segmentStatistics, cellObservation);
    }

    // compute cell features
    CellObservationMap::iterator observationIt = cellObservationMap.begin();
    CellObservationMap::iterator observationEnd = cellObservationMap.end();
    for (; observationIt != observationEnd; ++observationIt)
    {
        computeCellFeatures((*observationIt).second);
    }
}

template <class TIntensityImage, class TLabelImage, class TLabelToSegmentKeyFunctor>
float AnalysisImageFilter<TIntensityImage, TLabelImage, TLabelToSegmentKeyFunctor>::estimateAffinity(
        const CellObservation* previousObservation,
        const CellObservation* currentObservation)
{
    typedef itk::Vector<float, 2> FloatVec2D;

    const ImageRegion& prevRegion = previousObservation->getRegion();
    const ImageRegion& curRegion = currentObservation->getRegion();

    FloatVec2D prevCenter;
    prevCenter[0] = prevRegion.GetIndex()[0] + (prevRegion.GetSize()[0] / 2.0);
    prevCenter[1] = prevRegion.GetIndex()[1] + (prevRegion.GetSize()[1] / 2.0);

    FloatVec2D curCenter;
    curCenter[0] = curRegion.GetIndex()[0] + (curRegion.GetSize()[0] / 2.0);
    curCenter[1] = curRegion.GetIndex()[1] + (curRegion.GetSize()[1] / 2.0);

    float distance = (curCenter - prevCenter).GetNorm();
    if (distance > maxMatchingOffset_)
        return 0;
    else
        return (maxMatchingOffset_ - distance) / maxMatchingOffset_;
}

class AffinityVectorEntry 
{
public:
    float affinity;
    int oldCellId;
    Cell* cell;

    AffinityVectorEntry(float affinityParam, int oldCellIdParam, Cell* cellParam) :
        affinity(affinityParam),
        oldCellId(oldCellIdParam),
        cell(cellParam)
    {
    }

    bool operator<(const AffinityVectorEntry& entry) const
    {
        return affinity < entry.affinity;
    }
}; 

template <class TIntensityImage, class TLabelImage, class TLabelToSegmentKeyFunctor>
void AnalysisImageFilter<TIntensityImage, TLabelImage, TLabelToSegmentKeyFunctor>::integrateObservationsWithAnalysis(
        CellObservationMap& cellObservationMap,
        CellIdMap& cellIdMap)
{
    assert(analysis_ != 0);

    // iterate over previous time steps and try to find matching cells
    ImageKey prevImageKey = imageKey_;
    for (int timeStep = 1; timeStep <= matchingPeriod_; ++timeStep)
    {
        prevImageKey = prevImageKey.previous();

        // check whether we are still in the valid time range
        if (! prevImageKey.isValid())
            break;

        std::auto_ptr<CellSelection> cellsInPrevImage = analysis_->selectCellsInImage(prevImageKey);

        // initialize affinity vector
        std::vector<AffinityVectorEntry> affinityVector;
        {
            typename CellObservationMap::const_iterator cellObservationIt = cellObservationMap.begin();
            typename CellObservationMap::const_iterator cellObservationItEnd = cellObservationMap.end();
            for (;cellObservationIt != cellObservationItEnd; ++cellObservationIt)
            {
                int oldCellId = (*cellObservationIt).first;
                const CellObservation* cellObservation = (*cellObservationIt).second;

                CellSelection::CellIterator cellIt = cellsInPrevImage->getCellStart();
                CellSelection::CellIterator cellItEnd = cellsInPrevImage->getCellEnd();
                for (;cellIt != cellItEnd; ++cellIt)
                {
                    Cell* cell = *cellIt;

                    CellObservation* prevObservation = cell->getObservation(prevImageKey.time);
                    assert(prevObservation != 0);

                    float affinity = estimateAffinity(prevObservation, cellObservation);
                    if (affinity > 0)
                    {
                        AffinityVectorEntry entry(affinity, oldCellId, cell);
                        affinityVector.push_back(entry);
                    }
                }
            }
        }

        // sort affinity vector ascendingly by affinity
        sort(affinityVector.begin(), affinityVector.end());

        // assing CellObservation objects to Cell objects with the highest affinity
        // affinityVector is sorted ascendingly, so iterate in reverse direction
        std::vector<AffinityVectorEntry>::reverse_iterator affinityVectorIt = affinityVector.rbegin();
        std::vector<AffinityVectorEntry>::reverse_iterator affinityVectorItEnd = affinityVector.rend();
        for (; affinityVectorIt != affinityVectorItEnd; ++affinityVectorIt)
        {
            const AffinityVectorEntry& entry = *affinityVectorIt;
            int oldCellId = entry.oldCellId;
            Cell* cell = entry.cell;

            if (! cell->isObservedInImage(imageKey_))
            {
                CellObservationMap::iterator cellObservationIt = cellObservationMap.find(oldCellId);

                // check if CellObservation is still available, i. e. is not already assigned
                if (cellObservationIt != cellObservationMap.end())
                {
                    CellObservation* cellObservation = (*cellObservationIt).second;

                    cell->addObservation( std::auto_ptr<CellObservation>(cellObservation) );
                    cellIdMap.insert( typename CellIdMap::value_type(oldCellId, cell->getId()));

                    cellObservationMap.erase(cellObservationIt);
                }
            }
        }
    }

    // create new cells for observations that could not be assigned to any
    // existing cell
    typename CellObservationMap::iterator cellObservationIt = cellObservationMap.begin();
    typename CellObservationMap::iterator cellObservationItEnd = cellObservationMap.end();
    while (cellObservationIt != cellObservationItEnd)
    {
        int oldCellId = (*cellObservationIt).first;
        CellObservation* cellObservation = (*cellObservationIt).second;

        int newCellId = analysis_->getNumberOfCells() + 1;
        Cell* cell =  new Cell(newCellId, imageKey_.location);
        analysis_->addCell( std::auto_ptr<Cell>(cell) );

        cell->addObservation( std::auto_ptr<CellObservation>(cellObservation) );

        cellIdMap.insert( typename CellIdMap::value_type( oldCellId, newCellId ) );

        // it is important to increment the iterator before erasing the element
        ++cellObservationIt;

        cellObservationMap.erase(oldCellId);
    }
}

template <class TIntensityImage, class TLabelImage, class TLabelToSegmentKeyFunctor>
void AnalysisImageFilter<TIntensityImage, TLabelImage, TLabelToSegmentKeyFunctor>::GenerateData()
{
    const TIntensityImage* intensityImage = this->GetInput(0);
    const TLabelImage* labelImage = static_cast<const TLabelImage*>(this->itk::ProcessObject::GetInput(1));

    // size of intensity image must match size of label image
    ImageRegion intensityRegion = intensityImage->GetLargestPossibleRegion();
    assert(intensityRegion == labelImage->GetLargestPossibleRegion());

    // set up progress reporter
    // the number of pixels is multiplied by two, because we iterate two times
    // over the intensity image
    itk::ProgressReporter progressReporter(this, 0, intensityRegion.GetNumberOfPixels() * 2);

    // measure statistics for every label
    SegmentStatisticsMap segmentStatisticsMap;
    computeStatistics(intensityImage, labelImage, segmentStatisticsMap, progressReporter);

    // create and initialize CellObservation objects
    CellObservationMap cellObservationMap;
    createCellObservations(segmentStatisticsMap, cellObservationMap);

    // integrate CellObservation objects with analysis
    CellIdMap cellIdMap;
    integrateObservationsWithAnalysis(cellObservationMap, cellIdMap);

    // write output
    {
        this->AllocateOutputs();
        RGBAImage::Pointer outputImage = this->GetOutput();

        itk::ImageRegionConstIterator<TIntensityImage> intensityIt;
        intensityIt = itk::ImageRegionConstIterator<TIntensityImage>(intensityImage, outputImage->GetRequestedRegion());

        itk::ImageRegionConstIterator<TLabelImage> labelIt;
        labelIt = itk::ImageRegionConstIterator<TLabelImage>(labelImage, outputImage->GetRequestedRegion());

        itk::ImageRegionIterator<RGBAImage> outputIt;
        outputIt = itk::ImageRegionIterator<RGBAImage>(outputImage, outputImage->GetRequestedRegion());

        float intensityRange = (float)(maxIntensity_ - minIntensity_);

        intensityIt.GoToBegin();
        labelIt.GoToBegin();
        outputIt.GoToBegin();
        while ( !outputIt.IsAtEnd() )
        {
            typename TIntensityImage::PixelType intensity = intensityIt.Get();
            typename TLabelImage::PixelType label = labelIt.Get();

            float relativeIntensity = ((intensity - minIntensity_) / intensityRange);
            unsigned char rescaledIntensity = (unsigned char)(relativeIntensity * 255);

            SegmentKey segmentKey = labelToSegmentKeyFunctor_(label);

            int newCellId;
            if ( segmentKey.isBackground() )
            {
                newCellId = 0;
            }
            else
            {
                CellIdMap::iterator cellIdIt = cellIdMap.find(segmentKey.mainId);
                assert(cellIdIt != cellIdMap.end());
                newCellId = (*cellIdIt).second;
            }

            RGBAImage::PixelType outputValue;
            outputValue[0] = 0xff & newCellId;
            outputValue[1] = (0xff00 & newCellId) >> 8;
            outputValue[2] = 0xff & segmentKey.subId;
            outputValue[3] = 0xff & rescaledIntensity;

            outputIt.Set( outputValue );

            progressReporter.CompletedPixel();

            // increment the iterators
            ++intensityIt;
            ++labelIt;
            ++outputIt;
        }
    }
}

template <class TIntensityImage, class TLabelImage, class TLabelToSegmentKeyFunctor>
void AnalysisImageFilter<TIntensityImage, TLabelImage, TLabelToSegmentKeyFunctor>::GenerateInputRequestedRegion()
{
    Superclass::GenerateInputRequestedRegion();

    typename TIntensityImage::Pointer intensityInput = const_cast<TIntensityImage*>( this->GetInput(0) );
    if ( intensityInput )
    {
        intensityInput->SetRequestedRegionToLargestPossibleRegion();
    }

    const TLabelImage* constLabelImage = static_cast<const TLabelImage*>(this->itk::ProcessObject::GetInput(1));
    TLabelImage* labelImage = const_cast<TLabelImage*>(constLabelImage);
    if ( labelImage )
    {
        labelImage->SetRequestedRegionToLargestPossibleRegion();
    }
}

}
