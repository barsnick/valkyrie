/* ---------------------------------------------------------------------
 * Definition of MassifView                                massif_view.h
 * Massif's personal window
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (C) 2000-2008, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file COPYING for the full license details.
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
   MassifView( QWidget* parent, const char* name );
   ~MassifView();

   /* called by massif: set state for buttons; set cursor state */
   void setState( bool run );

public slots:
   void toggleToolbarLabels( bool );
};



#endif
