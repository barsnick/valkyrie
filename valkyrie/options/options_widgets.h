/* ---------------------------------------------------------------------- 
 * Definition of class OptionWidget                     options_widgets.h
 * Various widgets used on the 'pages' to control user input
 * ---------------------------------------------------------------------- 
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
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
   void setEnabled( bool enable );

   virtual void reset() = 0;
   virtual void resetDefault() = 0;
   virtual void saveEdit();
   virtual void cancelEdit();
   virtual QHBoxLayout* hlayout();
   virtual QVBoxLayout* vlayout();

signals:
   void valueChanged( bool, OptionWidget * );

protected:
   QWidget*     m_widg;
   QLabel*      m_wLabel;
   QHBoxLayout* m_hBox;
   QVBoxLayout* m_vBox;

   QString      m_initialValue;
   QString      m_currentValue;
   Option*      m_opt;
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
   QCheckBox* m_cbox;
   bool       m_initialState;
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
   QRadioButton* m_radio;
   bool          m_initialState;
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
   void addButton(QWidget* parent, const QObject* receiver, 
                  const char* slot, QString txt=QString::null,
                  bool icon=false );
   void setReadOnly( bool );
   QPushButton* button();
   QHBoxLayout* hlayout();

signals:
   void returnPressed();

private slots:
   void leChanged(const QString& txt);

private:
   QLineEdit*   m_ledit;
   QPushButton* m_pb;
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
   int        m_currIdx;
   QComboBox* m_combo;
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
                    int step=1, QString sep_char=" : " );
   QHBoxLayout* hlayout();

private slots:
   void spChanged( const QString &val );

private:
   IntSpin* m_intspin;
   int      m_numSections;
};



/* class LbWidget: QListBox -------------------------------------------- 

   NOTE: This widget was specifically written to handle suppression files
   stuff and nothing else.
*/
class LbWidget : public OptionWidget
{
   Q_OBJECT
public:
   LbWidget( QWidget* parent, Option* vkopt, bool mklabel );
   ~LbWidget();
   void reset();
   void resetDefault();
   void setCurrValue(const QString &);

public slots:
   void insertItem( const QString& );
signals:
   void itemSelected( const QString& );
   void listChanged();

private slots:
   void popupMenu( QListBoxItem*, const QPoint & );
   void selectItem( QListBoxItem* lb_item);
   void popupSuppDirs( QListBoxItem* );
   void popupSuppAvail( QListBoxItem* );
   void popupSuppSel( QListBoxItem* );

private:
   QString lbText();
   void lbChanged();
   void lbLoad();

private:
   QListBox* m_lbox;
   QChar     m_sep;      /* so we don't have to keep asking vkConfig */

   /* 
      LB_SUPPDIRS  = valgrind::supps-dirs   = dirs to scan for supps
      LB_SUPPAVAIL = valgrind::supps-avail  = availalbe supps to use
      LB_SUPPSEL   = valgrind::suppressions = selected supps
   */
   enum LBMode{ LB_SUPPDIRS, LB_SUPPAVAIL, LB_SUPPSEL };
   LBMode    m_mode;
};

#endif
