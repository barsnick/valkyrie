/* ---------------------------------------------------------------------- 
 * Definition of class OptionsPage                         options_page.h
 * 
 * Each vkObject has different options | flags | prefs, and 
 * creates its own 'page', which is inherited from this base class. 
 * The 'page' is contained within the top-level Options Window.
 * ---------------------------------------------------------------------- 
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#ifndef __VK_OPTIONS_PAGE_H
#define __VK_OPTIONS_PAGE_H


#include <qapplication.h>
#include <qgroupbox.h>
#include <qfiledialog.h>
#include <qintdict.h>
#include <qlayout.h>
#include <qradiobutton.h>

#include "options_widgets.h"



/* class OptionsPage --------------------------------------------------- */
class VkObject;
class OptionsPage : public QWidget
{
  Q_OBJECT

public:
  OptionsPage( QWidget* parent, VkObject* obj, const char* name );
  ~OptionsPage();

  bool acceptEdits();
  bool rejectEdits();
  bool applyEdits();
  bool isModified() { return mod; }

  void resetDefaults();
  virtual bool applyOptions( int id, bool undo=false ) = 0;

signals:
  void modified();
  void apply();

public slots:
  void updateEditList( bool, OptionWidget * );

protected:
  QFrame* sep( QWidget* parent, const char* name );
  OptionWidget* optionWidget( int optid, QWidget* parent, bool mklabel );

protected:
  bool mod; 
  int topSpace;
	int space, margin;

  VkObject * vkObj;

  /* prime numbers: 5, 7, 11, 13, 17, 19, 23, 29 */
  QIntDict<OptionWidget> itemList;
  QPtrList<OptionWidget> editList;
  QPtrList<OptionWidget> undoList;
};


#endif
