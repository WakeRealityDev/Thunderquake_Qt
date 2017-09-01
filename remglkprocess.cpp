/* Thunderquake Qt
 *   Copyright (C) 2017 Stephen A Gutknecht. All rights reserved.
 *
 * References
 * =============
 * QProcess on Android
 *    http://www.qtcentre.org/threads/56859-Starting-Qt-console-application-on-Android-using-QProcess
 * */

#include "remglkprocess.h"

#include <QDebug>
#include <QColor>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QMessageBox>
#include <QFileInfo>

#include "glk.h"

#include "remglkcompletertextedit.h"

RemGlkProcess::RemGlkProcess(QObject *parent, RemGlkCompleterTextEdit *textEditOutput) : QObject(parent)
{
    qDebug() << "---! RemGlkProcess constructor";
    textEdit = textEditOutput;
    remGlkInitMessage = "{ \"type\": \"init\", \"gen\": 0, \"metrics\": { \"width\":720, \"height\":400, \"charwidth\":9, \"charheight\":16 }, \"support\": [ \"timer\", \"hyperlinks\", \"graphics\", \"graphicswin\" ] }";

    // does this init cause deadlock?
    //   {"type":"init","gen":0, "metrics":{"width":0,"height":0,"gridcharheight":16,"gridcharwidth":8.375,"gridmarginx":20,"gridmarginy":12,"buffercharheight":16,"buffercharwidth":7.5,"buffermarginx":35,"buffermarginy":12,"graphicsmarginx":0,"graphicsmarginy":0,"outspacingx":0,"outspacingy":0,"inspacingx":0,"inspacingy":0}, "support":["timer","graphics","graphicswin","hyperlinks"]}
}

void RemGlkProcess::setOutputTextEdit(RemGlkCompleterTextEdit *textEditOutput)
{
    textEdit = textEditOutput;
    // outputLayout.textEdit = textEditOutput;
}

void RemGlkProcess::setRemGlkInitMessage(QString payload)
{
    remGlkInitMessage = payload;
    qDebug() << ":::::::: set RemGlk INIT " << payload;
}


bool setupOnceA = false;

void RemGlkProcess::launchRemGlkEngine() {
    storyEngineState = 1;

    // assistant/remotecontrol.cpp
    if (remGlkDebug_process_steps) {
        textEdit->append("\n:::: creating QProcess object");
    }

    // Beyond the very first launch since app start, skip some steps.
    if (!setupOnceA) {
        setupOnceA = true;
        dataTempStream.setCodec("UTF-8");
        dataTempStream.setString(&dataTempBuffer,QIODevice::ReadWrite | QIODevice::Text);
    }

    process = new QProcess(this);
    connect(process, SIGNAL(finished(int,QProcess::ExitStatus)),
        this, SLOT(engineProcessClosed()));
    connect(process, SIGNAL(readyReadStandardOutput()),
        this, SLOT(processStandardOutputFromRemGlk()));
    connect(process, SIGNAL(stateChanged(QProcess::ProcessState)),
        this, SLOT(processStateChange(const QProcess::ProcessState)));

    if (process->state() != QProcess::Running) {
        launchRemGlkEngineGo();
    } else {
        qDebug() << "CONFUSED QProcess state " << process->state();
        textEdit->append("\n<b>:::: CONFUSED process state in launchRemGlkEngine.</b>");
    }
}


void RemGlkProcess::resetForNewLaunch()
{
    engineInputMode = -1;
    engineInputCharGen = -1;
    engineInputLineGen = -1;
    engineInputLineMaxLen = -1;
    engineInputLineWindowId = -1;
    engineInputCharWindowId = -1;

    processCycle = 0;
    incomingRemGlkStanzaCount = 0;
    rawIncomingPileJSON = "";
    rawIncomingPileAdditions = 0;
    updateGeneration = -1;
    stanzaIncomingFlag = false;

    engineInputLineGenPreviousSent = -1;
    engineInputCharGenPreviousSent = -1;
}

void RemGlkProcess::stopRunningEngine()
{
    if (storyEngineState > 0) {
        textEdit->append("\n::::: stopping running engine.");
        process->close();
        process->kill();
        resetForNewLaunch();
        textEdit->append(QString(" ::::: ready for new launch. storyEngineState: %1\n").arg(storyEngineState));
    }
    storyEngineState = 0;
}


