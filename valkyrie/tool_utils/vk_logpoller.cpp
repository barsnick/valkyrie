/* --------------------------------------------------------------------- 
 * Implementation of class VkLogPoller                  vk_logpoller.cpp
 * Polls a log for updates
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (C) 2000-2008, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file COPYING for the full license details.
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


bool VkLogPoller::start( int interval/*=100*/)
{
   return m_timer->start( interval );
}


void VkLogPoller::stop()
{
   if (m_timer->isActive())
      m_timer->stop();
}


bool VkLogPoller::isActive()
{
   return m_timer->isActive();
}

