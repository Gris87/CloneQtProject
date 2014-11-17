#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QDir>
#include <QTextStream>

#include "mainwindow.h"
#include "ui_mainwindow.h"



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , mCloneThread(0)
    , mBatchMode(false)
{
    ui->setupUi(this);

    setWindowFlags(
                    Qt::Window
                    | Qt::MSWindowsFixedSizeDialogHint
                  );

    loadData();
}

MainWindow::~MainWindow()
{
    stopCloneThread();

    delete ui;
}

void MainWindow::OnProgressChanged(const QString &file, const quint8 fileProgress, const quint8 totalProgress)
{
    ui->fileNameLabel->setText(QDir::toNativeSeparators(file));

    ui->fileProgressBar->setValue(fileProgress);
    ui->totalProgressBar->setValue(totalProgress);
}

void MainWindow::OnCloneThreadFinished()
{
    if (mCloneThread && mCloneThread->getError() != "")
    {
        QMessageBox::critical(this, tr("Error"), mCloneThread->getError());
    }

    stop();
}

void MainWindow::on_proFileButton_clicked()
{
    QFileDialog dialog(this, tr("Open file"), ui->proFileComboBox->currentText());

    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setFileMode(QFileDialog::ExistingFile);

    QStringList filters;
    filters.append(tr("Qt project") + " (*.pro)");
    filters.append(tr("Any file")   + " (*.*)");

    dialog.setNameFilters(filters);

    dialog.setDefaultSuffix("pro");

    if (dialog.exec())
    {
        insertTextToComboBox(ui->proFileComboBox, QDir::toNativeSeparators(dialog.selectedFiles().at(0)));

        saveData();
    }
}

void MainWindow::on_destinationButton_clicked()
{
    QFileDialog dialog(this, tr("Open Directory"), ui->destinationComboBox->currentText());

    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setOption(QFileDialog::ShowDirsOnly);
    dialog.setOption(QFileDialog::DontUseNativeDialog);

    if (dialog.exec())
    {
        insertTextToComboBox(ui->destinationComboBox, QDir::toNativeSeparators(dialog.selectedFiles().at(0)));

        saveData();
    }
}

void MainWindow::on_startButton_clicked()
{
    if (ui->settingsGroupBox->isEnabled())
    {
        start();
    }
    else
    {
        stop();
    }
}

bool MainWindow::batchClone(const QString &pathToProFile, const QString &destinationPath)
{
    mBatchMode = true;

    insertTextToComboBox(ui->proFileComboBox,     QDir::toNativeSeparators(pathToProFile));
    insertTextToComboBox(ui->destinationComboBox, QDir::toNativeSeparators(destinationPath));

    return start();
}

void MainWindow::stopCloneThread()
{
    if (mCloneThread)
    {
        mCloneThread->blockSignals(true);
        mCloneThread->stop();
        mCloneThread->wait();
        delete mCloneThread;
        mCloneThread = 0;
    }
}

bool MainWindow::start()
{
    QString pathToProFile   = ui->proFileComboBox->currentText();
    QString destinationPath = ui->destinationComboBox->currentText();

    if (pathToProFile == "")
    {
        QMessageBox::information(this, tr("Select pro file"), tr("Please select Qt project file"));
        return false;
    }

    if (destinationPath == "")
    {
        QMessageBox::information(this, tr("Select destination path"), tr("Please select destination path"));
        return false;
    }

    if (!QFile::exists(pathToProFile))
    {
        QMessageBox::information(this, tr("Pro file not exists"), tr("Qt project file \"%1\" is not found").arg(pathToProFile));
        return false;
    }

    if (!QDir(destinationPath).exists())
    {
        QMessageBox::information(this, tr("Destination path not exists"), tr("Destination path \"%1\" is not found").arg(destinationPath));
        return false;
    }

    if (
        ui->proFileComboBox->currentIndex() != 0
        ||
        ui->destinationComboBox->currentIndex() != 0
       )
    {
        ui->proFileComboBox->removeItem(ui->proFileComboBox->currentIndex());
        ui->destinationComboBox->removeItem(ui->destinationComboBox->currentIndex());

        ui->proFileComboBox->insertItem(0, pathToProFile);
        ui->destinationComboBox->insertItem(0, destinationPath);

        ui->proFileComboBox->setCurrentIndex(0);
        ui->destinationComboBox->setCurrentIndex(0);

        saveData();
    }

    ui->settingsGroupBox->setEnabled(false);
    ui->startButton->setText(tr("Stop"));    

    mCloneThread = new CloneThread(pathToProFile, destinationPath, this);
    connect(mCloneThread, SIGNAL(OnProgressChanged(QString,quint8,quint8)), this, SLOT(OnProgressChanged(QString,quint8,quint8)));
    connect(mCloneThread, SIGNAL(finished()), this, SLOT(OnCloneThreadFinished()));
    mCloneThread->start(QThread::TimeCriticalPriority);

    return true;
}

void MainWindow::stop()
{
    stopCloneThread();

    ui->settingsGroupBox->setEnabled(true);

    ui->fileNameLabel->setText("-");
    ui->fileProgressBar->setValue(0);
    ui->totalProgressBar->setValue(0);

    ui->startButton->setText(tr("Start"));

    if (mBatchMode)
    {
        close();
    }
}

void MainWindow::insertTextToComboBox(QComboBox *comboBox, const QString &text)
{
    int index = comboBox->findText(text);

    if (index >= 0)
    {
        comboBox->removeItem(index);
    }

    comboBox->insertItem(0, text);
    comboBox->setCurrentIndex(0);
}

void MainWindow::loadToComboBox(QComboBox *comboBox, const QString &fileName)
{
    QFile file(fileName);

    if (file.exists())
    {
        file.open(QIODevice::ReadOnly);

        QTextStream stream(&file);
        stream.setCodec("UTF-8");

        while (!stream.atEnd())
        {
            QString line = stream.readLine().trimmed();

            if (line != "")
            {
                comboBox->addItem(QDir::toNativeSeparators(line));
            }
        }

        file.close();

        comboBox->setCurrentIndex(0);
    }
}

void MainWindow::saveFromComboBox(QComboBox *comboBox, const QString &fileName)
{
    QFile file(fileName);
    file.open(QIODevice::WriteOnly);

    QTextStream stream(&file);
    stream.setGenerateByteOrderMark(true);
    stream.setCodec("UTF-8");

    for (int i=0; i<comboBox->count(); ++i)
    {
        stream << comboBox->itemText(i) << "\n";
    }

    file.close();
}

void MainWindow::loadData()
{
    QString dir = QApplication::applicationDirPath();

    loadToComboBox(ui->proFileComboBox,     dir + "/pro_files.txt");
    loadToComboBox(ui->destinationComboBox, dir + "/destinations.txt");
}

void MainWindow::saveData()
{
    QString dir = QApplication::applicationDirPath();

    saveFromComboBox(ui->proFileComboBox,     dir + "/pro_files.txt");
    saveFromComboBox(ui->destinationComboBox, dir + "/destinations.txt");
}
