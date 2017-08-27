/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
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

/*
 * This code seems small enough, that I'm thinking to keep it a window and just not surface it
 * Can I create an instance of this class and build the list of files - and show and hide the
 * window?
 *
 * ToDo: repurpose the full text search input field to match filenames?
 *    Allow filtering of the names, typing chars?
 * */

#include <QtWidgets>

#include "findstorieswindow.h"
#include "findfilesthread.h"

//! [0]
FindStoriesWindow::FindStoriesWindow(QWidget *parent)
    : QWidget(parent)
{
    QPushButton *browseButton = new QPushButton(tr("&Browse..."), this);
    connect(browseButton, &QAbstractButton::clicked, this, &FindStoriesWindow::browse);
    findButton = new QPushButton(tr("&Find"), this);
    connect(findButton, &QAbstractButton::clicked, this, &FindStoriesWindow::find);

    fileComboBox = createComboBox(tr("*.gblorb"));
    connect(fileComboBox->lineEdit(), &QLineEdit::returnPressed,
            this, &FindStoriesWindow::animateFindClick);
    textComboBox = createComboBox();
    connect(textComboBox->lineEdit(), &QLineEdit::returnPressed,
            this, &FindStoriesWindow::animateFindClick);
    bool searchCurrentPathDefualt = false;
    if (searchCurrentPathDefualt) {
        directoryComboBox = createComboBox(QDir::toNativeSeparators(QDir::currentPath()));
    } else {
        directoryComboBox = createComboBox(QDir::toNativeSeparators(defaultHardcodedPath));
    }
    connect(directoryComboBox->lineEdit(), &QLineEdit::returnPressed,
            this, &FindStoriesWindow::animateFindClick);

    filesFoundLabel = new QLabel;

    createFoundResultsOutput();
//! [0]

//! [1]
    QGridLayout *mainLayout = new QGridLayout(this);
// #ifdef SEARCH_FULL_FEATURE_A
    mainLayout->addWidget(new QLabel(tr("Named:")), 0, 0);
    mainLayout->addWidget(fileComboBox, 0, 1, 1, 2);
    mainLayout->addWidget(new QLabel(tr("Containing text:")), 1, 0);
    mainLayout->addWidget(textComboBox, 1, 1, 1, 2);
// #endif
    mainLayout->addWidget(new QLabel(tr("In directory:")), 2, 0);
    mainLayout->addWidget(directoryComboBox, 2, 1);
    mainLayout->addWidget(browseButton, 2, 2);
    mainLayout->addWidget(foundResultsOutput, 3, 0, 1, 3);
    mainLayout->addWidget(filesFoundLabel, 4, 0, 1, 2);
    mainLayout->addWidget(findButton, 4, 2);

    setWindowTitle(tr("Find Interactive Fiction Story Files - HTML"));
    const QRect screenGeometry = QApplication::desktop()->screenGeometry(this);
    resize(screenGeometry.width() / 2, screenGeometry.height() / 3);

    connect(&thread, SIGNAL(foundFiles(QStringList)), this, SLOT(updateFoundFiles(QStringList)));
    connect(&thread, SIGNAL(statusReportFoundFiles(QString, int)), this, SLOT(showStatusReportFoundFiles(QString, int)));


#ifdef Q_OS_ANDROID
    // This works on Desktop but fails on Android
    //   Android issues:
    //     http://www.qtcentre.org/threads/62217-Style-sheet-working-well-on-all-platfors-except-Android
    // textEdit->verticalScrollBar()->setStyleSheet("QScrollBar:vertical { width: 2px; }");
    // Some recent experience with DPI issues:
    //   https://github.com/owncloud/client/issues/3414

    setStyleSheet(QString::fromUtf8("QScrollBar:vertical {"
        "    border: 1px solid #999999;"
        "    background:white;"
        "    width:24px;    "
        "    margin: 0px 0px 0px 0px;"
        "}"
        "QScrollBar::handle:vertical {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
        "    stop: 0 rgb(32, 47, 130), stop: 0.5 rgb(32, 47, 130), stop:1 rgb(32, 47, 130));"
        "    min-height: 0px;"
        "}"
        "QScrollBar::add-line:vertical {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
        "    stop: 0 rgb(32, 47, 130), stop: 0.5 rgb(32, 47, 130),  stop:1 rgb(32, 47, 130));"
        "    height: 0px;"
        "    subcontrol-position: bottom;"
        "    subcontrol-origin: margin;"
        "}"
        "QScrollBar::sub-line:vertical {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
        "    stop: 0  rgb(32, 47, 130), stop: 0.5 rgb(32, 47, 130),  stop:1 rgb(32, 47, 130));"
        "    height: 0 px;"
        "    subcontrol-position: top;"
        "    subcontrol-origin: margin;"
        "}"
        ));
#endif
}
//! [1]

//! [2]
void FindStoriesWindow::browse()
{
    QString directory =
        QDir::toNativeSeparators(QFileDialog::getExistingDirectory(this, tr("Find Files"), QDir::currentPath()));

    if (!directory.isEmpty()) {
        if (directoryComboBox->findText(directory) == -1)
            directoryComboBox->addItem(directory);
        directoryComboBox->setCurrentIndex(directoryComboBox->findText(directory));
    }
}
//! [2]

static void updateComboBox(QComboBox *comboBox)
{
    if (comboBox->findText(comboBox->currentText()) == -1)
        comboBox->addItem(comboBox->currentText());
}

//! [13]



