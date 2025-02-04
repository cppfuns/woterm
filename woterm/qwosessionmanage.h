/*******************************************************************************************
*
* Copyright (C) 2022 Guangzhou AoYiDuo Network Technology Co.,Ltd. All Rights Reserved.
*
* Contact: http://www.aoyiduo.com
*
*   this file is used under the terms of the GPLv3[GNU GENERAL PUBLIC LICENSE v3]
* more information follow the website: https://www.gnu.org/licenses/gpl-3.0.en.html
*
*******************************************************************************************/

#ifndef QWOSESSIONMANAGE_H
#define QWOSESSIONMANAGE_H

#include "qwoglobal.h"
#include "qwohostlistmodel.h"

#include <QDialog>
#include <QPointer>
#include <QStandardItemModel>

namespace Ui {
class QWoSessionManage;
}

class QKxTermWidget;
class QStringListModel;
class QSortFilterProxyModel;
class QWoTreeView;


class QWoSessionManage : public QDialog
{
    Q_OBJECT

public:
    explicit QWoSessionManage(QWidget *parent = nullptr);
    ~QWoSessionManage();

signals:
    void readyToConnect(const QString& host, int type);
    void readyToConnect(const QStringList& host, bool diff);

private slots:
    void onEditTextChanged(const QString &txt);
    void onSshConnectReady();
    void onSftpConnectReady();
    void onTelnetConnectReady();
    void onRLoginConnectReady();
    void onMstscConnectReady();
    void onVncConnectReady();
    void onSerialPortConnectReady();
    void onOpenReady();
    void onDeleteReady();
    void onModifyReady();
    void onNewReady();
    void onImportReady();
    void onTreeViewOpenInSamePage();
    void onTreeViewOpenInDifferentPage();
    void onTreeItemSelected(const QModelIndex& idx);
    void onTreeItemDoubleClicked(const QModelIndex& idx);
private:
    void refreshList();
    bool handleTreeViewContextMenu(QContextMenuEvent *ev);
protected:
    bool eventFilter(QObject *obj, QEvent *ev);
private:
    Ui::QWoSessionManage *ui;
    QPointer<QWoHostListModel> m_model;
    QPointer<QSortFilterProxyModel> m_proxyModel;
    QPointer<QWoTreeView> m_tree;
};

#endif // QWOSESSIONPROPERTY_H
