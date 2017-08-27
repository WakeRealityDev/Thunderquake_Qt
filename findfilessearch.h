#ifndef FINDFILESSEARCH_H
#define FINDFILESSEARCH_H

#include <QObject>

class FindFilesSearch : public QObject
{
    Q_OBJECT
public:
    explicit FindFilesSearch(QObject *parent = nullptr);

signals:

public slots:
};

#endif // FINDFILESSEARCH_H