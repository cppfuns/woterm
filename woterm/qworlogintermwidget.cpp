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

#include "qworlogintermwidget.h"
#include "qwosetting.h"
#include "qwosshconf.h"
#include "qwoutils.h"
#include "qwoglobal.h"
#include "qwotermwidgetimpl.h"
#include "qworlogin.h"
#include "qwotermmask.h"
#include "qwopasswordinput.h"
#include "qwohostsimplelist.h"
#include "qmodem.h"
#include "qwomainwindow.h"
#include "qwoevent.h"
#include "qwosessionproperty.h"
#include "qwofloatwindow.h"
#include "qwoshower.h"

#include "qkxtermwidget.h"
#include "qkxtermitem.h"


#include <QTimer>
#include <QGuiApplication>
#include <QClipboard>
#include <QMenu>
#include <QFileDialog>
#include <QMessageBox>
#include <QProcess>
#include <QDebug>
#include <QResizeEvent>

QWoRLoginTermWidget::QWoRLoginTermWidget(const QString& target, int gid, QWidget *parent)
    : QWoTermWidget(target, gid, parent)
    , m_savePassword(false)
{
    QObject::connect(m_term, SIGNAL(termSizeChanged(int,int)), this, SLOT(onTermSizeChanged(int,int)));
    QObject::connect(m_term, SIGNAL(sendData(const QByteArray&)), this, SLOT(onSendData(const QByteArray&)));
    QObject::connect(m_term, SIGNAL(titleChanged(const QString&)), this, SLOT(onTitleChanged(const QString&)));
    m_modem = QWoModemFactory::instance()->create(false);
    QObject::connect(m_modem, SIGNAL(dataArrived(const QByteArray&)), this, SLOT(onZmodemDataArrived(const QByteArray&)));
    QObject::connect(m_modem, SIGNAL(statusArrived(const QByteArray&)), this, SLOT(onZmodemStatusArrived(const QByteArray&)));
    QObject::connect(m_modem, SIGNAL(finished()), this, SLOT(onZmodemFinished()));

    QMetaObject::invokeMethod(this, "reconnect", Qt::QueuedConnection);
}

QWoRLoginTermWidget::~QWoRLoginTermWidget()
{
    QWoRLoginFactory::instance()->release(m_rlogin);
    QWoModemFactory::instance()->release(m_modem);
}

void QWoRLoginTermWidget::onFinishArrived(int code)
{
    //qDebug() << "exitcode" << code;
    showLoading(false);
    if(m_mask && m_mask->isVisible()) {
        return;
    }

    if(m_mask == nullptr) {
        m_mask = new QWoTermMask(this);
        QObject::connect(m_mask, SIGNAL(aboutToClose(QCloseEvent*)), this, SLOT(onForceToCloseThisSession()));
        QObject::connect(m_mask, SIGNAL(reconnect()), this, SLOT(onSessionReconnect()));
    }
    m_mask->setGeometry(0, 0, width(), height());
    m_mask->show();
}

void QWoRLoginTermWidget::onDataArrived(const QByteArray &buf)
{
    static QRegExp rgxUser(".* login: $");
    static QRegExp rgxPwd(".*Password: $");

    showLoading(false);
    if(m_modem->isRunning()) {
        if(!m_modem->onReceive(buf)) {
            //qDebug() << "onDataArrived" << buf;
        }
    }else{
        if(m_term->appMode()) {
            m_term->parse(buf);
        }else{
            if(m_loginCount == 0) {
                if(rgxUser.exactMatch(buf)) {
                    HostInfo hi = QWoSshConf::instance()->find(m_target);
                    m_rlogin->write(hi.user.toUtf8() + "\r\n");
                    m_loginCount = 1;
                }else if(rgxPwd.exactMatch(buf)) {
                    HostInfo hi = QWoSshConf::instance()->find(m_target);
                    QByteArray pass = hi.password.toUtf8();
                    m_rlogin->write(pass + "\r\n");
                    m_loginCount = 2;
                }
            }else if(m_loginCount == 1) {
                if(rgxPwd.exactMatch(buf)) {
                    HostInfo hi = QWoSshConf::instance()->find(m_target);
                    m_rlogin->write(hi.password.toUtf8() + "\r\n");
                    m_loginCount = 2;
                }
            }

            int err = isZmodemCommand(buf);
            m_term->parse(buf);
            if(err == 0) {
                QMetaObject::invokeMethod(this, "onZmodemRecv", Qt::QueuedConnection, Q_ARG(bool, false));
            }else if(err == 1) {
                QMetaObject::invokeMethod(this, "onZmodemSend", Qt::QueuedConnection, Q_ARG(bool, false));
            }
        }
    }
}

