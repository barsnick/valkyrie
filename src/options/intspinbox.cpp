/* ---------------------------------------------------------------------- 
 * Implemention of class SpinWidget                        intspinbox.cpp
 * ---------------------------------------------------------------------- 
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#include <qapplication.h>
#include <qpainter.h>
#include <qstyle.h>
#include <qpixmap.h>

#include "intspinbox.h"
#include "vk_utils.h"


/* class SpinWidget ---------------------------------------------------- */
SpinWidget::SpinWidget( QWidget* parent, const char* name )
  : QWidget( parent, name )
{
  upEnabled   = true;
  downEnabled = true;

  ed         = 0;
  timerUp    = 0;
  theButton  = 0;
  buttonDown = 0;

  connect( &auRepTimer, SIGNAL( timeout() ), 
           this,        SLOT( timerDone() ) );
  setFocusPolicy( StrongFocus );
  arrange();
  updateDisplay();
}

QRect SpinWidget::UpRect() const 
{ return upRect; }

void SpinWidget::startTimer( int msec ) 
{ auRepTimer.start( msec, true ); }

void SpinWidget::startTimer( bool up, int msec ) 
{ 
  timerUp = up; 
  startTimer( msec ); 
}

void SpinWidget::stopTimer() 
{ auRepTimer.stop(); }

void SpinWidget::setEditWidget( QWidget* w )
{
  if ( w ) {
    w->reparent( this, QPoint( 0, 0 ) );
    setFocusProxy( w );
  }
  ed = w;
  arrange();
  updateDisplay();
}

QRect SpinWidget::querySubControlMetrics ( QStyle::SubControl sc )
{
  int fw = style().pixelMetric( QStyle::PM_SpinBoxFrameWidth, this);
  QSize buttSize;
  buttSize.setHeight( this->height()/2 - fw );
  if ( buttSize.height() < 8 )
    buttSize.setHeight( 8 );
  /* 8/5 = 1.6 => approximate golden mean */
  //  buttSize.setWidth( QMIN( buttSize.height() * 8 / 5, this->width() / 4 ) );
  buttSize.setWidth( buttSize.height() * 8 / 5 );
  /* Ensure we're not smaller than the smallest allowable size: */
  buttSize = buttSize.expandedTo( QApplication::globalStrut() );
  int top      = fw;
  int buttLeft = this->width() - fw - buttSize.width();
  int left     = fw;
  int edWidth  = buttLeft - fw;
  
  switch ( sc ) {
  case QStyle::SC_SpinWidgetUp:
    return QRect(buttLeft, top, buttSize.width(), buttSize.height());
  case QStyle::SC_SpinWidgetDown:
    return QRect(buttLeft, top + buttSize.height(), buttSize.width(), buttSize.height());
  case QStyle::SC_SpinWidgetButtonField:
    return QRect(buttLeft, top, buttSize.width(), this->height() - 2*fw);
  case QStyle::SC_SpinWidgetEditField:
    return QRect(left, fw, edWidth, this->height() - 2*fw);
  case QStyle::SC_SpinWidgetFrame:
    return this->rect();
  default:
    break;
  }
  return QRect();
}


void SpinWidget::arrange()
{
  QRect mr_up   = querySubControlMetrics( QStyle::SC_SpinWidgetUp );
  upRect        = QStyle::visualRect( mr_up, this );

  QRect mr_down = querySubControlMetrics( QStyle::SC_SpinWidgetDown );
  downRect      = QStyle::visualRect( mr_down, this );

  if ( ed ) {
    QRect mr_ed = querySubControlMetrics( QStyle::SC_SpinWidgetEditField );
    ed->setGeometry( QStyle::visualRect( mr_ed, this ) );
  }
}

void SpinWidget::stepUp()
{  emit stepUpPressed(); }

void SpinWidget::stepDown()
{ emit stepDownPressed(); }

void SpinWidget::timerDone()
{ QTimer::singleShot( 1, this, SLOT( timerDoneEx() ) ); }

