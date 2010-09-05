/****************************************************************************
** SpWidget definition
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

#ifndef __VK_OPTION_SP_WIDGET_H
#define __VK_OPTION_SP_WIDGET_H

#include <QSpinBox>

#include "opt_base_widget.h"


// ============================================================
class IntSpin;

// ============================================================
class SpWidget : public OptionWidget
{
   Q_OBJECT
public:
   SpWidget( QWidget* parent, VkOption* vkopt, bool mklabel, int num_sections );
   ~SpWidget();
   
   void addSection( int min, int max, int defval = 0,
                    int step = 1, QString sep_char = " : " );
   QHBoxLayout* hlayout();

private:
   void update( const QString& txt );

private:
   IntSpin* m_intspin;
   int      m_numSections;
};




class IntSpin : public QSpinBox
{
public:
   IntSpin( QWidget* parent );
   
   void addSection( int min, int max, int defval,
                    int step, QString sep_char );
                    
private:
   void stepBy( int step );
   bool use_pwr2;
};


#endif  // __VK_OPTION_SP_WIDGET_H
