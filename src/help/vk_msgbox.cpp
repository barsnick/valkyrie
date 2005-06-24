/*---------------------------------------------------------------------- 
  Message boxes                                            vk_msgbox.cpp
  ----------------------------------------------------------------------
*/

#include "vk_msgbox.h"
#include "vk_config.h"
#include "msgbox_icons.h"

#include <stdarg.h>           /* va_start, va_end */

#include <qapplication.h>
#include <qobjectlist.h>
#include <qpushbutton.h>
#include <qstyle.h>


MsgBox::~MsgBox() 
{ }


MsgBox::MsgBox( QWidget *parent, Icon icon, QString msg, 
                const QString& hdr, int num_buttons )
  : QDialog( parent, "msgbox", true, 
             WDestructiveClose | WStyle_Customize | 
             WStyle_DialogBorder | WStyle_Title | WStyle_SysMenu )
{

  if ( !hdr.isEmpty() ) {
		QString msg2 = "<b>" +hdr+ "</b><br/>";
    msg.prepend( msg2 );
		minWidth = fontMetrics().width( msg2 );
  }

  numButtons = num_buttons;
  defButton  = 0;
  button[0]  = vkYes;

  QString caption;
  QPixmap pm_file;
  switch ( icon ) {
  case Query:   
    caption = "Query";
    pm_file = QPixmap( msg_query_xpm ); 
    break;
  case Info:
    caption = "Information";
    pm_file = QPixmap( msg_info_xpm ); 
    break;
  case Warning:
    caption = "Warning";
    pm_file = QPixmap( msg_warn_xpm ); 
    break;
	case Error:
		caption = "Error";
		pm_file = QPixmap( msg_error_xpm );
		break;
  case Fatal:   
    caption = "Fatal Error";
    pm_file = QPixmap( msg_fatal_xpm );
    msg    += "<p><b>Quitting !</b><br /></p>";
    break;
  case About:
    caption.sprintf("About %s", vkConfig->vkName() );
    pm_file = vkConfig->pixmap( "valkyrie.xpm" );
    break;
  default:
    break;
  }
	minWidth += pm_file.width();

  button[defButton] |= MsgBox::Default;
  switch( numButtons ) {
  case 1:        // ok button
    escButton = -1;
    break;
  case 2:        // yes + no
    button[1] = vkNo  | MsgBox::Escape;
    escButton = 1;
    break;
  case 3:       // yes + no + cancel
    button[1] = vkNo;
    button[2] = vkCancel | MsgBox::Escape;
    escButton = 2;
    break;
  }

  for ( int i=0; i<numButtons; i++ ) {
    int b = button[i];
    b &= ButtonMask;
    button[i] = b;

    QCString buttonName;
    buttonName.sprintf( "button%d", i+1 );
    pb[i] = new QPushButton( this, buttonName );
    if ( defButton == i ) {
      pb[i]->setDefault( true );
      pb[i]->setFocus();
    }
    pb[i]->setAutoDefault( true );
    pb[i]->setFocusPolicy( QWidget::StrongFocus );
    connect( pb[i], SIGNAL(clicked()), 
             this,  SLOT(pbClicked()) );
  }

  resizeButtons();

  msgLabel = new QLabel( this, "msg_lbl" );
  msgLabel->setAlignment( AlignAuto | ExpandTabs );
  msgLabel->setText( msg );

  iconLabel = new QLabel( this, "icon_lbl" );
  iconLabel->setPixmap( pm_file );

  setCaption( caption );
}


void MsgBox::resizeButtons()
{
  int i;
  QSize maxSize;
  for ( i=0; i<numButtons; i++ ) {
    QSize s = pb[i]->sizeHint();
    maxSize.setWidth(  QMAX(maxSize.width(), s.width()) );
    maxSize.setHeight( QMAX(maxSize.height(),s.height()) );
  }

  buttonSize = maxSize;
  for ( i=0; i<numButtons; i++ ) {
    pb[i]->resize( buttonSize );
	}
}


void MsgBox::setButtonTexts( const QStringList &btexts )
{
  for ( int i=0; i<numButtons; i++ ) {
    pb[i]->setText( btexts[i] );
  }
}


void MsgBox::pbClicked()
{
  int reply = 0;
  const QObject *s = sender();
  for ( int i=0; i<numButtons; i++ ) {
    if ( pb[i] == s )
      reply = button[i];
  }
  done( reply );
}


/* Adjusts the size of the message box to fit the contents just
   before QDialog::exec() or QDialog::show() is called.
   This function will not be called if the message box has been
   explicitly resized before showing it. */
