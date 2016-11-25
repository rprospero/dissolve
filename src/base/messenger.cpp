/*
	*** Messaging Routines
	*** src/base/messenger.cpp
	Copyright T. Youngs 2012-2016

	This file is part of dUQ.

	dUQ is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	dUQ is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with dUQ.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "base/messenger.h"
#include "base/sysfunc.h"
#include "base/processpool.h"
#include <stdarg.h>
#include <stdio.h>

// Static Members
bool Messenger::quiet_ = false;
bool Messenger::verbose_ = false;
bool Messenger::redirect_ = false;
bool Messenger::masterOnly_ = false;
LineParser Messenger::parser_;
char Messenger::text_[8096];
char Messenger::workingText_[8096];

// Constructor
Messenger::Messenger()
{
}

// Set status of quiet mode
void Messenger::setQuiet(bool b)
{
	quiet_ = b;
}

// Set status of verbose mode
void Messenger::setVerbose(bool b)
{
	verbose_ = b;
}

// Set status of master-only mode
void Messenger::setMasterOnly(bool b)
{
	masterOnly_ = b;
}

// Master text creation / formatting routine
void Messenger::createText(const char* indentText, const char* format, va_list arguments)
{
	text_[0] = '\0';
	
	// First, vsprintf the supplied format/arguments into workingText_, prepending a newline to it
	Dnchar newFormat;
	newFormat.sprintf("\n%s", format);
	vsprintf(workingText_, newFormat.get(), arguments);

	// Now, use strtok to split the workingText_ up into lines, prepending the indentText and/or process id to each
	Dnchar prependedLine;
	char* line = strtok(workingText_, "\n");
	while (line != NULL)
	{
		// If we were given some 'indentText', print copies of it before the message
		if (indentText != NULL)
		{
			if (redirect_ || (ProcessPool::nWorldProcesses() == 1)) prependedLine.sprintf("%s %s\n", indentText, line);
			else prependedLine.sprintf("[%i] %s %s\n", ProcessPool::worldRank(), indentText, line);
		}
		else
		{
			if (redirect_ || (ProcessPool::nWorldProcesses() == 1)) prependedLine.sprintf("%s\n", line);
			else prependedLine.sprintf("[%i] %s\n", ProcessPool::worldRank(), line);
		}
		strcat(text_, prependedLine.get());

		line = strtok(NULL, "\n");
	}
}

// Print text
void Messenger::printText(const char* text)
{
	// Print the passed string
	if (redirect_) parser_.writeLineF("%s", text);
	else printf("%s", text);
}

// Master text creation / formatting routine
void Messenger::createAndPrintText(const char* indentText, const char* format, va_list arguments)
{
	if (masterOnly_ && (!ProcessPool::isWorldMaster())) return;

	createText(indentText, format, arguments);

	printText(text_);
}

// Print standard message
void Messenger::print(const char* fmt, ...)
{
	if (quiet_) return;

	va_list arguments;
	va_start(arguments,fmt);
	createAndPrintText(NULL, fmt, arguments);
	va_end(arguments);
}

// Print verbose message
void Messenger::printVerbose(const char* fmt, ...)
{
	if (quiet_ || (!verbose_)) return;

	va_list arguments;
	va_start(arguments,fmt);
	createAndPrintText(NULL, fmt, arguments);
	va_end(arguments);
}

// Print error message
void Messenger::error(const char* fmt, ...)
{
	va_list arguments;
	va_start(arguments,fmt);
	printText("\n***  ERROR\n");
	createAndPrintText("***  ERROR    ", fmt, arguments);
	printText("***  ERROR\n\n");
	va_end(arguments);
}

// Print warning message
void Messenger::warn(const char* fmt, ...)
{
	va_list arguments;
	va_start(arguments, fmt);
	printText("\n!!! WARNING\n");
	createAndPrintText("!!! WARNING   ", fmt, arguments);
	printText("!!! WARNING\n\n");
	va_end(arguments);
}

// Print banner message of specified width
void Messenger::banner(const char* fmt, ...)
{
	static Dnchar bannerChars;
	const int width = 80;
	if (bannerChars.length() < width)
	{
		bannerChars.createEmpty(width+1);
		bannerChars.fill('=');
	}

	// First, create the text using vsprintf
	va_list arguments;
	va_start(arguments, fmt);
	vsprintf(workingText_, fmt, arguments);
	va_end(arguments);
	Dnchar bannerText = workingText_;

	// Now, get the length of the banner text and create a format for printing it into a line 80 chars wide
	int leftPad = (width - bannerText.length()) / 2 - 1;
	int rightPad = width - bannerText.length() - leftPad - 2;
	char bannerFormat[64];
	sprintf(bannerFormat, "%%s\n%%c%%%is%%s%%%is%%c\n%%s", leftPad, rightPad);

	// Finally, print the banner
	printText("\n");
	print(bannerFormat, bannerChars.get(), '*', " ", bannerText.get(), " ", '*', bannerChars.get());
	printText("\n");
}

/*
 * File Redirection
 */

// Enable redirection of all messaging to specified file
bool Messenger::enableRedirect(const char* filename)
{
	parser_.openOutput(filename, true);
	if (!parser_.isFileGoodForWriting())
	{
		Messenger::print("Couldn't open output file '%s' for writing.\n", filename);
		return false;
	}
	
	redirect_ = true;
	return true;
}
