/* ---------------------------------------------------------------------- 
 * Definition of class VkObject                              vk_objects.h
 * 
 * Essential functionality is contained within a VkObject.
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (C) 2000-2008, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file COPYING for the full license details.
 */

/* TODO: don't have enum values for the objOpts; instead, init an int
   array in the constructor.  This means will have to implement addOpt
   somewhat differently, as won't have enums available */

#ifndef __VK_OBJECTS_H
#define __VK_OBJECTS_H


#include <qkeysequence.h>
#include <qobject.h>
#include <qptrlist.h>
#include <qmainwindow.h>

#include "vk_option.h"          /* class Option */
#include "options_window.h"
#include "options_page.h"

class VkObject;
typedef QPtrList<VkObject> VkObjectList;
typedef QPtrList<Option>   OptionList;


/* class VkObject ------------------------------------------------------ */
class VkObject : public QObject 
{
   Q_OBJECT
public:
   /* VkObject ids: tool ids begin from ID_TOOL0 */
   enum { ID_VALKYRIE=0, ID_VALGRIND, ID_TOOL0/*first tool*/  };

   VkObject( const QString& capt, const QString& txt,
             const QKeySequence& key, int objId );
   ~VkObject();

   QString name()          const { return m_caption.lower(); }
   QString title()         const { return m_caption;         }
   QString accelTitle()    const { return m_accelText;       }
   QKeySequence accelKey() const { return m_accel_Key;       }

   /* return max number of opts */
   virtual unsigned int maxOptId() = 0;

   /* check argval for this option, updating if necessary.
      called by parseCmdArgs() and gui option pages */
   virtual int checkOptArg(int optid, QString& argval ) = 0; 

   /* returns a list of options to be written to the config file */
   virtual QString configEntries();

   /* also called by OptionsPage::optionWidget() */
   Option* findOption( int optid );

   virtual OptionsPage* createOptionsPage( OptionsWindow* parent ) = 0;

   OptionList optList() { return m_optList; }

   int objId() { return m_objId; }

protected:
   void addOpt( int key, VkOPTION::ArgType arg_type, VkOPTION::WidgetType w_type,
                QString cfg_group,  QChar   short_flag, QString long_flag, 
                QString flag_desc,  QString poss_vals,  QString default_val,
                QString shelp,      QString lhelp,      const char* url );

protected:
   QString      m_caption;      /* eg. Memcheck */

   QString      m_accelText;    /* eg. &Memcheck */
   QKeySequence m_accel_Key;    /* accelerator key */

   OptionList   m_optList;      /* list of options for this object */

private:
   int m_objId;
};


#endif // #ifndef __VK_OBJECTS_H


