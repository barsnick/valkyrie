/****************************************************************************
** CbWidget definition
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

#ifndef __VK_OPTION_CB_WIDGET_H
#define __VK_OPTION_CB_WIDGET_H

#include <QComboBox>

#include "opt_base_widget.h"


// ============================================================
class VkOption;

// ============================================================
// has-a QComboBox
class CbWidget : public OptionWidget
{
   Q_OBJECT
public:
   CbWidget( QWidget* parent, VkOption* vkopt, bool mklabel );
   ~CbWidget();
   
   QHBoxLayout* hlayout();
   
private slots:
   void update( const QString& txt );

private:
   int        m_currIdx;
   QComboBox* m_combo;
};

#endif  // __VK_OPTION_CB_WIDGET_H
