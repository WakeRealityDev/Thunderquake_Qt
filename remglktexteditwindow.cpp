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
 * Why based on Qt Rich Text / HTML Text edtior?
 * One advantage of this approach is that a transcript can be saved as a document directly.
 *
 * QProcess on Android
 *    http://www.qtcentre.org/threads/56859-Starting-Qt-console-application-on-Android-using-QProcess
 * */


#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QColorDialog>
#include <QComboBox>
#include <QFontComboBox>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontDatabase>
#include <QMenu>
#include <QMenuBar>
#include <QTextCodec>
#include <QTextEdit>
#include <QStatusBar>
#include <QToolBar>
#include <QTextCursor>
#include <QTextDocumentWriter>
#include <QTextList>
#include <QtDebug>
#include <QCloseEvent>
#include <QMessageBox>
#include <QMimeData>
#if defined(QT_PRINTSUPPORT_LIB)
#include <QtPrintSupport/qtprintsupportglobal.h>
#if QT_CONFIG(printer)
#if QT_CONFIG(printdialog)
#include <QPrintDialog>
#endif
#include <QPrinter>
#if QT_CONFIG(printpreviewdialog)
#include <QPrintPreviewDialog>
#endif
#endif
#endif

#include <QTimer>
#include <QProcess>
#include <QLineEdit>
#include <QLabel>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLayout>
#include <QGridLayout>

#include "remglktexteditwindow.h"
#include "completerwindow.h"
#include "remglkcompletertextedit.h"


RemGlkTextEditWindow::RemGlkTextEditWindow(QWidget *parent)
    : CompleterWindow(parent)
{
    qDebug() << "---! RemGlkTextEditWindow constructor";

#ifdef Q_OS_OSX
    setUnifiedTitleAndToolBarOnMac(true);
#endif
    setWindowTitle(QCoreApplication::applicationName());

    connect(textEdit, &QTextEdit::currentCharFormatChanged,
            this, &RemGlkTextEditWindow::currentCharFormatChanged);
    connect(textEdit, &QTextEdit::cursorPositionChanged,
            this, &RemGlkTextEditWindow::cursorPositionChanged);
    setCentralWidget(textEdit);

    setToolButtonStyle(Qt::ToolButtonFollowStyle);
    setupFileActions();
    setupEditActions();
    setupTextActions();
    setupFormatActions();
    setupInteractiveFictionActions();

    {
        QMenu *helpMenu = menuBar()->addMenu(tr("Help"));
        helpMenu->addAction(tr("About"), this, &RemGlkTextEditWindow::about);
        helpMenu->addAction(tr("About &Qt"), qApp, &QApplication::aboutQt);
    }

    QFont textFont("Helvetica");
    textFont.setStyleHint(QFont::SansSerif);
    textEdit->setFont(textFont);
    fontChanged(textEdit->font());
    colorChanged(textEdit->textColor());
    alignmentChanged(textEdit->alignment());

    connect(textEdit->document(), &QTextDocument::modificationChanged,
            actionSave, &QAction::setEnabled);
    connect(textEdit->document(), &QTextDocument::modificationChanged,
            this, &QWidget::setWindowModified);
    connect(textEdit->document(), &QTextDocument::undoAvailable,
            actionUndo, &QAction::setEnabled);
    connect(textEdit->document(), &QTextDocument::redoAvailable,
            actionRedo, &QAction::setEnabled);

    setWindowModified(textEdit->document()->isModified());
    actionSave->setEnabled(textEdit->document()->isModified());
    actionUndo->setEnabled(textEdit->document()->isUndoAvailable());
    actionRedo->setEnabled(textEdit->document()->isRedoAvailable());

#ifndef QT_NO_CLIPBOARD
    actionCut->setEnabled(false);
    actionCopy->setEnabled(false);

    connect(QApplication::clipboard(), &QClipboard::dataChanged, this, &RemGlkTextEditWindow::clipboardDataChanged);
#endif

    textEdit->setFocus();
    setCurrentFileName(QString());

    dataSpot.engineProcess.setOutputTextEdit(textEdit);

    // reference:
    //     connect(comboStoryResponse->lineEdit(), SIGNAL(returnPressed()), this, SLOT(storyRespondCall()));
    // Tutorial:  https://woboq.com/blog/how-qt-signals-slots-work.html
    connect(textEdit, SIGNAL(characterToEngine(QString, int)), this, SLOT(storyResponseFromTextEdit(QString, int)));

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


    // Activate thread to start file search.
    switch (findFilesMethod) {
    case 0:
        // findFilesWindow.show();
        // find function is designed to populare without showing window, pre-loading content.
        dataSpot.findFilesWindow.find();
        connect(&dataSpot.findFilesWindow, SIGNAL(dataFilePicked(QString)), this, SLOT(loadDataFilePicked(QString)));
        break;
    case 1:
        dataSpot.findStoriesWindow.find();
        connect(&dataSpot.findStoriesWindow, SIGNAL(dataFilePicked(QString)), this, SLOT(loadDataFilePicked(QString)));
        break;
    }

    if (windowOpenAutoLaunchMode == 1) {
        QTimer::singleShot(500, [=] {
                dataSpot.engineProcess.setDataFilePath("/spot/CameraCornetGit0/textAdvent/storyGames/storyGames_TechTest0/Six.gblorb");
                dataSpot.engineProcess.setRemGlkEngine(1);
                dataSpot.engineProcess.launchRemGlkEngine();
            });
    }

    /*
     * Window maximized issues
     *  https://stackoverflow.com/questions/44005852/qdockwidgetrestoregeometry-not-working-correctly-when-qmainwindow-is-maximized
     * */
    if (isMaximized()) {
        qDebug() << "Window is MAXIMIZED";
        // restoreGeometry();
    }

    addExtraWindowElements();

#ifdef HIDE_TEXT_EDIT_TOOLBARS
    appToolBarEdit->hide();
    appToolBarFile->hide();
    appToolBarText->hide();
    appToolBarFormat->hide();
#endif

    // connect(&dataSpot.webViewWindow, SIGNAL(toRemGlkEngineForward(const QString, int)), &dataSpot.engineProcess, SLOT(sendCommand(const QString)));
    connect(&dataSpot.webViewWindow, SIGNAL(toRemGlkEngineForward(const QString, int)), this, SLOT(proxyRemGlkForward(const QString, int)));

    connect(&dataSpot.engineProcess, SIGNAL(incomingRemGlkStanzaReady(QString)), &dataSpot.webViewWindow, SLOT(incomingRemGlkStanza(QString)));
}

