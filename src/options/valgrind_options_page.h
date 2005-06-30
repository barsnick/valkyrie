/* ---------------------------------------------------------------------- 
 * Definition of ValgrindOptionsPage              valgrind_options_page.h
 * subclass of OptionsPage to hold valgrind-specific options | flags.
 * ---------------------------------------------------------------------- 
 */

#ifndef VALGRIND_OPTIONS_PAGE_H
#define VALGRIND_OPTIONS_PAGE_H

#include "options_page.h"


class ValgrindOptionsPage : public OptionsPage
{
	Q_OBJECT
public:
	ValgrindOptionsPage( QWidget* parent, VkObject* obj );
  bool applyOptions( int id, bool undo=false );

private slots:
	void dummy();

};


#endif
