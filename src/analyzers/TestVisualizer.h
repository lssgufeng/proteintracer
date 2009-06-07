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

#ifndef TestVisualizer_h
#define TestVisualizer_h

#include <memory>
#include <string>
#include <vector>

#include <itkImageFileReader.h>

#include <filters/RescaleIntensityRGBImageFilter.h>
#include <Analyzer.h>

namespace PT 
{

class TestVisualizer : public Visualizer
{
public:
    TestVisualizer();

    virtual const std::vector<std::string>& getVisualizationNames() const;

    virtual std::auto_ptr<Visualization> createVisualization(
        const std::string& visualizationName, 
        const Assay& assay, 
        const ImageMetadata& imageMetadata) throw(VisualizerException);
private:
        typedef itk::ImageFileReader<UShortImage> FileReader;
        typedef RescaleIntensityRGBImageFilter<UShortImage> SourceVisualizationFilter;

        FileReader::Pointer fileReader_;
        SourceVisualizationFilter::Pointer sourceVisualizationFilter_;
};

}
#endif
