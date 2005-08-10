/* ---------------------------------------------------------------------
 * Definition of VKLogMerge                                vk_logmerge.h
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#ifndef __VK_LOGMERGE_OBJECT_H
#define __VK_LOGMERGE_OBJECT_H

#include <qdom.h>
#include <qstring.h>

bool mergeVgLogs( QDomDocument& master_doc, QDomDocument& slave_doc );

#endif // #ifndef __VK_LOGMERGE_OBJECT_H
