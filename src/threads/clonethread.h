#ifndef CLONETHREAD_H
#define CLONETHREAD_H

#include <QThread>

class CloneThread : public QThread
{
    Q_OBJECT
public:
    explicit CloneThread(const QString &pathToProFile, const QString &destinationPath, QObject *parent = 0);

    void stop();

protected:
    void run();

private:
    bool    mTerminated;
    QString mPathToProFile;
    QString mDestinationPath;
};

#endif // CLONETHREAD_H
