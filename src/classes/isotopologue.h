// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2020 Team Dissolve and contributors

#pragma once

#include "data/elements.h"
#include "templates/list.h"
#include "templates/refdatalist.h"
#include <memory>
#include <tuple>
#include <vector>

// Forward Declarations
class AtomType;
class Isotope;
class Species;

/*
 * Isotopologue Definition
 */
class Isotopologue : public ListItem<Isotopologue>
{
    public:
    Isotopologue();
    ~Isotopologue() = default;

    /*
     * Basic Information
     */
    private:
    // Parent Species
    Species *parent_;
    // Descriptive name
    std::string name_;

    public:
    // Set parent Species
    void setParent(Species *parent);
    // Return parent Species
    Species *parent() const;
    // Set name of Isotopologue
    void setName(std::string_view name);
    // Return name of Isotopologue
    std::string_view name() const;

    /*
     * Isotope Definition
     */
    private:
    // List of AtomType references and their assigned Isotopes
    std::vector<std::tuple<std::shared_ptr<AtomType>, Isotope *>> isotopes_;

    public:
    // Set AtomType/Isotope pair in list
    void setAtomTypeIsotope(std::shared_ptr<AtomType> at, Isotope *isotope);
    // Return Isotope for specified AtomType
    Isotope *atomTypeIsotope(std::shared_ptr<AtomType> at) const;
    // Return AtomType/Isotope pairs list
    const std::vector<std::tuple<std::shared_ptr<AtomType>, Isotope *>> &isotopes() const;
};
