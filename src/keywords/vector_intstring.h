// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2021 Team Dissolve and contributors

#pragma once

#include "keywords/data.h"
#include <optional>
#include <tuple>
#include <vector>

using IntegerStringVectorKeywordData = std::vector<std::tuple<std::vector<int>, std::vector<std::string>>>;

// Keyword with list of Tuples of Vectors
class IntegerStringVectorKeyword : public KeywordData<IntegerStringVectorKeywordData &>
{
    public:
    IntegerStringVectorKeyword(IntegerStringVectorKeywordData &data, int nRequiredIntegers,
                               std::optional<int> nRequiredValues = std::nullopt);
    ~IntegerStringVectorKeyword() override;

    /*
     * Data
     */
    private:
    // Number of required integers
    int nRequiredIntegers_;
    // Number of required values (or, if not specified, one or more)
    std::optional<int> nRequiredValues_;

    /*
     * Arguments
     */
    public:
    // Return minimum number of arguments accepted
    int minArguments() const override;
    // Return maximum number of arguments accepted
    int maxArguments() const override;
    // Parse arguments from supplied LineParser, starting at given argument offset
    bool read(LineParser &parser, int startArg, const CoreData &coreData) override;
    // Write keyword data to specified LineParser
    bool write(LineParser &parser, std::string_view keywordName, std::string_view prefix) const override;
};
