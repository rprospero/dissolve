/*
	*** SpeciesTab Functions - Sites
	*** src/gui/speciestab_sites.cpp
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

#include "gui/speciestab.h"
#include "templates/variantpointer.h"
#include "gui/helpers/listwidgetupdater.h"

/*
 * Private Functions
 */

// Return currently-selected SpeciesSite
SpeciesSite* SpeciesTab::currentSite()
{
	// Get current item from tree, and check the parent item
	QListWidgetItem* item = ui_.SiteList->currentItem();
	if (!item) return NULL;
	return VariantPointer<SpeciesSite>(item->data(Qt::UserRole));
}

/*
 * Private Slots
 */

void SpeciesTab::on_SiteAddButton_clicked(bool checked)
{
	species_->addSite("NewSite");

	updateSitesTab();
}

void SpeciesTab::on_SiteRemoveButton_clicked(bool checked)
{
	printf("NOT IMPLEMENTED YET\n");
}

void SpeciesTab::on_SiteList_currentItemChanged(QListWidgetItem* currentItem, QListWidgetItem* previousItem)
{
	ui_.SiteViewerWidget->setSite(currentSite());

	updateSitesTab();
}

void SpeciesTab::on_SiteList_itemChanged(QListWidgetItem* item)
{
	if (refreshLock_.isLocked()) return;
}

void SpeciesTab::on_SiteOriginMassWeightedCheck_clicked(bool checked)
{
	SpeciesSite* site = currentSite();
	if (refreshLock_.isLocked() || (!site)) return;

	site->setOriginMassWeighted(checked);

	ui_.SiteViewerWidget->postRedisplay();
}

/*
 * Public Functions
 */

// Update sites tab
void SpeciesTab::updateSitesTab()
{
	Locker refreshLocker(refreshLock_);

	// Check for valid species
	if (!species_)
	{
		ui_.SiteList->clear();
		return;
	}

	// Grab and store current site for reference
	SpeciesSite* current = currentSite();

	// Update the site list
	ListWidgetUpdater<SpeciesTab,SpeciesSite> siteUpdater(ui_.SiteList, species_->sites(), Qt::ItemIsEnabled);
	if ((current == NULL) && (species_->nSites() != 0)) ui_.SiteList->setCurrentRow(0);

	// Check for current site
	SpeciesSite* site = currentSite();
	ui_.SiteRemoveButton->setEnabled(site != NULL);
	ui_.SiteOriginGroup->setEnabled(site != NULL);
	ui_.SiteXAxisGroup->setEnabled(site != NULL);
	ui_.SiteYAxisGroup->setEnabled(site != NULL);
	if (!site)
	{
		ui_.SiteOriginAtomsEdit->clear();
		ui_.SiteXAxisAtomsEdit->clear();
		ui_.SiteYAxisAtomsEdit->clear();
		return;
	}

	// Set origin atom indices
	Array<int> originAtoms = site->originAtomIndices();
	QString originText = originAtoms.nItems() == 0 ? QString() : QString::number(originAtoms.constAt(0));
	for (int n=1; n<originAtoms.nItems(); ++n) originText += QString(" %1").arg(originAtoms.constAt(n));
	ui_.SiteOriginAtomsEdit->setText(originText);
	ui_.SiteOriginMassWeightedCheck->setCheckState(site->originMassWeighted() ? Qt::Checked : Qt::Unchecked);

	// Set x axis atom indices
	Array<int> xAxisAtoms = site->xAxisAtomIndices();
	QString xAxisText = xAxisAtoms.nItems() == 0 ? QString() : QString::number(xAxisAtoms.constAt(0));
	for (int n=1; n<xAxisAtoms.nItems(); ++n) xAxisText += QString(" %1").arg(xAxisAtoms.constAt(n));
	ui_.SiteXAxisAtomsEdit->setText(xAxisText);

	// Set y axis atom indices
	Array<int> yAxisAtoms = site->yAxisAtomIndices();
	QString yAxisText = yAxisAtoms.nItems() == 0 ? QString() : QString::number(yAxisAtoms.constAt(0));
	for (int n=1; n<yAxisAtoms.nItems(); ++n) yAxisText += QString(" %1").arg(yAxisAtoms.constAt(n));
	ui_.SiteYAxisAtomsEdit->setText(yAxisText);

	// If the current site has changed, also regenerate the SpeciesSite renderable
	if (current != currentSite()) ui_.SiteViewerWidget->setSite(currentSite());
}