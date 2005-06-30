/* ---------------------------------------------------------------------- 
 * Definition of MemcheckOptionsPage              memcheck_options_page.h
 * subclass of OptionsPage to hold memcheck-specific options | flags.
 * ---------------------------------------------------------------------- 
 */

#ifndef MEMCHECK_OPTIONS_PAGE_H
#define MEMCHECK_OPTIONS_PAGE_H

#include "options_page.h"


class MemcheckOptionsPage : public OptionsPage
{
	Q_OBJECT
public:
	MemcheckOptionsPage( QWidget* parent, VkObject* obj );
  bool applyOptions( int id, bool undo=false );

private slots:

};


#endif
