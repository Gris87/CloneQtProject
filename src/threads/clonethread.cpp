#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QDateTime>
#include <QDebug>

#include "clonethread.h"



#define BUFFER_SIZE 4096



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

bool CloneThread::getFiles(QStringList &files)
{
    QFile file(mPathToProFile);

    if (file.exists())
    {
        if (!file.open(QIODevice::ReadOnly))
        {
            mError = tr("Impossible to open pro file");
            qCritical() << "Error:" << mError;

            return false;
        }

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
                    mError = tr("There is no \"=\" sign in pro file in the line: %1").arg(line);
                    qCritical() << "Error:" << mError;

                    return false;
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
                }
                else
                {
                    addingMode = false;
                }

                newFile = line.trimmed();

                if (newFile != "")
                {
                    files.append(newFile);
                }
            }
        }

        file.close();

        if (files.length() == 0)
        {
            mError = tr("There is no files in pro file");
            qCritical() << "Error:" << mError;

            return false;
        }

        files.sort(Qt::CaseInsensitive);
    }
    else
    {
        mError = tr("Pro file not found");
        qCritical() << "Error:" << mError;

        return false;
    }

    return !mTerminated;
}

QString CloneThread::getCommonPath(const QStringList &files)
{
    const QString &firstFile = files.at(0);

    int slashPos = firstFile.length();

    while (!mTerminated)
    {
        slashPos = firstFile.lastIndexOf("/", slashPos - 1);

        if (slashPos < 0)
        {
            break;
        }

        QString base = firstFile.left(slashPos + 1);

        bool good = true;

        for (int i=1; !mTerminated && i<files.length(); ++i)
        {
            if (!files.at(i).startsWith(base))
            {
                good = false;
                break;
            }
        }

        if (good)
        {
            return base;
        }
    }

    return "";
}

bool CloneThread::getAbsolutePaths(const QStringList &files, QStringList &absoluteSourceFiles, QStringList &absoluteDestinationFiles)
{
    if (QDir::isAbsolutePath(files.at(0)))
    {
        for (int i=1; !mTerminated && i<files.length(); ++i)
        {
            if (!QDir::isAbsolutePath(files.at(i)))
            {
                mError = tr("All paths should be absolute");
                qCritical() << "Error:" << mError;

                return false;
            }
        }

        QString commonPath   = getCommonPath(files);
        int     commonLength = commonPath.length();

        if (mTerminated)
        {
            return false;
        }

        if (commonLength == 0)
        {
            mError = tr("There is no common path in source files");
            qCritical() << "Error:" << mError;

            return false;
        }

        absoluteSourceFiles = files;

        for (int i=0; !mTerminated && i<files.length(); ++i)
        {
            absoluteDestinationFiles.append(mDestinationPath + "/" + files.at(i).mid(commonLength));
        }
    }
    else
    {
        for (int i=1; !mTerminated && i<files.length(); ++i)
        {
            if (QDir::isAbsolutePath(files.at(i)))
            {
                mError = tr("All paths should be relative");
                qCritical() << "Error:" << mError;

                return false;
            }
        }

        QString projectFolder = mPathToProFile.left(mPathToProFile.lastIndexOf("/"));

        for (int i=0; !mTerminated && i<files.length(); ++i)
        {
            const QString &file = files.at(i);

            absoluteSourceFiles.append(     projectFolder    + "/" + file);
            absoluteDestinationFiles.append(mDestinationPath + "/" + file);
        }
    }

    return !mTerminated;
}

void CloneThread::printLists(const QStringList &files, const QStringList &absoluteSourceFiles, const QStringList &absoluteDestinationFiles)
{
    qDebug() << "Files to clone:";

    for (int i=0; !mTerminated && i<files.length(); ++i)
    {
        qDebug() << files.at(i);
    }

    qDebug() << "";
    qDebug() << "Absolute source files:";

    for (int i=0; !mTerminated && i<absoluteSourceFiles.length(); ++i)
    {
        qDebug() << absoluteSourceFiles.at(i);
    }

    qDebug() << "";
    qDebug() << "Absolute destination files:";

    for (int i=0; !mTerminated && i<absoluteDestinationFiles.length(); ++i)
    {
        qDebug() << absoluteDestinationFiles.at(i);
    }

    qDebug() << "";
}

quint64 CloneThread::getTotalSize(const QStringList &absoluteSourceFiles)
{
    quint64 res = 0;

    for (int i=0; !mTerminated && i<absoluteSourceFiles.length(); ++i)
    {
        res += QFile(absoluteSourceFiles.at(i)).size();
    }

    return res;
}