int onForward = 0;

void RemGlkTextEditWindow::proxyRemGlkForward(const QString payload, int payloadType) {
    onForward++;
    qDebug() << "[DataToEngine][tempFoward] Payload " << payload << " type " << payloadType << " onForward " << onForward;
    switch (payloadType) {
    case 0:   // Regular RemGlk generations, story turns
        dataSpot.engineProcess.sendCommand(payload);
        break;
    case 1:   // Init string and other special
        dataSpot.engineProcess.setRemGlkInitMessage(payload);
        break;
    }
}

void RemGlkTextEditWindow::addExtraWindowElements()
{
    glkWindowTopStatus = new QLabel;
    glkWindowTopStatus->setText("Glk TextGrid status window here");

    // QPushButton *button1 = new QPushButton("button1");
    // QPushButton *button2 = new QPushButton("button2");

    int layoutPick = 2;
    if (layoutPick == 1) {
        QHBoxLayout *layout = new QHBoxLayout;

        // layout->addWidget(button1);
        // layout->addWidget(button2);
        layout->addWidget(textEdit);

        setCentralWidget(new QWidget);
        centralWidget()->setLayout(layout);
    }

    if (layoutPick == 2) {
        mainLayout = new QGridLayout;

        //Row and Column counts are set Automatically
        // mainLayout->addWidget(button1, 0, 0);
        // mainLayout->addWidget(button2, 1, 0);
        mainLayout->addWidget(glkWindowTopStatus, 2, 0);
        mainLayout->addWidget(textEdit, 3, 0);

        QWidget *centralWidget = new QWidget(this);
        centralWidget->setLayout(mainLayout);
        setCentralWidget(centralWidget);

        // mainLayout->setSizeConstraint(QLayout::SetMinimumSize);
        mainLayout->setSizeConstraint(QLayout::SetNoConstraint);

        dataSpot.engineProcess.outputLayout.setOutputWidgets(textEdit, mainLayout, glkWindowTopStatus);
    }
}

void RemGlkTextEditWindow::loadDataFilePicked(QString dataFilePath)
{
    bool doLaunch = false;
    if (dataSpot.engineProcess.storyEngineState > 0) {
        if (maybeSave()) {
            qDebug() << "!!!!!!!!!!!!!!! Confirmation dialog on close";
            dataSpot.engineProcess.stopRunningEngine();
            doLaunch = true;
        }
    }
    else {
        doLaunch = true;
    }

    if (doLaunch) {
        dataSpot.engineProcess.setDataFilePath(dataFilePath);
        dataSpot.engineProcess.setRemGlkEngine(1);
        dataSpot.engineProcess.launchRemGlkEngine();
    }
}

