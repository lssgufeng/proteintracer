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

#ifndef ParameterSet_h
#define ParameterSet_h

#include <string>
#include <vector>

namespace PT 
{

class Parameter
{

public:

    enum Type
    {
        TYPE_INT,
        TYPE_DOUBLE,
        TYPE_BOOL,
        TYPE_LIST
    };

    Parameter(const std::string& name, const std::string& description, int value, int min, int max);
    Parameter(const std::string& name, const std::string& description, double value, double min, double max);
    Parameter(const std::string& name, const std::string& description, bool value);
    Parameter(const std::string& name, const std::string& description, int listIndex, std::vector<std::string> list);

    const std::string& getName() const;
    const std::string& getDescription() const;
    Type getType() const;

    void setIntValue(int value);
    int getIntValue() const;
    int getIntMin() const;
    int getIntMax() const;

    void setDoubleValue(double value);
    double getDoubleValue() const;
    double getDoubleMin() const;
    double getDoubleMax() const;

    void setBoolValue(bool value);
    bool getBoolValue() const;

    void setListIndex(int listIndex);
    int getListIndex();
    const std::vector<std::string>& getList();

private:

    std::string name_;

    std::string description_;
    
    Type type_;

    union
    {
        struct
        {
            int value;
            int min;
            int max;
        } intData_;

        struct
        {
            double value;
            double min;
            double max;
        } doubleData_;

        struct
        {
            bool value;
        } boolData_;

    };

    struct
    {
        int listIndex;
        std::vector<std::string> list;
    } listData_;

};

class ParameterSet
{
public:
    typedef std::vector<Parameter>::iterator Iterator;
    typedef std::vector<Parameter>::const_iterator ConstIterator;

    Iterator getParameterStart();

    ConstIterator getParameterStart() const;

    Iterator getParameterEnd();

    ConstIterator getParameterEnd() const;

    int getSize();

    Parameter& getParameter(const std::string& name);

    const Parameter& getParameter(const std::string& name) const;
    
protected:
    ParameterSet(const std::vector<Parameter>& params);

    virtual ~ParameterSet() {}

private:
    std::vector<Parameter> parameters_;
};

}
#endif
