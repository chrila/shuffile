#include "shuffler.h"

/**
  * Constructor
  *
  * Creates a new instance of Shuffler.
  * Initializes the randomizer and the Map for the sequence numbers.
  */
Shuffler::Shuffler(QObject *parent) :
    QObject(parent)
{
    QTime time = QTime::currentTime();
    qsrand((uint)time.msec());

    sequence = new QMap<QString, int>();
    globalSequence = 0;
}

/**
  * readDirectory
  *
  * Reads all files in the given directory.
  * The method returns true, if successful and false if not.
  */
bool Shuffler::readDirectory(QString &path, bool recursive)
{
    fileList = new QFileInfoList();

    dir = QDir(path);

    if (recursive)
    {
        qDebug() << "Recursive mode.";
        // get subdirectories
        QFileInfoList subdirList = dir.entryInfoList(QDir::AllDirs|QDir::NoDot|QDir::NoDotDot);

        // iterate over subdirectories and append list of files
        foreach(QFileInfo info, subdirList)
        {
            qDebug() << "Entering directory: " << info.absoluteFilePath();
            QDir temp(info.absoluteFilePath());
            fileList->append(temp.entryInfoList(QDir::Files));
        }
    }

    fileList->append(dir.entryInfoList(QDir::Files));

    return (fileList->size() > 0);
}

/**
  * getFileList
  *
  * Returns the list of read files.
  */
QFileInfoList *Shuffler::getFileList()
{
    return fileList;
}

/**
  * newFileName
  *
  * Creates a new file name according to the given settings.
  * This method creates a filename that either has a new prefix or replaces the existing prefix
  * with the given number, amount of digits and separator.
  */
QString Shuffler::newFileName(const QString filename, int n, int digits, QString sep, bool newPrefix)
{
    QString fname = filename;
    QString prefix = QString("%1").arg(QString("%1").arg(n), digits, QLatin1Char('0'));

    if (newPrefix)
    {
        fname = prefix + sep + fname;

    } else
    {
        if (prefix.length() < fname.length())
            prefix = prefix + sep;

        fname = prefix + fname.right(fname.length()-prefix.length());
    }

    return fname;
}

/**
  * shuffleFiles
  *
  * Shuffles the file list and then renames the files on the disk accordingly.
  */
void Shuffler::shuffleFiles(int digits, QString sep, bool newPrefix, bool newInsideFolders)
{
    emit statusUpdate(tr("Shuffling file list..."), -1);

    // reset sequences
    sequence = new QMap<QString, int>();
    globalSequence = 0;

    // shuffle the file list
    for (int i=0; i<fileList->size(); i++)
    {
        int j = qrand() % fileList->size();

        QFileInfo temp = fileList->at(i);
        fileList->replace(i, fileList->at(j));
        fileList->replace(j, temp);
    }

    // rename the files according to the new list
    for (int i=0; i<fileList->size(); i++)
    {
        QString fname = fileList->at(i).absoluteFilePath();
        int nameStart = fname.lastIndexOf(QChar('/')) +1;

        int seqNo;
        if (newInsideFolders)
            seqNo = nextSeqEntry(new QString(fileList->at(i).absolutePath()));
        else
            seqNo = nextSeqEntry(NULL);

        QString fnameNew = fname.left(nameStart) + newFileName(fname.right(fname.length() - nameStart), seqNo, digits, sep, newPrefix);
        dir.rename(fname, fnameNew);

        emit statusUpdate(tr("Renaming files..."), i);
    }

    emit statusUpdate(tr("Done."), -1);
}

/**
  * nextSeqEntry
  *
  * Returns next sequence entry for the given directory.
  * The method looks in the HashMap, if there already is an entry for the given directory. If not,
  * a new entry will be created.
  * If dir is NULL, then the global sequence will be returned.
  */
int Shuffler::nextSeqEntry(QString *dir)
{
    int seq;

    // increase global sequence
    globalSequence++;

    if (dir == NULL)
        return globalSequence;

    if (sequence->contains(*dir))
    {
        seq = sequence->value(*dir) + 1;
        sequence->insert(*dir, seq);
    }
    else
    {
        sequence->insert(*dir, 1);
        seq = 1;
    }

    return seq;
}

/**
  * createDirectories
  *
  * Creates directories which can later contain the given amount of files per directory.
  */
void Shuffler::createDirectories(int filesPerDir, QString *dirPrefix)
{
    // calculate the number of needed directories
    int dirs = ceil((fileList->size()*1.0f / filesPerDir));
    qDebug() << "--> needed dirs: " << dirs;

    int digits = QString("%1").arg(dirs).length();
    for (int i=1; i<= dirs; i++)
    {
        QString newdir = *dirPrefix + QString("%1").arg(QString("%1").arg(i), digits, QLatin1Char('0'));
        dir.mkdir(newdir);
    }
}

/**
  * moveFilesToDirectories
  *
  * Moves the shuffled files to the previously created directories.
  */
void Shuffler::moveFilesToDirectories(int filesPerDir)
{
    // get subdirectories
    QFileInfoList subdirList = dir.entryInfoList(QDir::AllDirs|QDir::NoDot|QDir::NoDotDot);

    for (int i=0; i<subdirList.size(); i++)
    {
        qDebug() << "Subdirlist[" << i << "] = " << subdirList.at(i).absoluteFilePath();
        QFileInfoList files = dir.entryInfoList(QDir::Files);
        for (int j=0; j<min(files.size(), filesPerDir); j++)
        {
            qDebug() << "About to move " << files.at(j).absoluteFilePath() << "\tto " << subdirList.at(i).absoluteFilePath()+"/"+files.at(j).fileName();
            dir.rename(files.at(j).absoluteFilePath(), subdirList.at(i).absoluteFilePath()+"/"+files.at(j).fileName());
        }
    }
}

/**
  * deleteOldDirs
  *
  * Deletes the (empty) directories where the files were before.
  */
void Shuffler::deleteOldDirs()
{
    // get subdirectories
    QFileInfoList subdirList = dir.entryInfoList(QDir::AllDirs|QDir::NoDot|QDir::NoDotDot);

    // iterate over subdirectories and append list of files
    foreach(QFileInfo info, subdirList)
    {
        qDebug() << "About to delete directory: " << info.absoluteFilePath();
        dir.rmdir(info.absoluteFilePath());
    }
}

/**
  * moveFilesToTopDir
  *
  * Moves the files from the subdirectories to the top directory.
  */
void Shuffler::moveFilesToTopDir()
{
    // get subdirectories
    QFileInfoList subdirList = dir.entryInfoList(QDir::AllDirs|QDir::NoDot|QDir::NoDotDot);

    // iterate over subdirectories and append list of files
    foreach(QFileInfo info, subdirList)
    {
        QDir temp(info.absoluteFilePath());
        QFileInfoList subfiles = temp.entryInfoList(QDir::Files);

        foreach(QFileInfo file, subfiles)
        {
            qDebug() << "About to move " << file.absoluteFilePath() << " to " << dir.absolutePath()+"/"+file.fileName();
            dir.rename(file.absoluteFilePath(), dir.absolutePath()+"/"+file.fileName());
        }
    }
}
