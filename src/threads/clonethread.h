#ifndef CLONETHREAD_H
#define CLONETHREAD_H

#include <QThread>

class CloneThread : public QThread
{
    Q_OBJECT
public:
    explicit CloneThread(QObject *parent = 0);

};

#endif // CLONETHREAD_H
