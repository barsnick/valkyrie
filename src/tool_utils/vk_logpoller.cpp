/* --------------------------------------------------------------------- 
 * Implementation of class VkLogPoller                  vk_logpoller.cpp
 * Polls a log for updates
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#include "vk_logpoller.h"

VkLogPoller::~VkLogPoller()
{
   /* m_timer deleted by it's parent: this */
}


VkLogPoller::VkLogPoller ( QObject* parent/*=0*/, const char* name/*=0*/ )
   : QObject( parent, name )
{
   m_timer = new QTimer( this );

   connect( m_timer, SIGNAL(timeout()),
            this,    SIGNAL(logUpdated()) );
}


void VkLogPoller::start()
{
   m_timer->start( 100 );
}


void VkLogPoller::stop( bool lastSignal/*=false*/ )
{
   if (m_timer->isActive())
      m_timer->stop();

   if (lastSignal) {
      /* emit final death throes... */
      QTimer::singleShot( 5, this, SIGNAL(logUpdated()) );

      /* ... and sometimes one just ain't enough... */
      QTimer::singleShot( 5, this, SIGNAL(logUpdated()) );  // TODO: why?!
   }
}


bool VkLogPoller::isActive()
{
   return m_timer->isActive();
}

