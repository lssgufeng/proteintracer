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

#ifndef SegmentSizeImageFilter_txx
#define SegmentSizeImageFilter_txx

#include <itkImageRegionIterator.h>
#include <itkProgressReporter.h>
#include <itk_hash_map.h>

namespace PT
{

template <class TInputImage>
void SegmentSizeImageFilter<TInputImage>::GenerateData()
{
    typename TInputImage::ConstPointer input = this->GetInput();
    typename TInputImage::Pointer output = this->GetOutput();

    // Setup a progress reporter.  We have 2 stages to the algorithm so
    // use the total number of pixels accessed. We walk the entire input
    // in the first pass, then walk just the output requested region in
    // the second pass.
    itk::ProgressReporter progress( this, 0, 
            input->GetRequestedRegion().GetNumberOfPixels() 
            + output->GetRequestedRegion().GetNumberOfPixels() );

    // walk the entire input image and identify segments and the respective
    // sizes
    typedef itk::hash_map<InputPixelType, Segment> SegmentMap;
    SegmentMap segmentMap;
    {
        Segment segment;
        segment.sizeInPixels = 1;

        typename TInputImage::PixelType inputValue;
        itk::ImageRegionConstIterator<TInputImage> it;
        typename SegmentMap::iterator segmentMapIt;
        it = itk::ImageRegionConstIterator<TInputImage>(input, input->GetRequestedRegion());
        it.GoToBegin();
        while (!it.IsAtEnd())
        {
            inputValue = it.Get();

            // Does this label already exist
            segmentMapIt = segmentMap.find( inputValue );
            if ( segmentMapIt == segmentMap.end() )
            {
                // label is not currently in the map
                segment.label = inputValue;
                segmentMap.insert( typename SegmentMap::value_type( inputValue, segment ) );
            }
            else
            {
                // label is already in the map, update the values
                (*segmentMapIt).second.sizeInPixels++;
            }

            ++it;
            progress.CompletedPixel();
        }
    }

    // sort the segments by size
    typedef std::vector<Segment> SegmentVector;
    SegmentVector segmentVector;
    {
        typename SegmentMap::iterator segmentMapIt;
        for (segmentMapIt = segmentMap.begin(); segmentMapIt != segmentMap.end(); ++segmentMapIt)
        {
            segmentVector.push_back( (*segmentMapIt).second );
        }
        std::sort(segmentVector.begin(), segmentVector.end(), SegmentSizeComparator() );
    }

    // map the original labels to new labels
    // sort out labels which are too big or too small
    // determine object count
    typedef std::map<InputPixelType, InputPixelType> LabelMap;
    LabelMap labelMap;
    { 
        segmentSizesInPixels_.resize(segmentVector.size() + 1);
        segmentSizesInPixels_[0] = 0;

        numberOfSegments_ = 0;
        unsigned long segmentSize;
        for (typename SegmentVector::iterator vit = segmentVector.begin();
                vit != segmentVector.end(); ++vit)
        {
            segmentSize = (*vit).sizeInPixels;

            if ((numberOfSegments_ == maxNumberOfSegments_) ||
                (maximumSegmentSize_ > 0 && segmentSize > maximumSegmentSize_ ) ||
                (minimumSegmentSize_ > 0 && segmentSize < minimumSegmentSize_) )
            {
                // map to background
                labelMap.insert( typename LabelMap::value_type( (*vit).label, 0 ) );
                segmentSizesInPixels_[0] += segmentSize;
            }
            else
            {
                ++numberOfSegments_;
                labelMap.insert( typename LabelMap::value_type( (*vit).label, numberOfSegments_ ) );
                segmentSizesInPixels_[numberOfSegments_] = segmentSize;
            }
        }

        // plus 1 because segmentSizesInPixels_ serves as a map and labels
        // start at 1
        segmentSizesInPixels_.resize(numberOfSegments_ + 1);
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
void SegmentSizeImageFilter<TInputImage>::GenerateInputRequestedRegion()
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