void QWoRLoginTermWidget::onErrorArrived(const QByteArray &buf)
{
    QKxTermItem *qterm = termItem();
    qterm->parseError(buf);
}

void QWoRLoginTermWidget::onInputArrived(const QString &title, const QString &prompt, bool visble)
{
    showPasswordInput(title, prompt, visble);
}

void QWoRLoginTermWidget::onPasswordArrived(const QString &host, const QByteArray &pass)
{
    if(m_savePassword){
        QWoSshConf::instance()->updatePassword(host, pass);
    }
}

void QWoRLoginTermWidget::onTermSizeChanged(int lines, int columns)
{
    if(m_rlogin) {
        m_rlogin->updateSize(columns, lines);
    }
}

void QWoRLoginTermWidget::onSendData(const QByteArray &buf)
{
    if(m_rlogin) {
        m_term->scrollToEnd();
        if(!m_modem->isRunning()) {
            m_rlogin->write(buf);
        }else{
            if(buf == "\003") {
                onZmodemAbort();
            }
        }
    }
}

void QWoRLoginTermWidget::onCopyToClipboard()
{
    termItem()->tryToCopy();
}

void QWoRLoginTermWidget::onPasteFromClipboard()
{
    termItem()->tryToPaste();
}

void QWoRLoginTermWidget::onForceToReconnect()
{
    reconnect();
}

void QWoRLoginTermWidget::onPasswordInputResult(const QString &pass, bool isSave)
{
    m_savePassword = isSave;
    if(m_rlogin){
        m_rlogin->setInputResult(pass);
    }
}

void QWoRLoginTermWidget::onSessionReconnect()
{    
    reconnect();
}

void QWoRLoginTermWidget::onVerticalSplitView()
{
    splitWidget(m_target, m_gid, true);
}

void QWoRLoginTermWidget::onHorizontalSplitView()
{
    splitWidget(m_target, m_gid, false);
}

void QWoRLoginTermWidget::onVerticalInviteView()
{
    QWoHostSimpleList dlg(Telnet, this);
    dlg.setWindowTitle(tr("session list"));
    dlg.exec();
    HostInfo hi;
    if(dlg.result(&hi)) {
        splitWidget(hi.name, QWoUtils::gid(), true);
    }
}

void QWoRLoginTermWidget::onHorizontalInviteView()
{
    QWoHostSimpleList dlg(Telnet, this);
    dlg.setWindowTitle(tr("session list"));
    dlg.exec();
    HostInfo hi;
    if(dlg.result(&hi)) {
        splitWidget(hi.name, QWoUtils::gid(), false);
    }
}

void QWoRLoginTermWidget::onCloseThisSession()
{
    closeAndDelete();
}

void QWoRLoginTermWidget::onForceToCloseThisSession()
{
    closeAndDelete();
}

void QWoRLoginTermWidget::onShowFindBar()
{
    setFindBarVisible(true);
}

void QWoRLoginTermWidget::onDuplicateInNewWindow()
{
    QWoUtils::openself("rlogin", m_target, false);
}

void QWoRLoginTermWidget::onModifyThisSession()
{
    if(!QWoSshConf::instance()->exists(m_target)){
        QMessageBox::warning(this, tr("Error"), tr("can't find the session, maybe it had been delete ago"));
        return;
    }
    QWoSessionProperty dlg(this);
    dlg.setSession(m_target);
    int ret = dlg.exec();
    if(ret == QWoSessionProperty::Save) {
        initCustom();
    }
}

void QWoRLoginTermWidget::onZmodemSend(bool local)
{
    if(local) {
        if(!checkProgram("rz")) {
            return;
        }
    }
    QString pathLast = QWoSetting::value("zmodem/lastPath").toString();
    QStringList files = QFileDialog::getOpenFileNames(this, tr("Select Files"), pathLast);
    //qDebug() << "zmodem send " << files;
    if(files.isEmpty()) {
        return;
    }
    QStringList args;
    QString path = files.front();
    int idx = path.lastIndexOf('/');
    if(idx > 0) {
        path = path.left(idx);
        QWoSetting::setValue("zmodem/lastPath", path);
    }

    m_modem->ZSendFiles(files);
}

void QWoRLoginTermWidget::onZmodemRecv(bool local)
{
    QString path = QWoSetting::value("zmodem/lastPath").toString();
    QString filePath = QFileDialog::getExistingDirectory(this, tr("Open Directory"), path,  QFileDialog::ShowDirsOnly);
    qDebug() << "filePath" << filePath;
    if(filePath.isEmpty()) {
        onZmodemAbort();
        return;
    }
    filePath = QDir::toNativeSeparators(filePath);
    QWoSetting::setValue("zmodem/lastPath", filePath);
    m_modem->ZReceive(filePath);
}