void SpinWidget::timerDoneEx()
{
  if ( !buttonDown )
    return;
  if ( timerUp )
    stepUp();
  else
    stepDown();
  startTimer( 100 );
}

void SpinWidget::windowActivationChange( bool oldActive )
{
  if ( oldActive && buttonDown ) {
    stopTimer();
    buttonDown = 0;
    theButton = 0;
  }
  QWidget::windowActivationChange( oldActive );
}

void SpinWidget::resizeEvent( QResizeEvent* )
{ arrange(); }

void SpinWidget::mousePressEvent( QMouseEvent *e )
{
  if ( e->button() != LeftButton ) {
    stopTimer();
    buttonDown = 0;
    theButton = 0;
    repaint( downRect.unite( upRect ), false );
    return;
  }

  uint oldButtonDown = buttonDown;

  if ( downRect.contains( e->pos() ) && downEnabled )
    buttonDown = 1;
  else if ( upRect.contains( e->pos() ) && upEnabled )
    buttonDown = 2;
  else
    buttonDown = 0;

  theButton = buttonDown;
  if ( oldButtonDown != buttonDown ) {
    if ( !buttonDown ) {
      repaint( downRect.unite( upRect ), false );
    } else if ( buttonDown & 1 ) {
      repaint( downRect, false );
      stepDown();
      startTimer( false, 300 );
    } else if ( buttonDown & 2 ) {
      repaint( upRect, false );
      stepUp();
      startTimer( true, 300 );
    }
  }
}

void SpinWidget::mouseReleaseEvent( QMouseEvent *e )
{
  if ( e->button() != LeftButton )
    return;

  uint oldButtonDown = theButton;
  theButton = 0;
  if ( oldButtonDown != theButton ) {
    if ( oldButtonDown & 1 )
      repaint( downRect, false );
    else if ( oldButtonDown & 2 )
      repaint( upRect, false );
  }
  stopTimer();
  buttonDown = 0;
}

void SpinWidget::mouseMoveEvent( QMouseEvent *e )
{
  if ( !(e->state() & LeftButton ) )
    return;

  uint oldButtonDown = theButton;
  if ( oldButtonDown & 1 && !downRect.contains( e->pos() ) ) {
    stopTimer();
    theButton = 0;
    repaint( downRect, false );
  } else if ( oldButtonDown & 2 && !upRect.contains( e->pos() ) ) {
    stopTimer();
    theButton = 0;
    repaint( upRect, false );
  } else if ( !oldButtonDown && upRect.contains( e->pos() ) 
              && buttonDown & 2 ) {
    startTimer( 500 );
    theButton = 2;
    repaint( upRect, false );
  } else if ( !oldButtonDown && downRect.contains( e->pos() ) 
              && buttonDown & 1 ) {
    startTimer( 500 );
    theButton = 1;
    repaint( downRect, false );
  }
}


