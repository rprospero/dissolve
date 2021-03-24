// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2021 Team Dissolve and contributors

#pragma once

#include "data/ff/atomtype.h"
#include "main/dissolve.h"

#include <QAbstractTableModel>
#include <QModelIndex>
#include <pugixml.hpp>

#include <map>
#include <tuple>
#include <vector>

class XmlTreeIndex
{
    private:
    int row_, column_;
    pugi::xml_node node_;

    public:
    XmlTreeIndex(int row, int column, pugi::xml_node node);
    XmlTreeIndex() = default;
};

class XmlTreeModel : public QAbstractItemModel
{
    Q_OBJECT
    private:
    const Dissolve &dissolve_;
    pugi::xml_node root_;
    XmlTreeIndex rootIndex_;

    public slots:
    // Read from an XML file
    void readFile(const pugi::xml_node &);

    public:
    XmlTreeModel(Dissolve &);

    QModelIndex parent(const QModelIndex &index) const override;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
};