void RemGlkTextEditWindow::storyResponseFromTextEdit(QString dataPayload, int inputMode)
{
    qDebug() << "[TextEdit] highest level just got Input? '" << dataPayload << "' inputMode " << inputMode;
    switch (inputMode) {
    case 1:   // line only
    case 3:   // line + char. ToDo: currently defaults to LINE, ignoring char mode
    case 5:   // line + HyperLink
        dataSpot.engineProcess.storyRespond(dataPayload);
        break;
    case 2:   // char only
        dataSpot.engineProcess.storyRespondChar(dataPayload);
        break;
    }
}

void RemGlkTextEditWindow::closeEvent(QCloseEvent *e)
{
    if (maybeSave())
        e->accept();
    else
        e->ignore();
}

void RemGlkTextEditWindow::setupFileActions()
{
    appToolBarFile = addToolBar(tr("File Actions"));
    QMenu *menu = menuBar()->addMenu(tr("&File"));

    const QIcon newIcon = QIcon::fromTheme("document-new", QIcon(rsrcPath + "/filenew.png"));
    QAction *a = menu->addAction(newIcon,  tr("&New"), this, &RemGlkTextEditWindow::fileNew);
    appToolBarFile->addAction(a);
    a->setPriority(QAction::LowPriority);
    a->setShortcut(QKeySequence::New);

    const QIcon openIcon = QIcon::fromTheme("document-open", QIcon(rsrcPath + "/fileopen.png"));
    a = menu->addAction(openIcon, tr("&Open..."), this, &RemGlkTextEditWindow::fileOpen);
    a->setShortcut(QKeySequence::Open);
    appToolBarFile->addAction(a);

    menu->addSeparator();

    const QIcon saveIcon = QIcon::fromTheme("document-save", QIcon(rsrcPath + "/filesave.png"));
    actionSave = menu->addAction(saveIcon, tr("&Save"), this, &RemGlkTextEditWindow::fileSave);
    actionSave->setShortcut(QKeySequence::Save);
    actionSave->setEnabled(false);
    appToolBarFile->addAction(actionSave);

    a = menu->addAction(tr("Save &As..."), this, &RemGlkTextEditWindow::fileSaveAs);
    a->setPriority(QAction::LowPriority);
    menu->addSeparator();

#ifndef QT_NO_PRINTER
    const QIcon printIcon = QIcon::fromTheme("document-print", QIcon(rsrcPath + "/fileprint.png"));
    a = menu->addAction(printIcon, tr("&Print..."), this, &RemGlkTextEditWindow::filePrint);
    a->setPriority(QAction::LowPriority);
    a->setShortcut(QKeySequence::Print);
    appToolBarFile->addAction(a);

    const QIcon filePrintIcon = QIcon::fromTheme("fileprint", QIcon(rsrcPath + "/fileprint.png"));
    menu->addAction(filePrintIcon, tr("Print Preview..."), this, &RemGlkTextEditWindow::filePrintPreview);

    const QIcon exportPdfIcon = QIcon::fromTheme("exportpdf", QIcon(rsrcPath + "/exportpdf.png"));
    a = menu->addAction(exportPdfIcon, tr("&Export PDF..."), this, &RemGlkTextEditWindow::filePrintPdf);
    a->setPriority(QAction::LowPriority);
    a->setShortcut(Qt::CTRL + Qt::Key_D);
    appToolBarFile->addAction(a);

    menu->addSeparator();
#endif

    a = menu->addAction(tr("&Quit"), this, &QWidget::close);
    a->setShortcut(Qt::CTRL + Qt::Key_Q);
}

