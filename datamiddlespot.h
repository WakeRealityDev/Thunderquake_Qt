#ifndef DATAMIDDLESPOT_H
#define DATAMIDDLESPOT_H

#include <webviewwindow.h>
#include "findfileswindow.h"
#include "findstorieswindow.h"
#include "remglkprocess.h"

class DataMiddleSpot
{
public:
    DataMiddleSpot();

    WebViewWindow webViewWindow;

    FindFilesWindow findFilesWindow;
    FindStoriesWindow findStoriesWindow;

    RemGlkProcess engineProcess;
};

#endif // DATAMIDDLESPOT_H
