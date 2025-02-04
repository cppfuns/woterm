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

#ifndef QWOIDENTIFYCREATEDIALOG_H
#define QWOIDENTIFYCREATEDIALOG_H

#include <QDialog>

namespace Ui {
class QWoIdentifyCreateDialog;
}

class QWoIdentifyCreateDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QWoIdentifyCreateDialog(QWidget *parent = 0);
    ~QWoIdentifyCreateDialog();
private slots:
    void onTypeCurrentIndexChanged(const QString &txt);
    void onButtonCreateClicked();
private:
    Ui::QWoIdentifyCreateDialog *ui;
};

#endif // QWOIDENTIFYCREATEDIALOG_H
