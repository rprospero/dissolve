// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2021 Team Dissolve and contributors

#pragma once

#include "gui/referencepoint.h"
#include <QAbstractTableModel>
#include <QModelIndex>
#include <vector>

class DataManagerReferencePointModel : public QAbstractTableModel
{
    Q_OBJECT

    private:
    std::vector<ReferencePoint> *referencePoints_;

    public:
    DataManagerReferencePointModel(std::vector<ReferencePoint> *referencePoints = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Register all changes to the model
    void update();

    QHash<int, QByteArray> roleNames() const override;
};
