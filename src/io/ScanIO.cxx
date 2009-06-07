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

#include <io/ScanIO.h>

#include <tinyxml.h>

#include <common.h>

namespace PT
{

void saveScan(const Scan& scan, const char *filepath)
{
	TiXmlDocument document;
    
    // initialize document
    {
        TiXmlDeclaration *decl = new TiXmlDeclaration("1.0", "", "");
        document.LinkEndChild(decl);
    }

    // create document
    {
        TiXmlElement *scanElement = new TiXmlElement("scan");
        document.LinkEndChild(scanElement);

        {
            Scan::ImageMetadataConstIterator it = scan.getImageMetadataStart();
            Scan::ImageMetadataConstIterator end = scan.getImageMetadataEnd();
            for (;it != end; ++it)
            {
                const ImageMetadata& imageMetadata = *it;
                const ImageKey& key = imageMetadata.key;

                TiXmlElement *metadataElement = new TiXmlElement("image-metadata");
                metadataElement->SetAttribute("well", key.location.well);
                metadataElement->SetAttribute("position", key.location.position);
                metadataElement->SetAttribute("slide", key.location.slide);
                metadataElement->SetAttribute("time", key.time);

                TiXmlText *text = new TiXmlText(imageMetadata.filepath);
                metadataElement->LinkEndChild(text);

                scanElement->LinkEndChild(metadataElement);
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

std::auto_ptr<Scan> loadScan(const char *filepath)
{
    TiXmlDocument document(filepath);
    std::auto_ptr<Scan> scan(new Scan());

    // load document
    {
        bool successful = document.LoadFile();
        if (!successful)
            throw IOException();
    }

    TiXmlHandle documentHandle(&document);
    TiXmlHandle scanHandle = documentHandle.FirstChildElement("scan");
    TiXmlElement *scanElement = scanHandle.Element();
    if (scanElement == NULL)
        throw IOException();

    // parse image metadata
    {
        ImageMetadata imageMetadata;
        TiXmlElement *metadataElement = scanHandle.FirstChildElement("image-metadata").Element();
        while (metadataElement)
        {
            // parse key
            {
                int value;
                const char *att = metadataElement->Attribute("well", &value);
                if (att)
                    imageMetadata.key.location.well = value;
                else
                    throw IOException();

                att = metadataElement->Attribute("position", &value);
                if (att)
                    imageMetadata.key.location.position = value;
                else
                    throw IOException();

                att = metadataElement->Attribute("slide", &value);
                if (att)
                    imageMetadata.key.location.slide = value;
                else
                    throw IOException();

                att = metadataElement->Attribute("time", &value);
                if (att)
                    imageMetadata.key.time = value;
                else
                    throw IOException();
            }

            const char *filepath=metadataElement->GetText();
            if (filepath) 
                imageMetadata.filepath = filepath;
            else
                throw IOException();

            scan->addImageMetadata(imageMetadata);

            metadataElement = metadataElement->NextSiblingElement("image-metadata");
        }

        if (scan->getNumberOfImages() == 0)
            throw IOException();
    }

    return scan;
}

}
