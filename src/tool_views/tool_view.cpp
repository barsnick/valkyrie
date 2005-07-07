/* ---------------------------------------------------------------------
 * Implementation of class ToolView                        tool_view.cpp
 * Base class for all tool views
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, Donna Robinson <donna@valgrind.org>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#include "tool_view.h"

#include <qlayout.h>


ToolView::~ToolView() { }

ToolView::ToolView( QWidget* parent, QString name, VkObject::ObjectId id )
  : QWidget( parent, name, WDestructiveClose )
{
  objId = id;

  name[0] = name[0].upper();
  setCaption( name );

  toolBar = 0;

#if 0 // CAB: Need this?
  QWidget* central = new QWidget( this );
  setCentralWidget( central );
  QVBoxLayout* topLayout = new QVBoxLayout( central, 5, 5 );
  topLayout->setResizeMode( QLayout::FreeResize );
#endif
}
