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

#include <analyzers/TestVisualizer.h>

namespace PT
{

static const std::string VISUALIZATION_SOURCE("Source");
static const std::string VISUALIZATION_EXCEPTION("Exception");

TestVisualizer::TestVisualizer()
{
    fileReader_ = FileReader::New();
    sourceVisualizationFilter_ = SourceVisualizationFilter::New();
    sourceVisualizationFilter_->SetInput(fileReader_->GetOutput());
}

const std::vector<std::string>& TestVisualizer::getVisualizationNames() const
{
    static std::vector<std::string> visualizations;

    // initialize
    if (visualizations.empty())
    {
        visualizations.push_back(VISUALIZATION_SOURCE);
        visualizations.push_back(VISUALIZATION_EXCEPTION);
    }

    return visualizations;
}

class TestVisualization : public Visualization
{
public:
    TestVisualization(RGBImage::ConstPointer image, ImageKey imageKey) : Visualization(image, imageKey) {}

    std::string getPixelLabel(int x, int y)
    {
        ImageRegion largestPossibleRegion = image_->GetLargestPossibleRegion();
        ImageIndex index = {x, y};
        if (largestPossibleRegion.IsInside(index))
        {
            RGBImage::PixelType pixel = image_->GetPixel(index);
            std::stringstream text;
            text << "RGB Pixel: " <<  pixel;
            return text.str();
        }
        else
        {
            return "";
        }
    }
};

std::auto_ptr<Visualization> TestVisualizer::createVisualization(
    const std::string& visualizationName, 
    const Assay& assay, 
	const ImageMetadata& imageMetadata) throw(VisualizerException)
{
    // update filter parameters
    {
        const std::string& filepath = imageMetadata.filepath;
        fileReader_->SetFileName(filepath.c_str());
    }

    if (visualizationName == VISUALIZATION_SOURCE)
    {
        sourceVisualizationFilter_->Update();

        RGBImage::ConstPointer image = sourceVisualizationFilter_->GetOutput();
        UShortImage::Pointer labelImage = fileReader_->GetOutput();

        return std::auto_ptr<Visualization>(new TestVisualization(image, imageMetadata.key));
    }
    else if (visualizationName == VISUALIZATION_EXCEPTION)
    {
        throw VisualizerException("this is just a test exception");
    }
    else
    {
		throw InvalidArgumentException("invalid visualization name");
    }
}

}