void RemGlkTextEditWindow::setupEditActions()
{
    appToolBarEdit = addToolBar(tr("Edit Actions"));
    QMenu *menu = menuBar()->addMenu(tr("&Edit"));

    const QIcon undoIcon = QIcon::fromTheme("edit-undo", QIcon(rsrcPath + "/editundo.png"));
    actionUndo = menu->addAction(undoIcon, tr("&Undo"), textEdit, &QTextEdit::undo);
    actionUndo->setShortcut(QKeySequence::Undo);
    appToolBarEdit->addAction(actionUndo);

    const QIcon redoIcon = QIcon::fromTheme("edit-redo", QIcon(rsrcPath + "/editredo.png"));
    actionRedo = menu->addAction(redoIcon, tr("&Redo"), textEdit, &QTextEdit::redo);
    actionRedo->setPriority(QAction::LowPriority);
    actionRedo->setShortcut(QKeySequence::Redo);
    appToolBarEdit->addAction(actionRedo);
    menu->addSeparator();

#ifndef QT_NO_CLIPBOARD
    const QIcon cutIcon = QIcon::fromTheme("edit-cut", QIcon(rsrcPath + "/editcut.png"));
    actionCut = menu->addAction(cutIcon, tr("Cu&t"), textEdit, &QTextEdit::cut);
    actionCut->setPriority(QAction::LowPriority);
    actionCut->setShortcut(QKeySequence::Cut);
    appToolBarEdit->addAction(actionCut);

    const QIcon copyIcon = QIcon::fromTheme("edit-copy", QIcon(rsrcPath + "/editcopy.png"));
    actionCopy = menu->addAction(copyIcon, tr("&Copy"), textEdit, &QTextEdit::copy);
    actionCopy->setPriority(QAction::LowPriority);
    actionCopy->setShortcut(QKeySequence::Copy);
    appToolBarEdit->addAction(actionCopy);

    const QIcon pasteIcon = QIcon::fromTheme("edit-paste", QIcon(rsrcPath + "/editpaste.png"));
    actionPaste = menu->addAction(pasteIcon, tr("&Paste"), textEdit, &QTextEdit::paste);
    actionPaste->setPriority(QAction::LowPriority);
    actionPaste->setShortcut(QKeySequence::Paste);
    appToolBarEdit->addAction(actionPaste);
    if (const QMimeData *md = QApplication::clipboard()->mimeData())
        actionPaste->setEnabled(md->hasText());
#endif
}

void RemGlkTextEditWindow::setupTextActions()
{
    appToolBarText = addToolBar(tr("Format Actions"));
    QMenu *menu = menuBar()->addMenu(tr("F&ormat"));

    const QIcon boldIcon = QIcon::fromTheme("format-text-bold", QIcon(rsrcPath + "/textbold.png"));
    actionTextBold = menu->addAction(boldIcon, tr("&Bold"), this, &RemGlkTextEditWindow::textBold);
    actionTextBold->setShortcut(Qt::CTRL + Qt::Key_B);
    actionTextBold->setPriority(QAction::LowPriority);
    QFont bold;
    bold.setBold(true);
    actionTextBold->setFont(bold);
    appToolBarText->addAction(actionTextBold);
    actionTextBold->setCheckable(true);

    const QIcon italicIcon = QIcon::fromTheme("format-text-italic", QIcon(rsrcPath + "/textitalic.png"));
    actionTextItalic = menu->addAction(italicIcon, tr("&Italic"), this, &RemGlkTextEditWindow::textItalic);
    actionTextItalic->setPriority(QAction::LowPriority);
    actionTextItalic->setShortcut(Qt::CTRL + Qt::Key_I);
    QFont italic;
    italic.setItalic(true);
    actionTextItalic->setFont(italic);
    appToolBarText->addAction(actionTextItalic);
    actionTextItalic->setCheckable(true);

    const QIcon underlineIcon = QIcon::fromTheme("format-text-underline", QIcon(rsrcPath + "/textunder.png"));
    actionTextUnderline = menu->addAction(underlineIcon, tr("&Underline"), this, &RemGlkTextEditWindow::textUnderline);
    actionTextUnderline->setShortcut(Qt::CTRL + Qt::Key_U);
    actionTextUnderline->setPriority(QAction::LowPriority);
    QFont underline;
    underline.setUnderline(true);
    actionTextUnderline->setFont(underline);
    appToolBarText->addAction(actionTextUnderline);
    actionTextUnderline->setCheckable(true);

    menu->addSeparator();

    const QIcon leftIcon = QIcon::fromTheme("format-justify-left", QIcon(rsrcPath + "/textleft.png"));
    actionAlignLeft = new QAction(leftIcon, tr("&Left"), this);
    actionAlignLeft->setShortcut(Qt::CTRL + Qt::Key_L);
    actionAlignLeft->setCheckable(true);
    actionAlignLeft->setPriority(QAction::LowPriority);
    const QIcon centerIcon = QIcon::fromTheme("format-justify-center", QIcon(rsrcPath + "/textcenter.png"));
    actionAlignCenter = new QAction(centerIcon, tr("C&enter"), this);
    actionAlignCenter->setShortcut(Qt::CTRL + Qt::Key_E);
    actionAlignCenter->setCheckable(true);
    actionAlignCenter->setPriority(QAction::LowPriority);
    const QIcon rightIcon = QIcon::fromTheme("format-justify-right", QIcon(rsrcPath + "/textright.png"));
    actionAlignRight = new QAction(rightIcon, tr("&Right"), this);
    actionAlignRight->setShortcut(Qt::CTRL + Qt::Key_R);
    actionAlignRight->setCheckable(true);
    actionAlignRight->setPriority(QAction::LowPriority);
    const QIcon fillIcon = QIcon::fromTheme("format-justify-fill", QIcon(rsrcPath + "/textjustify.png"));
    actionAlignJustify = new QAction(fillIcon, tr("&Justify"), this);
    actionAlignJustify->setShortcut(Qt::CTRL + Qt::Key_J);
    actionAlignJustify->setCheckable(true);
    actionAlignJustify->setPriority(QAction::LowPriority);

    // Make sure the alignLeft  is always left of the alignRight
    QActionGroup *alignGroup = new QActionGroup(this);
    connect(alignGroup, &QActionGroup::triggered, this, &RemGlkTextEditWindow::textAlign);

    if (QApplication::isLeftToRight()) {
        alignGroup->addAction(actionAlignLeft);
        alignGroup->addAction(actionAlignCenter);
        alignGroup->addAction(actionAlignRight);
    } else {
        alignGroup->addAction(actionAlignRight);
        alignGroup->addAction(actionAlignCenter);
        alignGroup->addAction(actionAlignLeft);
    }
    alignGroup->addAction(actionAlignJustify);

    appToolBarText->addActions(alignGroup->actions());
    menu->addActions(alignGroup->actions());

    menu->addSeparator();

    QPixmap pix(16, 16);
    pix.fill(Qt::black);
    actionTextColor = menu->addAction(pix, tr("&Color..."), this, &RemGlkTextEditWindow::textColor);
    appToolBarText->addAction(actionTextColor);
}

