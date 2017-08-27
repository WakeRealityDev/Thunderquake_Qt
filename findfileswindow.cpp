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

#include "findfileswindow.h"
#include "findfilesthread.h"

//! [17]
enum { absoluteFileNameRole = Qt::UserRole + 1 };
//! [17]

//! [18]
static inline QString fileNameOfItem(const QTableWidgetItem *item)
{
    return item->data(absoluteFileNameRole).toString();
}
//! [18]

//! [14]
static inline void openFile(const QString &fileName)
{
    // QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));
    qDebug() << "openFile " << fileName;
}
//! [14]

//! [0]
FindFilesWindow::FindFilesWindow(QWidget *parent)
    : QWidget(parent)
{
    QPushButton *browseButton = new QPushButton(tr("&Browse..."), this);
    connect(browseButton, &QAbstractButton::clicked, this, &FindFilesWindow::browse);
    findButton = new QPushButton(tr("&Find"), this);
    connect(findButton, &QAbstractButton::clicked, this, &FindFilesWindow::find);

    fileComboBox = createComboBox(tr("*.gblorb"));
    connect(fileComboBox->lineEdit(), &QLineEdit::returnPressed,
            this, &FindFilesWindow::animateFindClick);
    textComboBox = createComboBox();
    connect(textComboBox->lineEdit(), &QLineEdit::returnPressed,
            this, &FindFilesWindow::animateFindClick);
    bool searchCurrentPathDefualt = false;
    if (searchCurrentPathDefualt) {
        directoryComboBox = createComboBox(QDir::toNativeSeparators(QDir::currentPath()));
    } else {
        directoryComboBox = createComboBox(QDir::toNativeSeparators(defaultHardcodedPath));
    }
    connect(directoryComboBox->lineEdit(), &QLineEdit::returnPressed,
            this, &FindFilesWindow::animateFindClick);

    filesFoundLabel = new QLabel;

    createFilesTable();
//! [0]

//! [1]
    QGridLayout *mainLayout = new QGridLayout(this);
//#ifdef SEARCH_FULL_FEATURE_A
    mainLayout->addWidget(new QLabel(tr("Named:")), 0, 0);
    mainLayout->addWidget(fileComboBox, 0, 1, 1, 2);
    mainLayout->addWidget(new QLabel(tr("Containing text:")), 1, 0);
    mainLayout->addWidget(textComboBox, 1, 1, 1, 2);
//#endif
    mainLayout->addWidget(new QLabel(tr("In directory:")), 2, 0);
    mainLayout->addWidget(directoryComboBox, 2, 1);
    mainLayout->addWidget(browseButton, 2, 2);
    mainLayout->addWidget(filesTable, 3, 0, 1, 3);
    mainLayout->addWidget(filesFoundLabel, 4, 0, 1, 2);
    mainLayout->addWidget(findButton, 4, 2);

    setWindowTitle(tr("Find Interactive Fiction Story Files"));
    const QRect screenGeometry = QApplication::desktop()->screenGeometry(this);
    resize(screenGeometry.width() / 2, screenGeometry.height() / 3);

    connect(&thread, SIGNAL(foundFiles(QStringList)), this, SLOT(updateFoundFiles(QStringList)));
    connect(&thread, SIGNAL(statusReportFoundFiles(QString, int)), this, SLOT(showStatusReportFoundFiles(QString, int)));
}
//! [1]

//! [2]
void FindFilesWindow::browse()
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



