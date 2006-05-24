/* ---------------------------------------------------------------------- 
 * Definition of class VkLogPoller                         vk_logpoller.h
 * Polls a log for updates
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#ifndef VK_LOGPOLLER_H
#define VK_LOGPOLLER_H

#include <qobject.h>
#include <qtimer.h>


/* Future intention is to have another thread running select on the
   given file, so triggering signal when the file is updated.
*/

/* class VkLogPoller ------------------------------------------------ */
class VkLogPoller : public QObject
{
   Q_OBJECT
public:
   VkLogPoller( QObject * parent = 0, const char * name = 0 );
   ~VkLogPoller();

   bool start( int interval=100 );
   void stop();
   bool isActive();
   int  interval();

signals:
   void logUpdated();

private:
   QTimer* m_timer;
};

#endif // VK_LOGPOLLER_H
