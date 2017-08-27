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

#include "remglkcompletertextedit.h"
#include <QCompleter>
#include <QKeyEvent>
#include <QAbstractItemView>
#include <QtDebug>
#include <QApplication>
#include <QModelIndex>
#include <QAbstractItemModel>
#include <QScrollBar>
#include <QTimer>

//! [0]
RemGlkCompleterTextEdit::RemGlkCompleterTextEdit(QWidget *parent)
: QTextBrowser(parent), c(0)
{
    setPlainText(tr("This TextEdit provides autocompletions for words that have more than"
                    " 3 characters. You can trigger autocompletion using ") +
                    QKeySequence("Ctrl+E").toString(QKeySequence::NativeText));

    setReadOnly(false);
}
//! [0]

//! [1]
RemGlkCompleterTextEdit::~RemGlkCompleterTextEdit()
{
}
//! [1]

//! [2]
void RemGlkCompleterTextEdit::setCompleter(QCompleter *completer)
{
    if (c)
        QObject::disconnect(c, 0, this, 0);

    c = completer;

    if (!c)
        return;

    c->setWidget(this);
    c->setCompletionMode(QCompleter::PopupCompletion);
    c->setCaseSensitivity(Qt::CaseInsensitive);
    QObject::connect(c, SIGNAL(activated(QString)),
                     this, SLOT(insertCompletion(QString)));
}
//! [2]

//! [3]
QCompleter *RemGlkCompleterTextEdit::completer() const
{
    return c;
}
//! [3]

//! [4]
void RemGlkCompleterTextEdit::insertCompletion(const QString& completion)
{
    qDebug() << "-------- insertCompletion";
    if (c->widget() != this)
        return;
    QTextCursor tc = textCursor();
    int extra = completion.length() - c->completionPrefix().length();
    tc.movePosition(QTextCursor::Left);
    tc.movePosition(QTextCursor::EndOfWord);
    tc.insertText(completion.right(extra));
    setTextCursor(tc);

    // substitute the partial for full with RemGlk word collection
    previousInputData = completion;
    qDebug() << "-------- insertCompletion completion: " << completion;
}
//! [4]

//! [5]
QString RemGlkCompleterTextEdit::textUnderCursor() const
{
    QTextCursor tc = textCursor();
    tc.select(QTextCursor::WordUnderCursor);
    return tc.selectedText();
}
//! [5]

//! [6]
void RemGlkCompleterTextEdit::focusInEvent(QFocusEvent *e)
{
    if (c)
        c->setWidget(this);
    QTextEdit::focusInEvent(e);
}
//! [6]


/*
 * App_ToDo: erase the input after [enter] is pressed, as the IF engine will echo it back - appearing twice.
 * */

