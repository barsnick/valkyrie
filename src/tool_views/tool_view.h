/* ---------------------------------------------------------------------
 * definition of ToolView                                    tool_view.h
 * base class for all tool views
 * ---------------------------------------------------------------------
 */

#ifndef __VK_TOOL_VIEW_H
#define __VK_TOOL_VIEW_H

#include <qmainwindow.h>
#include <qprocess.h>

#include "vk_objects.h"

class ToolView : public QMainWindow
{
  Q_OBJECT
public:
  ToolView( QWidget* parent, VkObject* obj );
	~ToolView();

	int id();
  bool isRunning();
	QString currFlags();

	virtual bool run() = 0;
	virtual void stop() = 0;
	virtual void clear() = 0;

signals:
	//void closing( int );
	void running( bool );
	void message( QString );
	//void showFlags( QString );

public slots:
	virtual void processExited() = 0;
	/* called on startup, and via the options dialog */
	void flagsChanged();
  virtual void toggleToolbarLabels(bool) = 0;

protected:
  bool isEdited();
	virtual void killProc();
  //virtual void procFinished() = 0;
	virtual void closeEvent( QCloseEvent* ce );

protected:
	bool is_Edited;       /* eg. output might not be saved to file */
	bool is_Running;      /* whether this view is doing stuff */
	QStringList flags;    /* list of flags relevant to this view */

  QProcess* proc;
	VkObject* vkObj;
};


#endif
