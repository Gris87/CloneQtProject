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

    quint8 fileProgress  = 0;
    quint8 totalProgress = 0;

    while (!mTerminated)
    {
        msleep(200);

        ++fileProgress;
        totalProgress+=3;

        emit OnProgressChanged(fileProgress, totalProgress);

        if (totalProgress>100)
        {
            break;
        }
    }
}
