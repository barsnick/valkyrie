/* ---------------------------------------------------------------------
 * Definition of HelpAbout                                  help_about.h
 * Small tabbed dialog showing misc. info re license etc.
 * --------------------------------------------------------------------- 
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#ifndef __VK_HELP_ABOUT_H
#define __VK_HELP_ABOUT_H

#include <qdialog.h>
#include <qtabwidget.h>
#include <qtextedit.h>


/* class HelpAbout ----------------------------------------------------- */
class TextEdit;

class HelpAbout : public QDialog
{
   Q_OBJECT
public:
   enum TabId { ABOUT_VK=0, ABOUT_QT, LICENSE, SUPPORT };

   HelpAbout( QWidget* parent, TabId tabid );
   ~HelpAbout();

private slots:
   void showTab( QWidget* );

private:
   QString title;

   QTabWidget* tabParent;
   TextEdit* aboutVk;
   TextEdit* aboutQt;
   TextEdit* license;
   TextEdit* support;
};


/* class TextEdit ------------------------------------------------------ */
class TextEdit : public QTextEdit
{ 
public:
   TextEdit( QWidget* parent, HelpAbout::TabId tabid, const char* name );
   ~TextEdit();
   bool load();

private:
   bool loaded;
   QString html_file;
   HelpAbout::TabId tabId;
};


#endif
