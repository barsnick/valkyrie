/* ---------------------------------------------------------------------- 
 * Definition of CachegrindOptionsPage          cachegrind_options_page.h
 * subclass of OptionsPage to hold cachegrind-specific options | flags.
 * ---------------------------------------------------------------------- 
 */

#ifndef CACHEGRIND_OPTIONS_PAGE_H
#define CACHEGRIND_OPTIONS_PAGE_H

#include "options_page.h"


class CachegrindOptionsPage : public OptionsPage
{
	Q_OBJECT
public:
	CachegrindOptionsPage( QWidget* parent, VkObject* obj );
  bool applyOptions( int id, bool undo=false );

private slots:
	void getPidFile();
  void getIncludeDirs();
};


#endif
