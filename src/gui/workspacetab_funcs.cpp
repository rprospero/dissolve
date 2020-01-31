/*
	*** WorkspaceTab Functions
	*** src/gui/workspacetab_funcs.cpp
	Copyright T. Youngs 2012-2019

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

#include "gui/workspacetab.h"
#include "gui/gui.h"
#include "gui/tmdiarea.hui"
#include "gui/gettabnamedialog.h"
#include "gui/gizmos/gizmos.h"
#include "main/dissolve.h"
#include "classes/configuration.h"
#include "base/lineparser.h"
#include "base/sysfunc.h"
#include "templates/variantpointer.h"
#include <QMdiSubWindow>
#include <QMenu>

// Static Singletons
RefList<Gizmo> WorkspaceTab::allGizmos_;

// Constructor
WorkspaceTab::WorkspaceTab(DissolveWindow* dissolveWindow, Dissolve& dissolve, MainTabsWidget* parent, const char* title) : ListItem<WorkspaceTab>(), MainTab(dissolveWindow, dissolve, parent, title, this)
{
	ui.setupUi(this);

	Locker refreshLocker(refreshLock_);

	// Add a TMdiArea to the main layout
	mdiArea_ = new TMdiArea(dissolveWindow);
	ui.verticalLayout->addWidget(mdiArea_);
	connect(mdiArea_, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));
}

// Destructor
WorkspaceTab::~WorkspaceTab()
{
}

/*
 * MainTab Reimplementations
 */

// Return tab type
MainTab::TabType WorkspaceTab::type() const
{
	return MainTab::WorkspaceTabType;
}

// Raise suitable dialog for entering / checking new tab name
QString WorkspaceTab::getNewTitle(bool& ok)
{
	// Get a new, valid name for the Configuration
	GetTabNameDialog nameDialog(this, dissolveWindow_->allTabs());
	ok = nameDialog.get(this, title());

	if (ok)
	{
		// Rename our Workspace, and flag that our data has been modified
		title_ = qPrintable(nameDialog.newName());
	}

	return nameDialog.newName();
}

// Return whether the title of the tab can be changed
bool WorkspaceTab::canChangeTitle() const
{
	return true;
}

/*
 * Update
 */

// Update controls in tab
void WorkspaceTab::updateControls()
{
	// Update our MDI subwindows
	for (Gizmo* gizmo : gizmos_) gizmo->updateControls();
}

// Disable sensitive controls within tab
void WorkspaceTab::disableSensitiveControls()
{
	// Disable sensitive controls in subwindows
	for (Gizmo* gizmo : gizmos_) gizmo->disableSensitiveControls();
}

// Enable sensitive controls within tab
void WorkspaceTab::enableSensitiveControls()
{
	// Enable sensitive controls in subwindows
	for (Gizmo* gizmo : gizmos_) gizmo->enableSensitiveControls();
}

/*
 * Gizmo Management
 */

// Remove Gizmo with specified unique name
void WorkspaceTab::removeGizmo(QString uniqueName)
{
	// Find the Gizmo...
	Gizmo* gizmo = findGizmo(qPrintable(uniqueName));
	if (!gizmo)
	{
		Messenger::error("Received signal to remove gizmo '%s' but it cannot be found...\n", qPrintable(uniqueName));
		return;
	}

	gizmos_.remove(gizmo);
	allGizmos_.remove(gizmo);
}

// Return unique name for Gizmo based on basename provided
const char* WorkspaceTab::uniqueGizmoName(const char* base)
{
	static CharString uniqueName;
	CharString baseName = base;
	uniqueName = baseName;
	int suffix = 0;

	// Must always have a baseName
	if (baseName.isEmpty()) baseName = "NewGizmo";

	// Find an unused name starting with the baseName provided
	while (findGizmo(uniqueName))
	{
		// Increase suffix value and regenerate uniqueName from baseName
		++suffix;
		uniqueName.sprintf("%s%i", baseName.get(), suffix);
	}

	return uniqueName;
}

// Create Gizmo with specified type
Gizmo* WorkspaceTab::createGizmo(const char* type)
{
	Gizmo* gizmo = NULL;
	QWidget* widget = NULL;

	// Check the type of the provided gizmo...
	if (DissolveSys::sameString(type, "Graph"))
        {
		GraphGizmo* graph = new GraphGizmo(dissolveWindow_->dissolve(), uniqueGizmoName("Graph"));
		connect(graph, SIGNAL(windowClosed(QString)), this, SLOT(removeGizmo(QString)));
		gizmo = graph;
		widget = graph;
        }
	else if (DissolveSys::sameString(type, "Integrator1D"))
        {
		Integrator1DGizmo* integrator1D = new Integrator1DGizmo(dissolveWindow_->dissolve(), uniqueGizmoName("Integrator1D"));
		connect(integrator1D, SIGNAL(windowClosed(QString)), this, SLOT(removeGizmo(QString)));
		gizmo = integrator1D;
		widget = integrator1D;
        }
	else
	{
		Messenger::error("Couldn't add gizmo to workspace '%s - unrecognised type '%s' encountered.\n", title(), type);
		return NULL;
	}

	// Create a new window for the Gizmo's widget and show it
	QMdiSubWindow* window = mdiArea_->addSubWindow(widget);
	window->setWindowTitle(gizmo->uniqueName());
	window->show();
	gizmo->setWindow(window);

	// Update the Gizmo's controls, and add it to our lists
	gizmo->updateControls();
	gizmos_.own(gizmo);
	allGizmos_.append(gizmo);

	return gizmo;
}

