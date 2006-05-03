/* ---------------------------------------------------------------------- 
 * Definition of class SpinWidget                            intspinbox.h
 * ---------------------------------------------------------------------- 
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#ifndef __VK_INT_SPINBOX_H
#define __VK_INT_SPINBOX_H

#include <qptrlist.h>
#include <qtimer.h>
#include <qwidget.h>
#include <qstyle.h>


/* class SpinWidget ---------------------------------------------------- */
class SpinWidget : public QWidget
{
   Q_OBJECT
public:
   SpinWidget( QWidget* parent=0, const char* name=0 );
   ~SpinWidget() { }
   int  downRectWidth() const;
   void setEditWidget( QWidget * widget );
   void setUpEnabled( bool on );
   void setDownEnabled( bool on );
   void arrange();
   QRect UpRect() const;

signals:
   void stepUpPressed();
   void stepDownPressed();

   public slots:
      void stepUp();
   void stepDown();

protected:
   void mousePressEvent( QMouseEvent *e );
   void resizeEvent( QResizeEvent* ev );
   void mouseReleaseEvent( QMouseEvent *e );
   void mouseMoveEvent( QMouseEvent *e );
   void paintEvent( QPaintEvent * );
   void windowActivationChange( bool );

   private slots:
      void timerDone();
   void timerDoneEx();

private:
   void updateDisplay();
   void startTimer( int msec );
   void startTimer( bool up, int msec );
   void stopTimer();
   QRect querySubControlMetrics ( QStyle::SubControl sc );

   uint upEnabled : 1;
   uint downEnabled : 1;
   uint theButton : 2;
   uint buttonDown : 2;
   uint timerUp : 1;

   QWidget* ed;               /* ptr to line edit */
   QRect upRect;
   QRect downRect;
   QTimer auRepTimer;
};




/* class NumberSection ------------------------------------------------- */
class NumberSection
{
public:
   NumberSection( int idx=-1, QString sep_char=" " );
   void stepUp();
   void stepDown();
   void setValue( int v );
   void setValues( int _min, int _max, int _curr, int _step );
   void setSelectionStart( int s );
   void setSelectionEnd( int s );
   int  value();
   int  minVal();
   int  maxVal();
   int  index() const;
   int  selectionStart() const;
   int  selectionEnd() const;
   bool withinRange( int num );
   bool usesPowers();
   QString separator() const;

private:
   int power( int idx );
   int getPowIndex( int v );

private:
   int secIndex;
   int selStart;
   int selEnd;
   int min, max, val, step;
   bool usePowers;
   QString sep;
};




/* class Editor -------------------------------------------------------- 
   this is a specialised line edit widget-type thingy */
class IntSpin;
class Editor : public QWidget
{
   Q_OBJECT
public:
   Editor( IntSpin* parent=0, const char* name=0 );
   ~Editor();
   int  focusSection() const;
   void setFocusSection( int sec );
   void updateDisplay();

protected:
   bool event( QEvent* ev );
   bool eventFilter( QObject* obj, QEvent* ev );
   void paintEvent( QPaintEvent* pe );
   void resizeEvent( QResizeEvent* re );
   void mousePressEvent( QMouseEvent* me );

private:
   void makePixmap();
   int  xPosToCursorPos( int press_pos ) const;

private:
   int frameW;
   int focusSec;  /* currently selected number section */
   bool pmDirty;
   IntSpin* cw;
   QPixmap* pmBuf;
};




/* class IntSpin ------------------------------------------------------- */
class IntSpin: public QWidget
{
   Q_OBJECT
public:
   IntSpin( QWidget* parent, const char* name=0 );
   ~IntSpin();
   void addSection( int min, int max, int curr=0, 
                    int step=1, QString sep_char=" : " );
   void setValue( int v, int sec=0 );
   QSize sizeHint() const;
   QSize minimumSizeHint() const;
   QString formattedText();
   int selStart( int fsec );
   int selEnd( int fsec );
   unsigned int sectionCount();
   bool setFocusSection( int sec );
   void validate();
   void addNumber( int fsec, int num );

   public slots:
      void stepUp();
   void stepDown();

signals:
   void valueChanged( const QString& );

protected:
   bool event( QEvent * ev );
   void resizeEvent( QResizeEvent *re );
   void timerEvent( QTimerEvent *te );

private:
   QString currValueText();
   QString sectionText( int sec );
   void setSectionSelection( int sec, int selstart, int selend );

private:
   int  timerId;
   int  numSecs; 

   bool changed;
   bool overwrite;
   bool typing;

   QString tmpBuf;  /* for addNumber() */

   Editor* ed;
   SpinWidget* controls;
   QPtrList<NumberSection> sections;
};


#endif
