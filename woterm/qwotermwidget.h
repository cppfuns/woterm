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

#pragma once

#include <qkxtermwidget.h>

#include <QPointer>

class QMenu;
class QWoTermMask;
class QWoPasswordInput;
class QWoTermWidgetImpl;
class QSplitter;
class QLabel;

class QWoTermWidget;
class QWoLoadingWidget;

class QWoTermCreator
{
public:
    virtual QWoTermWidget *create(const QString& title, QWidget *parent) = 0;
protected:
};

template <class T>
class QWoTermCreatorT : public QWoTermCreator
{
public:
    virtual QWoTermWidget *create(const QString& title, QWidget *parent){
        return new T(title, parent);
    }
};

class QWoTermWidget : public QKxTermWidget
{
    Q_OBJECT
public:
    explicit QWoTermWidget(const QString& target, int gid, QWidget *parent=nullptr);
    virtual ~QWoTermWidget();

    void closeAndDelete();

    void splitWidget(const QString& target, int gid, bool vertical);

    void triggerPropertyCheck();

    QString target() const;

    void showLoading(bool on);
    void reloadProperty();
signals:
    void aboutToClose(QCloseEvent* event);

protected:
    virtual void closeEvent(QCloseEvent *event);
    virtual void resizeEvent(QResizeEvent *event);
    virtual bool event(QEvent* ev);
    // for close safely.
    virtual QList<QString> collectUnsafeCloseMessage() = 0;
protected:
    void initDefault();
    void initCustom();
protected slots:
    void onCleanHistory();
    void onOutputHistoryToFile();
    void onStopOutputHistoryFile();
    void onFloatThisTab();
private:
    void resetProperty(QVariantMap data);

    QWoTermWidgetImpl *findTermImpl();
    void addToTermImpl();
    void removeFromTermImpl();

    void onBroadcastMessage(int type, QVariant msg);
    bool handleWoEvent(QEvent *ev);
protected:
    friend class QWoTermWidgetImpl;
    QPointer<QWoLoadingWidget> m_loading;
    QString m_target;
    int m_gid;
    bool m_bexit;
    QString m_historyFile;
};
