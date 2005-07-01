/* ---------------------------------------------------------------------
 * definition of CachegrindView                        cachegrind_view.h
 * ---------------------------------------------------------------------
 */

#ifndef __VK_VIEW_CACHEGRIND_H
#define __VK_VIEW_CACHEGRIND_H

#include "tool_view.h"

class CachegrindView : public ToolView
{
  Q_OBJECT
public:
  CachegrindView( QWidget* parent, VkObject* obj );
  ~CachegrindView();

  bool run() { return true; }
  void stop() { }
  void clear() { }

public slots:
  // void processExited() { }
  void toggleToolbarLabels(bool);

protected:
  //virtual void procFinished() { }
};


#endif
