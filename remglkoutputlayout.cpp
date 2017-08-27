#include "remglkoutputlayout.h"
#include "remglktexteditwindow.h"

#include <QDebug>
#include <QTimer>

#include "glk.h"
#include <QMap>
#include "glkwindowdata.h"


RemGlkOutputLayout::RemGlkOutputLayout(QWidget *parent)
    : QObject(parent)
{
    qDebug() << "~~~! RemGlkOtputLayout constructor";
}


void RemGlkOutputLayout::setOutputWidgets(RemGlkCompleterTextEdit *textEdit, QGridLayout *mainLayout, QLabel *topStatus)
{
    this->textEdit = textEdit;
    this->mainLayout = mainLayout;
    this->topStatus = topStatus;
}


QString RemGlkOutputLayout::toStringJsonObject(const QJsonObject jsonObj) {
    QJsonDocument doc(jsonObj);
    QString strJson(doc.toJson(QJsonDocument::Compact));
    return strJson;
}

void RemGlkOutputLayout::inlineOutputRemGlkDebug(int format, QString debugOutput) {
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

/*
 * Typical RemGlk incoming data:
 * window  "{\"gridheight\":1,\"gridwidth\":80,\"height\":16,\"id\":20,\"left\":0,\"rock\":202,\"top\":0,\"type\":\"grid\",\"width\":720}"
 * window  "{\"height\":384,\"id\":17,\"left\":0,\"rock\":201,\"top\":16,\"type\":\"buffer\",\"width\":720}"
 * */
void RemGlkOutputLayout::stanzaRemGlkWindows(QJsonArray windows) {
    if (! windows.isEmpty()) {
        inlineOutputRemGlkDebug(0, " windows [");
        for(int i=0; i< windows.count(); ++i){
            QJsonObject windowObject = windows.at(i).toObject();
            int windowId = windowObject["id"].toInt();

            GlkWindowData glkWindowData;
            if (glkWindows.contains(windowId)) {
                glkWindowData = glkWindows.value(windowId);
            }

            glkWindowData.windowId = windowId;

            glkWindows.insert(windowId, glkWindowData);

            inlineOutputRemGlkDebug(0, " !! ");
            inlineOutputRemGlkDebug(0, toStringJsonObject(windowObject));
            qDebug() << "window " << toStringJsonObject(windowObject);
        }
        inlineOutputRemGlkDebug(0, "] ");
    }
    qDebug() << "%%%%%% glkWindows " << glkWindows.count();
}


// ToDo: create OutputStyledChunk object and put into window.outputChunks List
void RemGlkOutputLayout::stanzaRemGlkContentStyledChunk(QJsonObject endpointStyledChunk) {
    OutputStyledChunk styledChunk;
    if (endpointStyledChunk.contains("text")) {
        int glkStyle = style_Normal;
        QString chunkStyle = "[null]";
        if (endpointStyledChunk.contains("style")) {
            chunkStyle = endpointStyledChunk["style"].toString();
            if (chunkStyle == "input") {
                glkStyle = style_Input;
            }
            if (chunkStyle == "header") {
                glkStyle = style_Header;
            }
            if (chunkStyle == "subheader") {
                glkStyle = style_Subheader;
            }
        }

        const QString text(endpointStyledChunk["text"].toString());
        switch (glkStyle) {
        case style_Normal:
            textEdit->insertPlainText(text);
            textEdit->insertPlainText(" ");
            break;
        case style_Input:
            // save
            fw = textEdit->fontWeight();
            tc = textEdit->textColor();
            saveFontPointSize = textEdit->fontPointSize();
            // append
            textEdit->setFontWeight(QFont::DemiBold);
            textEdit->setFontPointSize(saveFontPointSize * 1.05);
            textEdit->setTextColor(QColor("DarkGreen"));
            textEdit->insertPlainText(text);
            // restore
            textEdit->setFontWeight(fw);
            textEdit->setTextColor(tc);
            textEdit->setFontPointSize(saveFontPointSize);
            break;
        case style_Header:
            // save
            fw = textEdit->fontWeight();
            tc = textEdit->textColor();
            saveFontPointSize = textEdit->fontPointSize();
            // append
            textEdit->setFontWeight(QFont::DemiBold);
            textEdit->setFontPointSize(saveFontPointSize * 1.3);
            textEdit->setTextColor(QColor("red"));
            textEdit->insertPlainText(text);
            // restore
            textEdit->setFontWeight(fw);
            textEdit->setTextColor(tc);
            textEdit->setFontPointSize(saveFontPointSize);
            break;
        case style_Subheader:
            // save
            fw = textEdit->fontWeight();
            tc = textEdit->textColor();
            saveFontPointSize = textEdit->fontPointSize();
            // append
            textEdit->setFontWeight(QFont::DemiBold);
            textEdit->setFontPointSize(saveFontPointSize * 1.125);
            textEdit->setTextColor(QColor("Black"));
            textEdit->insertPlainText(text);
            // restore
            textEdit->setFontWeight(fw);
            textEdit->setTextColor(tc);
            textEdit->setFontPointSize(saveFontPointSize);
            break;
        default:
            textEdit->insertHtml(" <b>UNEXPECTED_STYLE:</b> ");
            textEdit->insertPlainText(chunkStyle);
            textEdit->insertPlainText(" ");
            textEdit->insertPlainText(text);
            textEdit->insertPlainText(" ");
            break;
        }

        styledChunk.glkStyle = glkStyle;
        //qDebug() << "content chunk " << toStringJsonObject(endpointStyledChunk);
    } else {
        //qDebug() << "content chunk NO-text " << toStringJsonObject(endpointStyledChunk);
    }
}


QString RemGlkOutputLayout::stanzaRemGlkContentStyledChunkToHTML(QJsonObject endpointStyledChunk) {
    QString outHTML = "<b>ERROR: MISSING CHUNK TEXT</b>";
    if (endpointStyledChunk.contains("text")) {
        int glkStyle = style_Normal;
        QString chunkStyle = "[null]";
        if (endpointStyledChunk.contains("style")) {
            chunkStyle = endpointStyledChunk["style"].toString();
            if (chunkStyle == "input") {
                glkStyle = style_Input;
            }
            if (chunkStyle == "header") {
                glkStyle = style_Header;
            }
            if (chunkStyle == "subheader") {
                glkStyle = style_Subheader;
            }
        }

        const QString text(endpointStyledChunk["text"].toString());
        outHTML = text;

        return outHTML;
//****---***** ends here ****---*****
        switch (glkStyle) {
        case style_Normal:
            textEdit->insertPlainText(text);
            textEdit->insertPlainText(" ");
            break;
        case style_Input:
            // save
            fw = textEdit->fontWeight();
            tc = textEdit->textColor();
            saveFontPointSize = textEdit->fontPointSize();
            // append
            textEdit->setFontWeight(QFont::DemiBold);
            textEdit->setFontPointSize(saveFontPointSize * 1.05);
            textEdit->setTextColor(QColor("DarkGreen"));
            textEdit->insertPlainText(text);
            // restore
            textEdit->setFontWeight(fw);
            textEdit->setTextColor(tc);
            textEdit->setFontPointSize(saveFontPointSize);
            break;
        case style_Header:
            // save
            fw = textEdit->fontWeight();
            tc = textEdit->textColor();
            saveFontPointSize = textEdit->fontPointSize();
            // append
            textEdit->setFontWeight(QFont::DemiBold);
            textEdit->setFontPointSize(saveFontPointSize * 1.3);
            textEdit->setTextColor(QColor("red"));
            textEdit->insertPlainText(text);
            // restore
            textEdit->setFontWeight(fw);
            textEdit->setTextColor(tc);
            textEdit->setFontPointSize(saveFontPointSize);
            break;
        case style_Subheader:
            // save
            fw = textEdit->fontWeight();
            tc = textEdit->textColor();
            saveFontPointSize = textEdit->fontPointSize();
            // append
            textEdit->setFontWeight(QFont::DemiBold);
            textEdit->setFontPointSize(saveFontPointSize * 1.125);
            textEdit->setTextColor(QColor("Black"));
            textEdit->insertPlainText(text);
            // restore
            textEdit->setFontWeight(fw);
            textEdit->setTextColor(tc);
            textEdit->setFontPointSize(saveFontPointSize);
            break;
        default:
            textEdit->insertHtml(" <b>UNEXPECTED_STYLE:</b> ");
            textEdit->insertPlainText(chunkStyle);
            textEdit->insertPlainText(" ");
            textEdit->insertPlainText(text);
            textEdit->insertPlainText(" ");
            break;
        }

        //qDebug() << "content chunk " << toStringJsonObject(endpointStyledChunk);
    } else {
        //qDebug() << "content chunk NO-text " << toStringJsonObject(endpointStyledChunk);
    }

    return outHTML;
}


/*
  {"id":20, "clear":true, "text": [
  {"append":true},
  {},
  {},
  {},
  {"content":[{ "style":"normal", "text":"When the seventh day comes and it is time for you to return to the castle in the forest, your sisters cling to your sleeves."}]},
  {},
  {"content":[{ "style":"normal", "text":"\"Don't go back,\" they say, and \"When will we ever see you again?\" But you imagine they will find consolation somewhere."}]},
  {},
  {"content":[{ "style":"normal", "text":"Your father hangs back, silent and moody. He has spent the week as far from you as possible, working until late at night. Now he speaks only to ask whether the Beast treated you \"properly.\" Since he obviously has his own ideas about what must have taken place over the past few years, you do not reply beyond a shrug."}]},
  {},
  {"content":[{ "style":"normal", "text":"You breathe more easily once you're back in the forest, alone."}]},
  {},
  {},
  {},
  {"content":[{ "style":"header", "text":"Bronze"}]},
  {"content":[{ "style":"normal", "text":"A fractured fairy tale by Emily Short, updated to 6M62 by Shin"}]},
  {"content":[{ "style":"normal", "text":"Release 12 / Serial number 161003 / Inform 7 build 6M62 (I6/v6.33 lib 6/12N) "}]},
  {},
  {"content":[{ "style":"normal", "text":"Have you played interactive fiction before? >"}]}
 ] }
 * */
void RemGlkOutputLayout::stanzaRemGlkContent(QJsonArray content) {
    if (! content.isEmpty()) {
        inlineOutputRemGlkDebug(0, "content [\n");
        for(int i=0; i< content.count(); ++i){
            inlineOutputRemGlkDebug(1, "  &nbsp; <font color=\"Brown\">!!</font> ");
            QJsonObject contentWindow = content.at(i).toObject();
            if (contentWindow.contains("text")) {
                QJsonArray contentWindowChunks = contentWindow["text"].toArray();
                if (! contentWindowChunks.isEmpty()) {
                    for(int c=0; c< contentWindowChunks.count(); ++c){
                        // "pileOfChunks" is basically a paragraph of story / book. It can be nothing more than a newline, an empty paragraph.
                        QJsonObject pileOfChunks = contentWindowChunks.at(c).toObject();
                        if (pileOfChunks.isEmpty()) {
                            inlineOutputRemGlkDebug(0, "   ~NL~  ");
                            // Put in Newline, pargraph break.
                            textEdit->insertPlainText("\n");
                            qDebug() << "content chunk is newline";
                        } else {
                            // One newline per set of StyledChunks, unless append indicates otherwise.
                            if (! pileOfChunks.contains("append")) {
                                textEdit->insertPlainText("\n");
                            }
                            if (pileOfChunks.contains("content")) {
                                QJsonArray setStyledChunks = pileOfChunks["content"].toArray();
                                for(int ssc=0; ssc < setStyledChunks.count(); ++ssc){
                                    stanzaRemGlkContentStyledChunk(setStyledChunks.at(ssc).toObject());
                                }
                            }
                        }
                    }
                }
            } else {
                if (contentWindow.contains("lines")) {
                    QString outAllText = "";
                    int highestLineNumber = 0;
                    QJsonArray contentWindowLines = contentWindow["lines"].toArray();
                    for(int line=0; line < contentWindowLines.count(); ++line){
                        QJsonObject oneLineObject = contentWindowLines.at(line).toObject();
                        QJsonArray oneLinePileOfChunks = oneLineObject["content"].toArray();
                        int lineNumber = oneLineObject["line"].toInt();
                        if (lineNumber > highestLineNumber) {
                            highestLineNumber = lineNumber;
                        }
                        if (line > 0) {
                            outAllText += "\n";
                        }
                        if (glkWindowDebug_TextGrid) {
                            outAllText += "line " + QString::number(lineNumber) + ": ";
                        }
                        for(int ssc=0; ssc < oneLinePileOfChunks.count(); ++ssc){
                            outAllText += stanzaRemGlkContentStyledChunkToHTML(oneLinePileOfChunks.at(ssc).toObject());
                        }
                    }

                    // ToDo: Study the TextGrid window, and only consolidate if it is one-line "classic" style.
                    // ToDo: setting to enable or disable.
                    if (highestLineNumber == 0)
                    {
                        // For 5" phone in portrait display mode, the story "She's Actual Size" shows unneeded line wrapping.
                        //  This can consolidate spaces.
                        //  ToDo: know how long our lines are and skip, desktop resized and phone small alike.
                        if (glkGridWindowRuntimeConsolidate != "") {
                            // simplified should consolidate whitespaces to a single space.
                            // outAllText.simplified();
                            // Replace with interpunct to give visual hint of change
                            outAllText.replace(QRegExp( "[ \t]{4,}" ), " · ");
                            // outAllText.replace("···", " · ");
                        }
                    }

                    topStatus->setText(outAllText);
                    topStatus->setWordWrap(true);
                } else {
                    qDebug() << "#####::: non-text non-lines content " << toStringJsonObject(contentWindow);
                    /*
                     * "{\"id\":21,\"lines\":[{\"content\":[{\"style\":\"normal\",\"text\":
                     *      \"                                                                                \"}]
                     * ,\"line\":0}]}"
                     * */
                }
            }
        }
        inlineOutputRemGlkDebug(0, "] ");
    }
}
