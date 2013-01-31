#ifndef SHUFFLER_H
#define SHUFFLER_H

#include <QObject>
#include <QFile>
#include <QDir>
#include <QFileInfoList>
#include <QStringList>
#include <QDebug>
#include <QTime>
#include <QMap>
#include <math.h>
#include <stdio.h>

using namespace std;

/**
  * Class Shuffler
  *
  * Provides methods for reading a directory and shuffling the names of the contained files.
  */

class Shuffler : public QObject
{
    Q_OBJECT

private:
    QFileInfoList *fileList;
    QDir dir;
    QMap<QString, int> *sequence;
    int globalSequence;

    int nextSeqEntry(QString *dir);

public:
    explicit Shuffler(QObject *parent = 0);
    bool readDirectory(QString &path, bool recursive);
    QFileInfoList *getFileList();

    QString newFileName(const QString filename, int n, int digits, QString sep, bool newPrefix);
    void shuffleFiles(int digits, QString sep, bool newPrefix, bool newInsideFolders);
    void createDirectories(int filesPerDir, QString *dirPrefix);
    void moveFilesToDirectories(int filesPerDir);
    void deleteOldDirs();
    void moveFilesToTopDir();
    
signals:
    void statusUpdate(QString msg, int progress);
};

#endif // SHUFFLER_H
