#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtGui>

#include "shuffler.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
private slots:
    void on_btOpenDir_clicked();
    void on_rbPrefixNew_toggled(bool checked);
    void on_rbPrefixExisting_toggled(bool checked);
    void on_actionQuit_activated();
    void on_actionAbout_activated();
    void on_btRename_clicked();
    void on_btReload_clicked();
    void on_chkRecursive_toggled(bool checked);
    void on_chkCreateSubDirs_toggled(bool checked);
    void onSettingsChanged();
    void onStatusUpdate(QString msg, int progress);

private:
    Ui::MainWindow *ui;
    QProgressBar *progressBar;
    Shuffler *shuffler;
    QFileInfoList *fileList;
    QString path;

    int minFileNameLength(QFileInfoList *fileList);
    QStringList fileNameList(QFileInfoList *fileList);
    void readDirectory(QString &path);
};

#endif // MAINWINDOW_H