void RemGlkProcess::setDataFilePath(QString dataFilePath)
{
    this->dataFilePath = dataFilePath;
}

void RemGlkProcess::setRemGlkEngine(int engineCode)
{
    switch (enginePickA) {
    case 0:
        appRemGlkBinaryPath = "./remglk_glulxe";
        break;
    case 1:
        // if inside this APK use:
        appRemGlkBinaryPath = "../lib/lib_app_git.so";
        // if inside the Thunderfall APK, use this (or modify to be -2 APK variation)
        // NOTE: this needs to distinguish x86 vs arm CPU, currently does not
        appRemGlkBinaryPath = "/data/app/com.wakereality.thunderfall-1/lib/x86/lib_app_git.so";
        break;
    case 2:
        // if inside this APK use:
        appRemGlkBinaryPath = "../lib/lib_app_glulxe.so";
        // if inside the Thunderfall APK, use this (or modify to be -2 APK variation)
        // NOTE: this needs to distinguish x86 vs arm CPU, currently does not
        appRemGlkBinaryPath = "/data/app/com.wakereality.thunderfall-1/lib/x86/lib_app_glulxe.so";
        break;
    }
}


/*
 * Start the child RemGlk executable process
 * */
void RemGlkProcess::launchRemGlkEngineGo()
{
    if (remGlkDebug_process_steps) {
        textEdit->append("\r\n:::: QProcess launchButton_clicked");
    }
    if (process->state() == QProcess::Running)
        return;

    QStringList args;

    args << dataFilePath;

    if (! QFileInfo(appRemGlkBinaryPath).exists()) {
        qDebug() << "Process target binary does not exist. appBinary path: " << appRemGlkBinaryPath;
        QMessageBox::critical(textEdit, tr("Thunderquake Engine Launch"),  tr("Could not locate IF Interpreter engine at path: %1 pwd: %2").arg(appRemGlkBinaryPath).arg(QDir::currentPath()));
        textEdit->append(QString("\nERROR PEL-A00. PROBLEM: Thunderquake Engine Launch. Could not locate IF Interpreter engine at path: %1 pwd: %2").arg(appRemGlkBinaryPath).arg(QDir::currentPath()));
        return;
    }

    if (! QFileInfo(dataFilePath).exists()) {
        qDebug() << "IF story data file path does not exist: " << dataFilePath;
        QMessageBox::critical(textEdit, tr("Thunderquake Engine Launch"),  tr("Could not locate IF story data file path: %1").arg(dataFilePath));
        textEdit->append(QString("\nERROR PEL-A10. PROBLEM: Thunderquake Engine Launch. Could not locate IF story data file at path: %1").arg(dataFilePath));
        return;
    }

    process->start(appRemGlkBinaryPath, args);
    if (remGlkDebug_process_steps) {
        textEdit->append("\n:::: QProcess start issued");
    }
    if (!process->waitForStarted()) {
        qDebug() << "Process start failed, show MessageBox to user";
        QMessageBox::critical(textEdit, tr("Thunderquake Engine Launch"),  tr("Could not start RemGlk engine executable %1.").arg(appRemGlkBinaryPath));
        textEdit->append(QString("\nERROR PEL-A20. PROBLEM: Thunderquake Engine Launch. Could not start RemGlk engine executable %1.").arg(appRemGlkBinaryPath));
        return;
    }

    if (remGlkDebug_process_steps) {
        textEdit->append("\n:::: QProcess started");
    }

    // RemGlk Initialize the screen layout and features support.
    // ToDo: create a class to hold screen choice parameters. Create user interface to allow fixed-size selection. Measure dynamic size of window and offer as an option.
    sendCommand(remGlkInitMessage);
}


void RemGlkProcess::sendCommand(const QString &cmd)
{
    if (storyEngineState < 1) {
        qDebug() << "[DataToEngine] PROBLEM!!!!!!!!!!!! process not yet created, storyEngineState " << storyEngineState;
        return;
    }
    if (process->state() != QProcess::Running) {
        qDebug() << "[DataToEngine] PROBLEM!!!!!!!!!!!! process state not ready.";
        return;
    }
    qDebug() << "[DataToEngine][SpotDS0] " << cmd;
// ToDo: revisit this, as "local" may not be good for Unicode
    process->write(cmd.toLocal8Bit() + '\n');
    if (remGlkDebug_process_steps) {
        textEdit->append("\n:::: command sent");
    }
    qDebug() << "[DataToEngine] after send";
}