void RemGlkCompleterTextEdit::keyPressEvent(QKeyEvent *e)
{
    keystrokesSinceRemGlkSend++;
    QString codeTraceA = "";

    /*
     * Thoughts on keystrokesSinceRemGlkSend - we need to keep track of EVERY keystroke entered into the TextEdit
     *    if there are zero entered - do not allow backspace or other delete operations.
     * */

    // is the completer visible?
    if (c && c->popup()->isVisible()) {
        // The following keys are forwarded by the completer to the widget
       switch (e->key()) {
       case Qt::Key_Enter:
       case Qt::Key_Return:
           e->ignore();
           switch (remGlkInputMode) {
           case 1:   // line input only
           case 5:   // line + HyperLink
               qDebug() << "[completer] :::::: LINE got enter @ top. keystrokesSinceRemGlkSend " << keystrokesSinceRemGlkSend;
               break;
           }
           // Drop through to the RemGlk logic
           // break;
           return;
       case Qt::Key_Escape:
       case Qt::Key_Tab:
       case Qt::Key_Backtab:
            e->ignore();
            return; // let the completer do default behavior
       default:
           break;
       }
       codeTraceA += "A";
    } else {
        // Not yet visible, indication we are in first few keystrokes
        codeTraceA += "B";
        qDebug() << "[completer] not visible. key: " << e->key() << " keystrokesSinceRemGlkSend " << keystrokesSinceRemGlkSend;
    }

    bool isShortcut = ((e->modifiers() & Qt::ControlModifier) && e->key() == Qt::Key_H); // CTRL+H
    // do not process the shortcut when we have a completer
    if (!c || !isShortcut) {
        codeTraceA += "C";
        QTextEdit::keyPressEvent(e);
    }

    const bool ctrlOrShift = e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);
    // Not visible, shift or ctrl by themsleves
    if (!c || (ctrlOrShift && e->text().isEmpty())) {
        return;
    }

     // end of word
    static QString endOfWord("~!@#$%^&*()_+{}|:\"<>?,./;'[]\\-=");
    const bool hasModifier = (e->modifiers() != Qt::NoModifier) && !ctrlOrShift;
    const QString completionPrefix = textUnderCursor();

    // Less optimzied logic perhaps, but far easier to grasp for modding.
    if (!isShortcut)
    {
        // Time to close the visible popup?
        if (hasModifier || e->text().isEmpty()) {
            c->popup()->hide();
            return;
        }

        codeTraceA += "M";
        qDebug() << "[completer] SPOT_A length is " << completionPrefix.length() << " key " << e->key() << " e->text() " << e->text()
                 << " completionPrefix " << completionPrefix << " keystrokesSinceRemGlkSend " << keystrokesSinceRemGlkSend
                 << " previousInputData " << previousInputData << " previousInputMultiData " << previousInputMultiWordData
                 << " mode " << remGlkInputMode << " codeTraceA " << codeTraceA;

        int endingDetected = 0;

        // Very first character, in RemGlk char mode that's useful!
        // is RemGlk wanting a single character?
        switch (remGlkInputMode) {
            case 1:   // line input only
            case 3:   // line + char. ToDo: currently defaults to LINE, ignoring char mode
            case 5:   // line + HyperLink
                if (e->text() == "\r") {
                    endingDetected = 1;
                } else if (e->text() == "\n") {
                    endingDetected = 1;
                } else if (e->key() == Qt::Key_Space) {
                    endingDetected = 2;
                } else if (e->key() == Qt::Key_Tab) {
                    endingDetected = 2;
                }

                switch (endingDetected) {
                case 1:
                    qDebug() << "[completer] :::::: LINE got enter. before: " << previousInputMultiWordData << " latest: " << previousInputData;
                    previousInputMultiWordData += previousInputData;
                    // Backspace to clear the words we just put into the document.
                    for (int charIndex = 0; charIndex <= previousInputMultiWordData.length(); charIndex++) {
                        textCursor().deletePreviousChar();
                    }
                    emit characterToEngine(previousInputMultiWordData, remGlkInputMode);
                    keystrokesSinceRemGlkSend = 0;
                    previousInputData = "";
                    previousInputMultiWordData = "";
                    break;
                case 2:
                    qDebug() << "[completer] :::::: LINE got space " << previousInputMultiWordData;
                    previousInputMultiWordData += previousInputData + " ";
                    previousInputData = "";
                    break;
                case 0:
                    previousInputData = completionPrefix;
                    break;
                }
                break;
            case 2:   // char input only
                QString outPayload = completionPrefix;
                if (outPayload=="") {
                    qDebug() << "[completer] WARNING, empty on key " << e->key();
                    switch (e->key()) {
                    case Qt::Key_Enter:
                    case Qt::Key_Return:
                        // RemGlk rgdata.c keycodes
                        outPayload = "return";
                        qDebug() << "[completer] :::::: CHAR got enter " << previousInputData;
                        break;
                    case Qt::Key_Space:
                        outPayload = " ";
                        break;
                    }
                }
                emit characterToEngine(outPayload, remGlkInputMode);
                keystrokesSinceRemGlkSend = 0;

                // Backspace to clear the character we just put into the document.
                // Use timer delay to give visual feedback of keypress, like entering a password letter that changes to '*' briefly after.
                // ToDo: keep exact position of this inserted char, in case incoming text starts to come before the timer fires?
                // Another idea is to animate or blink color the background - that way incoming RemGlk next generation text won't be interfering.
                QTimer::singleShot(80, [=] {
                    textCursor().clearSelection();
                    textCursor().deletePreviousChar();
                    });
                return;
                break;
        }

        // Made it this far (did not return), do more checks
        if (completionPrefix.length() < completeMinimumLength || endOfWord.contains(e->text().right(1)))
        {
            codeTraceA += "Q";
            c->popup()->hide();
            return;
        }
    }

    // qDebug() << "[completer] codeTraceA " << codeTraceA;

    if (completionPrefix != c->completionPrefix()) {
        c->setCompletionPrefix(completionPrefix);
        c->popup()->setCurrentIndex(c->completionModel()->index(0, 0));
    }
    QRect cr = cursorRect();
    cr.setWidth(c->popup()->sizeHintForColumn(0)
                + c->popup()->verticalScrollBar()->sizeHint().width());
    // popup it up!
    c->complete(cr);
}


void RemGlkCompleterTextEdit::setInputMode(int inputMode)
{
    remGlkInputMode = inputMode;
    qDebug() << "[completer] TextEdit inputMode now " << inputMode;
}
