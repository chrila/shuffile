#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->chkNewInsideFolders->setEnabled(ui->chkRecursive->isChecked());

    ui->spinFilesPerDir->setEnabled(ui->chkCreateSubDirs->isChecked());
    ui->edDirectoryPrefix->setEnabled(ui->chkCreateSubDirs->isChecked());
    ui->spinFilesPerDir->setMinimum(1);

    progressBar = new QProgressBar(this);
    progressBar->setVisible(false);
    ui->statusBar->addWidget(progressBar);

    shuffler = new Shuffler(this);
    fileList = new QFileInfoList();

    path = "~";
    setWindowTitle("Shuffile");

    connect(ui->rbPrefixExisting, SIGNAL(toggled(bool)), this, SLOT(onSettingsChanged()));
    connect(ui->rbPrefixNew, SIGNAL(toggled(bool)), this, SLOT(onSettingsChanged()));
    connect(ui->spinNoDigits, SIGNAL(valueChanged(int)), this, SLOT(onSettingsChanged()));
    connect(ui->edSeparator, SIGNAL(textChanged(QString)), this, SLOT(onSettingsChanged()));
    connect(ui->listFiles, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(onSettingsChanged()));
    connect(shuffler, SIGNAL(statusUpdate(QString,int)), this, SLOT(onStatusUpdate(QString,int)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::readDirectory(QString &path)
{
    ui->listFiles->clear();

    if (shuffler->readDirectory(path, ui->chkRecursive->isChecked()))
    {
        fileList = shuffler->getFileList();
        int minLength = minFileNameLength(fileList);

        ui->listFiles->addItems(fileNameList(fileList));
        ui->listFiles->setEnabled(true);
        ui->widgetRightSide->setEnabled(true);
        ui->chkDeleteOldDirs->setEnabled(ui->chkRecursive->isChecked() && ui->chkCreateSubDirs->isChecked());

        int fileCount = shuffler->getFileList()->size();
        ui->lbFileCountValue->setText(QString("%1").arg(fileCount));
        int minDigits = ui->lbFileCountValue->text().length();
        ui->spinNoDigits->setMinimum(minDigits);
        ui->spinNoDigits->setMaximum(minLength);
        ui->spinNoDigits->setValue(minDigits);

        if (minDigits > minLength)
            ui->rbPrefixExisting->setEnabled(false);
    } else
    {
        ui->widgetRightSide->setEnabled(false);
    }
}

void MainWindow::on_btOpenDir_clicked()
{
    path = QFileDialog::getExistingDirectory(this, tr("Choose a directory"), path, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    readDirectory(path);
}

void MainWindow::on_rbPrefixNew_toggled(bool checked)
{
    if (checked)
        ui->rbPrefixExisting->setChecked(false);
}

void MainWindow::on_rbPrefixExisting_toggled(bool checked)
{
    if (checked)
        ui->rbPrefixNew->setChecked(false);
}

void MainWindow::on_actionQuit_activated()
{
    exit(0);
}

void MainWindow::on_actionAbout_activated()
{
    QMessageBox::about(this, "About Shuffile", "Shuffile v0.2\n\n(c) 2012-2013 by Christian Lampl <christian.lampl@gmx.at>");
}

void MainWindow::on_btRename_clicked()
{
    progressBar->setMinimum(0);
    progressBar->setMaximum(fileList->size());

    if (ui->chkDeleteOldDirs->isChecked())
    {
        shuffler->moveFilesToTopDir();
        shuffler->deleteOldDirs();
    }

    shuffler->readDirectory(path, false);
    shuffler->shuffleFiles(ui->spinNoDigits->value(), ui->edSeparator->text(), ui->rbPrefixNew->isChecked(), ui->chkNewInsideFolders->isChecked());

    if (ui->chkCreateSubDirs->isChecked())
    {
        int filesPerDir = ui->spinFilesPerDir->value();
        shuffler->createDirectories(filesPerDir, new QString(ui->edDirectoryPrefix->text()));
        shuffler->moveFilesToDirectories(filesPerDir);
    }

    readDirectory(path);
}

void MainWindow::on_btReload_clicked()
{
    readDirectory(path);
}

void MainWindow::on_chkRecursive_toggled(bool checked)
{
    ui->chkNewInsideFolders->setEnabled(checked);
    ui->chkDeleteOldDirs->setEnabled(checked && ui->chkCreateSubDirs->isChecked());
}

void MainWindow::on_chkCreateSubDirs_toggled(bool checked)
{
    ui->spinFilesPerDir->setEnabled(checked);
    ui->edDirectoryPrefix->setEnabled(checked);

    ui->chkDeleteOldDirs->setEnabled(checked && ui->chkRecursive->isChecked());
}

void MainWindow::onSettingsChanged()
{
    // update example
    if (fileList)
    {
        ui->lbExampleValue->setText(shuffler->newFileName(
                                        fileList->at(0).fileName(),
                                        1,
                                        ui->spinNoDigits->value(),
                                        ui->edSeparator->text(),
                                        ui->rbPrefixNew->isChecked()));
    }
}

void MainWindow::onStatusUpdate(QString msg, int progress)
{
    ui->statusBar->showMessage(msg);

    if (progress > 0)
    {
        progressBar->setVisible(true);
        progressBar->setValue(progress);
    } else
    {
        progressBar->setVisible(false);
    }
}

int MainWindow::minFileNameLength(QFileInfoList *list)
{
    if (list->length() == 0)
        return 0;

    int minLength = list->at(0).fileName().length();

    foreach (QFileInfo info, *list)
    {
        if (info.fileName().length() < minLength)
            minLength = info.fileName().length();
    }

    return minLength;
}

QStringList MainWindow::fileNameList(QFileInfoList *list)
{
    QStringList *strlist = new QStringList();

    foreach (QFileInfo inf, *list)
    {
        strlist->append(inf.fileName());
    }

    return *strlist;
}
