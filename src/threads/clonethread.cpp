#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QDebug>

#include "clonethread.h"



CloneThread::CloneThread(const QString &pathToProFile, const QString &destinationPath, QObject *parent)
    : QThread(parent)
    , mTerminated(false)
    , mPathToProFile(QDir::fromNativeSeparators(pathToProFile))
    , mDestinationPath(QDir::fromNativeSeparators(destinationPath))
{
    QString projectName = mPathToProFile.mid(mPathToProFile.lastIndexOf("/")+1);
    projectName         = projectName.left(projectName.lastIndexOf("."));

    mDestinationPath.append("/");
    mDestinationPath.append(projectName);
}

void CloneThread::stop()
{
    mTerminated = true;
}

void CloneThread::run()
{
    QStringList files;

    // Get files list from pro file
    {
        QFile file(mPathToProFile);

        if (file.exists())
        {
            file.open(QIODevice::ReadOnly);

            QTextStream stream(&file);
            stream.setCodec("UTF-8");

            bool addingMode = false;

            while (!mTerminated && !stream.atEnd())
            {
                QString line = stream.readLine().simplified();

                if (
                    !addingMode
                    &&
                    (
                     line.startsWith("SOURCES")
                     ||
                     line.startsWith("HEADERS")
                     ||
                     line.startsWith("FORMS")
                    )
                   )
                {
                    int index = line.indexOf("=");

                    if (index < 0)
                    {
                        mError = tr("There is no \"=\" sign in line: %1").arg(line);
                        qCritical() << "Error:" << mError;

                        return;
                    }

                    line.remove(0, index + 1);

                    addingMode = true;
                }

                if (addingMode)
                {
                    QString newFile;

                    if (line.endsWith("\\"))
                    {
                        line.remove(line.length() - 1, 1);
                        newFile = line.trimmed();
                    }
                    else
                    {
                        addingMode = false;
                        newFile = line;
                    }

                    if (newFile != "")
                    {
                        files.append(newFile);
                    }
                }
            }

            file.close();
        }
    }

    if (mTerminated)
    {
        return;
    }

    if (files.length() == 0)
    {
        mError = tr("There is no files in pro file");
        qCritical() << "Error:" << mError;

        return;
    }

    files.sort(Qt::CaseInsensitive);

    qDebug() << "Files to clone:";

    for (int i=0; i<files.length(); ++i)
    {
        qDebug() << files.at(i);
    }

    qDebug() << "";

    QStringList absoluteSourceFiles;
    QStringList absoluteDestinationFiles;

    // Get absolute source and destination paths
    {
        if (QDir::isAbsolutePath(files.at(0)))
        {
            for (int i=1; i<files.length(); ++i)
            {
                if (!QDir::isAbsolutePath(files.at(i)))
                {
                    mError = tr("All paths should be absolute");
                    qCritical() << "Error:" << mError;

                    return;
                }
            }

            absoluteSourceFiles = files;
        }
        else
        {
            QString projectFolder = mPathToProFile.left(mPathToProFile.lastIndexOf("/"));

            for (int i=0; i<files.length(); ++i)
            {
                const QString &file = files.at(i);

                absoluteSourceFiles.append(     projectFolder    + "/" + file);
                absoluteDestinationFiles.append(mDestinationPath + "/" + file);
            }
        }
    }

    qDebug() << "Absolute source files:";

    for (int i=0; i<absoluteSourceFiles.length(); ++i)
    {
        qDebug() << absoluteSourceFiles.at(i);
    }

    qDebug() << "";

    qDebug() << "Absolute destination files:";

    for (int i=0; i<absoluteDestinationFiles.length(); ++i)
    {
        qDebug() << absoluteDestinationFiles.at(i);
    }

    qDebug() << "";

    quint64 totalSize = 0;

    // Get total size
    {
        // TODO: Implement it
    }

    // TODO: Implement it

    quint8 fileProgress  = 0;
    quint8 totalProgress = 0;

    while (!mTerminated)
    {
        msleep(20);

        ++fileProgress;
        totalProgress += 3;

        emit OnProgressChanged(fileProgress, totalProgress);

        if (totalProgress>100)
        {
            break;
        }
    }
}

const QString& CloneThread::getError() const
{
    return mError;
}
