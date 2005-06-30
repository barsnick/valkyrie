/* ---------------------------------------------------------------------- 
 * Definition of ValkyrieOptionsPage              valkyrie_options_page.h
 * subclass of OptionsPage to hold valkyrie-specific options | flags.
 * ---------------------------------------------------------------------- 
 */

#ifndef VALKYRIE_OPTIONS_PAGE_H
#define VALKYRIE_OPTIONS_PAGE_H

#include "options_page.h"


class ValkyrieOptionsPage : public OptionsPage
{
	Q_OBJECT
public:
	ValkyrieOptionsPage( QWidget* parent, VkObject* obj );
  bool applyOptions( int id, bool undo=false );

private slots:
	void chooseFont();
	void fontClicked(bool);
	void checkEditor();
  void getBinary();
	void getVgExec();
	void getSuppDir();
};


#endif
