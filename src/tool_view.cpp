/* ---------------------------------------------------------------------
 * implementation of class ToolView                        tool_view.cpp
 * base class for all tool views
 * ---------------------------------------------------------------------
 */

#include <qlayout.h>
#include <qtimer.h>

#include "tool_view.h"
#include "vk_include.h"
#include "vk_config.h"
#include "vk_msgbox.h"


ToolView::~ToolView() { }

ToolView::ToolView( QWidget* parent, VkObject* obj )
  : QMainWindow( parent, obj->name(), WDestructiveClose )
{
  proc       = 0;
  vkObj      = obj;
  is_Running = false;
	is_Edited  = false;
  setCaption( vkObj->title() );

  QWidget* central = new QWidget( this );
  setCentralWidget( central );
  QVBoxLayout* topLayout = new QVBoxLayout( central, 5, 5 );
  topLayout->setResizeMode( QLayout::FreeResize );
  flagsChanged(); 
}


int ToolView::id()
{ return vkObj->id(); }


bool ToolView::isRunning()
{ return is_Running; }

bool ToolView::isEdited()
{ return is_Edited; }


/* called when MainWin's showFlags button is set on so user can see
   exactly what is being fed to valgrind on the command-line */
QString ToolView::currFlags()
{ return flags.join( "\n" ); }


/* a toolview always holds a list of non-default flags relevant only
   to itself */
void ToolView::flagsChanged()
{ flags = vkConfig->modFlags( vkObj ); }


void ToolView::closeEvent( QCloseEvent* ce )
{
	/* current output might not have been saved to file */
	if ( isEdited() ) {
		int ok = vkQuery( this, "Unsaved File", 
											"&Save;&Discard;&Cancel",
											"<p>The current output is not saved."
											"Do you want to save it ?</p>" );
		if ( ok == MsgBox::vkYes ) {           // save
			printf("TODO: save();\n");
		} else if ( ok == MsgBox::vkCancel ) { // procrastinate
			ce->ignore();
			return;
		}
	}

  /* if current process is not yet finished, ask user if they really
     want to close */
	if ( isRunning() ) {
		int ok = vkQuery( this, "Process Running", "&Abort;&Cancel",
											"<p>The current process is not yet finished."
											"Do you want to abort it ?</p>?" );
		if ( ok == MsgBox::vkYes ) {         // abort
			stop();     // killProc();
		} else if ( ok == MsgBox::vkNo ) {
			ce->ignore();
			return;
		}
	}

	//emit closing( id() );
  ce->accept();
}


/* kill proc if it is running */
void ToolView::killProc()
{
  if ( proc != 0 ) {
    if ( proc->isRunning() ) {
      /* if this view is closed, don't leave the process running */
      proc->tryTerminate();
      QTimer::singleShot( 5000, proc, SLOT( kill() ) );
    }
    delete proc;
    proc = 0;
  }

  /* set state for MainWin's run, restart, stop buttons */
  emit running( false );
}

