/****************************************************************************
** VkLogPoller definition
**  - polls a log for updates
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

#ifndef VK_LOGPOLLER_H
#define VK_LOGPOLLER_H

#include <QObject>
#include <QTimer>


/*!
  NOTE:
  Future intention is to have another thread running select on the
  given file, so triggering signal when the file is updated.
  Or something...
*/

// ============================================================
/*!
  class VkLogPoller
*/
class VkLogPoller : public QObject
{
   Q_OBJECT
public:
   VkLogPoller( QObject* parent );
   ~VkLogPoller();
   
   void start( int interval = 100 ); // msec
   void stop();
   bool isActive();
   int  interval();
   
signals:
   void logUpdated();
   
private:
   QTimer* timer;
};

#endif // VK_LOGPOLLER_H
