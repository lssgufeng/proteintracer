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

#include <ParameterSet.h>

#include <common.h>

namespace PT
{

Parameter::Parameter(const std::string& name, const std::string& description, int value, int min, int max) : name_(name), description_(description), type_(TYPE_INT)
{
    intData_.value = value;
    intData_.min = min;
    intData_.max = max;
}

Parameter::Parameter(const std::string& name, const std::string& description, double value, double min, double max) : name_(name), description_(description), type_(TYPE_DOUBLE)
{
    doubleData_.value = value;
    doubleData_.min = min;
    doubleData_.max = max;
}

Parameter::Parameter(const std::string& name, const std::string& description, bool value) : name_(name), description_(description), type_(TYPE_BOOL)
{
    boolData_.value = value;
}

Parameter::Parameter(const std::string& name, const std::string& description, int listIndex, std::vector<std::string> list) : name_(name), description_(description), type_(TYPE_LIST)
{
    listData_.listIndex = listIndex;
    listData_.list = list;
}

const std::string& Parameter::getName() const
{
    return name_;
}

const std::string& Parameter::getDescription() const
{
    return description_;
}

Parameter::Type Parameter::getType() const
{
    return type_;
}

void Parameter::setIntValue(int value)
{
    assert(type_ == TYPE_INT);
    intData_.value = value;
}

int Parameter::getIntValue() const
{
    assert(type_ == TYPE_INT);
    return intData_.value;
}

int Parameter::getIntMin() const
{
    assert(type_ == TYPE_INT);
    return intData_.min;
}

int Parameter::getIntMax() const
{
    assert(type_ == TYPE_INT);
    return intData_.max;
}

void Parameter::setDoubleValue(double value)
{
    assert(type_ == TYPE_DOUBLE);
    doubleData_.value = value;
}

double Parameter::getDoubleValue() const
{
    assert(type_ == TYPE_DOUBLE);
    return doubleData_.value;
}

double Parameter::getDoubleMin() const
{
    assert(type_ == TYPE_DOUBLE);
    return doubleData_.min;
}

double Parameter::getDoubleMax() const
{
    assert(type_ == TYPE_DOUBLE);
    return doubleData_.max;
}

void Parameter::setBoolValue(bool value)
{
    assert(type_ == TYPE_BOOL);
    boolData_.value = value;
}

bool Parameter::getBoolValue() const
{
    assert(type_ == TYPE_BOOL);
    return boolData_.value;
}

void Parameter::setListIndex(int listIndex)
{
    assert(type_ == TYPE_LIST);
    listData_.listIndex = listIndex;
}

int Parameter::getListIndex()
{
    assert(type_ == TYPE_LIST);
    return listData_.listIndex;
}

const std::vector<std::string>& Parameter::getList()
{
    assert(type_ == TYPE_LIST);
    return listData_.list;
}

ParameterSet::ParameterSet(const std::vector<Parameter>& params)
{
    parameters_ = params;
}

ParameterSet::Iterator ParameterSet::getParameterStart()
{
    return parameters_.begin();
}

ParameterSet::ConstIterator ParameterSet::getParameterStart() const
{
    return parameters_.begin();
}

ParameterSet::Iterator ParameterSet::getParameterEnd()
{
    return parameters_.end();
}

ParameterSet::ConstIterator ParameterSet::getParameterEnd() const
{
    return parameters_.end();
}

int ParameterSet::getSize()
{
    return parameters_.size();
}

Parameter& ParameterSet::getParameter(const std::string& name)
{
    ParameterSet::Iterator it = getParameterStart();
    ParameterSet::Iterator end = getParameterEnd();

    while (it != end)
    {
       if ((*it).getName() == name)
            return *it;

        ++it;
    }

    throw NoSuchElementException("no such parameter");
}

const Parameter& ParameterSet::getParameter(const std::string& name) const
{
    ParameterSet::ConstIterator it = getParameterStart();
    ParameterSet::ConstIterator end = getParameterEnd();

    while (it != end)
    {
       if ((*it).getName() == name)
            return *it;

        ++it;
    }

    throw NoSuchElementException("no such parameter");
}

}
