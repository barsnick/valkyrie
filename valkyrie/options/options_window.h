/* ---------------------------------------------------------------------- 
 * Definition of class OptionsWindow                     options_window.h
 * A container class for each tool's options / flags 'pane'.
 * Not modal, so user can keep it open and change flags as they work.
 * ---------------------------------------------------------------------- 
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2006, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#ifndef __VK_OPTIONS_WINDOW_H
#define __VK_OPTIONS_WINDOW_H

#include <qdict.h>
#include <qlistbox.h>
#include <qmainwindow.h>
#include <qpushbutton.h>
#include <qwidgetstack.h>

#include "options_page.h"


/* class Categories ---------------------------------------------------- */
class Categories : public QListBox
{
public:
   Categories( QWidget *parent );
   int categHeight();

private:
   int m_categht;
};



/* class CategItem ----------------------------------------------------- */
class CategItem : public QListBoxItem 
{
public:
   CategItem( QListBox * parent, OptionsPage * op,
              const QString &text, int id );
   virtual int height( const QListBox * ) const;
   int catId() const;
   void setWidget( OptionsPage * op );
   OptionsPage * page() const;

protected:
   void paint( QPainter *p );

private:
   int          m_catid;
   OptionsPage* m_optpage;
};



/* class OptionsWindow ------------------------------------------------- */
class OptionsWindow : public QMainWindow
{
   Q_OBJECT
public:
   OptionsWindow( QWidget* parent=0 );
   ~OptionsWindow();
   void showPage( int catid );

signals:
   void flagsChanged();

protected:
   void closeEvent( QCloseEvent * );
   void moveEvent( QMoveEvent * );

private slots:
   void accept();
   void reject();
   void apply();
   void modified();
   void resetDefaults();
   void categoryClicked( QListBoxItem * );

private:
   void adjustPosition();
   void setCategory( int catid );
   void addCategory( VkObject* obj );
   OptionsPage* mkOptionsPage( int catid );

private:
   /* remember where user put the window */
   int m_xpos, m_ypos;

   QString      m_capt;
   QPushButton* m_applyButton;
   QPushButton* m_resetButton;

   /* so we can iterate over the vkOptions widgets */
   QPtrList<OptionsPage> m_optPages;
   Categories*           m_categories;
   QWidgetStack*         m_wStack;
};


#endif