void SpinWidget::paintEvent( QPaintEvent * )
{
  QPainter p( this );

  QStyle::SFlags flags = QStyle::Style_Default;
  if ( isEnabled() )
    flags |= QStyle::Style_Enabled;
  if ( hasFocus() || focusProxy() && focusProxy()->hasFocus() )
    flags |= QStyle::Style_HasFocus;

  QStyle::SCFlags active;
  if ( theButton & 1 )
    active = QStyle::SC_SpinWidgetDown;
  else if ( theButton & 2 )
    active = QStyle::SC_SpinWidgetUp;
  else
    active = QStyle::SC_None;

  QRect mr = style().querySubControlMetrics( 
                                       QStyle::CC_SpinWidget, 
                                       this, 
                                       QStyle::SC_SpinWidgetFrame );
  QRect fr = QStyle::visualRect( mr, this );

  QStyle::PrimitiveElement pe;
  QStyle::SCFlags controls = QStyle::SC_All;

  if ( controls & QStyle::SC_SpinWidgetFrame ) {
    /* draw a rectangle with two pixel line width */
    int x = fr.x();
    int y = fr.y();
    int w = fr.width();
    int h = fr.height();
    QColor c1 = colorGroup().dark();
    QColor c2 = colorGroup().light();
    QColor c3 = colorGroup().shadow();
    QColor c4 = colorGroup().midlight();
    if ( w < 2 || h < 2 )      /* can't do anything with that */
    return;

    QPen oldPen = p.pen();
    QPointArray a( 3 );
    a.setPoints( 3, x, y+h-2, x, y, x+w-2, y );
    p.setPen( c1 );
    p.drawPolyline( a );
    a.setPoints( 3, x, y+h-1, x+w-1, y+h-1, x+w-1, y );
    p.setPen( c2 );
    p.drawPolyline( a );

    if ( w > 4 && h > 4 ) {
      a.setPoints( 3, x+1, y+h-3, x+1, y+1, x+w-3, y+1 );
      p.setPen( c3 );
      p.drawPolyline( a );
      a.setPoints( 3, x+1, y+h-2, x+w-2, y+h-2, x+w-2, y+1 );
      p.setPen( c4 );
      p.drawPolyline( a );
    }
    p.setPen( oldPen );
  }

  if ( controls & QStyle::SC_SpinWidgetUp ) {
    flags = QStyle::Style_Default | QStyle::Style_Enabled;
    if ( active == QStyle::SC_SpinWidgetUp ) {
      flags |= QStyle::Style_On;
      flags |= QStyle::Style_Sunken;
    } else
      flags |= QStyle::Style_Raised;
    pe = QStyle::PE_SpinWidgetUp;
    
    QRect re = upRect;
    QColorGroup ucg = upEnabled ? colorGroup() : palette().disabled();
    style().drawPrimitive( QStyle::PE_ButtonBevel, 
                           &p, re, ucg, flags );
    style().drawPrimitive( pe, &p, re, ucg, flags );
  }

  if ( controls & QStyle::SC_SpinWidgetDown ) {
    flags = QStyle::Style_Default | QStyle::Style_Enabled;
    if ( active == QStyle::SC_SpinWidgetDown ) {
      flags |= QStyle::Style_On;
      flags |= QStyle::Style_Sunken;
    } else
      flags |= QStyle::Style_Raised;
    pe = QStyle::PE_SpinWidgetDown;

    QRect re = downRect;
    QColorGroup dcg = downEnabled ? colorGroup() : palette().disabled();
    style().drawPrimitive( QStyle::PE_ButtonBevel, &p, 
                           re, dcg, flags );
    style().drawPrimitive( pe, &p, re, dcg, flags );
  }

}

int SpinWidget::downRectWidth() const
{ return downRect.width(); }

void SpinWidget::updateDisplay()
{
  if ( !isEnabled() ) {
    upEnabled = false;
    downEnabled = false;
  }
  if ( theButton & 1 && ( downEnabled ) == 0 ) {
    theButton  &= ~1;
    buttonDown &= ~1;
  }

  if ( theButton & 2 && ( upEnabled ) == 0 ) {
    theButton &= ~2;
    buttonDown &= ~2;
  }
  repaint( false );
}

void SpinWidget::setUpEnabled( bool on )
{
  if ( (bool)upEnabled != on ) {
    upEnabled = on;
    updateDisplay();
  }
}

void SpinWidget::setDownEnabled( bool on )
{
  if ( (bool)downEnabled != on ) {
    downEnabled = on;
    updateDisplay();
  }
}




/* class NumberSection ------------------------------------------------- 
   of course '0' isn't a power of two - but we have to have some sort
   of default 'not used' units :) */
#define MAX_POW 18
static int powers[MAX_POW+1] = {
  0,   4,      8,      16,    32,     64,     128, 
     256,    512,    1024,  2048,   4096,   8192, 
   16384,  32768,  65536, 131072, 262144, 1048576
};

NumberSection::NumberSection( int idx/*=-1*/, QString sep_char/*=" : "*/ )
{
  sep      = sep_char;
  secIndex = idx;
  selStart = 0;
  selEnd   = 0;
}

