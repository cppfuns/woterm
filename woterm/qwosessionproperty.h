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

#ifndef QWOSESSIONPROPERTY_H
#define QWOSESSIONPROPERTY_H

#include <QDialog>
#include <QMap>

namespace Ui {
class QWoSessionProperty;
}

class QWoSessionProperty : public QDialog
{
    Q_OBJECT
public:
    enum EResult {
        Rejected = QDialog::Rejected,
        Accepted = QDialog::Accepted,
        Save,
        Cancel
    };
public:
    explicit QWoSessionProperty(QWidget *parent = 0);
    ~QWoSessionProperty();
    void setSession(const QString& name);

signals:
    void readyToConnect(const QString& host, int type);
private:
    void init();
    void updatePortTipState();
    void resizeWidget();
private slots:
    void onTypeCurrentIndexChanged(const QString& txt);
    void onTypeConnect();
    void onTypeSave();
    void onAuthCurrentIndexChanged(const QString& txt);
    void onPortTextChanged(const QString& txt);
    void onProxyJumpeBrowser();
    void onIdentifyFileBrowser();
    void onMoreConfig();
private:
    bool saveConfig();
private:
    Ui::QWoSessionProperty *ui;
    QString m_name;
    QMap<QString, QString> m_props;
};

#endif // QWOSESSIONPROPERTY_H