void RemGlkProcess::storyRespondTimer() {
    sendCommand(QString("{\"type\":\"timer\", \"gen\":%1}").arg(updateGeneration));
}

void RemGlkProcess::storyRespond(const QString storyInput) {
    storyRespond_returnPressed(storyInput);
}

void RemGlkProcess::storyRespond_returnPressed(const QString storyInput) {
    if (!stanzaIncomingFlag) {
        // Holding [enter] key down on the input combo box results in generation errors after 80 to 200. This logic improves that problem.
        //    It seemingly eliminate the problem - 5000 RemGlk generations were tested as fast as possible.
        if (engineInputLineGen > engineInputLineGenPreviousSent) {
            int engineInputLineGenCopy = engineInputLineGen;
            engineInputLineGenPreviousSent = engineInputLineGenCopy;
            sendCommand(QString("{\"type\":\"line\", \"gen\":%2, \"window\":%1, \"value\":\"%3\"}").arg(engineInputLineWindowId).arg(engineInputLineGenCopy).arg(storyInput));
        } else {
            qDebug() << "input ignored, stanzaIncomingFlag is false, however, generation is the same. " << engineInputLineGen << " line " << engineInputLineGenPreviousSent;
        }
    } else {
        qDebug() << "input ignored, stanzaIncomingFlag is true";
    }
}

void RemGlkProcess::storyRespondChar(const QString storyInput) {
    if (!stanzaIncomingFlag) {
        // Holding [enter] key down on the input combo box results in generation errors after 80 to 200. This logic improves that problem.
        //    It seemingly eliminate the problem - 5000 RemGlk generations were tested as fast as possible.
        if (engineInputCharGen > engineInputCharGenPreviousSent) {
            int engineInputCharGenCopy = engineInputCharGen;
            engineInputCharGenPreviousSent = engineInputCharGenCopy;
            // sendCommand(QString("{\"type\":\"char\", \"gen\":%2, \"window\":%1, \"value\":\"%3\"}").arg(engineInputCharWindowId).arg(engineInputCharGenCopy).arg(storyInput));
            sendCommand(QString("{\"type\":\"char\", \"gen\":%2, \"window\":%1, \"value\":\"%3\"}").arg(engineInputCharWindowId).arg(updateGeneration).arg(storyInput));
        } else {
            qDebug() << "input ignored, stanzaIncomingFlag is false, however, generation is the same. " << engineInputCharGen << " char " << engineInputCharGenPreviousSent;
        }
    } else {
        qDebug() << "input ignored, stanzaIncomingFlag is true";
    }
}

void RemGlkProcess::processStateChange(const QProcess::ProcessState newState)
{
    qDebug() << "processStateChange" << newState;
    if (remGlkDebug_process_steps) {
        textEdit->append("\n:::: QProcess processStateChange");
    }
    if (newState == QProcess::NotRunning)
    {
        // ToDo: some variable to track if disconnected from the remote RemGlk service. Imaginge playing on a cloud/ssh/telnet session.
        // setOnline(false);
    }
}

void RemGlkProcess::engineProcessClosed()
{
    qDebug() << "engineProcessClosed";
    textEdit->append(QString("\n:::: QProcess <b>closed</b> %1 %2").arg(QString::number(process->exitCode())).arg(QString::number(process->exitStatus())));
    storyEngineState = 4;
}


// ##################################################################################
// ## RemGlk JSON stanza processing
// ##################################################################################

QString RemGlkProcess::toStringJsonObject(const QJsonObject jsonObj) {
    QJsonDocument doc(jsonObj);
    QString strJson(doc.toJson(QJsonDocument::Compact));
    return strJson;
}

void RemGlkProcess::inlineOutputRemGlkDebug(int format, QString debugOutput) {
    if (remGlkDebug_JSON_parsing) {
        switch (format) {
        case 0:
            textEdit->insertPlainText(debugOutput);
            break;
        case 1:
            textEdit->insertHtml(debugOutput);
            break;
        }
    }
}


