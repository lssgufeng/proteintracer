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

#ifndef SegmentSelectionAndMergingImageFilter_txx
#define SegmentSelectionAndMergingImageFilter_txx

#include <itkConstShapedNeighborhoodIterator.h>
#include <itkImageRegionIterator.h>
#include <itkNeighborhoodAlgorithm.h>
#include <itkProgressReporter.h>
#include <itk_hash_map.h>

namespace PT
{

template <class TInputImage>
void SegmentSelectionAndMergingImageFilter<TInputImage>::GenerateData()
{
    typename TInputImage::ConstPointer input = this->GetInput();
    typename TInputImage::Pointer output = this->GetOutput();

    // Setup a progress reporter.  The algorithm
    // has two stages so use the total number of
    // pixels accessed. In the first pass walk the
    // entire input, then walk just the output
    // requested region in the second pass.
    itk::ProgressReporter progress( this, 0, 
            input->GetRequestedRegion().GetNumberOfPixels() 
            + output->GetRequestedRegion().GetNumberOfPixels() );

    typedef itk::hash_map<InputPixelType, Segment> SegmentMap;
    typedef typename SegmentMap::iterator SegmentMapIterator;
    typedef typename SegmentMap::value_type SegmentMapValueType;

    // walk the entire input image and identify segments
    SegmentMap segmentMap;
    {

        typedef itk::ConstShapedNeighborhoodIterator<TInputImage> ShapedNeighborhoodIterator;
        typename ShapedNeighborhoodIterator::RadiusType iteratorRadius;
        iteratorRadius.Fill(1);

        typedef itk::NeighborhoodAlgorithm::ImageBoundaryFacesCalculator<TInputImage> FaceCalculator;
        typedef typename FaceCalculator::FaceListType FaceList;
        typedef typename FaceList::iterator FaceIterator;

        FaceCalculator faceCalculator;
        FaceList faceList = faceCalculator(input, input->GetRequestedRegion(), iteratorRadius);
        for (FaceIterator fit = faceList.begin(); fit != faceList.end(); ++fit)
        {
            ShapedNeighborhoodIterator inputIt(iteratorRadius, input, *fit);

            // initialize neighborhood iterator
            for (int y = -1; y <= 1; y++)
            {
                for (int x = -1; x <= 1; x++)
                {     
                    if (x*x + y*y <= 1) // don't activate diagonal offsets
                    {
                        typename ShapedNeighborhoodIterator::OffsetType off;
                        off[0] = x;
                        off[1] = y;
                        inputIt.ActivateOffset(off);
                    }
                }
            }

            for (inputIt.GoToBegin(); !inputIt.IsAtEnd(); ++inputIt)
            {
                InputPixelType label = inputIt.GetCenterPixel();

                // find segment for label
                SegmentMapIterator segmentMapIt = segmentMap.find( label );

                // create new segment, if it does not exist
                if (segmentMapIt == segmentMap.end())
                {
                    std::pair<SegmentMapIterator, bool> insResult =
                        segmentMap.insert( SegmentMapValueType(label, Segment()) );
                    segmentMapIt = insResult.first;
                    assert(insResult.second);
                }

                Segment& segment = (*segmentMapIt).second;

                // increase segment size
                segment.sizeInPixels++;

                // find segment neighbors
                typename ShapedNeighborhoodIterator::ConstIterator neighborhoodIt;
                for (neighborhoodIt = inputIt.Begin(); 
                        neighborhoodIt != inputIt.End();
                        ++neighborhoodIt)
                {
                    InputPixelType neighborhoodLabel = neighborhoodIt.Get();
                    if (neighborhoodLabel != label)
                        segment.neighbors.insert(neighborhoodLabel);
                }

                progress.CompletedPixel();
            }
        }
    }

    typedef std::map<InputPixelType, InputPixelType> LabelMap;
    typedef typename LabelMap::iterator LabelMapIterator;
    typedef typename LabelMap::value_type LabelMapValueType;

    // map the original labels to new labels
    // sort out segments which are too big, merge
    // others, sort out solitary object which are too
    // small, determine segment count
    LabelMap labelMap;
    {
        segmentSizesInPixels_.resize(segmentMap.size() + 1);
        segmentSizesInPixels_[0] = 0;

        numberOfSegments_ = 0;

        // remove segments that are too big from the
        // segmentMap and map their labels to
        // background
        if (maximumSegmentSize_ > 0)
        {
            SegmentMapIterator segmentIt = segmentMap.begin();
            SegmentMapIterator segmentEnd = segmentMap.end();
            while (segmentIt != segmentEnd)
            {
                InputPixelType label = (*segmentIt).first;
                Segment& segment = (*segmentIt).second;

                // ensure that iterator is incremented before removal of segment
                ++segmentIt;

                // map to background if too big
                if (segment.sizeInPixels > maximumSegmentSize_)
                {
                    // erase reference to this segment from neighbors
                    typename LabelSet::iterator neighborLabelIt = segment.neighbors.begin();
                    typename LabelSet::iterator neighborLabelEnd = segment.neighbors.end();
                    for (;neighborLabelIt != neighborLabelEnd; ++neighborLabelIt)
                    {
                        InputPixelType neighborLabel = *neighborLabelIt;
                        assert(neighborLabel != label);

                        SegmentMapIterator neighborSegmentIt = segmentMap.find(neighborLabel);
                        assert(neighborSegmentIt != segmentMap.end());

                        Segment& neighborSegment = (*neighborSegmentIt).second;
                        neighborSegment.neighbors.erase(label);
                    }

                    labelMap.insert( typename LabelMap::value_type( label, 0 ) );
                    segmentSizesInPixels_[0] += segment.sizeInPixels;
                    segmentMap.erase(label);
                }
            }
        }

        // try to merge segments with neighbors
        {
            SegmentMapIterator segmentIt = segmentMap.begin();
            SegmentMapIterator segmentEnd = segmentMap.end();
            while (segmentIt != segmentEnd)
            {
                InputPixelType label = (*segmentIt).first;
                Segment& segment = (*segmentIt).second;

                // ensure that iterator is incremented before removal of segment
                ++segmentIt;

                bool merged = false;
                InputPixelType mergeLabel;

                typename LabelSet::iterator neighborLabelIt = segment.neighbors.begin();
                typename LabelSet::iterator neighborLabelEnd = segment.neighbors.end();
                while (neighborLabelIt != neighborLabelEnd)
                {
                    InputPixelType neighborLabel = *neighborLabelIt;
                    assert(neighborLabel != label);

                    // ensure that iterator is incremented before removal of neighbor label
                    ++neighborLabelIt;

                    SegmentMapIterator neighborSegmentIt = segmentMap.find(neighborLabel);
                    assert(neighborSegmentIt != segmentMap.end());

                    Segment& neighborSegment = (*neighborSegmentIt).second;

                    if (! merged)
                    {
                        // merge with neighbor segment
                        neighborSegment.neighbors.erase(label);
                        neighborSegment.mergedLabels.insert(label);
                        neighborSegment.mergedLabels.insert(
                                segment.mergedLabels.begin(),
                                segment.mergedLabels.end());
                        neighborSegment.sizeInPixels += segment.sizeInPixels;

                        // copy neighbors to the segment to merge with
                        segment.neighbors.erase(neighborLabel);
                        neighborSegment.neighbors.insert(
                                segment.neighbors.begin(), 
                                segment.neighbors.end());

                        mergeLabel = neighborLabel;
                        merged = true;
                    }
                    else
                    {
                        // tell other neighbor segments about merging
                        neighborSegment.neighbors.erase(label);
                        neighborSegment.neighbors.insert(mergeLabel);
                    }
                }

                if (merged)
                    segmentMap.erase(label);
            }
        }

        // remove segments that are too small from the
        // segmentMap and map their labels to background
        // create new labels for all other segments
        {
            SegmentMapIterator segmentIt = segmentMap.begin();
            SegmentMapIterator segmentEnd = segmentMap.end();
            while (segmentIt != segmentEnd)
            {
                InputPixelType label = (*segmentIt).first;
                Segment& segment = (*segmentIt).second;

                // assert that all neighbors were merged or sorted out
                assert(segment.neighbors.size() == 0);

                // ensure that iterator is incremented before removal of segment
                ++segmentIt;

                InputPixelType newLabel;

                // map to background if too small
                if ( (minimumSegmentSize_ > 0 && segment.sizeInPixels < minimumSegmentSize_)
                     || (maximumSegmentSize_ > 0 && segment.sizeInPixels > maximumSegmentSize_) )
                {
                    newLabel = 0;
                    segmentSizesInPixels_[0] += segment.sizeInPixels;
                }
                else
                {
                    // create new mapping
                    ++numberOfSegments_;
                    newLabel = numberOfSegments_;
                    segmentSizesInPixels_[newLabel] = segment.sizeInPixels;
                }

                // map label of neighbors
                typename LabelSet::iterator mergedLabelIt = segment.mergedLabels.begin();
                typename LabelSet::iterator mergedLabelEnd = segment.mergedLabels.end();
                for (;mergedLabelIt != mergedLabelEnd; ++mergedLabelIt)
                {
                    std::pair<LabelMapIterator, bool> insResult =
                        labelMap.insert( LabelMapValueType(*mergedLabelIt, newLabel) );

                    // assert that label was not mapped before
                    assert(insResult.second);
                }

                labelMap.insert(LabelMapValueType(label, newLabel));
                segmentMap.erase(label);
            }
        }

        segmentSizesInPixels_.resize(labelMap.size());
    }

    // walk just the output requested region and relabel the necessary pixels.
    {
        this->AllocateOutputs();

        // Remap the labels.  
        // Note we only walk the region of the output that was requested.
        // This may be a subset of the input image.
        itk::ImageRegionConstIterator<TInputImage> it;
        itk::ImageRegionIterator<TInputImage> oit;
        it = itk::ImageRegionConstIterator<TInputImage>(input, output->GetRequestedRegion());
        oit = itk::ImageRegionIterator<TInputImage>(output, output->GetRequestedRegion());

        it.GoToBegin();
        oit.GoToBegin();
        InputPixelType inputValue;
        InputPixelType outputValue;
        while ( !oit.IsAtEnd() )
        {
            inputValue = it.Get();
            outputValue = static_cast<InputPixelType>(labelMap[inputValue]); 
            oit.Set( outputValue );

            // increment the iterators
            ++it;
            ++oit;
            progress.CompletedPixel();
        }
    }
}

template <class TInputImage>
void SegmentSelectionAndMergingImageFilter<TInputImage>::GenerateInputRequestedRegion()
{
    Superclass::GenerateInputRequestedRegion();

    typename TInputImage::Pointer input = const_cast<TInputImage*>( this->GetInput() );
    if( input )
    {
        input->SetRequestedRegionToLargestPossibleRegion();
    }
}

}

#endif
