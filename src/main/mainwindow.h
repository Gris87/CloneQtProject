#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QComboBox>

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

    void start();
    void stop();

    void loadToComboBox(QComboBox *comboBox, const QString &fileName);
    void saveFromComboBox(QComboBox *comboBox, const QString &fileName);

    void loadData();
    void saveData();

private slots:
    void on_proFileButton_clicked();
    void on_destinationButton_clicked();
    void on_startButton_clicked();
};

#endif // MAINWINDOW_H