// Find Gizmo with unique name provided
Gizmo* WorkspaceTab::findGizmo(const char* uniqueName)
{
	for (Gizmo* gizmo : allGizmos_) if (DissolveSys::sameString(gizmo->uniqueName(), uniqueName)) return gizmo;

	return NULL;
}

/*
 * Context Menu
 */

// Custom context menu requested
void WorkspaceTab::showContextMenu(const QPoint& pos)
{
	// Check that we are not over an existing gizmo
	for (Gizmo* gizmo : gizmos_) if (gizmo->window()->geometry().contains(pos)) return;

	QMenu menu;
	menu.setFont(font());
	QFont italicFont = font();
	italicFont.setItalic(true);

	// Gizmos
	QMenu* gizmoMenu = menu.addMenu("Add &Gizmo...");
	gizmoMenu->setFont(font());
	connect(gizmoMenu->addAction("Graph"), SIGNAL(triggered(bool)), this, SLOT(contextMenuAddGizmo(bool)));
	connect(gizmoMenu->addAction("Integrator1D"), SIGNAL(triggered(bool)), this, SLOT(contextMenuAddGizmo(bool)));

	// Modules within Configurations
// 	menuItem = parent->addAction("Configurations");
// 	menuItem->setFont(italicFont);
// 	menuItem->setEnabled(false);
// 	for (Configuration* cfg : dissolve_.configurations())
// 	{
// 		QMenu* cfgMenu = parent->addMenu(cfg->name());
// 		if (cfg->nModules() == 0)
// 		{
// 			QAction* moduleItem = cfgMenu->addAction("No Local Modules");
// 			moduleItem->setFont(italicFont);
// 			moduleItem->setEnabled(false);
// 		}
// 		for (Module* module: cfg->modules())
// 		{
// 			QAction* moduleItem = cfgMenu->addAction(CharString("%s (%s)", module->type(), module->uniqueName()).get());
// 			moduleItem->setData(VariantPointer<Module>(module));
// 			connect(moduleItem, SIGNAL(triggered(bool)), this, SLOT(contextMenuModuleSelected(bool)));
// 		}
// 	}

	menu.exec(mapToGlobal(pos));
}

// Create Gizmo from context menu item
void WorkspaceTab::contextMenuAddGizmo(bool checked)
{
	// Get the sender QAction
	QAction* action = dynamic_cast<QAction*>(sender());
	if (!action) return;

	// The text of the sender QAction is the type of the Gizmo we need to create
	createGizmo(qPrintable(action->text()));
}

/*
 * State
 */

// Read widget state through specified LineParser
bool WorkspaceTab::readState(LineParser& parser, const CoreData& coreData)
{
	// Read tab state information:   nGizmos
	if (parser.getArgsDelim(LineParser::Defaults) != LineParser::Success) return false;
	const int nWidgets = parser.argi(0);

	// Read in widgets
	for (int n=0; n<nWidgets; ++n)
	{
		// Read line from the file, which should contain the gizmo type
		if (parser.getArgsDelim() != LineParser::Success) return false;
		Gizmo* gizmo = createGizmo(parser.argc(0));
		if (gizmo == NULL) return Messenger::error("Unrecognised gizmo type '%s' in workspace '%s'.\n", parser.argc(0), title());

		// Read in the widget's geometry / state / flags
		if (parser.getArgsDelim(LineParser::Defaults) != LineParser::Success) return false;
		gizmo->window()->setGeometry(parser.argi(0), parser.argi(1), parser.argi(2), parser.argi(3));
		// -- Is the window maximised, or shaded?
		if (parser.argb(4)) gizmo->window()->showMaximized();
		else if (parser.argb(5)) gizmo->window()->showShaded();

		// Now call the widget's local readState()
		if (!gizmo->readState(parser)) return false;
	}

	return true;
}

// Write widget state through specified LineParser
bool WorkspaceTab::writeState(LineParser& parser) const
{
	// Write tab state information:   nGizmos
	if (!parser.writeLineF("%i      # NGizmos\n", gizmos_.nItems())) return false;

	// Loop over our subwindow list
	for (Gizmo* gizmo : gizmos_)
	{
		// Write Gizmo type
		if (!parser.writeLineF("%s\n", gizmo->type())) return false;

		// Write window geometry / state
		QRect geometry = gizmo->window()->geometry();
		if (!parser.writeLineF("%i %i %i %i %s %s\n", geometry.x(), geometry.y(), geometry.width(), geometry.height(), DissolveSys::btoa(gizmo->window()->isMaximized()), DissolveSys::btoa(gizmo->window()->isShaded()))) return false;

		// Write gizmo-specific state information
		if (!gizmo->writeState(parser)) return false;
	}

	return true;
}
