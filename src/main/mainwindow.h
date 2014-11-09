#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QComboBox>

#include "threads/clonethread.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    CloneThread    *mCloneThread;

    void stopCloneThread();

    void start();
    void stop();

    void loadToComboBox(QComboBox *comboBox, const QString &fileName);
    void saveFromComboBox(QComboBox *comboBox, const QString &fileName);

    void loadData();
    void saveData();

private slots:
    void OnProgressChanged(const quint8 fileProgress, const quint8 totalProgress);
    void OnCloneThreadFinished();

    void on_proFileButton_clicked();
    void on_destinationButton_clicked();
    void on_startButton_clicked();
};

#endif // MAINWINDOW_H
