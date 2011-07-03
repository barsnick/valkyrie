/****************************************************************************
** LogViewFilterMC definition
** --------------------------------------------------------------------------
**
** Copyright (C) 2011-2011, OpenWorks LLP. All rights reserved.
** <info@open-works.co.uk>
**
** This file is part of Valkyrie, a front-end for Valgrind.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file COPYING included in the packaging of
** this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef LOGVIEWFILTER_MC_H
#define LOGVIEWFILTER_MC_H

#include "toolview/vglogview.h"

#include <QComboBox>
#include <QPushButton>
#include <QStackedWidget>
#include <QTreeWidget>
#include <QWidget>


class LogViewFilterMC : public QWidget
{
    Q_OBJECT
public:
    LogViewFilterMC(QWidget *parent, QTreeWidget* view );

public slots:
    void showHideItem( VgOutputItem* item );
    void enableFilter( bool enable );
    
private slots:
    void setupFilter( int idx );
    void updateView();
    void edited();
    void refresh();

private:
    QTreeWidget* m_view;        // hold on to this to rescan entire tree.
    
    QPushButton* butt_refresh;  // refresh the filter after editing
    QComboBox* combo_xmltag;    // combobox of xmltags to filter on
    QStackedWidget* cmpWidgStack;    // hold the different compare comboboxes
    QStackedWidget* filterWidgStack; // hold the different filter value widgets

    enum XmlTagType { XML_KND, XML_LBY, XML_LBL, XML_OBJ, XML_FUN, XML_DIR, XML_FIL, XML_LIN };
    enum CmpType { CMP_KND, CMP_STR, CMP_INT };
    enum CmpFunType { FUN_EQL, FUN_NEQL, FUN_LSTHN, FUN_GRTHN, FUN_CONT,
                      FUN_NCONT, FUN_STRT, FUN_NSTRT, FUN_END, FUN_NEND };
    QMap<XmlTagType, CmpType> map_xmltag_cmptype;
    
    bool xmlCompare( VgOutputItem* errItem, const QString& tag,
                     const QString& str_flt, CmpFunType cmpFun, CmpType cmp_type );
    bool compare_strings( const QStringList& list_xml, const QString& str_flt, CmpFunType cmpfuntype );
    bool compare_integers( const QStringList& list_xml, const QString& str_flt, CmpFunType cmpfuntype );
};

#endif // LOGVIEWFILTER_MC_H