void QWoRLoginTermWidget::onZmodemAbort()
{
    m_modem->stop();
}

void QWoRLoginTermWidget::onZmodemDataArrived(const QByteArray &buf)
{
    if(m_modem->isRunning()) {
        //qDebug() << "onZmodemDataArrived" << objectName() << buf;
        m_rlogin->write(buf);
    }
}

void QWoRLoginTermWidget::onZmodemStatusArrived(const QByteArray &buf)
{
    if(m_modem->isRunning()) {
        m_term->parse(buf);
    }
}

void QWoRLoginTermWidget::onZmodemFinished()
{
    //qDebug() << "onZmodemFinished" << objectName();
    m_rlogin->write("\r");
    m_term->parse("\033[?25h");
}

void QWoRLoginTermWidget::onSftpConnectReady()
{
    QMetaObject::invokeMethod(QWoMainWindow::instance(), "onSessionReadyToSftpConnect", Qt::QueuedConnection, Q_ARG(QString, m_target));
}

void QWoRLoginTermWidget::onTitleChanged(const QString &title)
{
    m_loginCount = 100;
}

void QWoRLoginTermWidget::showPasswordInput(const QString &title, const QString &prompt, bool echo)
{
    if(m_passInput == nullptr) {
        m_passInput = new QWoPasswordInput(this);
        QObject::connect(m_passInput, SIGNAL(result(const QString&,bool)), this, SLOT(onPasswordInputResult(const QString&,bool)));
    }
    if(m_passInput->isVisible()) {
        return;
    }
    m_passInput->reset(title, prompt, echo);
    m_passInput->setGeometry(0, 0, width(), height());
    m_passInput->show();
}

int QWoRLoginTermWidget::isZmodemCommand(const QByteArray &data)
{
    bool isApp = m_term->appMode();
    if(isApp || data.length() < 6) {
        return -1;
    }
    // hex way.
    //char zmodem_init_hex[] = {'*','*','\030', 'B', '0', '0', '\0'};
    const char *buf = data.data();
    for(int i = 0; i < data.length() && i < 100; i++) {
        if(buf[i] == '*' && buf[i+1] == '*' && buf[i+2] == '\030'
                && buf[i+3] == 'B' && buf[i+4] == '0') {
            if(buf[i+5] == '1') {
                // shell trigger rz command.
                return 1;
            }else if(buf[i+5] == '0') {
                // shell trigger sz command.
                return 0;
            }
            return -1;
        }
    }
    return -1;
}

bool QWoRLoginTermWidget::checkProgram(const QByteArray &name)
{
    if(m_modem->isRunning()) {
        return false;
    }
    if(m_term->appMode()) {
        return false;
    }
    QString txt;
    int code;
    if(!m_term->trapCommand("which "+name, txt, code)) {
        return false;
    }
    return code == 0;
}

void QWoRLoginTermWidget::reconnect()
{
    showLoading(true);
    m_loginCount = 0;
    if(m_rlogin) {
        QWoRLoginFactory::instance()->release(m_rlogin);
    }
    m_term->resetState();
    QSize sz = m_term->termSize();
    m_rlogin = QWoRLoginFactory::instance()->create();
    m_rlogin->start(m_target, sz.width(), sz.height());
    QObject::connect(m_rlogin, SIGNAL(finishArrived(int)), this, SLOT(onFinishArrived(int)));
    QObject::connect(m_rlogin, SIGNAL(dataArrived(const QByteArray&)), this, SLOT(onDataArrived(const QByteArray&)));
    QObject::connect(m_rlogin, SIGNAL(errorArrived(const QByteArray&)), this, SLOT(onErrorArrived(const QByteArray&)));
    QObject::connect(m_rlogin, SIGNAL(passwordArrived(const QString&,const QByteArray&)), this, SLOT(onPasswordArrived(const QString&,const QByteArray&)));
    QObject::connect(m_rlogin, SIGNAL(inputArrived(const QString&,const QString&,bool)), this, SLOT(onInputArrived(const QString&,const QString&,bool)));  
}

void QWoRLoginTermWidget::resizeEvent(QResizeEvent *ev)
{
    QWoTermWidget::resizeEvent(ev);
    QSize sz = ev->size();
    //qDebug() << "resizeEvent" << objectName() << sz << ev->oldSize();
    if(m_mask) {
        m_mask->setGeometry(0, 0, sz.width(), sz.height());
    }
    if(m_passInput) {
        m_passInput->setGeometry(0, 0, sz.width(), sz.height());
    }
}

