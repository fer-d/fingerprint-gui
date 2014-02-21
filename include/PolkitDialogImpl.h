/*
 * Project "Fingerprint GUI": Services for fingerprint authentication on Linux
 * Module: PolkitDialog.cpp, PolkitDialog.h
 * Purpose: GUI dialog for polkit agent for fingerprint authentication
 *
 * @author  Wolfgang Ullrich
 * Copyright (C) 2008-2012  Wolfgang Ullrich
 */

/*
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef POLKITDIALOGIMPL_H
#define	POLKITDIALOGIMPL_H

#include <QtGui>
#include <polkit-qt-1/PolkitQt1/Authority>
#include <polkit-qt-1/PolkitQt1/Details>
#include "ui_PolkitDialog.h"
#include "ui_PolkitDetails.h"
#include "ui_FingerprintPlugin.h"
#include "Globals.h"
#include "PluginDialog.h"

class QStandardItemModel;

namespace PolkitQt1
{
class Details;
class Identity;
class ActionDescription;
}

using namespace std;

//------------------------------------------------------------------------------
class PolkitDetailsImpl : public QWidget, private Ui::PolkitDetails {
    Q_OBJECT
public:
#ifdef LIBPOLKIT_QT_1_1
    PolkitDetailsImpl(const PolkitQt1::Details &details,
#else
    PolkitDetailsImpl(PolkitQt1::Details *details,
#endif //LIBPOLKIT_QT_1_1
                PolkitQt1::ActionDescription *actionDescription,
                const QString &appname,
                QWidget *parent);
};

//------------------------------------------------------------------------------
class PolkitDialogImpl : public QDialog, public Ui::PolkitDialog {
    Q_OBJECT
public:
#ifdef LIBPOLKIT_QT_1_1
    PolkitDialogImpl(
            const QString &actionId,
            const QString &message,
            const QString &iconName,
            const PolkitQt1::Details &details,
            const PolkitQt1::Identity::List &identities,
            QWidget * parent = 0, Qt::WFlags f = 0);
#else
    PolkitDialogImpl(
            const QString &actionId,
            const QString &message,
            const QString &iconName,
            PolkitQt1::Details *details,
            QList<PolkitQt1::Identity *> identities,
            QWidget * parent = 0, Qt::WFlags f = 0);
#endif //LIBPOLKIT_QT_1_1
    virtual ~PolkitDialogImpl();
    QString password() const;
    void authenticationFailure();
    void setOptions();
    void setRequest(const QString &request,bool requiresAdmin);
    PluginDialog *pluginDialog;
#ifdef LIBPOLKIT_QT_1_1
    PolkitQt1::Identity userSelected();
#else
    PolkitQt1::Identity *userSelected();
#endif //LIBPOLKIT_QT_1_1
    PolkitQt1::ActionDescription *actionDescription;
    bool preselectUser();

signals:
#ifdef LIBPOLKIT_QT_1_1
    void adminUserSelected(PolkitQt1::Identity);
#else
    void adminUserSelected(PolkitQt1::Identity *);
#endif //LIBPOLKIT_QT_1_1
private slots:
    void userComboboxIndexChanged(int index);
    void showDetails();
public slots:
    virtual void accept();
    void resize();
private:
    PolkitDetailsImpl *detailsDialog;
    QString appname;
#ifdef LIBPOLKIT_QT_1_1
    void createUserCombobox(const PolkitQt1::Identity::List &identities);
#else
    QStandardItemModel *userModelSIM;
    void createUserCombobox(QList<PolkitQt1::Identity *> identities);
#endif //LIBPOLKIT_QT_1_1
};
#endif	/* POLKITDIALOGIMPL_H */
