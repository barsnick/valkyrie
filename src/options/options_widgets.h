/* ---------------------------------------------------------------------- 
 * Definition of class OptionWidget                     options_widgets.h
 * Various widgets used on the 'pages' to control user input
 * ---------------------------------------------------------------------- 
 */

#ifndef __VK_OPTION_WIDGETS_H
#define __VK_OPTION_WIDGETS_H

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qpushbutton.h>
#include <qradiobutton.h>

#include "intspinbox.h"


/* class OptionWidget -------------------------------------------------- */
class Option;
class OptionWidget : public QObject
{
  Q_OBJECT
public:
  OptionWidget( QWidget* parent, const char* name, 
								Option* vkopt, bool mklabel );
  ~OptionWidget() { }

  int id();
  QLabel*  label();
  QWidget* widget();
  QString currValue();
  QString initValue();

  virtual void reset() = 0;
	virtual void resetDefault() = 0;
  virtual void saveEdit( bool perm );
  virtual void cancelEdit();
  virtual QHBoxLayout* hlayout();
  virtual QVBoxLayout* vlayout();

signals:
  void valueChanged( bool, OptionWidget * );

protected:
  QWidget* widg;
  QLabel*  wLabel;
  QHBoxLayout* hBox;
  QVBoxLayout* vBox;

  QString initialValue;
  QString currentValue;
  Option* opt;
};



/* class CkWidget: QCheckBox ------------------------------------------- */
class CkWidget : public OptionWidget
{
  Q_OBJECT
public:
  CkWidget( QWidget* parent, Option* vkopt, bool mklabel );
  ~CkWidget();

  bool isOn();
  void reset();
	void resetDefault();
  void setOn( bool on );

signals:
  void clicked(int);
  void changed(bool);

private slots:
  void ckChanged(bool);

private:
  QCheckBox* cbox;
  bool initialState;
};



/* class RbWidget: QRadioButton ---------------------------------------- */
class RbWidget : public OptionWidget
{
  Q_OBJECT
public:
  RbWidget( QWidget* parent, Option* vkopt, bool mklabel );
  ~RbWidget();

  bool isOn();
  void reset();
	void resetDefault();
  void setOn( bool on );

signals:
  void clicked(int);
  void changed(bool);

private slots:
  void rbChanged(bool);

private:
  QRadioButton* radio;
  bool initialState;
};



/* class LeWidget: QLineEdit ------------------------------------------- */
class LeWidget : public OptionWidget
{
  Q_OBJECT
public:
  LeWidget( QWidget* parent, Option* vkopt, bool mklabel );
  ~LeWidget();

  void reset();
	void resetDefault();
  void setCurrValue(const QString &);
  void addCurrValue(const QString &);
  void addButton(QWidget *parent, const QObject * receiver, 
                 const char * slot, QString txt=QString::null,
                 bool icon=true );
  QPushButton* button();
  QHBoxLayout* hlayout();

private slots:
  void leChanged(const QString& txt);

private:
  QLineEdit* ledit;
  QPushButton* pb;
};




/* class CbWidget: QComboBox ------------------------------------------- */
class CbWidget : public OptionWidget
{
  Q_OBJECT
public:
  CbWidget( QWidget* parent, Option* vkopt, bool mklabel );
  ~CbWidget();

  void reset();
	void resetDefault();
  QHBoxLayout* hlayout();

private slots:
  void cbChanged(const QString& txt);

private:
  int currIdx;
  QComboBox* combo;
};




/* class SpWidget: IntSpin --------------------------------------------- */
class SpWidget : public OptionWidget
{
  Q_OBJECT
public:
  SpWidget(QWidget* parent, Option* vkopt, bool mklabel, int num_sections);
  ~SpWidget();

  void reset();
	void resetDefault();
  void addSection( int min, int max, int defval=0,
                   int step=1, QString sep_char=" " );
  QHBoxLayout* hlayout();

private slots:
  void spChanged( const QString &val );

private:
  IntSpin* intspin;
  int numSections;
};



/* class LbWidget: QListBox -------------------------------------------- */
class LbWidget : public OptionWidget
{
  Q_OBJECT
public:
  LbWidget( QWidget* parent, Option* vkopt, bool mklabel );
  ~LbWidget();

  void reset();
	void resetDefault();

private slots:
  void lbChanged(const QString&);
  void popupMenu(QListBoxItem*, const QPoint & );

private:
  void load();

private:
  int numSelected;
  int maxSelections;
  QListBox *lbox;
  QPixmap pmSel;
  QPixmap pmUnsel;
};



/* class SuppFile ------------------------------------------------------ */
class SuppFile : public QListBoxPixmap
{
public:
  SuppFile( const QPixmap &pix, QString fname, QString mark )
    : QListBoxPixmap( pix ), selMark(mark), fName(fname) {
    /* expects an absolute filepath,
       eg. /usr/local/bin/file.supp */
    int posL = fname.find( '/', 2 );
    int posR = fname.findRev( '/' );
    QString tmp = fname.left( posL+1 ) 
                + "..." + fname.right( fname.length() - posR );
    setText( tmp );
  }
  QString fileName() { return fName; }
  bool selected() { return selMark == "[+]"; }
  QString selMark;
  QString fName;
};


#endif
