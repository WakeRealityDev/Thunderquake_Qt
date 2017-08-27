#ifndef STORYRUNPARAMETERS_H
#define STORYRUNPARAMETERS_H

#include <QString>

class StoryRunParameters
{
public:
    StoryRunParameters();

public:
    // Interactie Fiction story Data file path
    QString storyDataPath = "";
    // The folder or directory where data files will be saved by RemGlk
    QString engineWorkingPath = "";
    int engine = 0;
};

#endif // STORYRUNPARAMETERS_H

/*
 * Qt Learning thoughts
 *    1. File open dialog for rich text - can we make it remember last folder it was in, even between restarts?
 *    2. Qt code samples to search entir Linux storage system for file extension?
 *
 * */
