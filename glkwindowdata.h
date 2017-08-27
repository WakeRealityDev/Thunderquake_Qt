#ifndef GLKWINDOWDATA_H
#define GLKWINDOWDATA_H

#include <QList>
#include "outputstyledchunk.h"

class GlkWindowData
{
public:
    explicit GlkWindowData();

public:
    int windowId = -1;
    int type = -1;
    int rockId = -1;
    int gridLines = -1;
    int height = -1;
    int width = -1;

public:
    // outputChunks keeps a list of all the output text that can be rendered in order to reproduce the window output.
    QList<OutputStyledChunk> outputChunks;
};

#endif // GLKWINDOWDATA_H
