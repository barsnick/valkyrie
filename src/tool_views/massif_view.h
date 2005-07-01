/* ---------------------------------------------------------------------
 * definition of MassifView                                massif_view.h
 * ---------------------------------------------------------------------
 */

#ifndef __VK_VIEW_MASSIF_H
#define __VK_VIEW_MASSIF_H

#include "tool_view.h"

class MassifView : public ToolView
{
  Q_OBJECT
public:
  MassifView( QWidget* parent, VkObject* obj );
  ~MassifView();

  bool run() { return true; }
  void stop() { }
  void clear() { }

public slots:
  //void processExited() { }
  void toggleToolbarLabels(bool);

protected:
  //virtual void procFinished() { }
};



#endif
