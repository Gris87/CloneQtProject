#ifndef CLONETHREAD_H
#define CLONETHREAD_H

#include <QThread>



class CloneThread : public QThread
{
    Q_OBJECT
public:
    explicit CloneThread(const QString &pathToProFile, const QString &destinationPath, QObject *parent = 0);

    void stop();

    const QString& getError() const;

protected:
    void run();

private:
    bool    mTerminated;
    QString mPathToProFile;
    QString mDestinationPath;
    QString mError;

    bool getFiles(QStringList &files);
    bool getAbsolutePaths(const QStringList &files, QStringList &absoluteSourceFiles, QStringList &absoluteDestinationFiles);
    void printLists(const QStringList &files, const QStringList &absoluteSourceFiles, const QStringList &absoluteDestinationFiles);
    quint64 getTotalSize(const QStringList &absoluteSourceFiles);
    bool cloneFiles(const QStringList &absoluteSourceFiles, const QStringList &absoluteDestinationFiles, const quint64 totalSize);

signals:
    void OnProgressChanged(const QString &file, const quint8 fileProgress, const quint8 totalProgress);
};

#endif // CLONETHREAD_H
