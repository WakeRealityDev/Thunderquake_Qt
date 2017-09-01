/****************************************************************************
**
** Portions Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file started from demonstration applications of the Qt Toolkit
**   under terms of the BSD License Usage. The original code is
**   git checkin b7bcf9640c58d936e6bb770ead47a18808090d71 and earlier.
**   All add, change, remove to code after git commit
**   b7bcf9640c58d936e6bb770ead47a18808090d71 are copyrighted under
**   Stephen A. Gutknecht's terms.
**
** BSD License Usage
** Commit b7bcf9640c58d936e6bb770ead47a18808090d71 and earlier used under
** the terms of the BSD license as follows:
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
**
****************************************************************************/

/* Thunderquake Qt
 *   License for git commits after b7bcf9640c58d936e6bb770ead47a18808090d71
 *   Copyright (C) 2017 Stephen A Gutknecht. All rights reserved.
 *
 * This class code started off using the BSD-licensed Qt 5.9.1 Example projects:
 *     textedit.pro
 *     customcompleter.pro
 *
 * QProcess on Android
 *    http://www.qtcentre.org/threads/56859-Starting-Qt-console-application-on-Android-using-QProcess
 * */


#ifndef REMGLKTEXTEDITWINDOW_H
#define REMGLKTEXTEDITWINDOW_H

#include <QMainWindow>
#include <QMap>
#include <QPointer>

QT_BEGIN_NAMESPACE
class QAction;
class QComboBox;
class QFontComboBox;
class QTextEdit;
class QTextCharFormat;
class QMenu;
class QPrinter;
class QGridLayout;
QT_END_NAMESPACE

#include "completerwindow.h"
#include "datamiddlespot.h"

QT_BEGIN_NAMESPACE
class RemGlkProcess;
QT_END_NAMESPACE

class RemGlkTextEditWindow : public CompleterWindow
{
    Q_OBJECT

public:
    RemGlkTextEditWindow(QWidget *parent = 0);

    bool load(const QString &f);

public slots:
    void fileNew();

protected:
    void closeEvent(QCloseEvent *e) override;

private:
    //RemGlkOutputLayout outputLayout;

private slots:
    void fileOpen();
    bool fileSave();
    bool fileSaveAs();
    void filePrint();
    void filePrintPreview();
    void filePrintPdf();

    void textBold();
    void textUnderline();
    void textItalic();
    void textFamily(const QString &f);
    void textSize(const QString &p);
    void textStyle(int styleIndex);
    void textColor();
    void textAlign(QAction *a);

    void currentCharFormatChanged(const QTextCharFormat &format);
    void cursorPositionChanged();

    void clipboardDataChanged();
    void about();
    void printPreview(QPrinter *);

    void addExtraWindowElements();

private:
    void setupFileActions();
    void setupEditActions();
    void setupTextActions();
    void setupFormatActions();
    void setupInteractiveFictionActions();
    bool maybeSave();
    void setCurrentFileName(const QString &fileName);

    void mergeFormatOnWordOrSelection(const QTextCharFormat &format);
    void fontChanged(const QFont &f);
    void colorChanged(const QColor &c);
    void alignmentChanged(Qt::Alignment a);

    QAction *actionSave;
    QAction *actionTextBold;
    QAction *actionTextUnderline;
    QAction *actionTextItalic;
    QAction *actionTextColor;
    QAction *actionAlignLeft;
    QAction *actionAlignCenter;
    QAction *actionAlignRight;
    QAction *actionAlignJustify;
    QAction *actionUndo;
    QAction *actionRedo;
#ifndef QT_NO_CLIPBOARD
    QAction *actionCut;
    QAction *actionCopy;
    QAction *actionPaste;
#endif

    QComboBox *comboStyle;
    QFontComboBox *comboFont;
    QComboBox *comboSize;

    QToolBar *tb;
    QString fileName;

public:
#ifdef Q_OS_MAC
    const QString rsrcPath = ":/images/mac";
#else
    const QString rsrcPath = ":/images/win";
#endif


private:
    QToolBar *appToolBarFile;
    QToolBar *appToolBarEdit;
    QToolBar *appToolBarText;
    QToolBar *appToolBarFormat;

    /*
     * Thunderquake changes
     */
public:
    DataMiddleSpot dataSpot;

    int windowOpenAutoLaunchMode = 0;
    QGridLayout *mainLayout;
    // Classic Glk window for status, as in Z-machine top status window.
    QLabel *glkWindowTopStatus;

public:
    QAction *actionStoryRespond;
    QComboBox *comboStoryResponse;
    // 0 = classic browser dialog with double-click. 1 = HTML
    int findFilesMethod = 1;

public slots:
    void storyRespondCall();
    void storyResponseFromTextEdit(QString dataPayload, int inputMode);
    void loadDataFilePicked(QString dataFilePath);
    void proxyRemGlkForward(const QString payload, int payloadType);
};

#endif // REMGLKTEXTEDITWINDOW_H
