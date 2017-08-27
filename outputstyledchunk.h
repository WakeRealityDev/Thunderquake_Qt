#ifndef OUTPUTSTYLEDCHUNK_H
#define OUTPUTSTYLEDCHUNK_H

class OutputStyledChunk
{
public:
    explicit OutputStyledChunk();

public:
    int windowId = -1;
    int type = -1;
    int rockId = -1;
    int gridLines = -1;
    int height = -1;
    int width = -1;

    // default to normal
    int glkStyle = 0;
};

#endif // OUTPUTSTYLEDCHUNK_H
