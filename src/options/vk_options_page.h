/****************************************************************************
** VkOptionsPage definition
**  - Each vkObject has different options | flags | prefs, and
**    creates its own 'page', which is inherited from this base class.
**  - The 'page' is contained within the top-level Options Window.
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

#ifndef __VK_OPTIONS_PAGE_H
#define __VK_OPTIONS_PAGE_H

#include <QFrame>
#include <QHash>
#include <QList>
#include <QVBoxLayout>
#include <QWidget>

#include "objects/vk_objects.h"
#include "options/vk_option.h"


// ============================================================
class OptionWidget;

// ============================================================
typedef QHash<int, OptionWidget*>::iterator It_OptWidgHash;


// ============================================================
// VkOptionsPage: abstract base class
class VkOptionsPage : public QWidget
{
   Q_OBJECT
   
public:
   VkOptionsPage( VkObject* obj );
   ~VkOptionsPage();

   void init();

   void rejectEdits();
   bool applyEdits();
   bool isModified() { return m_mod; }

   static QFrame* sep( QWidget* parent );
   
signals:
   void modified();
   void apply();
   
protected slots:
   void updateEditList( bool, OptionWidget* );
   bool checkOption( OptionWidget* opt );

protected:
   OptionWidget* insertOptionWidget( int optid, QWidget* parent, bool mklabel );
      
protected:
   bool m_mod;
   int lineHeight;
   
   QVBoxLayout* pageTopVLayout;
   VkObject* m_vkObj;
   
   QHash<int, OptionWidget*> m_itemList;
   QList<OptionWidget*> m_editList;

private:
   virtual void setupOptions() = 0;
};

#endif  // __VK_OPTIONS_PAGE_H
