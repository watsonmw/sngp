#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "sngpworker.h"

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
    void reset();
    void pauseResume();
    void goTimes();
    void step();
    void updateStats();
    void programSelected(const QModelIndex &index);
    void changeProblem(int index);

private:
    void updateNodeList();
    void showProgram(int index);

    QTimer* _timer;
    Ui::MainWindow* _ui;
    SNGPWorker _sngpWorker;
};

#endif // MAINWINDOW_H
