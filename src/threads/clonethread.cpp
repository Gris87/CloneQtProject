#include "clonethread.h"

CloneThread::CloneThread(const QString &pathToProFile, const QString &destinationPath, QObject *parent)
    : QThread(parent)
    , mTerminated(false)
    , mPathToProFile(pathToProFile)
    , mDestinationPath(destinationPath)
{
}

void CloneThread::stop()
{
    mTerminated = true;
}

void CloneThread::run()
{
    // TODO: Implement it

    while (!mTerminated)
    {
        msleep(1000);
    }
}
