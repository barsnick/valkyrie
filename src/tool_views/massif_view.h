/* ---------------------------------------------------------------------
 * Definition of MassifView                                massif_view.h
 * Massif's personal window
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, Donna Robinson <donna@valgrind.org>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#ifndef __MASSIF_VIEW_H
#define __MASSIF_VIEW_H


#include "tool_view.h"

/* class MassifView ---------------------------------------------------- */
class Massif;
class MassifView : public ToolView
{
  Q_OBJECT
public:
  MassifView( QWidget* parent, Massif* ms );
  ~MassifView();

  /* called by massif: set state for buttons; set cursor state */
  void setState( bool run );

public slots:
  void toggleToolbarLabels( bool );

private:
  /* overriding to avoid casting everywhere */
  Massif* tool() { return (Massif*)m_tool; }
};



#endif
