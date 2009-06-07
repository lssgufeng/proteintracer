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

#include <io/AssayIO.h>

#include <tinyxml.h>

#include <analyzers/AnalyzerRegistry.h>
#include <common.h>

namespace PT
{

void saveAssay(const Assay& assay, const char *filepath)
{

	TiXmlDocument document;
    
    // initialize document
    {
        TiXmlDeclaration *decl = new TiXmlDeclaration("1.0", "", "");
        document.LinkEndChild(decl);
    }

    // create document
    {
        TiXmlElement *assayElement = new TiXmlElement("assay");
        assayElement->SetAttribute("analyzer", assay.getAnalyzerName());
        document.LinkEndChild(assayElement);

        {
            ParameterSet::ConstIterator it = assay.getParameterStart();
            ParameterSet::ConstIterator end = assay.getParameterEnd();
            for (;it != end; ++it)
            {
                const Parameter& param = *it;

                TiXmlElement *parameterElement = new TiXmlElement("parameter");
                parameterElement->SetAttribute("name", param.getName());

                switch (param.getType())
                {
                case Parameter::TYPE_INT:
                    parameterElement->SetAttribute("value", param.getIntValue());
                    break;
                case Parameter::TYPE_DOUBLE:
                    parameterElement->SetDoubleAttribute("value", param.getDoubleValue());
                    break;
                case Parameter::TYPE_BOOL:
                    {
                        const char* valueString = (param.getBoolValue()) ? "true" : "false";
                        parameterElement->SetAttribute("value", valueString);
                    }
                    break;
                case Parameter::TYPE_LIST:
                    assert(false); // not yet implemented
                    break;
                }

                assayElement->LinkEndChild(parameterElement);
            }
        }
    }

    // save document
	bool successful = document.SaveFile(filepath);
    if (!successful)
    {
        throw IOException();
    }
}

std::auto_ptr<Assay> loadAssay(const char *filepath)
{
    TiXmlDocument document(filepath);

    // load document
    {
        bool successful = document.LoadFile();
        if (!successful)
            throw IOException();
    }

    TiXmlHandle documentHandle(&document);
    TiXmlHandle assayHandle = documentHandle.FirstChildElement("assay");
    TiXmlElement *assayElement = assayHandle.Element();
    if (assayElement == NULL)
        throw IOException();

    // parse analyzer and initialize assay
    std::auto_ptr<Assay> assay;
    {
        const char *analyzerName = assayElement->Attribute("analyzer");
        if (analyzerName)
        {
            try
            {
                assay = AnalyzerRegistry::createAssay(analyzerName);
            }
            catch (NoSuchElementException&)
            {
                throw IOException();
            }
        }
        else
            throw IOException();
    }

    // parse parameters
    TiXmlElement *parameterElement = assayHandle.FirstChildElement("parameter").Element();
    for (;parameterElement; parameterElement = parameterElement->NextSiblingElement("parameter"))
    {
        // find parameter
        Parameter* parameter = 0;
        {
            const char *parameterName = parameterElement->Attribute("name");
            if (parameterName)
            {
                try 
                {
                    parameter = &assay->getParameter(parameterName);
                }
                catch (NoSuchElementException&)
                {
                    throw IOException();
                }
            }
            else
                throw IOException();
        }

        // parse parameter value
        switch (parameter->getType())
        {
            case Parameter::TYPE_INT:
                {
                    int value;
                    const char *att = parameterElement->Attribute("value", &value);
                    if (att)
                        parameter->setIntValue(value);
                    else
                        throw IOException();
                }
                break;
            case Parameter::TYPE_DOUBLE:
                {
                    double value;
                    const char *att = parameterElement->Attribute("value", &value);
                    if (att)
                        parameter->setDoubleValue(value);
                    else
                        throw IOException();
                }
                break;
            case Parameter::TYPE_BOOL:
                {
                    const char *att = parameterElement->Attribute("value");
                    if (att)
                    {
                        std::string valueString(att);
                        bool value = (valueString == "true");
                        parameter->setBoolValue(value);
                    }
                    else
                        throw IOException();
                }
                break;
            case Parameter::TYPE_LIST:
                assert(false); // not yet implemented
                break;
        }
    }

    return assay;
}

}
