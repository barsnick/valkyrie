/****************************************************************************
** VkObject definition
**  - abstract base class for all application 'objects'
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

#ifndef __VK_OBJECT_H
#define __VK_OBJECT_H

#include <QObject>
#include <QList>
#include <QWidget>

#include "options/vk_option.h"


// ============================================================
class VkOptionsPage;
class VkObject;


// ============================================================
typedef QList<VkObject*> VkObjectList;


// ============================================================
// VkObject: abstract base class
class VkObject : public QObject
{
   Q_OBJECT
public:
   VkObject( QString objectName );
   ~VkObject();
   
   virtual unsigned int maxOptId() = 0;
   
   /*! check argval for given option (updating argval if necessary).
       called by parseCmdArgs() and gui option pages */
   virtual int checkOptArg( int optid, QString& argval ) = 0;
   
   virtual void updateConfig( int optid, QString& argval );
   
   virtual void setConfigDefaults();
   
   virtual VkOptionsPage* createVkOptionsPage() = 0;
   
   VkOption* getOption( int optid );
   OptionHash& getOptions();
   
protected:
   virtual void setupOptions() = 0;
   VkOptionHash options;
};


#endif // __VK_OBJECT_H