/* if a value of 0 is passed for step, then we are using powers
   of two, not lineStep, as the value-to-increment/decrement. */
void NumberSection::setValues( int _min, int _max, 
                               int _val, int _step ) 
{ 
  usePowers = (_step == 0);
  if ( !usePowers ) {
    min  = _min; 
    max  = _max; 
    val  = _val;
    step = _step; 
  } else {
    min  = 0;
    max  = MAX_POW;
    val  = getPowIndex( _val );
    val  = (val == -1) ? 0 : val;
    step = 1;
  }
}

int NumberSection::power( int idx )
{ 
  val = idx;
  vk_assert( (val >= 0) && (val <= MAX_POW) );
  return powers[val]; 
}

int NumberSection::getPowIndex( int v )
{
  for ( int i=min; i<max; i++ ) {
    if ( v == powers[i] )
      return i;
  }
  return -1;
}

void NumberSection::setSelectionStart( int s ) 
{ selStart = s; }

void NumberSection::setSelectionEnd( int s )
{ selEnd = s; }

int  NumberSection::index() const
{ return secIndex; }

int  NumberSection::selectionStart() const
{ return selStart; }

int  NumberSection::selectionEnd() const
{ return selEnd; }

bool NumberSection::usesPowers()
{ return usePowers; }

QString NumberSection::separator() const
{ return sep; }

int NumberSection::minVal()
{ return ( usePowers) ? powers[min] : min; }

int NumberSection::maxVal()
{ return ( usePowers) ? powers[max] : max; }

int NumberSection::value()
{ return ( usePowers ) ? powers[val] : val; }

void NumberSection::setValue( int v )
{ 
  if ( !usePowers )
    val = v; 
  else {
    val = getPowIndex( v );
    val = ( val == -1 ) ? 0 : val;
  }
}

void NumberSection::stepUp() 
{
  if ( val+step > max )
    val = min;
  else
    val = val+step;
}

void NumberSection::stepDown() 
{
  if ( val-step < min )
    val = max;
  else
    val = val-step;
}

bool NumberSection::withinRange( int num )
{
  bool ok = false;
  if ( !usePowers ) {
    ok = ( (num >= min) && (num <= max) );
  } else {
    int v = getPowIndex( num );
    ok = ( (v >= min) && (v <= max) );
  }

  return ok;
}




/* class Editor -------------------------------------------------------- */
Editor::~Editor()
{
  if ( pmBuf )
    delete pmBuf;
}


Editor::Editor( IntSpin * parent,  const char * name )
  : QWidget( parent, name, WNoAutoErase )
{
  cw = parent;
  pmBuf = 0;
  pmDirty = true;
  frameW = 2;
  focusSec = 0;

  setBackgroundMode( PaletteBase );
  installEventFilter( this );
  setFocusPolicy( WheelFocus );
}

int Editor::focusSection() const 
{ return focusSec; }

void Editor::setFocusSection( int sec )
{ 
  focusSec = sec; 
  pmDirty = true;
}

void Editor::makePixmap()
{
  if ( pmBuf )
    return;

  QSize s( width() - frameW*2, height() - frameW*2 );
  if ( s.width() < 0 )
    s.setWidth( 0 );
  if ( s.height() < 0 )
    s.setHeight( 0 );
  pmBuf = new QPixmap( s );
  pmDirty = true;
}

int Editor::xPosToCursorPos( int press_pos ) const
{
  int x1 = 0;
  int x2 = 0;
  int i  = 0;
  QFontMetrics fm = fontMetrics();
  QString s  = cw->formattedText();
  press_pos -= (frameW + 2);
  
  while ( i < (int) s.length() ) {
    x2 = x1 + fm.width( s[i] );
    if ( QABS( x1 - press_pos ) < QABS( x2 - press_pos ) ) {
      return i;
    }
    i++;
    x1 = x2;
  }

  return i;
}

void Editor::resizeEvent( QResizeEvent *re )
{
  delete pmBuf;
  pmBuf = 0;
  QWidget::resizeEvent( re );
}