void RemGlkTextEditWindow::setupFormatActions()
{
    appToolBarFormat = addToolBar(tr("Format Actions"));
    appToolBarText->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
    addToolBarBreak(Qt::TopToolBarArea);
    addToolBar(appToolBarFormat);

    comboStyle = new QComboBox(appToolBarText);
    appToolBarFormat->addWidget(comboStyle);
    comboStyle->addItem("Standard");
    comboStyle->addItem("Bullet List (Disc)");
    comboStyle->addItem("Bullet List (Circle)");
    comboStyle->addItem("Bullet List (Square)");
    comboStyle->addItem("Ordered List (Decimal)");
    comboStyle->addItem("Ordered List (Alpha lower)");
    comboStyle->addItem("Ordered List (Alpha upper)");
    comboStyle->addItem("Ordered List (Roman lower)");
    comboStyle->addItem("Ordered List (Roman upper)");

    connect(comboStyle, QOverload<int>::of(&QComboBox::activated), this, &RemGlkTextEditWindow::textStyle);

    comboFont = new QFontComboBox(appToolBarText);
    appToolBarText->addWidget(comboFont);
    connect(comboFont, QOverload<const QString &>::of(&QComboBox::activated), this, &RemGlkTextEditWindow::textFamily);

    comboSize = new QComboBox(appToolBarText);
    comboSize->setObjectName("comboSize");
    appToolBarFormat->addWidget(comboSize);
    comboSize->setEditable(true);

    const QList<int> standardSizes = QFontDatabase::standardSizes();
    foreach (int size, standardSizes)
        comboSize->addItem(QString::number(size));
    comboSize->setCurrentIndex(standardSizes.indexOf(QApplication::font().pointSize()));

    connect(comboSize, QOverload<const QString &>::of(&QComboBox::activated), this, &RemGlkTextEditWindow::textSize);
}

