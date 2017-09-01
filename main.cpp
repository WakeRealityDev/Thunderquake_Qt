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
 * */

#include <QApplication>

#include "remglktexteditwindow.h"
// #include "texteditwindow.h"
#include "findfileswindow.h"

#include <QDesktopWidget>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QDebug>



/*
 * ToDo:
 *    Ubuntu lacks icon for app
 *     https://stackoverflow.com/questions/4328383/qt-creator-how-to-set-application-icon-for-ubuntu-linux
 *
 * Autocompleter is grabbing each keystroke. if RemGlk input char mode, send. Allowing the entire window to be an entry box, like Activity on Java Android.
 * */

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(remglktextedit);

    QApplication app(argc, argv);

    QCoreApplication::setOrganizationName("Wake Reality");
    QCoreApplication::setApplicationName("Thunderquake::Qt prototype");
    QCoreApplication::setApplicationVersion(QT_VERSION_STR);

    QCommandLineParser parser;
    parser.setApplicationDescription(QCoreApplication::applicationName());
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("file", "The file to open.");
    parser.process(app);

    RemGlkTextEditWindow mw;

#ifndef Q_OS_ANDROID
        qDebug() << "main() non-Android screen sizing path";

        /*
         * Code very poorly deals with 3x4k monitor config and tosses window off screen (none of the 3 heads).
         * This problem exists in the Qt 5.9.1 official example Text Edit app and several other examples.
         * ToDo: fix
        */
        // const QRect availableGeometry = QApplication::desktop()->availableGeometry(&mw);
        const QRect availableGeometry = QApplication::desktop()->screenGeometry(-1);
        QString dimens = QString("%1x%2").arg(QString::number(availableGeometry.width()), QString::number(availableGeometry.height()));
        if (dimens == "3840x2160") {
           // On Ubuntu 17.04 desktop with 3840x2160 screen, Unit-tweak used to turn off auto-maximize, it still auto-maximizes with default
           //    path.  Put in these slightly less values and it avoids the behavior.  ToDo: figure out why.
            mw.resize(1200, 1440);
        } else {
            mw.resize(availableGeometry.width() / 2, (availableGeometry.height() * 2) / 3);
        }
        mw.setWindowState(mw.windowState() ^ Qt::WindowNoState);
        qDebug() << "resize " << availableGeometry.width() / 2 << (availableGeometry.height() * 2) / 3
                     << " of " << availableGeometry.width() << availableGeometry.height();
        // mw.move((availableGeometry.width() - mw.width()) / 2, (availableGeometry.height() - mw.height()) / 2);
#endif

        // Load initial file?
        switch (0) {
        case 0:
            if (!mw.load(parser.positionalArguments().value(0, QLatin1String(":/example.html")))) {
                mw.fileNew();
            }
            break;
        case 1:
            mw.fileNew();
            break;
        }

        // NOTE Interesting observation: showNormal() on Android tablet reveals that Qt allows multiple windows on the screen at same time!
        // mw.showNormal();
        mw.show();

    return app.exec();
}