void FindStoriesWindow::updateFoundFiles(QStringList foundFiles) {
    qDebug() << "updateFoundFiles " << QThread::currentThread();

    QString text = textComboBox->currentText();

    // Full text content serach
    // ToDO: we don't need that for story running. Instead, do SHA-256 calculations and IFDB lookup
    if (!text.isEmpty()) {
        qDebug() << "find before findFiles text " << text;
        foundFiles = findFiles(foundFiles, text);
    }

    qDebug() << "find before showFiles";
    QCoreApplication::processEvents();
    showFiles(foundFiles);
    qDebug() << "find after showFiles";
}

void FindStoriesWindow::showStatusReportFoundFiles(QString onPath, int count)
{
    switch (count) {
    case 0:
        filesFoundLabel->setText(QString("PROGRESS: zero found so far, %2").arg(onPath));
        break;
    case 1:
        filesFoundLabel->setText(QString("PROGRESS: %1 file(s) found so far, %2").arg(QString::number(count)).arg(onPath));
        break;
    default:
        filesFoundLabel->setText(QString("PROGRESS: %1 files found so far, %2").arg(QString::number(count)).arg(onPath));
        break;
    }

    filesFoundLabel->setWordWrap(true);
}


//! [13]
//! [3]

void FindStoriesWindow::find()
{
    qDebug() << "FindFilesWindow find initiated";
    foundResultsOutput->setText("Search underway...\n");

    QString fileName = fileComboBox->currentText();
    QString path = QDir::cleanPath(directoryComboBox->currentText());
//! [3]

    updateComboBox(fileComboBox);
    updateComboBox(textComboBox);
    updateComboBox(directoryComboBox);

//! [4]

    currentDir = QDir(path);

    QCoreApplication::processEvents();

    // NOTE: .gblorb is default for the list
    QStringList findPatterns = QStringList(fileName.isEmpty() ? QStringLiteral("*.gblorb") : fileName);
    // -- already in default: findPatterns.append("*.gblorb");
    findPatterns.append("*.blorb");
    findPatterns.append("*.ulx");
    findPatterns.append("*.glb");

    qDebug() << "FindFilesWindow find thread initiated";
    thread.searchForFiles(findPatterns, path);
    // Will get a callback from the searching thread.
}
//! [4]

void FindStoriesWindow::animateFindClick()
{
    findButton->animateClick();
}

//! [5]
QStringList FindStoriesWindow::findFiles(const QStringList &files, const QString &text)
{
    QStringList foundFiles;
    // Full-text search code removed
    return foundFiles;
}
//! [7]

//! [8]
void FindStoriesWindow::showFiles(const QStringList &files)
{
    foundResultsOutput->clear();

    QString previousFilePath = "";
    int sameFolderCount = 0;

    for (int i = 0; i < files.size(); ++i) {
        const QString &singleFileFullpath = files.at(i);

        const QFileInfo fileInfo(singleFileFullpath);

        // Skip Text Fiction for Thunderword meta files
        if (fileInfo.fileName().startsWith("meta__")) {
            continue;
        }

        QString outItemHTML;
        if (fileInfo.path() != previousFilePath) {
            previousFilePath = fileInfo.path();
            if (i > 0) {
                foundResultsOutput->insertHtml("<br />");
            }
            outItemHTML = QString("Folder %1:  ").arg(fileInfo.path());
            foundResultsOutput->insertHtml(outItemHTML);
            sameFolderCount = 0;
        } else {
            sameFolderCount++;
            foundResultsOutput->insertHtml(", ");
        }
        outItemHTML = QString(" <a href=\"file://%1\">%2</a>").arg(singleFileFullpath).arg(fileInfo.fileName());
        foundResultsOutput->insertHtml(outItemHTML);

        if (i % 1000 == 0) {
            QCoreApplication::processEvents();
        }
    }
    if (files.size() == 1) {
        filesFoundLabel->setText(tr("%n file found (Click on file to open it)", 0, files.size()));
    } else {
        filesFoundLabel->setText(tr("%n files found (Click on file to open it)", 0, files.size()));
    }
    filesFoundLabel->setWordWrap(true);
}
//! [8]

//! [10]
QComboBox *FindStoriesWindow::createComboBox(const QString &text)
{
    QComboBox *comboBox = new QComboBox;
    comboBox->setEditable(true);
    comboBox->addItem(text);
    comboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    return comboBox;
}
//! [10]


void FindStoriesWindow::onClickedLink(QUrl url) {
    QString outPath = url.toLocalFile();
    qDebug() << "CLICKED url " << url << " outPath: " << outPath;
    emit dataFilePicked(outPath);
    // Hide my window, user picked story.
    this->hide();
}

//! [11]
void FindStoriesWindow::createFoundResultsOutput()
{
    foundResultsOutput = new QTextBrowser;
    foundResultsOutput->setOpenLinks(false);

    connect(foundResultsOutput, SIGNAL(anchorClicked(QUrl)), this, SLOT(onClickedLink(QUrl)));
}
//! [11]



//! [16]
void FindStoriesWindow::contextMenu(const QPoint &pos)
{
    //const QTableWidgetItem *item = filesTable->itemAt(pos);
    //if (!item)
        return;

    /*
    QMenu menu;
#ifndef QT_NO_CLIPBOARD
    QAction *copyAction = menu.addAction("Copy Name");
#endif
    QAction *openAction = menu.addAction("Open");
    QAction *action = menu.exec(filesTable->mapToGlobal(pos));
    if (!action)
        return;
    const QString fileName = fileNameOfItem(item);
    if (action == openAction)
        openFile(fileName);
#ifndef QT_NO_CLIPBOARD
    else if (action == copyAction)
        QGuiApplication::clipboard()->setText(QDir::toNativeSeparators(fileName));
#endif
    */
}
//! [16]
