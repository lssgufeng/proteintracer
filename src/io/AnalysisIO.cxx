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

#include <io/AnalysisIO.h>

#include <fstream>
#include <iostream>

#include <tinyxml.h>

#include <common.h>

namespace PT
{

void saveAnalysis(const Analysis& analysis)
{
	TiXmlDocument document;
    
    // initialize document
    TiXmlDeclaration *xmlDeclaration = new TiXmlDeclaration("1.0", "", "");
    document.LinkEndChild(xmlDeclaration);

    // create analysis element
    TiXmlElement *analysisElement = new TiXmlElement("analysis");
    document.LinkEndChild(analysisElement);

    // create metadata element
    {
        const AnalysisMetadata& metadata = analysis.getMetadata();

        TiXmlElement *metadataElement = new TiXmlElement("metadata");
        analysisElement->LinkEndChild(metadataElement);

        // add features
        {
            TiXmlElement *featuresElement = new TiXmlElement("features");
            metadataElement->LinkEndChild(featuresElement);

            std::vector<std::string>::const_iterator featureNameIt = metadata.featureNames.begin();
            std::vector<std::string>::const_iterator featureNameEnd = metadata.featureNames.end();
            for (;featureNameIt != featureNameEnd; ++featureNameIt)
            {
                const std::string& featureName = *featureNameIt;

                TiXmlElement *featureElement = new TiXmlElement("feature");
                featuresElement->LinkEndChild(featureElement);

                featureElement->SetAttribute("name", featureName);
            }
        }

        // add subregions
        {
            TiXmlElement *subregionsElement = new TiXmlElement("subregions");
            metadataElement->LinkEndChild(subregionsElement);

            std::vector<std::string>::const_iterator subregionNameIt = metadata.subregionNames.begin();
            std::vector<std::string>::const_iterator subregionNameEnd = metadata.subregionNames.end();
            for (;subregionNameIt != subregionNameEnd; ++subregionNameIt)
            {
                const std::string& subregionName = *subregionNameIt;

                TiXmlElement *subregionElement = new TiXmlElement("subregion");
                subregionsElement->LinkEndChild(subregionElement);

                subregionElement->SetAttribute("name", subregionName);
            }
        }
    }

    // create images element
    {
        TiXmlElement *imagesElement = new TiXmlElement("images");
        analysisElement->LinkEndChild(imagesElement);

        Analysis::ImageSeriesConstIterator imageSeriesIt = analysis.getImageSeriesStart();
        Analysis::ImageSeriesConstIterator imageSeriesEnd = analysis.getImageSeriesEnd();
        for (;imageSeriesIt != imageSeriesEnd; ++imageSeriesIt)
        {
            const ImageSeries& imageSeries = *imageSeriesIt;
            const ImageLocation& location = imageSeries.location;
            const ImageSeries::TimeRange& timeRange = imageSeries.timeRange;

            TiXmlElement *imageSeriesElement = new TiXmlElement("image-series");
            imagesElement->LinkEndChild(imageSeriesElement);

            imageSeriesElement->SetAttribute("well", location.well);
            imageSeriesElement->SetAttribute("position", location.position);
            imageSeriesElement->SetAttribute("slide", location.slide);
            imageSeriesElement->SetAttribute("start", timeRange.min);
            imageSeriesElement->SetAttribute("end", timeRange.max);
        }
    }

    // create cell elements
    {
        TiXmlElement *cellsElement = new TiXmlElement("cells");
        analysisElement->LinkEndChild(cellsElement);

        Analysis::CellIterator cellIt = (const_cast<Analysis&>(analysis)).getCellStart();
        Analysis::CellIterator cellEnd = (const_cast<Analysis&>(analysis)).getCellEnd();
        for (;cellIt != cellEnd; ++cellIt)
        {
            Cell* cell = *cellIt;

            TiXmlElement *cellElement = new TiXmlElement("cell");
            cellsElement->LinkEndChild(cellElement);

            cellElement->SetAttribute("id", cell->getId());

            // create image-location element
            {
                const ImageLocation& imageLocation = cell->getLocation();

                TiXmlElement *imageLocationElement = new TiXmlElement("image-location");
                cellElement->LinkEndChild(imageLocationElement);

                imageLocationElement->SetAttribute("w", imageLocation.well);
                imageLocationElement->SetAttribute("p", imageLocation.position);
                imageLocationElement->SetAttribute("s", imageLocation.slide);
            }

            // create observations elements
            Cell::ObservationIterator observationIt = cell->getObservationStart();
            Cell::ObservationIterator observationEnd = cell->getObservationEnd();
            for (;observationIt != observationEnd; ++observationIt)
            {
                CellObservation* observation = *observationIt;

                TiXmlElement *observationElement = new TiXmlElement("o");
                cellElement->LinkEndChild(observationElement);

                // set time attribute
                short time = observation->getTime();
                observationElement->SetAttribute("t", time);

                // create region element
                {
                    const ImageRegion& imageRegion = observation->getRegion();

                    TiXmlElement *regionElement = new TiXmlElement("r");
                    observationElement->LinkEndChild(regionElement);

                    regionElement->SetAttribute("x", imageRegion.GetIndex()[0]);
                    regionElement->SetAttribute("y", imageRegion.GetIndex()[1]);
                    regionElement->SetAttribute("w", imageRegion.GetSize()[0]);
                    regionElement->SetAttribute("h", imageRegion.GetSize()[1]);
                }

                // create feature value elements
                {
                    for (int featureIndex = 0; featureIndex < observation->getNumberOfFeatures(); ++featureIndex)
                    {
                        if (! observation->isFeatureAvailable(featureIndex))
                            continue;

                        float featureValue = observation->getFeature(featureIndex); 

                        TiXmlElement *featureValueElement = new TiXmlElement("f");
                        observationElement->LinkEndChild(featureValueElement);

                        featureValueElement->SetAttribute("i", featureIndex);
                        featureValueElement->SetDoubleAttribute("v", featureValue);
                    }
                }
            }
        }
    }

    // save document
    {
        std::stringstream filePathStream;
        filePathStream << analysis.getMetadata().baseDirectory;
        filePathStream << "analysis.xml";
        std::string filePath = filePathStream.str();

        std::ofstream file;
        file.open (filePath.c_str());
        file << document;
        file.close();

        if (file.bad() || file.fail())
        {
            throw IOException("cannot save document");
        }

        //bool successful = document.SaveFile(filePath.c_str());
        //if (!successful)
        //{
        //    throw IOException("cannot save document");
        //}
    }
}

TiXmlElement* readFirstChildElement(TiXmlNode* parentNode, const char* elementName)
{
    // TiXmlElement does not offer method FirstChildElement, so use TiXmlHandle
    TiXmlHandle parentHandle(parentNode);
    TiXmlHandle elementHandle = parentHandle.FirstChildElement(elementName);
    TiXmlElement *element = elementHandle.Element();

    if (! element)
    {
        std::stringstream message;
        message << "missing element \"" << elementName 
            << "\" below element \"" << parentNode->Value() << "\"";
        throw IOException(message.str().c_str());
    }
    return element;
}

std::string readStringAttribute(TiXmlElement* element, const char* attributeName)
{
    const char* attributeValue = element->Attribute(attributeName);
    if (! attributeValue)
    {
        std::stringstream message;
        message << "missing attribute \"" << attributeName << "\" of element \"" << element->Value() << "\"";
        throw IOException(message.str().c_str());
    }
    return std::string(attributeValue);
}

int readIntAttribute(TiXmlElement* element, const char* attributeName)
{
    int intValue;
    int result = element->QueryIntAttribute(attributeName, &intValue);
    if (result == TIXML_NO_ATTRIBUTE)
    {
        std::stringstream message;
        message << "missing attribute \"" << attributeName << "\" of element \"" << element->Value() << "\"";
        throw IOException(message.str().c_str());
    }
    else if (result == TIXML_WRONG_TYPE)
    {
        std::stringstream message;
        message << "attribute \"" << attributeName << "\" of element \"" << element->Value() << "\" has wrong type";
        throw IOException(message.str().c_str());
    }
    return intValue;
}

double readDoubleAttribute(TiXmlElement* element, const char* attributeName)
{
    double doubleValue;
    int result = element->QueryDoubleAttribute(attributeName, &doubleValue);
    if (result == TIXML_NO_ATTRIBUTE)
    {
        std::stringstream message;
        message << "missing attribute \"" << attributeName << "\" of element \"" << element->Value() << "\"";
        throw IOException(message.str().c_str());
    }
    else if (result == TIXML_WRONG_TYPE)
    {
        std::stringstream message;
        message << "attribute \"" << attributeName << "\" of element \"" << element->Value() << "\" has wrong type";
        throw IOException(message.str().c_str());
    }
    return doubleValue;
}

std::auto_ptr<Analysis> loadAnalysis(const char *filepath)
{
    TiXmlDocument document(filepath);

    // load document
    {
        bool successful = document.LoadFile();
        if (!successful)
        {
            std::stringstream message;
            message << "could not load file: " << filepath;
            throw IOException(message.str().c_str());
        }
    }

    TiXmlElement* analysisElement = readFirstChildElement(&document, "analysis");

    // read metadata element
    std::vector<std::string> featureNames;
    std::vector<std::string> subregionNames;    
    std::string baseDirectory;
    {
        TiXmlElement *metadataElement = readFirstChildElement(analysisElement, "metadata");

        // build base directory out of filepath
        baseDirectory = std::string(filepath);
        std::string::size_type slashPos = baseDirectory.rfind('/');
        baseDirectory = baseDirectory.substr(0, slashPos + 1);

        // read features
        TiXmlElement* featuresElement = readFirstChildElement(metadataElement, "features");
        TiXmlElement *featureElement = TiXmlHandle(featuresElement).FirstChildElement("feature").Element();
        for (;featureElement; featureElement = featureElement->NextSiblingElement("feature"))
        {
            std::string featureName = readStringAttribute(featureElement, "name");
            featureNames.push_back(featureName);
        }
        
        // read subregions
        TiXmlElement* subregionsElement = readFirstChildElement(metadataElement, "subregions");
        TiXmlElement *subregionElement = TiXmlHandle(subregionsElement).FirstChildElement("subregion").Element();
        for (;subregionElement; subregionElement = subregionElement->NextSiblingElement("subregion"))
        {
            std::string subregionName = readStringAttribute(subregionElement, "name");
            subregionNames.push_back(subregionName);
        }
    }
    AnalysisMetadata analysisMetadata(
            featureNames,
            subregionNames,
            baseDirectory);

    // create analysis object
    std::auto_ptr<Analysis> analysis(new Analysis(analysisMetadata));

    // read images element
    TiXmlElement* imagesElement = readFirstChildElement(analysisElement, "images");
    TiXmlElement* imageSeriesElement = TiXmlHandle(imagesElement).FirstChildElement("image-series").Element();
    for (;imageSeriesElement; imageSeriesElement = imageSeriesElement->NextSiblingElement("image-series"))
    {
        ImageLocation location;
        location.well = (short) readIntAttribute(imageSeriesElement, "well");
        location.position = (short) readIntAttribute(imageSeriesElement, "position");
        location.slide = (short) readIntAttribute(imageSeriesElement, "slide");

        ImageSeries::TimeRange timeRange;
        timeRange.min = (short) readIntAttribute(imageSeriesElement, "start");
        timeRange.max = (short) readIntAttribute(imageSeriesElement, "end");

        ImageSeries imageSeries(location, timeRange);
        analysis->addImageSeries(imageSeries);
    }

    // read cells element
    TiXmlElement* cellsElement = readFirstChildElement(analysisElement, "cells");
    TiXmlElement* cellElement = TiXmlHandle(cellsElement).FirstChildElement("cell").Element();
    for (;cellElement; cellElement = cellElement->NextSiblingElement("cell"))
    {
        int cellId = readIntAttribute(cellElement, "id");

        // read image-location element
        TiXmlElement* imageLocationElement = readFirstChildElement(cellElement, "image-location");
        int well = readIntAttribute(imageLocationElement, "w");
        int position = readIntAttribute(imageLocationElement, "p");
        int slide = readIntAttribute(imageLocationElement, "s");
        ImageLocation imageLocation(well, position, slide);

        Cell* cell = new Cell(cellId, imageLocation);
        analysis->addCell(std::auto_ptr<Cell>(cell));

        // read observation elements
        TiXmlElement* observationElement = TiXmlHandle(cellElement).FirstChildElement("o").Element();
        for (;observationElement; observationElement = observationElement->NextSiblingElement("o"))
        {
            // read time attribute
            int time = readIntAttribute(observationElement, "t");

            int numFeatures = analysisMetadata.featureNames.size();
            CellObservation* observation = new CellObservation(time, numFeatures);
            cell->addObservation(std::auto_ptr<CellObservation>(observation));

            // read region element
            TiXmlElement* regionElement = readFirstChildElement(observationElement, "r");
            int x = readIntAttribute(regionElement, "x");
            int y = readIntAttribute(regionElement, "y");
            int width = readIntAttribute(regionElement, "w");
            int height = readIntAttribute(regionElement, "h");
            ImageIndex index;
            index[0] = x;
            index[1] = y;
            ImageSize size;
            size[0] = width;
            size[1] = height;
            observation->setRegion(ImageRegion(index, size));

            // read feature value elements
            TiXmlElement* featureValueElement = TiXmlHandle(observationElement).FirstChildElement("f").Element();
            for (;featureValueElement; featureValueElement = featureValueElement->NextSiblingElement("f"))
            {
                int featureIndex = readIntAttribute(featureValueElement, "i");
                float featureValue = (float) readDoubleAttribute(featureValueElement, "v");
                observation->setFeature(featureIndex, featureValue);
            }
        }
    }

    return analysis;
}

}
