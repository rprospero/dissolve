/*
	*** Species Editor - Functions 
	*** src/gui/viewer/specieseditor_funcs.cpp
	Copyright T. Youngs 2013-2019

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

#include "gui/viewer/specieseditor.h"
#include "gui/widgets/elementselector.hui"
#include "procedure/nodes/addspecies.h"
#include "procedure/nodes/box.h"
#include "classes/coredata.h"
#include "classes/empiricalformula.h"
#include "classes/species.h"
#include "main/dissolve.h"
#include <QButtonGroup>

// Constructor
SpeciesEditor::SpeciesEditor(QWidget* parent) : QWidget(parent)
{
	// Set up our UI
	ui_.setupUi(this);

	// Create a button group for the interaction modes
	QButtonGroup* group = new QButtonGroup;
	group->addButton(ui_.InteractionViewButton);
	group->addButton(ui_.InteractionDrawButton);
	group->addButton(ui_.InteractionDeleteButton);

	// Connect signals / slots
	connect(ui_.SpeciesView, SIGNAL(dataModified()), this, SLOT(notifyDataModified()));
	connect(ui_.SpeciesView, SIGNAL(styleModified()), this, SLOT(notifyStyleModified()));
	connect(ui_.SpeciesView, SIGNAL(atomSelectionChanged()), this, SLOT(updateStatusBar()));
	connect(ui_.SpeciesView, SIGNAL(interactionModeChanged()), this, SLOT(updateStatusBar()));

	// Make sure our controls are consistent with the underlying viewer / data
	updateToolbar();
	updateStatusBar();
}

// Destructor
SpeciesEditor::~SpeciesEditor()
{
}

// Set main CoreData pointer
void SpeciesEditor::setCoreData(CoreData* coreData)
{
	coreData_ = coreData;
}

/*
 * UI
 */

// Notify that the style of displayed data in the underlying viewer has changed
void SpeciesEditor::notifyStyleModified()
{
	emit(styleModified());
}

// Notify that the displayed data in the underlying viewer has changed
void SpeciesEditor::notifyDataModified()
{
	emit(dataModified());
}

// Post redisplay in the underlying view
void SpeciesEditor::postRedisplay()
{
	ui_.SpeciesView->postRedisplay();
}

// Update toolbar to reflect current viewer state
void SpeciesEditor::updateToolbar()
{
	// Set current interaction mode
	switch (speciesViewer()->interactionMode())
	{
		case (SpeciesViewer::DefaultInteraction):
			ui_.InteractionViewButton->setChecked(true);
			break;
		case (SpeciesViewer::DrawInteraction):
			ui_.InteractionDrawButton->setChecked(true);
			break;
	}

	// Set drawing element symbol
	ui_.InteractionDrawElementButton->setText(speciesViewer()->drawElement()->symbol());

	// Set checkable buttons
	ui_.ViewAxesVisibleButton->setChecked(speciesViewer()->axesVisible());
	ui_.ViewSpheresButton->setChecked(speciesViewer()->renderableDrawStyle() != RenderableSpecies::LinesStyle);
}

// Update status bar
void SpeciesEditor::updateStatusBar()
{
	// Get displayed Species
	const Species* sp = speciesViewer()->species();

	// Set interaction mode text
	ui_.ModeLabel->setText(speciesViewer()->interactionModeText());

	// Set / update empirical formula for the Species and its current atom selection
	ui_.FormulaLabel->setText(sp ? EmpiricalFormula::formula(sp, true) : "--");
	ui_.SelectionLabel->setText(sp && (sp->nSelectedAtoms() > 0) ? EmpiricalFormula::formula(sp->selectedAtoms(), true) : "--");
}

/*
 * Species Viewer
 */

// Set target Species, updating widget as necessary
void SpeciesEditor::setSpecies(Species* sp)
{
	ui_.SpeciesView->setSpecies(sp);

	updateToolbar();
	updateStatusBar();
}

// Return contained SpeciesViewer
SpeciesViewer* SpeciesEditor::speciesViewer()
{
	return ui_.SpeciesView;
}

/*
 * Toolbar
 */

void SpeciesEditor::on_InteractionViewButton_clicked(bool checked)
{
	if (checked) speciesViewer()->setInteractionMode(SpeciesViewer::DefaultInteraction);
}

void SpeciesEditor::on_InteractionDrawButton_clicked(bool checked)
{
	if (checked) speciesViewer()->setInteractionMode(SpeciesViewer::DrawInteraction);
}

