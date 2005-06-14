/* ---------------------------------------------------------------------
 * Definition of HelpInfo                                    help_info.h
 * small tab dialog showing various information re licence etc.
 * --------------------------------------------------------------------- */

#ifndef __VK_HELP_INFO_H
#define __VK_HELP_INFO_H

#include <qdialog.h>
#include <qtabwidget.h>
#include <qtextedit.h>


/* class HelpInfo ------------------------------------------------------ */
class TextEdit;

class HelpInfo : public QDialog
{
  Q_OBJECT
public:
	enum TabId { ABOUT_VK=0, ABOUT_QT, LICENCE, SUPPORT };

  HelpInfo( QWidget* parent, TabId tabid );
  ~HelpInfo();

private slots:
  void showTab( QWidget* );

private:
  QString title;

	QTabWidget* tabParent;
  TextEdit* aboutVk;
	TextEdit* aboutQt;
	TextEdit* licence;
	TextEdit* support;
};


/* class TextEdit ------------------------------------------------------ */
class TextEdit : public QTextEdit
{ 
public:
  TextEdit( QWidget* parent, HelpInfo::TabId tabid, const char* name );
  ~TextEdit();
  bool load();

private:
	bool loaded;
  QString html_file;
	HelpInfo::TabId tabId;
};


#endif
