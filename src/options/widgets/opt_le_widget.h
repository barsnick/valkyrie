/****************************************************************************
** LeWidget definition
** --------------------------------------------------------------------------
**
** Copyright (C) 2000-2011, OpenWorks LLP. All rights reserved.
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

#ifndef __VK_OPTION_LE_WIDGET_H
#define __VK_OPTION_LE_WIDGET_H

#include <QLineEdit>
#include <QPushButton>

#include "opt_base_widget.h"


// ============================================================
class VkOption;


// ============================================================
// has-a QLineEdit
class LeWidget : public OptionWidget
{
   Q_OBJECT
public:
   LeWidget( QWidget* parent, VkOption* vkopt, bool mklabel );
   ~LeWidget();
   
   void addButton( QWidget* parent, const QObject* receiver,
                   const char* slot, QString txt = QString::null,
                   bool icon = false );
   void setReadOnly( bool );
   QPushButton* button();
   QHBoxLayout* hlayout();
   
public slots:
   void setDisabled( bool disable );

private slots:
   void editingFinished();
   void update( const QString& txt );

private:
   void setCurrValue( const QString& value ); // overloaded

private:
   QLineEdit*   m_ledit;
   QPushButton* m_pb;
};

#endif  // __VK_OPTION_LE_WIDGET_H