void RemGlkTextEditWindow::setupInteractiveFictionActions()
{
    /*
     * Thunderquake
     * */

    QToolBar *tb = addToolBar(tr("Story Actions"));
    QMenu *menu = menuBar()->addMenu(tr("&Story"));

    tb->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
    addToolBarBreak(Qt::TopToolBarArea);
    addToolBar(tb);

    // menu->addSeparator();

    comboStoryResponse = new QComboBox(tb);
    comboStoryResponse->setEditable(true);
    comboStoryResponse->setInsertPolicy(QComboBox::InsertAtTop);
    comboStoryResponse->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    connect(comboStoryResponse->lineEdit(), SIGNAL(returnPressed()), this, SLOT(storyRespondCall()));
    // ToDo: HACKY workaround DPI-specific for layout problems on Android
#ifdef Q_OS_ANDROID
    comboStoryResponse->setStyleSheet("QComboBox:editable{background: #DCEBF0; font: 14pt \"Roboto\"; color: black; }");
    comboStoryResponse->lineEdit()->setStyleSheet("QComboBox:editable{background: #DCEBF0; font: 12pt \"Roboto\"; color: black; }");
#endif
    tb->addWidget(comboStoryResponse);
    comboStoryResponse->addItem("Go West");
    comboStoryResponse->addItem("Go East");
    comboStoryResponse->addItem("Go North");
    comboStoryResponse->addItem("Go South");
    comboStoryResponse->addItem("Go Down");
    comboStoryResponse->addItem("Go Up");
    comboStoryResponse->addItem("restart");
    comboStoryResponse->addItem("yes");
    comboStoryResponse->addItem("no");

    const QIcon storyRespondIcon = QIcon::fromTheme("edit-undo", QIcon(rsrcPath + "/editundo.png"));
    actionStoryRespond = menu->addAction(storyRespondIcon, tr("&Respond"), this, &RemGlkTextEditWindow::storyRespondCall);
    actionStoryRespond->setShortcut(Qt::CTRL + Qt::Key_T);
    tb->addAction(actionStoryRespond);
}

void RemGlkTextEditWindow::storyRespondCall()
{
    dataSpot.engineProcess.storyRespond(comboStoryResponse->currentText());
}


bool RemGlkTextEditWindow::load(const QString &f)
{
    if (!QFile::exists(f))
        return false;
    QFile file(f);
    if (!file.open(QFile::ReadOnly))
        return false;

    QByteArray data = file.readAll();
    QTextCodec *codec = Qt::codecForHtml(data);
    QString str = codec->toUnicode(data);
    if (Qt::mightBeRichText(str)) {
        textEdit->setHtml(str);
    } else {
        str = QString::fromLocal8Bit(data);
        textEdit->setPlainText(str);
    }

    setCurrentFileName(f);
    return true;
}

bool RemGlkTextEditWindow::maybeSave()
{
    if (!textEdit->document()->isModified())
        return true;

    const QMessageBox::StandardButton ret =
        QMessageBox::warning(this, QCoreApplication::applicationName(),
            tr("Interactive Fiction story already in play."
               " Load this new story?"),
            QMessageBox::Discard | QMessageBox::Cancel);

    if (ret == QMessageBox::Save)
        return fileSave();
    else if (ret == QMessageBox::Cancel)
        return false;

    return true;
}

void RemGlkTextEditWindow::setCurrentFileName(const QString &fileName)
{
    this->fileName = fileName;
    textEdit->document()->setModified(false);

    QString shownName;
    if (fileName.isEmpty())
        shownName = "untitled.txt";
    else
        shownName = QFileInfo(fileName).fileName();

    setWindowTitle(tr("%1[*] - %2").arg(shownName, QCoreApplication::applicationName()));
    setWindowModified(false);
}

void RemGlkTextEditWindow::fileNew()
{
    if (maybeSave()) {
        textEdit->clear();
        setCurrentFileName(QString());
        dataSpot.engineProcess.stopRunningEngine();
    }
}

void RemGlkTextEditWindow::fileOpen()
{
    int codePath = 1;
    switch (codePath) {
    case 0: {
        QFileDialog fileDialog(this, tr("Open File..."));
        fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
        fileDialog.setFileMode(QFileDialog::ExistingFile);
        fileDialog.setMimeTypeFilters(QStringList() << "text/html" << "text/plain");
        if (fileDialog.exec() != QDialog::Accepted)
            return;
        const QString fn = fileDialog.selectedFiles().first();
        if (load(fn))
            statusBar()->showMessage(tr("Opened \"%1\"").arg(QDir::toNativeSeparators(fn)));
        else
            statusBar()->showMessage(tr("Could not open \"%1\"").arg(QDir::toNativeSeparators(fn)));
        }
        break;
    case 1:
        switch (findFilesMethod) {
        case 0:
            dataSpot.findFilesWindow.show();
            break;
        case 1:
            dataSpot.findStoriesWindow.show();
            break;
        }
        break;
    }
}

