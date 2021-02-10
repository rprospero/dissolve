// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2021 Team Dissolve and contributors

#pragma once

#include "gui/ui_importforcefieldwizard.h"
#include <QDialog>

// Forward Declarations
class Dissolve;
class ForcefieldWizard;

// Import ForcefieldWizard Dialog
class ImportForcefieldWizard : public QDialog
{
    Q_OBJECT

    public:
    ImportForcefieldWizard(QWidget *parent, const Dissolve &mainDissolveInstance) {
      ui_.setupUi(this);
    }
    ~ImportForcefieldWizard() {};

    private:
    // Main form declaration
    Ui::ImportForcefieldWizardDialog ui_;

    public:
    // Reset, ready for adding a new ForcefieldWizard
    void reset() {};
    /* // Copy imported ForcefieldWizard over to the specified Dissolve object */
    /* ForcefieldWizard *importForcefieldWizard(Dissolve &dissolve); */
};
