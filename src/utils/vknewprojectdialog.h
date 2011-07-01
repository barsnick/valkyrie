/****************************************************************************
** VkNewProjectDialog definition
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

#ifndef VKNEWPROJECTDIALOG_H
#define VKNEWPROJECTDIALOG_H

#include <QDialog>
#include <QDialogButtonBox>
#include <QString>
#include <QLabel>
#include <QLineEdit>


class VkNewProjectDialog : public QDialog
{
    Q_OBJECT
public:
    VkNewProjectDialog( QWidget *parent = 0 );

    
    QString getProjectPath();
    
 private slots:
    void accept();
    void browseDir();
    void checkInput();
    
 private:
    QLineEdit* edit_name;
    QLineEdit* edit_dir;
    QLabel* lbl_warn;
    QString lbl_style_normal;
    QString edit_style_normal;
    QDialogButtonBox* buttonBox;
};

#endif // VKNEWPROJECTDIALOG_H