bool CloneThread::deleteFolder(const QString &folder)
{
    QDir folderDir(folder);

    if (!folderDir.exists())
    {
        return true;
    }

    QFileInfoList files = folderDir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);

    for (int i=0; i<files.length(); ++i)
    {
        QString filePath = files.at(i).absoluteFilePath();

        if (files.at(i).isDir())
        {
            if (!deleteFolder(filePath))
            {
                return false;
            }
        }
        else
        {
            qDebug() << "Deleting:" << filePath;

            if (!QFile::remove(filePath))
            {
                mError = tr("Impossible to remove file: %1").arg(QDir::toNativeSeparators(filePath));
                qCritical() << "Error:" << mError;

                return false;
            }
        }
    }

    qDebug() << "Deleting folder:" << folder;

    if (!QDir().rmdir(folder))
    {
        mError = tr("Impossible to remove folder: %1").arg(QDir::toNativeSeparators(folder));
        qCritical() << "Error:" << mError;

        return false;
    }

    return !mTerminated;
}

bool CloneThread::cloneFiles(const QStringList &absoluteSourceFiles, const QStringList &absoluteDestinationFiles, const quint64 totalSize)
{
    if (absoluteSourceFiles.length() != absoluteDestinationFiles.length())
    {
        mError = tr("Amount of source files not equals to amount of destination files");
        qCritical() << "Error:" << mError;

        return false;
    }

    char buffer[BUFFER_SIZE];

    quint64 totalProgress = 0;
    qint64 timeStart = QDateTime::currentMSecsSinceEpoch();

    for (int i=0; !mTerminated && i<absoluteSourceFiles.length(); ++i)
    {
        QString srcFileName  = absoluteSourceFiles.at(i);
        QString destFileName = absoluteDestinationFiles.at(i);

        qDebug() << "Cloning:" << srcFileName;

        QFile srcFile(srcFileName);
        QFile destFile(destFileName);

        if (!srcFile.exists())
        {
            mError = tr("File not found: %1").arg(QDir::toNativeSeparators(srcFileName));
            qCritical() << "Error:" << mError;

            return false;
        }

        QString destFilePath = destFileName.left(destFileName.lastIndexOf("/"));

        if (!QDir().mkpath(destFilePath))
        {
            mError = tr("Impossible to create path: %1").arg(QDir::toNativeSeparators(destFilePath));
            qCritical() << "Error:" << mError;

            return false;
        }

        if (!srcFile.open(QIODevice::ReadOnly))
        {
            mError = tr("Impossible to open file: %1").arg(QDir::toNativeSeparators(srcFileName));
            qCritical() << "Error:" << mError;

            return false;
        }

        if (!destFile.open(QIODevice::WriteOnly))
        {
            mError = tr("Impossible to create file: %1").arg(QDir::toNativeSeparators(destFileName));
            qCritical() << "Error:" << mError;

            return false;
        }

        quint64 fileProgress = 0;
        quint64 fileSize = srcFile.size();

        while (!mTerminated && !srcFile.atEnd())
        {
            qint64 bytes = srcFile.read(buffer, BUFFER_SIZE);

            if (bytes < 0)
            {
                mError = tr("Impossible to read file: %1").arg(QDir::toNativeSeparators(srcFileName));
                qCritical() << "Error:" << mError;

                return false;
            }

            if (destFile.write(buffer, bytes) != bytes)
            {
                mError = tr("Impossible to write file: %1").arg(QDir::toNativeSeparators(destFileName));
                qCritical() << "Error:" << mError;

                return false;
            }

            fileProgress  += bytes;
            totalProgress += bytes;

            if (fileSize && totalSize)
            {
                qint64 curTime = QDateTime::currentMSecsSinceEpoch();

                if (curTime > timeStart + 1000)
                {
                    timeStart = curTime;

                    emit OnProgressChanged(srcFileName, fileProgress * 100 / fileSize, totalProgress * 100 / totalSize);
                }
            }
        }

        srcFile.close();
        destFile.close();
    }

    return !mTerminated;
}

void CloneThread::run()
{
    QStringList files;
    QStringList absoluteSourceFiles;
    QStringList absoluteDestinationFiles;
    quint64     totalSize;

    // Searching
    {
        emit OnProgressChanged(tr("Searching..."), 0, 0);

        if (!getFiles(files))
        {
            return;
        }

        if (!getAbsolutePaths(files, absoluteSourceFiles, absoluteDestinationFiles))
        {
            return;
        }

        printLists(files, absoluteSourceFiles, absoluteDestinationFiles);
    }

    // Calculating
    {
        emit OnProgressChanged(tr("Calculating..."), 0, 0);

        totalSize = getTotalSize(absoluteSourceFiles);

        if (mTerminated)
        {
            return;
        }

        qDebug() << "Total size: " << totalSize;
    }

    // Deleting
    {
        emit OnProgressChanged(tr("Deleting old folder..."), 0, 0);

        if (!deleteFolder(mDestinationPath))
        {
            return;
        }
    }

    // Cloning
    {
        if (!cloneFiles(absoluteSourceFiles, absoluteDestinationFiles, totalSize))
        {
            return;
        }
    }
}

const QString& CloneThread::getError() const
{
    return mError;
}