void QWoRLoginTermWidget::contextMenuEvent(QContextMenuEvent *ev)
{
//    QQuickItem *bar = findBar();
//    if(bar->isVisible()) {
//        QRectF rt = QRectF(bar->position(), QSizeF(bar->width(), bar->height()));
//        QPointF pt = ev->pos();
//        if(rt.contains(pt)) {
//            return ;
//        }
//    }
    if(m_menu == nullptr) {
        m_menu = new QMenu(this);
        m_copy = m_menu->addAction(tr("Copy"));
        QObject::connect(m_copy, SIGNAL(triggered()), this, SLOT(onCopyToClipboard()));
        m_paste = m_menu->addAction(tr("Paste"));
        QObject::connect(m_paste, SIGNAL(triggered()), this, SLOT(onPasteFromClipboard()));
        m_menu->addAction(QIcon(":/woterm/resource/skin/reload.png"), tr("Force Reconnect"), this, SLOT(onForceToReconnect()));
        QAction *vsplit = m_menu->addAction(QIcon(":/woterm/resource/skin/vsplit.png"), tr("Split Vertical"));
        QObject::connect(vsplit, SIGNAL(triggered()), this, SLOT(onVerticalSplitView()));
        QAction *hsplit = m_menu->addAction(QIcon(":/woterm/resource/skin/hsplit.png"), tr("Split Horizontal"));
        QObject::connect(hsplit, SIGNAL(triggered()), this, SLOT(onHorizontalSplitView()));
        //QAction *vinvite = m_menu->addAction(QIcon(":/woterm/resource/skin/vaddsplit.png"), tr("Add To Vertical"));
        //QObject::connect(vinvite, SIGNAL(triggered()), this, SLOT(onVerticalInviteView()));
        //QAction *hinvite = m_menu->addAction(QIcon(":/woterm/resource/skin/haddsplit.png"), tr("Add To Horizontal"));
        //QObject::connect(hinvite, SIGNAL(triggered()), this, SLOT(onHorizontalInviteView()));
        QAction *close = m_menu->addAction(tr("Close Session"));
        QObject::connect(close, SIGNAL(triggered()), this, SLOT(onCloseThisSession()));

        QWoFloatWindow *wfloat = qobject_cast<QWoFloatWindow*>(topLevelWidget());
        if(wfloat == nullptr) {
            m_menu->addAction(tr("Float This Tab"), this, SLOT(onFloatThisTab()));
        }

        m_menu->addAction(QIcon(":/woterm/resource/skin/find.png"), tr("Find..."), this, SLOT(onShowFindBar()), QKeySequence(Qt::CTRL +  Qt::Key_F));
        m_menu->addAction(QIcon(":/woterm/resource/skin/palette.png"), tr("Edit"), this, SLOT(onModifyThisSession()));
        //m_menu->addAction(QIcon(":/woterm/resource/skin/history.png"), tr("History"), this, SLOT(onSessionCommandHistory()));
        m_menu->addAction(tr("Duplicate In New Window"), this, SLOT(onDuplicateInNewWindow()));
        m_menu->addAction(tr("Clean history"), this, SLOT(onCleanHistory()));
        m_output = m_menu->addAction(tr("Output history to file"), this, SLOT(onOutputHistoryToFile()));
        m_stop = m_menu->addAction(tr("Stop history to file"), this, SLOT(onStopOutputHistoryFile()));

        m_menu->addAction(QIcon(":/woterm/resource/skin/upload.png"), tr("Zmodem Upload"), this, SLOT(onZmodemSend()));
        m_menu->addAction(QIcon(":/woterm/resource/skin/download.png"), tr("Zmodem Receive"), this, SLOT(onZmodemRecv()));
        m_menu->addAction(tr("Zmoddem Abort"), this, SLOT(onZmodemAbort()), QKeySequence(Qt::CTRL +  Qt::Key_C));

    }
    QKxTermItem *term = termItem();
    QString selTxt = term->selectedText();
    //qDebug() << "selectText" << selTxt;
    m_copy->setDisabled(selTxt.isEmpty());
    m_output->setVisible(m_historyFile.isEmpty());
    m_stop->setVisible(!m_historyFile.isEmpty());

    QClipboard *clip = QGuiApplication::clipboard();
    QString clipTxt = clip->text();
    m_paste->setDisabled(clipTxt.isEmpty());
    m_menu->exec(QCursor::pos());
    m_menu->deleteLater();
}

QList<QString> QWoRLoginTermWidget::collectUnsafeCloseMessage()
{
    QList<QString> all;
    if(m_modem->isRunning()) {
        QString msg = tr("Files are transfering...");
        all.append(msg);
    }
    return all;
}