void RemGlkProcess::processRemGlkStanza(const QJsonObject jsonObject) {
    stanzaIncomingFlag = true;
    if (storyEngineState == 1) {
        storyEngineState = 2;

        // Keep the initial document or clear it?
        switch (1) {
        case 0:
            textEdit->moveCursor(QTextCursor::Down);
            textEdit->moveCursor(QTextCursor::End);
            break;
        case 1:
            // This is first stanza of a newly laoded story.
            textEdit->clear();
            break;
        }
    }

    inlineOutputRemGlkDebug(0, "--::---------- BEGIN\n");

    if (jsonObject.contains("type")) {
        QString stanzaType = jsonObject["type"].toString();
        if (remGlkDebug_JSON_parsing) {
            textEdit->insertPlainText("type ");
            textEdit->insertPlainText(stanzaType);
        }

        if (stanzaType == "update") {
            if (jsonObject.contains("gen")) {
                updateGeneration = jsonObject["gen"].toVariant().toLongLong();
                if (remGlkDebug_JSON_parsing) {
                    textEdit->insertPlainText(" gen ");
                    textEdit->insertPlainText(QString::number(updateGeneration));
                }
            }
            if (jsonObject.contains("windows")) {
                outputLayout.stanzaRemGlkWindows(jsonObject["windows"].toArray());
            }
            if (jsonObject.contains("content")) {
                outputLayout.stanzaRemGlkContent(jsonObject["content"].toArray());
            }
            if (jsonObject.contains("input")) {
                stanzaRemGlkInput(jsonObject["input"].toArray());
            }
            if (jsonObject.contains("timer")) {
                stanzaRemGlkTimer(jsonObject["timer"]);
            }
        } else {
            textEdit->insertHtml(" <b>::: NOT UPDATE :::</b> ");
            textEdit->insertPlainText(toStringJsonObject(jsonObject));
        }
    }

    inlineOutputRemGlkDebug(0, "\n--::---------- END\n");
    // Position cursor to end of newly inserted text.
    textEdit->moveCursor(QTextCursor::End);
    stanzaIncomingFlag = false;
}


void RemGlkProcess::stanzaRemGlkInput(QJsonArray input) {
    engineInputMode = -1;
    int inputLineDetected = 0;
    int inputCharDetected = 0;
    int inputHyperLinkDetected = 0;
    if (! input.isEmpty()) {
        inlineOutputRemGlkDebug(1, " <font color=\"DeepPink\">input</font> [");
        for(int i=0; i< input.count(); ++i){
            if (input.at(i).isObject()) {
                inlineOutputRemGlkDebug(1, "  &nbsp; <font color=\"Brown\">!!</font> ");
                QJsonObject inputElement = input.at(i).toObject();
                if (inputElement.contains("type")) {
                    if (inputElement["type"] == "line") {
                        inputLineDetected++;
                        engineInputLineGen = inputElement["gen"].toInt();
                        engineInputLineMaxLen = inputElement["maxlen"].toInt();
                        engineInputLineWindowId = inputElement["id"].toInt();
                    } else if (inputElement["type"] == "char") {
                        inputCharDetected++;
                        engineInputCharGen = inputElement["gen"].toInt();
                        engineInputCharWindowId = inputElement["id"].toInt();
                    }
                }
                if (inputElement.contains("hyperlink")) {
                    inputHyperLinkDetected++;
                }

                inlineOutputRemGlkDebug(0, toStringJsonObject(inputElement));
                qDebug() << "input " << toStringJsonObject(inputElement);
            }
        }

        inlineOutputRemGlkDebug(0, "] ");
    }

    int newInputMode = 0;
    // multiple windows may have line input detected, we only need to know if one or more.
    if (inputLineDetected > 0)
        newInputMode += 1;
    if (inputCharDetected > 0)
        newInputMode += 2;
    if (inputHyperLinkDetected > 0)
        newInputMode += 4;

    if (newInputMode > 0) {
        engineInputMode = newInputMode;
    }
    textEdit->setInputMode(engineInputMode);
}



/*
 * ToDo: everything to do with the Glk Timer. This is incomplete.
 * Glk only has a single timer. Suggest creating a single thread and using sleep. However, keep in mind that
 *    any char or line input activity from the player cancels the timer cycle. Only one RemGlk response per generation
 *    be it a hyperlink click, line input, char input, or timer tock.
 * */