void Editor::mousePressEvent( QMouseEvent *me )
{
  int curpos = xPosToCursorPos( me->pos().x() );
  int sec = -1;
  for ( unsigned int i = 0; i < cw->sectionCount(); ++i ) {
    if ( curpos >= cw->selStart(i) &&
         curpos <= cw->selEnd(i) ) {
      sec = i;
      break;
    }
  }

  if ( sec != -1 ) {
    if ( cw->setFocusSection( sec ) ) 
      repaint( rect(), false );
  }
}

bool Editor::event( QEvent *ev )
{
  if ( ev->type() == QEvent::FocusIn || 
       ev->type() == QEvent::FocusOut ) {
    pmDirty = true;
    if ( ev->type() == QEvent::FocusOut ) {
      qApp->sendEvent( cw, ev );
    }
    update( rect() );
  } 
  return QWidget::event( ev );
}

void Editor::updateDisplay()
{
  pmDirty = true;
  repaint( rect(), false );
}

void Editor::paintEvent( QPaintEvent * )
{
  if ( !pmBuf || pmDirty ) {
    makePixmap();    
    if ( pmBuf->isNull() ) {
      delete pmBuf;
      pmBuf = 0;
      return;
    }
    QPainter p( pmBuf, this );
    const QColorGroup & cg = colorGroup();
    // erase previous contents
    QBrush bg = cg.brush(
      isEnabled() ? QColorGroup::Base : QColorGroup::Background);
    p.fillRect( 0, 0, width(), height(), bg );

    // draw the text, including the focus section 
    QString txt    = cw->formattedText();
    int markBegin  = cw->selStart( focusSec );
    int markEnd    = cw->selEnd( focusSec );
    QString pre    = txt.mid( 0,         markBegin );
    QString marked = txt.mid( markBegin, markEnd-markBegin );
    QString post   = txt.mid( markEnd,   txt.length() );
    QFontMetrics fm = fontMetrics();
    int y = (pmBuf->height() + fm.height())/2 - fm.descent() - 1 ;
    int x = 2;
    int w = fm.width( pre );
    if ( x < pmBuf->width() && x + w >= 0 ) {
      p.setPen( cg.text() );
      p.drawText( x, y, pre );
    }

    x += w;
    w = fm.width( marked );
    if ( x < pmBuf->width() && x + w >= 0 ) {
      if ( hasFocus() ) {
        p.fillRect( x, y-fm.ascent()-1, w, fm.height()+2,
                    cg.brush( QColorGroup::Highlight ) );
        p.setPen( cg.highlightedText() );
      } 
      p.drawText( x, y, marked );
    }

    x += w;
    w = fm.width( post );
    if ( x < pmBuf->width() && x + w >= 0 ) {
      p.setPen( cg.text() );
      p.drawText( x, y, post );
    }

    p.setPen( cg.text() );
    pmDirty = false;
  }

  QPainter p( this );
  p.drawPixmap( frameW, frameW, *pmBuf );
}

bool Editor::eventFilter( QObject* obj, QEvent* ev )
{
  if ( obj == this ) {
    if ( ev->type() == QEvent::KeyPress ) {
      QKeyEvent *ke = (QKeyEvent*)ev;
      switch ( ke->key() ) {
      case Key_Right:
        if ( focusSec < (int)cw->sectionCount() - 1 ) {
          if ( cw->setFocusSection( focusSec+1 ) )
            repaint( rect(), false );
        }
        return true;
      case Key_Left:
        if ( focusSec > 0 ) {
          if ( cw->setFocusSection( focusSec-1 ) )
            repaint( rect(), false );
        }
        return true;
      case Key_Up:
        cw->stepUp();
        return true;
      case Key_Down:
        cw->stepDown();
        return true;
      /*case Key_Backspace:
        cw->removeFirstNumber( focusSec );
        return true;
      case Key_Delete:
        cw->removeLastNumber( focusSec );
        return true;*/
      case Key_Enter:
      case Key_Return:
        cw->validate();
        return true;
      case Key_Tab:
      case Key_BackTab: {
        if ( ke->state() == Qt::ControlButton )
          return false;
        QWidget* w = this;
        w = w->parentWidget();
        if ( w ) {
          qApp->sendEvent( w, ev );
          return true;
        }
      } break;
      default:
        QString txt = ke->text().lower();
        int num = txt[0].digitValue();
        if ( num != -1 ) {
          cw->addNumber( focusSec, num );
          return true;
        }
      }
    }
  }
  return false;
}




