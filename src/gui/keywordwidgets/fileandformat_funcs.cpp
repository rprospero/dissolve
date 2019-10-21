/*
	*** Keyword Widget - FileAndFormat
	*** src/gui/keywordwidgets/fileandformat_funcs.cpp
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

#include "gui/keywordwidgets/fileandformat.h"
#include "gui/keywordwidgets/dropdown.h"
#include "gui/keywordwidgets/dialog.h"
#include "main/dissolve.h"
#include "io/fileandformat.h"
#include "genericitems/listhelper.h"
#include <QHBoxLayout>
#include <QComboBox>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>

// Constructor
FileAndFormatKeywordWidget::FileAndFormatKeywordWidget(QWidget* parent, KeywordBase* keyword, const CoreData& coreData) : KeywordDropDown(this), KeywordWidgetBase(coreData)
{
	// Create and set up our UI
	ui_.setupUi(dropWidget());

	// Cast the pointer up into the parent class type
	keyword_ = dynamic_cast<FileAndFormatKeyword*>(keyword);
	if (!keyword_)
	{
		Messenger::error("Couldn't cast base keyword '%s' into FileAndFormatKeyword.\n", keyword->name());
		return;
	}

	refreshing_ = true;

	// Populate combo with the file formats available
	ui_.FormatCombo->clear();
	for (int n=0; n < keyword_->data().nFormats(); ++n) ui_.FormatCombo->addItem(keyword_->data().format(n));

	// Set the availability of the setting button
	ui_.FormatOptionsButton->setEnabled(keyword_->data().keywords().keywords().nItems());

	// Connect signals / slots
	connect(ui_.FileEdit, SIGNAL(editingFinished()), this, SLOT(fileEdit_editingFinished()));
	connect(ui_.FileEdit, SIGNAL(returnPressed()), this, SLOT(fileEdit_returnPressed()));
	connect(ui_.FileSelectButton, SIGNAL(clicked(bool)), this, SLOT(fileSelectButton_clicked(bool)));
	connect(ui_.FormatCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(formatCombo_currentIndexChanged(int)));
	connect(ui_.FormatOptionsButton, SIGNAL(clicked(bool)), this, SLOT(formatOptionsButton_clicked(bool)));

	// Set current information
	updateWidgetValues(coreData_);

	refreshing_ = false;
}

/*
 * Widgets
 */

void FileAndFormatKeywordWidget::fileEdit_editingFinished()
{
	if (refreshing_) return;

	updateKeywordData();

	checkFileValidity();

	// Set summary text on KeywordDropDown button
	setSummaryText(keyword_->data().filename());

	emit(keywordValueChanged());
}

void FileAndFormatKeywordWidget::fileEdit_returnPressed()
{
	if (refreshing_) return;

	updateKeywordData();

	checkFileValidity();

	// Set summary text on KeywordDropDown button
	setSummaryText(keyword_->data().filename());

	emit(keywordValueChanged());
}

void FileAndFormatKeywordWidget::formatCombo_currentIndexChanged(int index)
{
	if (refreshing_) return;

	updateKeywordData();

	emit(keywordValueChanged());
}

void FileAndFormatKeywordWidget::formatOptionsButton_clicked(bool checked)
{
	// Construct a keywords dialog to edit the values
	KeywordsDialog keywordsDialog(this, keyword_->data().keywords(), coreData_);
	if (keywordsDialog.showOptions()) emit(keywordValueChanged());
}

void FileAndFormatKeywordWidget::fileSelectButton_clicked(bool checked)
{
	// Grab the target FileAndFormat
	FileAndFormat& fileAndFormat = keyword_->data();

	// Determine what sort of dialog we need to raise...
	QString filename;
	if (fileAndFormat.fileMustExist())
	{
		// Input File
		QFileInfo fileInfo(fileAndFormat.filename());
		filename = QFileDialog::getOpenFileName(this, tr("Select Input File"), fileInfo.absoluteFilePath(), tr("All Files (*.*)"));
	}
	else
	{
		// Output File
		QFileInfo fileInfo(fileAndFormat.filename());
		filename = QFileDialog::getSaveFileName(this, tr("Select Output File"), fileInfo.absoluteFilePath(), tr("All Files (*.*)"));
	}

	if (!filename.isEmpty())
	{
		// Set relative path to file, using the current input file as the reference point
		QFileInfo fileInfo(coreData_.inputFilename());
		ui_.FileEdit->setText(fileInfo.dir().relativeFilePath(filename));
		updateKeywordData();
		updateWidgetValues(coreData_);
		emit(keywordValueChanged());
	}
}

/*
 * Update
 */

// Check / set validity of current filename
void FileAndFormatKeywordWidget::checkFileValidity()
{
	// Grab the target FileAndFormat
	const FileAndFormat& fileAndFormat = keyword_->data();

	// If this is an export FileAndFormat, no need to show the indicator or check if the file exists
	if (fileAndFormat.fileMustExist())
	{
		ui_.FileExistsIndicator->setVisible(true);
		bool ok = fileAndFormat.hasFilename() ? QFile::exists(fileAndFormat.filename()) : false;
		ui_.FileExistsIndicator->setOK(ok);
		if (ok) setSummaryIcon(QPixmap(":/general/icons/general_true.svg"));
		else setSummaryIcon(QPixmap(":/general/icons/general_false.svg"));
	}
	else
	{
		ui_.FileExistsIndicator->setVisible(false);
	}
}

// Update value displayed in widget
void FileAndFormatKeywordWidget::updateValue()
{
	updateWidgetValues(coreData_);
}

// Update widget values data based on keyword data
void FileAndFormatKeywordWidget::updateWidgetValues(const CoreData& coreData)
{
	refreshing_ = true;

	// Grab the target FileAndFormat
	FileAndFormat& fileAndFormat = keyword_->data();

	// UPdate widgets
	ui_.FileEdit->setText(fileAndFormat.filename());
	ui_.FormatCombo->setCurrentIndex(fileAndFormat.formatIndex());
	checkFileValidity();

	// Set summary text on KeywordDropDown button
	setSummaryText(fileAndFormat.filename());
	
	refreshing_ = false;
}

// Update keyword data based on widget values
void FileAndFormatKeywordWidget::updateKeywordData()
{
	// Grab the target FileAndFormat
	FileAndFormat& fileAndFormat = keyword_->data();

	fileAndFormat.setFilename(qPrintable(ui_.FileEdit->text()));
	fileAndFormat.setFormatIndex(ui_.FormatCombo->currentIndex());

	keyword_->dataHasBeenSet();
}