void RemGlkProcess::stanzaRemGlkTimer(QJsonValue timer) {
    qDebug() << "[DataFromEngine][GlkTimer] :::::::::--- GLK Timer incoming value " << timer;
    // Glk timer value is tricky, review the GlkOte code comments - they are verbose and explinatiory ;)
    // is it null?
    if (timer == QJsonValue::Null) {
        qDebug() << "[DataFromEngine][GlkTimer] :::::::::--- GLK Timer value null";
    } else {
        long timerValue = timer.toInt(-1);
        if (timerValue >= 0 ) {
            qDebug() << "[DataFromEngine][GlkTimer] :::::::::--- GLK Timer value int " << timerValue << " engineInputMode " << engineInputMode;
            // Glulx story Six opening has a 500ms timer delay with no other input. Test with virgin run, delete data files sixdata0/sixdata1.
            if (engineInputMode == -1) {
                storyRespondTimer();
            }
        } else {
            qDebug() << "[DataFromEngine][GlkTimer] :::::::::--- GLK Timer value confused " << timer;
        }
    }
}


void RemGlkProcess::processStandardOutputFromRemGlk()
{
    if (remGlkDebug_process_steps) {
        textEdit->append("\n:::: QProcess processStandardOutput ");
    }

    int onLine = 0;
    QString line;
    QString assembledText = "";
    while (process->canReadLine())
    {
        dataTempStream << process->readLine();
        if (dataTempStream.readLineInto(&line))
        {
            onLine++;

            rawIncomingPileJSON.append(line);
            rawIncomingPileJSON.append("\n");
            rawIncomingPileAdditions++;

            if (line == "") {
                // End of RemGlk stanza, blank line
                assembledText = rawIncomingPileJSON;

                // WARNING: "This is extremely inefficient. When you assign readAll() to a QString you're causing a byte array to be converted from utf8 bytes to a utf16 QString. And then when you call fromJson you're causing the utf16 string to be converted back into a new utf8 byte array. Change strReply to a QByteArray and you avoid all this pointless conversion."
                QJsonDocument jsonResponse = QJsonDocument::fromJson(assembledText.toUtf8());
                if(jsonResponse.isObject()){
                    QJsonObject jsonObject = jsonResponse.object();

                    QString strJson(jsonResponse.toJson(QJsonDocument::Compact));
                    if (remGlkDebug_JSON_in_raw) {
                        qDebug() << "[DataFromEngine] incoming JSON: " << strJson;
                        assembledText = "got JSON~~~ ";
                        assembledText.append(strJson);
                        assembledText.append(" ~~~");
                        textEdit->append(assembledText);
                    }

                    incomingRemGlkStanzaCount++;
                    // Which front-end rendering solution for this assembled RemGlk JSON stanza?
                    switch (frontEndMode) {
                    case 0:
                        processRemGlkStanza(jsonObject);
                        break;
                    case 1:
                        // webViewWindow->incomingRemGlkStanza(assembledText);
                        emit incomingRemGlkStanzaReady(assembledText);
                        break;
                    case 2:
                        qDebug() << "!!!!!!!!!!!!!!!!! DOING NOTHING SPOT_REMGLK_AAAAA0 !!!!!!!!!!!!!!\n" << assembledText;
                        break;
                    }

                } else {
                    qDebug() << "ERROR: not JSONObject";
                    textEdit->append("\n::: JSON failure spot A");
                }

                /*
                 * LOGIC CONCERN WARNING
                 * What if two stanzas are together in one stdin batch, waiting to read. This loop
                 * will keep reading until there is no more stdin input - even if we found a blank line ending stanza.
                 * */

                // clear pile for start of next RemGlk stanza
                rawIncomingPileJSON = "";
                rawIncomingPileAdditions = 0;
            }
        }
    }

    processCycle++;

    if (remGlkDebug_process_steps) {
        textEdit->append("\n:::: QProcess processStandardOutput loop finished,");
        textEdit->append(QString::number(onLine));
    }

    /*
    // autoplay Story: Bronze. This has hard-coded response to the first couple turns of the story Bronze.
    switch (updateGeneration) {
    case 1:
        sendCommand(QString("{\"type\":\"line\", \"gen\":1, \"window\":%1, \"value\":\"yes\"}").arg(engineInputLineWindowId));
        break;
    case 2:
        sendCommand(QString("{\"type\":\"line\", \"gen\":%2, \"window\":%1, \"value\":\"north\"}").arg(engineInputLineWindowId).arg(engineInputLineGen));
        break;
    }
    */
}
