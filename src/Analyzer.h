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

#ifndef Analyzer_h
#define Analyzer_h

#include <memory>
#include <string>
#include <vector>

#include <Analysis.h>
#include <ParameterSet.h>
#include <Scan.h>
#include <common.h>
#include <images.h>

namespace PT 
{

class Assay : public ParameterSet
{
public:

    Assay(const std::string& analyzerName, 
        const std::vector<Parameter>& params) :
            analyzerName_(analyzerName), 
            ParameterSet(params) 
    {
    }

    const std::string& getAnalyzerName() const
    {
        return analyzerName_;
    }

private:
    std::string analyzerName_;
};

class AnalyzerEvent
{
public:
    enum Id
    {
        MESSAGE,
        PROGRESS,
        CHECK_GUI
    };

    AnalyzerEvent(const std::string& message) :
        id_(AnalyzerEvent::MESSAGE), 
        message_(message), 
        progress_(0.0)
    {
    }

    AnalyzerEvent(float progress) :
        id_(AnalyzerEvent::PROGRESS),
        progress_(progress)
    {
    }

    AnalyzerEvent() :
        id_(AnalyzerEvent::CHECK_GUI),
        progress_(0.0)
    {
    }

    int getId() const
    {
        return id_;
    }

    const std::string& getMessage() const
    {
        assert(id_ == AnalyzerEvent::MESSAGE);
        return message_;
    }

    float getProgress() const
    {
        assert(id_ == AnalyzerEvent::PROGRESS);
        return progress_;
    }

private:

    Id id_;

    std::string message_;

    float progress_;
};

class Analyzer : public EventGenerator<AnalyzerEvent>
{

public:

    /**
     * Ensure that destructor of subclasses is called.
     */
    virtual ~Analyzer() {}

    virtual void process() = 0;

    virtual void cancel() = 0;

};

class Visualization
{
public:

    Visualization(RGBImage::ConstPointer image, ImageKey imageKey)
        : image_(image), imageKey_(imageKey) 
    {
    }

    // ensure that derived classes are destroyed correctly
    virtual ~Visualization() {};

    RGBImage::ConstPointer getImage()
    {
        return image_;
    }

    ImageKey getImageKey()
    {
        return imageKey_;
    }

    virtual std::string getPixelLabel(int x, int y)
    {
        return "";
    }

protected:
    RGBImage::ConstPointer image_;
    ImageKey imageKey_;
};

class VisualizerException : public Exception
{
public:
    VisualizerException() { }
    VisualizerException(const char* what) : PT::Exception(what) { }
    VisualizerException(const itk::ExceptionObject& eo) : PT::Exception(eo.what()) { }
};

class Visualizer
{
public:
    // ensure that derived classes are destroyed correctly
    virtual ~Visualizer() {}

    virtual const std::vector<std::string>& getVisualizationNames() const = 0;

    virtual std::auto_ptr<Visualization> createVisualization(
        const std::string& visualizationName, 
        const Assay& assay, 
        const ImageMetadata& imageMetadata) throw(VisualizerException) = 0;
};

}
#endif