bool RemGlkTextEditWindow::fileSave()
{
    if (fileName.isEmpty())
        return fileSaveAs();
    if (fileName.startsWith(QStringLiteral(":/")))
        return fileSaveAs();

    QTextDocumentWriter writer(fileName);
    bool success = writer.write(textEdit->document());
    if (success) {
        textEdit->document()->setModified(false);
        statusBar()->showMessage(tr("Wrote \"%1\"").arg(QDir::toNativeSeparators(fileName)));
    } else {
        statusBar()->showMessage(tr("Could not write to file \"%1\"")
                                 .arg(QDir::toNativeSeparators(fileName)));
    }
    return success;
}

bool RemGlkTextEditWindow::fileSaveAs()
{
    QFileDialog fileDialog(this, tr("Save as..."));
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    QStringList mimeTypes;
    mimeTypes << "application/vnd.oasis.opendocument.text" << "text/html" << "text/plain";
    fileDialog.setMimeTypeFilters(mimeTypes);
    fileDialog.setDefaultSuffix("odt");
    if (fileDialog.exec() != QDialog::Accepted)
        return false;
    const QString fn = fileDialog.selectedFiles().first();
    setCurrentFileName(fn);
    return fileSave();
}

void RemGlkTextEditWindow::filePrint()
{
#if QT_CONFIG(printdialog)
    QPrinter printer(QPrinter::HighResolution);
    QPrintDialog *dlg = new QPrintDialog(&printer, this);
    if (textEdit->textCursor().hasSelection())
        dlg->addEnabledOption(QAbstractPrintDialog::PrintSelection);
    dlg->setWindowTitle(tr("Print Document"));
    if (dlg->exec() == QDialog::Accepted)
        textEdit->print(&printer);
    delete dlg;
#endif
}

void RemGlkTextEditWindow::filePrintPreview()
{
#if QT_CONFIG(printpreviewdialog)
    QPrinter printer(QPrinter::HighResolution);
    QPrintPreviewDialog preview(&printer, this);
    connect(&preview, &QPrintPreviewDialog::paintRequested, this, &RemGlkTextEditWindow::printPreview);
    preview.exec();
#endif
}

void RemGlkTextEditWindow::printPreview(QPrinter *printer)
{
#ifdef QT_NO_PRINTER
    Q_UNUSED(printer);
#else
    textEdit->print(printer);
#endif
}


void RemGlkTextEditWindow::filePrintPdf()
{
#ifndef QT_NO_PRINTER
//! [0]
    QFileDialog fileDialog(this, tr("Export PDF"));
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    fileDialog.setMimeTypeFilters(QStringList("application/pdf"));
    fileDialog.setDefaultSuffix("pdf");
    if (fileDialog.exec() != QDialog::Accepted)
        return;
    QString fileName = fileDialog.selectedFiles().first();
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);
    textEdit->document()->print(&printer);
    statusBar()->showMessage(tr("Exported \"%1\"")
                             .arg(QDir::toNativeSeparators(fileName)));
//! [0]
#endif
}

void RemGlkTextEditWindow::textBold()
{
    QTextCharFormat fmt;
    fmt.setFontWeight(actionTextBold->isChecked() ? QFont::Bold : QFont::Normal);
    mergeFormatOnWordOrSelection(fmt);
}

void RemGlkTextEditWindow::textUnderline()
{
    QTextCharFormat fmt;
    fmt.setFontUnderline(actionTextUnderline->isChecked());
    mergeFormatOnWordOrSelection(fmt);
}

void RemGlkTextEditWindow::textItalic()
{
    QTextCharFormat fmt;
    fmt.setFontItalic(actionTextItalic->isChecked());
    mergeFormatOnWordOrSelection(fmt);
}

void RemGlkTextEditWindow::textFamily(const QString &f)
{
    QTextCharFormat fmt;
    fmt.setFontFamily(f);
    mergeFormatOnWordOrSelection(fmt);
}

void RemGlkTextEditWindow::textSize(const QString &p)
{
    qreal pointSize = p.toFloat();
    if (p.toFloat() > 0) {
        QTextCharFormat fmt;
        fmt.setFontPointSize(pointSize);
        mergeFormatOnWordOrSelection(fmt);
    }
}