/* class IntSpin ------------------------------------------------------- */
#define HIDDEN_CHAR '0'

IntSpin::~IntSpin()
{
  sections.setAutoDelete( true );
  sections.clear();
}


IntSpin::IntSpin( QWidget* parent, const char* name )
  : QWidget( parent, name, WNoAutoErase )
{
  numSecs = 0;
  timerId = 0;
  changed = false;
  overwrite = true;
  typing = false;

  sections.setAutoDelete( false );

  ed = new Editor( this, "editor" );
  setFocusProxy( ed );

  controls = new SpinWidget( this, "controls" );
  controls->setEditWidget( ed );
  
  connect( controls, SIGNAL( stepUpPressed() ), 
           this,     SLOT( stepUp() ) );
  connect( controls, SIGNAL( stepDownPressed() ), 
           this,     SLOT( stepDown() ) );

  setSizePolicy( QSizePolicy( QSizePolicy::Minimum, 
                              QSizePolicy::Fixed ) );
}


void IntSpin::addSection( int min, int max, int curr/*=0*/, 
                          int step/*=1*/, QString sep_char/*=" : "*/ )
{
  sections.append( new NumberSection(numSecs, sep_char) );
  sections.at(numSecs)->setValues( min, max, curr, step );
  numSecs++;
}

void IntSpin::setValue( int v, int sec/*=0*/ )
{ 
  vk_assert( sec < (int)sections.count() );
  sections.at(sec)->setValue( v );
  ed->updateDisplay();
}

void IntSpin::resizeEvent( QResizeEvent * )
{ controls->resize( width(), height() ); }

QSize IntSpin::minimumSizeHint() const
{ return sizeHint(); }

/* make the widget exactly the right size such that if all numbers are
   their maximum values, everything is visible */
QSize IntSpin::sizeHint() const
{
  constPolish();
  QFontMetrics fm( font() );
  int h = QMAX( fm.lineSpacing(), 14) + 2;
  h += style().pixelMetric(QStyle::PM_DefaultFrameWidth) * 2;
  int w  = 0;
  int tw = 0;
  QString s;
  QPtrListIterator<NumberSection> it( sections );
  NumberSection* aSection;
  while ( (aSection = it.current()) != 0 ) {
    ++it;
    s = QString::number( aSection->minVal() );
    w = QMAX( w, fm.width( s ) );
    s = QString::number( aSection->maxVal() );
    w = QMAX( w, fm.width( s ) );
    tw += w;
  }
  tw += controls->UpRect().width();
  /* when we only have one section, the controls come out smaller,
     and we need just a bit more width for the number. ho hum. */
  if ( sections.count() == 1 ) {
    tw += fm.width("   ");
  }

  return style().sizeFromContents( QStyle::CT_LineEdit, this,
     QSize(tw,h).expandedTo( QApplication::globalStrut() ));
}


bool IntSpin::event( QEvent *ev )
{
  if ( ev->type() == QEvent::FocusOut ) {
    killTimer( timerId );
    validate();
    typing = false;
    if ( changed ) {
      emit valueChanged( currValueText() );
      changed = false;
    }
  } 
  return QWidget::event( ev );
}


QString IntSpin::currValueText()
{
  QString txt = "";
  for ( unsigned int sec = 0; sec < sections.count(); ++sec ) {
    txt += sectionText( sec );
    if ( sec < sections.count()-1 )
      txt += ",";
  }
  return txt;
}