void FindFilesWindow::updateFoundFiles(QStringList foundFiles) {
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

void FindFilesWindow::showStatusReportFoundFiles(QString onPath, int count)
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

void FindFilesWindow::find()
{
    qDebug() << "FindFilesWindow find initiated";
    filesTable->setRowCount(0);

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

void FindFilesWindow::animateFindClick()
{
    findButton->animateClick();
}

//! [5]
QStringList FindFilesWindow::findFiles(const QStringList &files, const QString &text)
{
    QProgressDialog progressDialog(this);
    progressDialog.setCancelButtonText(tr("&Cancel"));
    progressDialog.setRange(0, files.size());
    progressDialog.setWindowTitle(tr("Find Files"));
    QCoreApplication::processEvents();

//! [5] //! [6]
    QMimeDatabase mimeDatabase;
    QStringList foundFiles;

    for (int i = 0; i < files.size(); ++i) {
        progressDialog.setValue(i);
        progressDialog.setLabelText(tr("Searching file number %1 of %n...", 0, files.size()).arg(i));
        QCoreApplication::processEvents();
//! [6]

        if (progressDialog.wasCanceled())
            break;

//! [7]
        const QString fileName = files.at(i);
        const QMimeType mimeType = mimeDatabase.mimeTypeForFile(fileName);
        if (mimeType.isValid() && !mimeType.inherits(QStringLiteral("text/plain"))) {
            qWarning() << "Not searching binary file " << QDir::toNativeSeparators(fileName);
            continue;
        }
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly)) {
            QString line;
            QTextStream in(&file);
            while (!in.atEnd()) {
                if (progressDialog.wasCanceled())
                    break;
                line = in.readLine();
                if (line.contains(text, Qt::CaseInsensitive)) {
                    foundFiles << files[i];
                    break;
                }
            }
        }
    }
    return foundFiles;
}
//! [7]

//! [8]
void FindFilesWindow::showFiles(const QStringList &files)
{
    for (int i = 0; i < files.size(); ++i) {
        const QString &fileName = files.at(i);
        const QString toolTip = QDir::toNativeSeparators(fileName);
        const QString relativePath = QDir::toNativeSeparators(currentDir.relativeFilePath(fileName));
        const qint64 size = QFileInfo(fileName).size();
        QTableWidgetItem *fileNameItem = new QTableWidgetItem(relativePath);
        fileNameItem->setData(absoluteFileNameRole, QVariant(fileName));
        fileNameItem->setToolTip(toolTip);
        fileNameItem->setFlags(fileNameItem->flags() ^ Qt::ItemIsEditable);
        QTableWidgetItem *sizeItem = new QTableWidgetItem(tr("%1 KB")
                                             .arg(int((size + 1023) / 1024)));
        sizeItem->setData(absoluteFileNameRole, QVariant(fileName));
        sizeItem->setToolTip(toolTip);
        sizeItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        sizeItem->setFlags(sizeItem->flags() ^ Qt::ItemIsEditable);

        const int row = filesTable->rowCount();
        filesTable->insertRow(row);
        filesTable->setItem(row, 0, fileNameItem);
        filesTable->setItem(row, 1, sizeItem);

        if (i % 1000 == 0) {
            QCoreApplication::processEvents();
        }
    }
    filesFoundLabel->setText(tr("%n file(s) found (Double click on a file to open it)", 0, files.size()));
    filesFoundLabel->setWordWrap(true);
}
//! [8]

//! [10]
QComboBox *FindFilesWindow::createComboBox(const QString &text)
{
    QComboBox *comboBox = new QComboBox;
    comboBox->setEditable(true);
    comboBox->addItem(text);
    comboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    return comboBox;
}
//! [10]

//! [11]
void FindFilesWindow::createFilesTable()
{
    filesTable = new QTableWidget(0, 2);
    filesTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    QStringList labels;
    labels << tr("Filename") << tr("Size");
    filesTable->setHorizontalHeaderLabels(labels);
    filesTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    filesTable->verticalHeader()->hide();
    filesTable->setShowGrid(false);
//! [15]
    filesTable->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(filesTable, &QTableWidget::customContextMenuRequested,
            this, &FindFilesWindow::contextMenu);
    connect(filesTable, &QTableWidget::cellActivated,
            this, &FindFilesWindow::openFileOfItem);
//! [15]
}
//! [11]


//! [12]

void FindFilesWindow::openFileOfItem(int row, int /* column */)
{
    const QTableWidgetItem *item = filesTable->item(row, 0);
    qDebug() << "openFileOfItem " << row << fileNameOfItem(item);
// ToDo: should be possible to make a sizeOfItem(item) function - and replace size column with a SHA-256 sum value.
// FixMe: the stand-alone example code doesn't seem to fail, but an extra slash was seen, so why is this happening?
    // openFile(fileNameOfItem(item));
    emit dataFilePicked(fileNameOfItem(item));
}

//! [12]

//! [16]
void FindFilesWindow::contextMenu(const QPoint &pos)
{
    const QTableWidgetItem *item = filesTable->itemAt(pos);
    if (!item)
        return;
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
}
//! [16]
