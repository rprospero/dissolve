/*
    *** Keyword - Vector of Integer/Double Tuples
    *** src/keywords/vector_intdouble.cpp
    Copyright T. Youngs 2012-2020

    This file is part of Dissolve.

    Dissolve is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Dissolve is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Dissolve.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "keywords/vector_intdouble.h"
#include "base/lineparser.h"
#include "classes/coredata.h"

IntegerDoubleVectorKeyword::IntegerDoubleVectorKeyword::IntegerDoubleVectorKeyword(IntegerDoubleVectorKeywordData &data,
                                                                                   int nRequiredIntegers,
                                                                                   std::optional<int> nRequiredValues)
    : KeywordData<IntegerDoubleVectorKeywordData &>(KeywordBase::VectorIntegerDoubleData, data),
      nRequiredIntegers_(nRequiredIntegers), nRequiredValues_(nRequiredValues)
{
}

IntegerDoubleVectorKeyword::~IntegerDoubleVectorKeyword() {}

// Return minimum number of arguments accepted
int IntegerDoubleVectorKeyword::minArguments() const { return (nRequiredIntegers_ + nRequiredValues_.value_or(1)); }

// Return maximum number of arguments accepted
int IntegerDoubleVectorKeyword::maxArguments() const { return (nRequiredIntegers_ + nRequiredValues_.value_or(99)); }

// Parse arguments from supplied LineParser, starting at given argument offset
bool IntegerDoubleVectorKeyword::read(LineParser &parser, int startArg, CoreData &coreData)
{
    std::vector<int> i;
    std::vector<double> d;

    auto argIndex = startArg;

    // Read integer values
    for (auto n = 0; n < nRequiredIntegers_; ++n, ++argIndex)
    {
        if (parser.hasArg(argIndex))
            i.push_back(parser.argi(argIndex));
        else
            return false;
    }

    // Read values and check number
    for (auto n = argIndex; n < parser.nArgs(); ++n)
        d.push_back(parser.argd(n));
    if (d.size() < nRequiredValues_.value_or(1))
    {
        if (nRequiredValues_ == -1)
            return Messenger::error("Keyword {} expects at least one double value in addition.\n", name());
        else
            return Messenger::error("Keyword {} expects exactly {} double {} in addition.\n", name(), nRequiredValues_.value(),
                                    nRequiredValues_.value() == 1 ? "value" : "values");
    }

    // Add tuple to vector
    data_.emplace_back(i, d);

    hasBeenSet();

    return true;
}

// Write keyword data to specified LineParser
bool IntegerDoubleVectorKeyword::write(LineParser &parser, std::string_view keywordName, std::string_view prefix)
{
    for (const auto &idData : data_)
    {
        std::string line;
        for (const auto &i : std::get<0>(idData))
            line += fmt::format("  {}", i);
        for (const auto &d : std::get<1>(idData))
            line += fmt::format("  '{}'", d);

        if (!parser.writeLineF("{}{}{}\n", prefix, keywordName, line))
            return false;
    }

    return true;
}
