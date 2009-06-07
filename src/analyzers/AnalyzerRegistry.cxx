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

#include <analyzers/AnalyzerRegistry.h>

#include <analyzers/TestAnalyzer.h>
#include <analyzers/TestVisualizer.h>
#include <analyzers/WatershedAnalyzer.h>
#include <analyzers/WatershedVisualizer.h>

namespace PT
{

const std::vector<std::string>& AnalyzerRegistry::getAnalyzerNames()
{
    static std::vector<std::string> analyzerNames;

    // initialize
    if (analyzerNames.size() == 0)
    {
        analyzerNames.push_back(WatershedAnalyzer::getName());
        analyzerNames.push_back(TestAnalyzer::getName());
    }

    return analyzerNames;
}

const std::string& AnalyzerRegistry::getDefaultAnalyzerName()
{
    return WatershedAnalyzer::getName();
}

std::auto_ptr<Assay> AnalyzerRegistry::createAssay(const std::string& analyzerName)
{
    if (analyzerName == WatershedAnalyzer::getName())
    {
        return WatershedAnalyzer::createAssay();
    }
    else if (analyzerName == TestAnalyzer::getName())
    {
        return TestAnalyzer::createAssay();
    }
    else
    {
        throw NoSuchElementException("no such analyzer");
    }
}

std::auto_ptr<Analyzer> AnalyzerRegistry::createAnalyzer(const Assay& assay, const Scan* scan, const std::string& analysisDirectory)
{
    const std::string& analyzerName = assay.getAnalyzerName();
    if (analyzerName == WatershedAnalyzer::getName())
    {
        return std::auto_ptr<Analyzer>(new WatershedAnalyzer(assay, scan, analysisDirectory));
    }
    else if (analyzerName == TestAnalyzer::getName())
    {
        return std::auto_ptr<Analyzer>(new TestAnalyzer(assay, scan, analysisDirectory));
    }
    else
    {
        throw NoSuchElementException("no such analyzer");
    }
}

std::auto_ptr<Visualizer> AnalyzerRegistry::createVisualizer(const std::string& analyzerName)
{
    if (analyzerName == WatershedAnalyzer::getName())
    {
        return std::auto_ptr<Visualizer>( new WatershedVisualizer() );
    }
    else if (analyzerName == TestAnalyzer::getName())
    {
        return std::auto_ptr<Visualizer>( new TestVisualizer() );
    }
    else
    {
        throw NoSuchElementException("no such analyzer");
    }
}

}
