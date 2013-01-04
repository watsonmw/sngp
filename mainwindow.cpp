#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QTimer>
#include <QStringListModel>
#include <QDateTime>

enum PROBLEM {
    PROBLEM_Multiplexer,
    PROBLEM_EvenParity4,
    PROBLEM_EvenParity5,
    PROBLEM_EvenParity6,
    PROBLEM_EvenParity7,
    PROBLEM_SymbolRegression
};

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent),
    _ui(new Ui::MainWindow)
{
    _ui->setupUi(this);

    _sngpWorker.setProblem(new ProblemMultiplexer());
    _sngpWorker.reset();

    _timer = new QTimer(this);
    connect(_timer, SIGNAL(timeout()), this, SLOT(updateStats()));

    connect(_ui->stopGoButton, SIGNAL(clicked()), this, SLOT(pauseResume()));
    connect(_ui->goTimes10Button, SIGNAL(clicked()), this, SLOT(goTimes10()));
    connect(_ui->goTimes100Button, SIGNAL(clicked()), this, SLOT(goTimes100()));
    connect(_ui->resetButton, SIGNAL(clicked()), this, SLOT(reset()));
    connect(_ui->stepButton, SIGNAL(clicked()), this, SLOT(step()));
    connect(_ui->nodeListView, SIGNAL(clicked(const QModelIndex&)),
            this, SLOT(programSelected(const QModelIndex&)));

    _ui->problemComboBox->addItem("Multiplexer 6", PROBLEM_Multiplexer);
    _ui->problemComboBox->addItem("Even Parity 4", PROBLEM_EvenParity4);
    _ui->problemComboBox->addItem("Even Parity 5", PROBLEM_EvenParity5);
    _ui->problemComboBox->addItem("Even Parity 6", PROBLEM_EvenParity6);
    _ui->problemComboBox->addItem("Even Parity 7", PROBLEM_EvenParity7);
    _ui->problemComboBox->addItem("Symbolic Regression", PROBLEM_SymbolRegression);

    connect(_ui->problemComboBox, SIGNAL(activated(int)),
            this, SLOT(changeProblem(int)));

    changeProblem(0);
}

MainWindow::~MainWindow()
{
    delete _ui;
}

void MainWindow::reset()
{
    _sngpWorker.reset();
    updateStats();
}

void MainWindow::pauseResume()
{
    if (_sngpWorker.isRunning()) {
        _sngpWorker.pause();
        _timer->stop();
    } else {
        _sngpWorker.setNumTimesToRun(1);
        _sngpWorker.resume();
        _timer->start(1000);
    }
    updateStats();
    updateNodeList();
}

void MainWindow::goTimes(int times)
{
    if (_sngpWorker.isRunning()) {
        _sngpWorker.pause();
    }
    _sngpWorker.reset();
    _sngpWorker.setNumTimesToRun(times);
    _sngpWorker.resume();
    _timer->start(1000);
    updateStats();
    updateNodeList();
}

void MainWindow::goTimes10()
{
    goTimes(10);
}

void MainWindow::goTimes100()
{
    goTimes(100);
}

void MainWindow::step()
{
    if (!_sngpWorker.isRunning()) {
        _sngpWorker.step();
        updateStats();
    }
}

void MainWindow::updateStats()
{
    const SNodeStats& stats = _sngpWorker.getStats();
    _ui->scoreLabel->setText(QString::number(stats.avgScore));
    _ui->generationLabel->setText(QString::number(stats.generation));
    _ui->bestScoreLabel->setText(QString::number(stats.bestScoreEver));
    _ui->bestIndividualScoreLabel->setText(
        QString::number(stats.bestIndividualScore));
    _ui->bestIndividualScoreEverLabel->setText(
        QString::number(stats.bestIndividualScoreEver));
    _ui->hitsLabel->setText(QString("%1/%2").
        arg(stats.hits).arg(stats.runs));
    int64_t timeTakenMilliseconds = 0;
    if (stats.timeTakenMilliseconds == 0) {
        if (stats.startTimeMilliseconds > 0) {
            timeTakenMilliseconds = QDateTime::currentMSecsSinceEpoch() -
                                                stats.startTimeMilliseconds;
        }
    } else {
        timeTakenMilliseconds = stats.timeTakenMilliseconds;
    }
    _ui->timeTakenLabel->setText(QString("%1").
        arg(timeTakenMilliseconds / 1000.0, 0, 'f', 4));


    if (_sngpWorker.isRunning()) {
        _ui->stopGoButton->setText("Stop");
    } else {
        _ui->stopGoButton->setText("Go");
        updateNodeList();
        _timer->stop();
    }
}

void MainWindow::updateNodeList()
{
    QStringList nodeList;
    const std::vector<SNode>& nodes = _sngpWorker.getNodes();
    const std::vector<int>& values = _sngpWorker.getFitness();
    if (nodes.size() > 0) {
        for (int i = 0; i < (int)nodes.size(); ++i) {
          const SNode& node = nodes[i];
          QString nodeDetails;
          int val = 0;
          if ((int)values.size() > i) {
              val = values[i];
          }
          nodeDetails = QString("%1: %2 score: %3").arg(i).
                            arg(node.asString()).
                            arg(val);
          nodeList << nodeDetails;
        }
        QAbstractItemModel *model = new QStringListModel(nodeList);
        _ui->nodeListView->setModel(model);
    } else {
        _ui->nodeListView->reset();
    }
}

void MainWindow::programSelected(const QModelIndex& index)
{
    showProgram(index.row());
}

void MainWindow::changeProblem(int index)
{
    Problem* p = NULL;
    switch((PROBLEM)_ui->problemComboBox->itemData(index).toInt()) {
    case PROBLEM_EvenParity4:
        p = new ProblemEvenParity(4);
        break;
    case PROBLEM_EvenParity5:
        p = new ProblemEvenParity(5);
        break;
    case PROBLEM_EvenParity6:
        p = new ProblemEvenParity(6);
        break;
    case PROBLEM_EvenParity7:
        p = new ProblemEvenParity(7);
        break;
    case PROBLEM_SymbolRegression:
        p = new ProblemSymbolicRegression();
        break;
    case PROBLEM_Multiplexer:
        p = new ProblemMultiplexer();
    default:
        break;
    }
    if (_sngpWorker.isRunning()) {
        _sngpWorker.pause();
    }
    _sngpWorker.setProblem(p);
    _sngpWorker.reset();
    updateStats();
    updateNodeList();
}

void MainWindow::showProgram(int index)
{
    QString text = _sngpWorker.getProgramAsText(index);
    _ui->logBrowser->setPlainText(text);
}
