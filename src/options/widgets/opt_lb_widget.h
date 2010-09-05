/****************************************************************************
** LbWidget definition
** --------------------------------------------------------------------------
**
** Copyright (C) 2000-2009, OpenWorks LLP. All rights reserved.
** <info@open-works.co.uk>
**
** This file is part of Valkyrie, a front-end for Valgrind.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file COPYING included in the packaging of
** this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef __VK_OPTION_LB_WIDGET_H
#define __VK_OPTION_LB_WIDGET_H

#include <QListWidget>

#include "opt_base_widget.h"


// ============================================================
// listbox widget - for lists of files
class LbWidget : public OptionWidget
{
   Q_OBJECT
public:
   LbWidget( QWidget* parent, VkOption* vkopt, bool mklabel );
   ~LbWidget();

   QString printCurrValue();  // overloads base class

private slots:
   void popupMenu( const QPoint& );
   void update( const QString& txt );

private:
   QString lbText();
   void lbChanged();
   
private:
   QListWidget* m_lbox;
   QChar        m_sep;
};

#endif  // __VK_OPTION_LB_WIDGET_H
