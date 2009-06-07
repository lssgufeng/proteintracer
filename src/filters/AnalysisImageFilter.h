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

#ifndef AnalysisImageFilter_h
#define AnalysisImageFilter_h

#include <itkImageToImageFilter.h>
#include <itk_hash_map.h>

#include <Analysis.h>
#include <Scan.h>
#include <filters/SegmentKey.h>

namespace PT 
{

template <class TIntensityImage, class TLabelImage, class TLabelToSegmentKeyFunctor>
class AnalysisImageFilter : public itk::ImageToImageFilter<TIntensityImage, RGBAImage> 
{
public:
    typedef itk::ImageToImageFilter<TIntensityImage, RGBAImage> Superclass;

    class LabelToSegmentKeyFunctor
    {
    public:
        SegmentKey operator()(RGBAPixel label)
        {
            return SegmentKey( Analysis::decodeCellId(label),
                    Analysis::decodeSubregionIndex(label) );
        }
    };

    void setIntensityInput(const TIntensityImage* intensityImage) 
    {
        // Process object is not const-correct so the const casting is required.
        SetNthInput(0, const_cast<TIntensityImage*>(intensityImage));
    }

    void setLabelInput(const TLabelImage* labelImage) 
    {
        // Process object is not const-correct so the const casting is required.
        SetNthInput(1, const_cast<TLabelImage*>(labelImage));
    }

    void setMaxMatchingOffset(float maxMatchingOffset)
    {
        if (maxMatchingOffset != maxMatchingOffset_)
        {
            maxMatchingOffset_ = maxMatchingOffset;
        }
    }

    void setMatchingPeriod(int matchingPeriod)
    {
        assert(matchingPeriod >= 0);

        if (matchingPeriod != matchingPeriod_)
        {
            matchingPeriod_ = matchingPeriod;
            this->Modified();
        }
    }

    void setImage(const ImageKey& imageKey)
    {
        if (imageKey != imageKey_)
        {
            imageKey_ = imageKey;
            this->Modified();
        }
    }

    void setAnalysis(Analysis* analysis)
    {
        analysis_ = analysis;
    }

protected:

    class SegmentStatistics
    {
    public:

        SegmentStatistics() :
            pixelCount(0),
            minIntensity(itk::NumericTraits<float>::max()), 
            maxIntensity(itk::NumericTraits<float>::min()),
            intensitySum(itk::NumericTraits<float>::Zero),
            intensitySumOfSquares(itk::NumericTraits<float>::Zero)
        {
            regionMin[0] = itk::NumericTraits<ImageIndex::IndexValueType>::max();
            regionMin[1] = itk::NumericTraits<ImageIndex::IndexValueType>::max();

            regionMax[0] = itk::NumericTraits< ImageIndex::IndexValueType>::min();
            regionMax[1] = itk::NumericTraits< ImageIndex::IndexValueType>::min();
        }

        void includePixel(const ImageIndex& index, typename TIntensityImage::PixelType intensity)
        {
            float intensityVal = static_cast<float>(intensity);

            ++pixelCount;

            if (intensityVal < minIntensity) minIntensity = intensityVal;
            if (intensityVal > maxIntensity) maxIntensity = intensityVal;

            intensitySum += intensityVal;
            intensitySumOfSquares += intensityVal * intensityVal;

            if (regionMin[0] > index[0]) regionMin[0] = index[0];
            if (regionMin[1] > index[1]) regionMin[1] = index[1];

            if (regionMax[0] < index[0]) regionMax[0] = index[0];
            if (regionMax[1] < index[1]) regionMax[1] = index[1];
        }

        unsigned long pixelCount;

        float minIntensity;
        float maxIntensity;

        float intensitySum;
        float intensitySumOfSquares;

        ImageIndex regionMin;
        ImageIndex regionMax;
    };

    typedef itk::hash_map<typename TLabelImage::PixelType, SegmentStatistics> SegmentStatisticsMap;

    typedef itk::hash_map<int, CellObservation*> CellObservationMap;

    typedef itk::hash_map<int, int> CellIdMap;

    AnalysisImageFilter() :
        maxMatchingOffset_(0),
        matchingPeriod_(2),
        analysis_(0),
        minIntensity_( itk::NumericTraits< typename TIntensityImage::PixelType >::max() ),
        maxIntensity_( itk::NumericTraits< typename TIntensityImage::PixelType >::min() )
    {
    }

    void GenerateData();

    /** 
     * The SegmentSizeImageFilter needs the entire input. Therefore it must
     * provide an implementation GenerateInputRequestedRegion().
     */
    void GenerateInputRequestedRegion();

    virtual CellObservation* createCellObservation(short time) = 0;

    virtual void computeSubregionFeatures(
            const SegmentKey& segmentKey,
            const SegmentStatistics& segmentStatistics,
            CellObservation* cellObservation) = 0;

    /**
     * Called for every cell observation after computeSubregionFeatures was
     * called for every subregion that was found.
     */
    virtual void computeCellFeatures(
            CellObservation* cellObservation) = 0;

    float estimateAffinity(
            const CellObservation* previousObservation, 
            const CellObservation* currentObservation);

private:

    void computeStatistics(
            const TIntensityImage* intensityImage,
            const TLabelImage* labelImage,
            SegmentStatisticsMap& segmentStatisticsMap, 
            itk::ProgressReporter& progress);

    void createCellObservations(
            const SegmentStatisticsMap& segmentStatisticsMap, 
            CellObservationMap& cellObservationMap);

    void integrateObservationsWithAnalysis(
            CellObservationMap& cellObservationMap,
            CellIdMap& cellIdMap);

    typename TIntensityImage::PixelType minIntensity_;
    typename TIntensityImage::PixelType maxIntensity_;

    float maxMatchingOffset_;

    int matchingPeriod_;

    ImageKey imageKey_;

    Analysis* analysis_;

    TLabelToSegmentKeyFunctor labelToSegmentKeyFunctor_;
};

}

// include template implementation
#include "AnalysisImageFilter.txx"

#endif