void RemGlkTextEditWindow::textStyle(int styleIndex)
{
    QTextCursor cursor = textEdit->textCursor();

    if (styleIndex != 0) {
        QTextListFormat::Style style = QTextListFormat::ListDisc;

        switch (styleIndex) {
            default:
            case 1:
                style = QTextListFormat::ListDisc;
                break;
            case 2:
                style = QTextListFormat::ListCircle;
                break;
            case 3:
                style = QTextListFormat::ListSquare;
                break;
            case 4:
                style = QTextListFormat::ListDecimal;
                break;
            case 5:
                style = QTextListFormat::ListLowerAlpha;
                break;
            case 6:
                style = QTextListFormat::ListUpperAlpha;
                break;
            case 7:
                style = QTextListFormat::ListLowerRoman;
                break;
            case 8:
                style = QTextListFormat::ListUpperRoman;
                break;
        }

        cursor.beginEditBlock();

        QTextBlockFormat blockFmt = cursor.blockFormat();

        QTextListFormat listFmt;

        if (cursor.currentList()) {
            listFmt = cursor.currentList()->format();
        } else {
            listFmt.setIndent(blockFmt.indent() + 1);
            blockFmt.setIndent(0);
            cursor.setBlockFormat(blockFmt);
        }

        listFmt.setStyle(style);

        cursor.createList(listFmt);

        cursor.endEditBlock();
    } else {
        // ####
        QTextBlockFormat bfmt;
        bfmt.setObjectIndex(-1);
        cursor.mergeBlockFormat(bfmt);
    }
}

void RemGlkTextEditWindow::textColor()
{
    QColor col = QColorDialog::getColor(textEdit->textColor(), this);
    if (!col.isValid())
        return;
    QTextCharFormat fmt;
    fmt.setForeground(col);
    mergeFormatOnWordOrSelection(fmt);
    colorChanged(col);
}

void RemGlkTextEditWindow::textAlign(QAction *a)
{
    if (a == actionAlignLeft)
        textEdit->setAlignment(Qt::AlignLeft | Qt::AlignAbsolute);
    else if (a == actionAlignCenter)
        textEdit->setAlignment(Qt::AlignHCenter);
    else if (a == actionAlignRight)
        textEdit->setAlignment(Qt::AlignRight | Qt::AlignAbsolute);
    else if (a == actionAlignJustify)
        textEdit->setAlignment(Qt::AlignJustify);
}

void RemGlkTextEditWindow::currentCharFormatChanged(const QTextCharFormat &format)
{
    fontChanged(format.font());
    colorChanged(format.foreground().color());
}

void RemGlkTextEditWindow::cursorPositionChanged()
{
    alignmentChanged(textEdit->alignment());
}

void RemGlkTextEditWindow::clipboardDataChanged()
{
#ifndef QT_NO_CLIPBOARD
    if (const QMimeData *md = QApplication::clipboard()->mimeData())
        actionPaste->setEnabled(md->hasText());
#endif
}

void RemGlkTextEditWindow::about()
{
    // ToDo: Should override in dependent class, but hand-edited anyway.
    QMessageBox::about(this, tr("About"), tr("WakeReality.com - Thunderquake::Qt (c) Copyright 2017 Stephen A. Gutknecht."));
}

void RemGlkTextEditWindow::mergeFormatOnWordOrSelection(const QTextCharFormat &format)
{
    QTextCursor cursor = textEdit->textCursor();
    if (!cursor.hasSelection())
        cursor.select(QTextCursor::WordUnderCursor);
    cursor.mergeCharFormat(format);
    textEdit->mergeCurrentCharFormat(format);
}

void RemGlkTextEditWindow::fontChanged(const QFont &f)
{
    comboFont->setCurrentIndex(comboFont->findText(QFontInfo(f).family()));
    comboSize->setCurrentIndex(comboSize->findText(QString::number(f.pointSize())));
    actionTextBold->setChecked(f.bold());
    actionTextItalic->setChecked(f.italic());
    actionTextUnderline->setChecked(f.underline());
}

void RemGlkTextEditWindow::colorChanged(const QColor &c)
{
    QPixmap pix(16, 16);
    pix.fill(c);
    actionTextColor->setIcon(pix);
}

void RemGlkTextEditWindow::alignmentChanged(Qt::Alignment a)
{
    if (a & Qt::AlignLeft)
        actionAlignLeft->setChecked(true);
    else if (a & Qt::AlignHCenter)
        actionAlignCenter->setChecked(true);
    else if (a & Qt::AlignRight)
        actionAlignRight->setChecked(true);
    else if (a & Qt::AlignJustify)
        actionAlignJustify->setChecked(true);
}