void MsgBox::adjustSize()
{
  if ( !testWState(WState_Polished) )
    polish();
  resizeButtons();
  msgLabel->adjustSize();
  QSize labelSize( msgLabel->size() );
  int n  = numButtons;
  int bw = buttonSize.width();
  int bh = buttonSize.height();
  int border = bh / 2 - style().pixelMetric(QStyle::PM_ButtonDefaultIndicator);
  if ( border <= 0 )
    border = 10;
  int btn_spacing = 10; //7
  if ( style().styleHint(QStyle::SH_GUIStyle) == MotifStyle )
    btn_spacing = border;
  int buttons = n/*umButtons*/ * bw + (n-1) * btn_spacing;
  int h = bh;
  if ( labelSize.height() )
    h += labelSize.height() + 3*border;
  else
    h += 2*border;
  int lmargin = 0;
  if ( iconLabel->pixmap() && iconLabel->pixmap()->width() )  {
    iconLabel->adjustSize();
    lmargin += iconLabel->width() + border;
    if ( h < iconLabel->height() + 3*border + bh )
      h = iconLabel->height() + 3*border + bh;
  }
  int w = QMAX( buttons, labelSize.width() + lmargin ) + 2*border;
	w = QMAX( minWidth, w );

  QRect screen = QApplication::desktop()->screenGeometry( pos() );
  if ( w > screen.width() )
    w = screen.width();
  resize( w, h );

  //setMinimumSize( size() );
  setMinimumSize( w, size().height() );
}


void MsgBox::resizeEvent( QResizeEvent * )
{
  int n  = numButtons;
  int num_butts  = n;
  int bw = buttonSize.width();
  int bh = buttonSize.height();
  int border = bh / 2 - style().pixelMetric(QStyle::PM_ButtonDefaultIndicator);
  if ( border <= 0 )
    border = 10;
  int btn_spacing = 10;  // 7;
  if ( style().styleHint(QStyle::SH_GUIStyle) == MotifStyle )
    btn_spacing = border;
  int lmargin = 0;
  iconLabel->adjustSize();
  iconLabel->move( border, border );
  if ( iconLabel->pixmap() && iconLabel->pixmap()->width() )
    lmargin += iconLabel->width() + border;
  msgLabel->setGeometry( lmargin+border, border,
                         width() - lmargin -2*border,
                         height() - 3*border - bh );
  int extra_space = ( width() - bw*num_butts - 2*border 
											- (num_butts-1)*btn_spacing );

  int i;
	for ( i=0; i<n; i++ ) {
		pb[i]->move( border + i*bw + extra_space/2 + i*btn_spacing,
								 height() - border - bh );
	}

}


void MsgBox::keyPressEvent( QKeyEvent *e )
{
  if ( e->key() == Key_Escape ) {
    if ( escButton >= 0 ) {
      pb[escButton]->animateClick();
      e->accept();
      return;
    }
  }

  if ( !( e->state() & AltButton ) ) {
    QObjectList *list = queryList( "QPushButton" );
    QObjectListIt it( *list );
    QPushButton *pb;
    while ( (pb = (QPushButton*)it.current()) ) {
      int key = e->key() & ~(MODIFIER_MASK|UNICODE_ACCEL);
      int acc = pb->accel() & ~(MODIFIER_MASK|UNICODE_ACCEL);
      if ( key && acc && acc == key ) {
        delete list;
        emit pb->animateClick();
        return;
      }
      ++it;
    }
    delete list;
  }

  QDialog::keyPressEvent( e );
}


void MsgBox::closeEvent( QCloseEvent *e )
{
  QDialog::closeEvent( e );
  if ( escButton != -1 )
    setResult( button[escButton] );
}


/*------ static functions -------------------------------*/

void MsgBox::about( QWidget *parent )
{
  char buf[512];
  sprintf( buf,
           "<h3>%s %s</h3>"
           "<p>%s is a graphical interface for Valgrind</p>"
           "<p>Copyright: %s %s<br />"
           "Email: %s</p>",
           vkConfig->vkname(),   vkConfig->vkVersion(), 
					 vkConfig->vkName(),   vkConfig->vkCopyright(), 
					 vkConfig->vkAuthor(), vkConfig->vkEmail() );

  MsgBox *mb = new MsgBox( parent, MsgBox::About, QString(buf) );
  mb->setButtonTexts( QStringList( "O&K" ) );

  Q_CHECK_PTR( mb );
  mb->exec();
}


