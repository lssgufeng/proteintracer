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

#include <analyzers/TestAnalyzer.h>

namespace PT
{

static const std::string ANALYZER_NAME("Test");

TestAnalyzer::TestAnalyzer(const Assay& assay, const Scan* scan, const std::string& analysisBaseDirectory)
{
}

const std::string& TestAnalyzer::getName()
{
    return ANALYZER_NAME;
}

std::auto_ptr<Assay> TestAnalyzer::createAssay()
{
    std::vector<Parameter> vec;

    Parameter intParam = Parameter(
            "Int Param", 
            "This is a test description\nof the Int Parameter.",
            5, 1, 10);
    vec.push_back(intParam);

    Parameter doubleParam = Parameter(
            "Double Param", 
            "This is a very long test description\nof the Double Parameter.",
            5E-1, 0.1, 1E0);
    vec.push_back(doubleParam);

    Parameter boolParam = Parameter(
            "Bool Param", 
            "This is an even looooooonger test description\nof the Bool Parameter.",
            true);
    vec.push_back(boolParam);

    std::auto_ptr<Assay> assay = std::auto_ptr<Assay>(new Assay(getName(), vec));
    return assay;
}

}
