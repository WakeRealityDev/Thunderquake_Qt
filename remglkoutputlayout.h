#ifndef REMGLKOUTPUTLAYOUT_H
#define REMGLKOUTPUTLAYOUT_H

// #include "remglktexteditwindow.h"
#include "remglkcompletertextedit.h"

#include <QtWidgets>
#include <QObject>
#include <QGridLayout>
#include <QLabel>

#include <QJsonArray>
#include <QJsonDocument>

#include <QMap>

#include <glkwindowdata.h>
#include <outputstyledchunk.h>


class RemGlkOutputLayout : public QObject
{
    Q_OBJECT

public:
    RemGlkOutputLayout(QWidget *parent = 0);
    void setOutputWidgets(RemGlkCompleterTextEdit *textEdit, QGridLayout *mainLayout, QLabel *topStatus);

public:
    // RemGlkTextEditWindow *outputWindow;
    int testA = 0;

    // main Glk window content
    RemGlkCompleterTextEdit *textEdit;
    QGridLayout *mainLayout;
    QLabel *topStatus;

public:
    void inlineOutputRemGlkDebug(int format, QString debugOutput);
    void stanzaRemGlkWindows(QJsonArray windows);
    void stanzaRemGlkContent(QJsonArray content);
    void stanzaRemGlkContentStyledChunk(QJsonObject endpointStyledChunk);
    QString stanzaRemGlkContentStyledChunkToHTML(QJsonObject endpointStyledChunk);
    QString toStringJsonObject(const QJsonObject jsonObj);

protected:
    int fw;
    float saveFontPointSize;
    QColor tc;

public:
    // set to empty string to disable, allows regex
    // QString glkGridWindowRuntimeConsolidate = "        ";
    QString glkGridWindowRuntimeConsolidate = "[ \t]{4,}";

protected:
    //
    // INLINE WINDOW OUTPUT - DEBUG CONTROLS
    //
    bool remGlkDebug_process_steps = false;
    bool remGlkDebug_JSON_in_raw = false;
    bool remGlkDebug_JSON_parsing = false;

    bool glkWindowDebug_TextGrid = false;

    // Data structures for Glk windows
public:
    QMap<int, GlkWindowData> glkWindows;
};

#endif // REMGLKOUTPUTLAYOUT_H
