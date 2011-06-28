/****************************************************************************
** OptionWidget definition
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

#ifndef __VK_OPTION_WIDGETS_H
#define __VK_OPTION_WIDGETS_H

#include <QHBoxLayout>
#include <QLabel>
#include <QString>
#include <QWidget>
#include <QVBoxLayout>

#include <iostream>
using namespace std;


// ============================================================
class VkOption;


// ============================================================
class OptionWidget : public QObject
{
   Q_OBJECT
public:
   OptionWidget( QWidget* parent, VkOption* vkopt, bool mklabel );
   ~OptionWidget();
   
   int id();
   QLabel*  label();
   QWidget* widget();

   QString currValue();
   QString initValue();
   QString changedValue();
   void setEnabled( bool enable );

   void setValue( const QString& txt );
   void reset();
   void resetDefault(); //TODO: not used: wanted?

   virtual QString printCurrValue();

   virtual void saveEdit();
   virtual void cancelEdit();

   virtual QHBoxLayout* hlayout();
   virtual QVBoxLayout* vlayout();
   
signals:
   void valueChanged( bool, OptionWidget* );
   void editDone( OptionWidget* );

protected slots:
   virtual void setCurrValue( const QString& txt );
   virtual void update( const QString& txt ) = 0;

protected:
   QWidget*     m_widg;
   QLabel*      m_wLabel;
   QHBoxLayout* m_hBox;
   QVBoxLayout* m_vBox;
   
   QString      m_initialValue;
   QString      m_currentValue;
   VkOption*    m_opt;
};

#endif
