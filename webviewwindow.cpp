/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the demonstration applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "webviewwindow.h"
#include "ui_mainwindow.h"

#include <QFile>
#include <QFileDialog>
#include <QFontDatabase>
#include <QMessageBox>
#include <QTextStream>
#include <QWebChannel>
#include <QThread>

WebViewWindow::WebViewWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    qDebug() << "~~~! MainWindow constructor";
    ui->setupUi(this);
    ui->editor->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    ui->preview->setContextMenuPolicy(Qt::NoContextMenu);

    page = new WebViewPage(this);
    ui->preview->setPage(page);

    //connect(ui->editor, &QPlainTextEdit::textChanged,
    //       [this]() { m_content.setText(ui->editor->toPlainText()); });

    QWebChannel *channel = new QWebChannel(this);
    // channel->registerObject(QStringLiteral("content"), &m_content);
    channel->registerObject(QStringLiteral("qtRemGlkForwarder"), this);
    page->setWebChannel(channel);

    // ui->preview->setUrl(QUrl("qrc:/index.html"));
    // ui->preview->setUrl(QUrl("http://localhost:4000/"));
    ui->preview->setUrl(QUrl("qrc:/GlkOte/play_if_remote.html"));

    // Test object access, is it crashing app?
    qDebug() << ":::::::: url " << page->url();
    qDebug() << ":::::::: url " << ui->preview->page()->url();

    connect(ui->actionNew, &QAction::triggered, this, &WebViewWindow::onFileNew);
    //connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::onFileOpen);
    //connect(ui->actionSave, &QAction::triggered, this, &MainWindow::onFileSave);
    //connect(ui->actionSaveAs, &QAction::triggered, this, &MainWindow::onFileSaveAs);
    connect(ui->actionExit, &QAction::triggered, this, &WebViewWindow::onExit);

    //connect(ui->editor->document(), &QTextDocument::modificationChanged,
    //        ui->actionSave, &QAction::setEnabled);

    QFile defaultTextFile(":/default.md");
    defaultTextFile.open(QIODevice::ReadOnly);
    ui->editor->setPlainText(defaultTextFile.readAll());
}

WebViewWindow::~WebViewWindow()
{
    delete ui;
}

QString dynamicJavaScriptCode;
int stanzaRun = 0;

int WebViewWindow::incomingRemGlkStanza(QString inRawJSON)
{
    stanzaRun++;
    // ToDo: better handoff technique to WebView, as RemGlk JSON stanzas can be very large when bitmap graphics are used.
    QString cleanPassalong = inRawJSON.replace("\\", "\\\\").replace("'", "\\'").replace("\n", " ");
    dynamicJavaScriptCode = "console.log('JSON_before'); incomingA('" + cleanPassalong + "'); console.log('JSON_after " + QString::number(stanzaRun) + "');";
    try {
        qDebug() << "OUT dynamicJavaScriptCode: " << dynamicJavaScriptCode;
        qDebug() << ":::::[DATA]:::: thread " << QThread::currentThreadId();
        ui->editor->setPlainText(inRawJSON);
        page->runJavaScript(dynamicJavaScriptCode);
    } catch(...)
    {
        qDebug() << "Exception";
    }
    return 1;
}

int windowStatus = 0;
QString remGlkInitMessage = "";

void WebViewWindow::toRemGlkEngine(QString payload)
{
    if (windowStatus == 0) {
        windowStatus = 1;
        this->show();
        remGlkInitMessage = payload;
        qDebug() << "::::: INIT message for RemGlk? ::: " << payload;
// :::: FixMe: CRASHING HERE, Deadlock ::::
// DO NOT send Init from GlkOte, as it seems to cause deadlock of entire Ubuntu desktop system
        // emit toRemGlkEngineForward(payload, 1);
        return;
    }
    qDebug() << "::::: OUT TO RemGlk? ::: " << payload;
    emit toRemGlkEngineForward(payload, 0);
}



void WebViewWindow::openFile(const QString &path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, windowTitle(),
                             tr("Could not open file %1: %2").arg(
                                 QDir::toNativeSeparators(path), f.errorString()));
        return;
    }
    m_filePath = path;
    ui->editor->setPlainText(f.readAll());
}

bool WebViewWindow::isModified() const
{
    return ui->editor->document()->isModified();
}

int newCount = 0;
void WebViewWindow::onFileNew()
{
    if (isModified()) {
        QMessageBox::StandardButton button = QMessageBox::question(this, windowTitle(),
                             tr("You have unsaved changes. Do you want to create a new document anyway?"));
        if (button != QMessageBox::Yes)
            return;
    }

    m_filePath.clear();
    newCount++;
    ui->editor->setPlainText(tr("## New document A"));
    ui->editor->document()->setModified(false);


// TEST access to the Qt window and WebView page
    // qDebug() << ":::::[TEST]::: URL " << page->url();
    qDebug() << ":::::[TEST]::: URL " << ui->preview->page()->url();
    qDebug() << ":::::[TEST]:::: thread " << QThread::currentThreadId();
    if (dynamicJavaScriptCode.length() > 2) {
        ui->editor->setPlainText(dynamicJavaScriptCode);
        ui->preview->page()->runJavaScript(dynamicJavaScriptCode);
    } else {
        ui->editor->setPlainText("JSON dynamicJavaScriptCode is not ready " + QString::number(newCount));
    }
}

void WebViewWindow::onFileOpen()
{
    if (isModified()) {
        QMessageBox::StandardButton button = QMessageBox::question(this, windowTitle(),
                             tr("You have unsaved changes. Do you want to open a new document anyway?"));
        if (button != QMessageBox::Yes)
            return;
    }

    QString path = QFileDialog::getOpenFileName(this,
        tr("Open MarkDown File"), "", tr("MarkDown File (*.md)"));
    if (path.isEmpty())
        return;

    openFile(path);
}

void WebViewWindow::onFileSave()
{
    if (m_filePath.isEmpty()) {
        onFileSaveAs();
        return;
    }

    QFile f(m_filePath);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text))  {
        QMessageBox::warning(this, windowTitle(),
                             tr("Could not write to file %1: %2").arg(
                                 QDir::toNativeSeparators(m_filePath), f.errorString()));
        return;
    }
    QTextStream str(&f);
    str << ui->editor->toPlainText();

    ui->editor->document()->setModified(false);
}

void WebViewWindow::onFileSaveAs()
{
    QString path = QFileDialog::getSaveFileName(this,
        tr("Save MarkDown File"), "", tr("MarkDown File (*.md, *.markdown)"));
    if (path.isEmpty())
        return;
    m_filePath = path;
    onFileSave();
}

void WebViewWindow::onExit()
{
    if (isModified()) {
        QMessageBox::StandardButton button = QMessageBox::question(this, windowTitle(),
                             tr("You have unsaved changes. Do you want to exit anyway?"));
        if (button != QMessageBox::Yes)
            return;
    }
    close();
}
