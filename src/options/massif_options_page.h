/* ---------------------------------------------------------------------- 
 * Definition of MassifOptionsPage                  massif_options_page.h
 * subclass of OptionsPage to hold massif-specific options | flags.
 * ---------------------------------------------------------------------- 
 */

#ifndef MASSIF_OPTIONS_PAGE_H
#define MASSIF_OPTIONS_PAGE_H

#include "options_page.h"


class MassifOptionsPage : public OptionsPage
{
	Q_OBJECT
public:
	MassifOptionsPage( QWidget* parent, VkObject* obj );
  bool applyOptions( int id, bool undo=false );
};


#endif
