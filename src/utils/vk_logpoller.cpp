/****************************************************************************
** VkLogPoller implementation
**  - polls a log for updates
** --------------------------------------------------------------------------
**
** Copyright (C) 2000-2010, OpenWorks LLP. All rights reserved.
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

#include "utils/vk_logpoller.h"


VkLogPoller::VkLogPoller( QObject* parent )
   : QObject( parent )
{
   this->setObjectName( "logpoller" );
   
   timer = new QTimer( this );
   
   connect( timer, SIGNAL( timeout() ),
            this,  SIGNAL( logUpdated() ) );
}


VkLogPoller::~VkLogPoller()
{
   // timer deleted by it's parent: this
}


// start with interval msecs
void VkLogPoller::start( int interval )
{
   timer->start( interval );
}


void VkLogPoller::stop()
{
   if ( timer->isActive() ) {
      timer->stop();
   }
}


bool VkLogPoller::isActive()
{
   return timer->isActive();
}