QString IntSpin::formattedText()
{
  int len;
  int offset = 0;
  QString txt, tmp;
  for ( int sec = 0; sec < numSecs; ++sec ) {

    tmp = sectionText( sec );
    len = tmp.length();
    /* only pad a single integer if we have > 1 section */
    if ( sections.count() > 1 ) {
      if ( len == 1 ) {
        tmp = tmp.rightJustify( 2, HIDDEN_CHAR );
        len = 2;
      }
    }
    offset += len;
    setSectionSelection( sec, offset-len, offset );
    offset += sections.at(sec)->separator().length();
    txt += tmp;

    if ( sec < numSecs-1 ) {
      txt += sections.at(sec)->separator();
    }
  }
  
  return txt;
}

void IntSpin::setSectionSelection( int sec, 
                                   int selstart, int selend )
{
  if ( sec < 0 || sec > numSecs )
    return;
  sections.at(sec)->setSelectionStart( selstart );
  sections.at(sec)->setSelectionEnd( selend );
}

QString IntSpin::sectionText( int sec )
{
  sec = sections.at(sec)->index();
  QString txt = QString::number( sections.at(sec)->value() );
  return txt;
}

int IntSpin::selStart( int sec )
{ return sections.at(sec)->selectionStart(); }

int IntSpin::selEnd( int sec )
{ return sections.at(sec)->selectionEnd(); }

unsigned int IntSpin::sectionCount() 
{ return (unsigned int)numSecs; }

bool IntSpin::setFocusSection( int sec )
{
  if ( sec < 0 || sec > numSecs )
    return false;

  if ( sec != ed->focusSection() ) {
    killTimer( timerId );
    validate();
    typing = false;
    overwrite = true;

    if ( changed ) {
      emit valueChanged( currValueText() );
      changed = false;
    }

    ed->setFocusSection( sec );
    return true;
  }

  return false;
}

void IntSpin::stepUp()
{
  int fsec = ed->focusSection();
  int sec = sections.at(fsec)->index();

  sections.at(sec)->stepUp();
  ed->updateDisplay();
  emit valueChanged( currValueText() );
}

void IntSpin::stepDown()
{
  int fsec = ed->focusSection();
  int sec = sections.at(fsec)->index();

  sections.at(sec)->stepDown();
  ed->updateDisplay();
  emit valueChanged( currValueText() );
}

void IntSpin::timerEvent( QTimerEvent * )
{ 
  validate();
  overwrite = true; 
}

/* user might have typed a number. 
   at this point, ed->focusSection() hasn't yet changed. */
void IntSpin::validate()
{
  if ( !typing )
    return;
  int fsec = ed->focusSection();
  if ( !sections.at(fsec)->usesPowers() )
    return;

  int temp = tmpBuf.toInt();
  if ( sections.at(fsec)->withinRange( temp ) ) {
    sections.at(fsec)->setValue( temp );
    changed = true;
    ed->updateDisplay();
    emit valueChanged( currValueText() );
  }
  tmpBuf = "";  /* clear buffer */
  typing = false;
}

void IntSpin::addNumber( int fsec, int num )
{
  killTimer( timerId );
  typing = true;

  if ( sections.at(fsec)->usesPowers() ) {
    tmpBuf += QString::number( num );
  } else {
    bool accepted   = false;
    bool over_write = false;
    QString txt = sectionText( fsec );
    if ( overwrite ) {
      if ( sections.at(fsec)->withinRange( num ) ) {
        accepted = true;
        sections.at(fsec)->setValue( num );
        over_write = false;
      }
    } else {
      txt += QString::number( num );
      int temp = txt.toInt();
      if ( sections.at(fsec)->withinRange( temp ) ) {
        accepted = true;
        sections.at(fsec)->setValue( temp );
      }
    }
    changed   = accepted;
    overwrite = over_write;
    if ( changed ) {
      ed->updateDisplay();
      emit valueChanged( currValueText() );
    }
  }

  timerId = startTimer( qApp->doubleClickInterval()*4 );
}
