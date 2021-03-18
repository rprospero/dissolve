#include "gui/models/speciesAtomModel.h"
#include "classes/atomtype.h"
#include "classes/species.h"

SpeciesAtomModel::SpeciesAtomModel(Species *species) : species_(species) {}

int SpeciesAtomModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return species_->nAtoms();
}

int SpeciesAtomModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 6;
}

QVariant SpeciesAtomModel::data(const QModelIndex &index, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    auto atom = species_->atom(index.row());

    switch (index.column())
    {
        case 0:
            return Elements::name(atom->Z()).data();
        case 1:
            return atom->atomType()->name().data();
        case 2:
        case 3:
        case 4:
            return atom->r().get(index.column() - 2);
        case 5:
            return atom->charge();
        default:
            return QVariant();
    }
}

QVariant SpeciesAtomModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return QVariant();
    switch (section)
    {
        case 0:
            return "Element";
        case 1:
            return "Atom Type";
        case 2:
            return "X";
        case 3:
            return "Y";
        case 4:
            return "Z";
        case 5:
            return "Charge";
        default:
            return QVariant();
    }
}

Qt::ItemFlags SpeciesAtomModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags result = Qt::ItemIsEnabled;
    if (index.column() == 4)
        result |= Qt::ItemIsEditable;
    return result;
}

// bool SpeciesAtomModel::setData(const QModelIndex &index, const QVariant &value, int role)
// {
//     if (index.row() >= atoms_.size() || index.row() < 0)
//         return false;

//     // Only set the linked AtomType
//     if (index.column() != 4)
//         return false;

//     // Find the requested AtomType
//     auto type = dissolve_.findAtomType(value.toString().toStdString());
//     if (!type)
//         return false;

//     std::get<4>(atoms_[index.row()]) = type->index();

//     return true;
// }