void SpeciesEditor::on_InteractionDrawElementButton_clicked(bool checked)
{
	// Select a new element for drawing
	bool ok;
	Element* newElement = ElementSelector::getElement(this, "Choose Element", "Select element to use for drawn atoms", speciesViewer()->drawElement(), &ok);
	if (!ok) return;

	speciesViewer()->setDrawElement(newElement);

	updateToolbar();
}

void SpeciesEditor::on_InteractionDeleteButton_clicked(bool checked)
{
	if (checked) speciesViewer()->setInteractionMode(SpeciesViewer::DeleteInteraction);
}

void SpeciesEditor::on_ViewResetButton_clicked(bool checked)
{
	speciesViewer()->view().showAllData();
	speciesViewer()->view().resetViewMatrix();

	speciesViewer()->postRedisplay();
}

void SpeciesEditor::on_ViewSpheresButton_clicked(bool checked)
{
	speciesViewer()->setRenderableDrawStyle(checked ? RenderableSpecies::SpheresStyle : RenderableSpecies::LinesStyle);

	speciesViewer()->notifyDataModified();

	speciesViewer()->postRedisplay();
}

void SpeciesEditor::on_ViewAxesVisibleButton_clicked(bool checked)
{
	speciesViewer()->setAxesVisible(checked);

	speciesViewer()->postRedisplay();
}

void SpeciesEditor::on_ViewCopyToClipboardButton_clicked(bool checked)
{
	speciesViewer()->copyViewToClipboard(checked);
}

// Tools
void SpeciesEditor::on_ToolsCalculateBondingButton_clicked(bool checked)
{
	// Get displayed Species
	Species* sp = speciesViewer()->species();
	if (!sp) return;

	// Calculate missing bonds
	sp->addMissingBonds();

	// Signal that the data shown has been modified
	speciesViewer()->postRedisplay();
	speciesViewer()->notifyDataModified();
}

void SpeciesEditor::on_ToolsMinimiseButton_clicked(bool checked)
{
	// Get displayed Species
	Species* sp = speciesViewer()->species();
	if (!sp) return;

	// Apply forcefield terms now?
	if (sp->forcefield())
	{
		sp->applyForcefieldTerms(*coreData_);
	}

	// Check that the Species set up is valid
	if (!sp->checkSetUp()) return;

	// Create a temporary CoreData and Dissolve
	CoreData temporaryCoreData;
	Dissolve temporaryDissolve(temporaryCoreData);
	if (!temporaryDissolve.registerMasterModules()) return;

	// Copy our target species to the temporary structure, and create a simple Configuration from it
	Species* temporarySpecies = temporaryDissolve.copySpecies(sp);
	Configuration* temporaryCfg = temporaryDissolve.addConfiguration();
	Procedure& generator = temporaryCfg->generator();
	generator.addRootSequenceNode(new BoxProcedureNode(Vec3<double>(1.0,1.0,1.0), Vec3<double>(90,90,90), true));
	AddSpeciesProcedureNode* addSpeciesNode = new AddSpeciesProcedureNode(temporarySpecies, 1, 0.0001);
	addSpeciesNode->setKeyword<bool>("Rotate", false);
	addSpeciesNode->setEnumeration<AddSpeciesProcedureNode::PositioningType>("Positioning", AddSpeciesProcedureNode::CentralPositioning);
	generator.addRootSequenceNode(addSpeciesNode);
	if (!temporaryCfg->initialiseContent(temporaryDissolve.worldPool(), 15.0)) return;

	// Create a Geometry Optimisation Module in a new processing layer, and set everything up
	if (!temporaryDissolve.createModuleInLayer("GeometryOptimisation", "Processing", temporaryCfg)) return;
	if (!temporaryDissolve.generatePairPotentials()) return;

	// Run the calculation
	if (!temporaryDissolve.prepare()) return;
	temporaryDissolve.iterate(1);

	// Copy the optimised coordinates from the temporary Configuration to the target Species
	int index = 0;
	for (SpeciesAtom* i : sp->atoms()) sp->setAtomCoordinates(i, temporaryCfg->atom(index++)->r());

	// Centre the Species back at the origin
	sp->centreAtOrigin();

	// Need to update the SpeciesViewer, and signal that the data shown has been modified
	speciesViewer()->view().showAllData();
	speciesViewer()->postRedisplay();
	speciesViewer()->notifyDataModified();
}
