#ifndef REMGLKPROCESS_H
#define REMGLKPROCESS_H

#include <QObject>
#include <QProcess>

#include <QMainWindow>
#include <QMap>
#include <QPointer>
#include <QProcess>
#include <QTextStream>
#include <QJsonObject>
#include <QJsonArray>

#include <QTextEdit>

#include <remglkoutputlayout.h>
#include "document.h"
#include "webviewwindow.h"

QT_BEGIN_NAMESPACE
class QAction;
class QComboBox;
class QFontComboBox;
class RemGlkCompleterTextEdit;
class QTextCharFormat;
class QMenu;
class QPrinter;

class QCompleter;
QT_END_NAMESPACE

class RemGlkProcess : public QObject
{
    Q_OBJECT

public:
    explicit RemGlkProcess(QObject *parent = nullptr, RemGlkCompleterTextEdit *textEditOutput = nullptr);
    void setOutputTextEdit(RemGlkCompleterTextEdit *textEditOutput);
    void setDataFilePath(QString dataFilePath);
    void setRemGlkEngine(int engineCode);

signals:

public slots:
    // assistant/remotecontrol.cpp
    void engineProcessClosed();
    void launchRemGlkEngine();
    void launchRemGlkEngineGo();
    void stopRunningEngine();
    void resetForNewLaunch();
    void sendCommand(const QString &cmd);
    void processStandardOutputFromRemGlk();
    void processStateChange(const QProcess::ProcessState newState);
    void processRemGlkStanza(QJsonObject incomingStanza);
    void stanzaRemGlkInput(QJsonArray input);
    void stanzaRemGlkTimer(QJsonValue timer);
    void setRemGlkInitMessage(QString payload);

    void storyRespondTimer();
    void storyRespond(const QString storyInput);
    void storyRespond_returnPressed(const QString storyInput);
    void inlineOutputRemGlkDebug(int format, QString debugOutput);
    QString toStringJsonObject(const QJsonObject jsonObj);
    void storyRespondChar(const QString storyInput);

public:
    // assistant/remotecontrol.cpp
    QProcess *process;

    // ToDo: array of multiple RemGlk "plugin" pipe apps and a sequence number for them
    QProcess *pluginProcess;

    RemGlkCompleterTextEdit *textEdit;
    //RemGlkOutputLayout outputLayout;
    WebViewWindow *webViewWindow;

    QString remGlkInitMessage = "";

protected:
    //
    // INLINE WINDOW OUTPUT - DEBUG CONTROLS
    //
    bool remGlkDebug_process_steps = false;
    bool remGlkDebug_JSON_in_raw = false;
    bool remGlkDebug_JSON_parsing = false;

protected:
    QString dataTempBuffer;
    QTextStream dataTempStream;
    QString dataFilePath = "";
    QString appRemGlkBinaryPath = "remglk_glulxe";

public:
    // 0 = not started, 1 = just started, 2 = first stanza, 4 = closed
    int storyEngineState = 0;

protected:

    int frontEndMode = 1;
    int engineInputMode = -1;
    int engineInputCharGen = -1;
    int engineInputLineGen = -1;
    int engineInputLineMaxLen = -1;
    int engineInputLineWindowId = -1;
    int engineInputCharWindowId = -1;

    int processCycle = 0;
    int incomingRemGlkStanzaCount = 0;
    QString rawIncomingPileJSON = "";
    int rawIncomingPileAdditions = 0;
    long updateGeneration = -1;
    volatile bool stanzaIncomingFlag = false;

    int engineInputLineGenPreviousSent = -1;
    int engineInputCharGenPreviousSent = -1;

public:
    RemGlkOutputLayout outputLayout;

#ifdef Q_OS_ANDROID
#ifdef HOST_ANDROID_ARM
    int enginePickA = 2;
#else
    int enginePickA = 1;
#endif
#else
    int enginePickA = 0;
#endif

signals:
    void incomingRemGlkStanzaReady(const QString);
};

#endif // REMGLKPROCESS_H