void MsgBox::info( QWidget *parent, QString hdr, QString msg )
{
  MsgBox *mb = new MsgBox( parent, MsgBox::Info, msg, hdr, 1 );
  mb->setButtonTexts( QStringList( "O&K" ) );
  Q_CHECK_PTR( mb );
  mb->exec();
}

int MsgBox::query( QWidget* parent, QString hdr, QString msg,
									 int nbutts )
{
  MsgBox *mb = new MsgBox( parent, MsgBox::Query, msg, hdr, nbutts );
  QStringList names;
  names << "&Yes" << "&No" << "&Cancel";
  mb->setButtonTexts( names );
  Q_CHECK_PTR( mb );
  return mb->exec();
}

int MsgBox::query( QWidget* parent, QString hdr, QString msg,
									 QString buttonNames )
{
	QStringList buttonLabels( QStringList::split( ";", buttonNames ) );
	int nbutts = buttonLabels.count();
  MsgBox *mb = new MsgBox( parent, MsgBox::Query, msg, hdr, nbutts );
  mb->setButtonTexts( buttonLabels );
  Q_CHECK_PTR( mb );
  return mb->exec();
}

int MsgBox::warning( QWidget * parent, QString hdr, QString msg )
{
  MsgBox *mb = new MsgBox( parent, MsgBox::Warning, msg, hdr, 2 );
  QStringList names;
  names << "&Yes" << "&No";
  mb->setButtonTexts( names );
  Q_CHECK_PTR( mb );
  return mb->exec();
}

bool MsgBox::error( QWidget *parent, QString hdr, QString msg )
{
  MsgBox *mb = new MsgBox( parent, MsgBox::Error, msg, hdr );
  mb->setButtonTexts( QStringList( "O&K" ) );
  Q_CHECK_PTR( mb );
  return ( mb->exec() != vkYes );  /* ### evil hack */
}

void MsgBox::fatal( QWidget *parent, QString hdr, QString msg )
{
  MsgBox *mb = new MsgBox( parent, MsgBox::Fatal, msg, hdr );
  mb->setButtonTexts( QStringList( "O&K" ) );
  Q_CHECK_PTR( mb );
  mb->exec();
}



/* message handlers ---------------------------------------------------- */
#define VK_BUFLEN 8196

/* show information message */
void vkInfo( QWidget *w, QString hdr, const char *msg, ... ) 
{
  char buf[VK_BUFLEN];
  va_list ap;
  va_start( ap, msg );
  vsnprintf( buf, VK_BUFLEN, msg, ap );
  va_end( ap );
  MsgBox::info( w, hdr, buf );
}


/* ask user a question */
int vkQuery( QWidget *w, int nbutts, QString hdr, 
						 const char *msg, ... ) 
{
  char buf[VK_BUFLEN];
  va_list ap;
  va_start( ap, msg );
  vsnprintf( buf, VK_BUFLEN, msg, ap );
  va_end( ap );
  return MsgBox::query( w, hdr, buf, nbutts );
}

/* ask user a question, + set custom button labels */
int vkQuery( QWidget * w, QString hdr, 
						 QString labels, const char * msg, ... )
{
 char buf[VK_BUFLEN];
  va_list ap;
  va_start( ap, msg );
  vsnprintf( buf, VK_BUFLEN, msg, ap );
  va_end( ap );
  return MsgBox::query( w, hdr, buf, labels );
}

/* warn user about impending doom ... */
int vkWarn( QWidget * w, QString hdr, const char * msg, ... )
{
 char buf[VK_BUFLEN];
  va_list ap;
  va_start( ap, msg );
  vsnprintf( buf, VK_BUFLEN, msg, ap );
  va_end( ap );
  return MsgBox::warning( w, hdr, buf );
}

/* error message box */
bool vkError( QWidget *w, QString hdr, const char *msg, ... ) 
{
  char buf[VK_BUFLEN];
  va_list ap;
  va_start( ap, msg );
  vsnprintf( buf, VK_BUFLEN, msg, ap );
  va_end( ap );
  return MsgBox::error( w, hdr, buf );
}

/* msg box widget with ok button, returns exit errno */
int vkFatal( QWidget *w, QString hdr, const char *msg, ... )
{
  char buf[VK_BUFLEN];
  va_list ap;
  va_start( ap, msg );
  vsnprintf( buf, VK_BUFLEN, msg, ap );
  va_end( ap );
  MsgBox::fatal( w, hdr, buf );
  return EXIT_FAILURE;     /* goodbye, cruel world */
}

