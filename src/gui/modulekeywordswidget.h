/*
	*** Module Keywords Widget
	*** src/gui/modulekeywordswidget.h
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

#ifndef DISSOLVE_MODULEKEYWORDSWIDGET_H
#define DISSOLVE_MODULEKEYWORDSWIDGET_H

#include "gui/keywordwidgets/base.h"
#include "templates/list.h"
#include "templates/reflist.h"
#include <QToolBox>

// Forward Declarations
class Dissolve;
class DissolveWindow;
class KeywordBase;
class KeywordGroup;
class KeywordList;

// Module Keywords Widget
class ModuleKeywordsWidget : public QToolBox
{
	public:
	// Constructor / Destructor
	ModuleKeywordsWidget(QWidget* parent);
	~ModuleKeywordsWidget();


	/*
	 * Controls
	 */
	private:
	// Whether the widget is currently refreshing
	bool refreshing_;
	// Pointer to DissolveWindow
	DissolveWindow* dissolveWindow_;
	// List of keyword widgets displayed
	RefList<KeywordWidgetBase> keywordWidgets_;

	private:
	// Create widget for specified keyword
	QWidget* createKeywordWidget(RefList<KeywordWidgetBase>& keywordWidgets, KeywordBase* keyword, const CoreData& coreData);

	public:
	// Set up controls for specified keyword list
	void setUp(DissolveWindow* dissolveWindow, const List<KeywordBase>& keywords, const List<KeywordGroup>& groups);
	// Update controls within widget
	void updateControls();
};

#endif
