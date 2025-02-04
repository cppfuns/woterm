﻿/*******************************************************************************************
*
* Copyright (C) 2022 Guangzhou AoYiDuo Network Technology Co.,Ltd. All Rights Reserved.
*
* Contact: http://www.aoyiduo.com
*
*   this file is used under the terms of the GPLv3[GNU GENERAL PUBLIC LICENSE v3]
* more information follow the website: https://www.gnu.org/licenses/gpl-3.0.en.html
*
*******************************************************************************************/

#ifndef QWOPASSWORDINPUT_H
#define QWOPASSWORDINPUT_H

#include "qwowidget.h"

namespace Ui {
class QWoPasswordInput;
}

class QWoPasswordInput : public QWoWidget
{
    Q_OBJECT

public:
    explicit QWoPasswordInput(QWidget *parent = nullptr);
    ~QWoPasswordInput();

    void reset(const QString& title, const QString& prompt, bool echo);

signals:
    void result(const QString& pass, bool isSave) const;
private slots:
    void onPasswordVisible(bool checked);
    void onClose();
private:
    void paintEvent(QPaintEvent* paint);
    void mousePressEvent(QMouseEvent *ev);
    void contextMenuEvent(QContextMenuEvent *ev);
private:
    Ui::QWoPasswordInput *ui;
};

#endif // QWOPASSWORDINPUT_H
