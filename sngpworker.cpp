#include "sngpworker.h"

#include <map>
#include <QVector>
#include <QTextStream>
#include <QtAlgorithms>
#include <QDateTime>

SNGPWorker::SNGPWorker()
  : _bRunning(false),
    _problem(NULL),
    _times(0),
    _maxGenerations(25000)
{
}

SNGPWorker::~SNGPWorker()
{
    // Exit the other thread before cleaning up
    pause();
    delete _problem;
}

void SNGPWorker::setProblem(Problem *problem)
{
    // Delete the old problem
    delete _problem;

    // Setup the eval engine according to the problem
    _problem = problem;
    _evalEngine.setNumInputs(_problem->getNumInputs());
    _evalEngine.setAvailableOps(_problem->getOps());
    _evalEngine.setSize(100);
    _evalEngine.setNumInputs(_problem->getNumInputs());
    _problem->initTestCaseResults(_evalEngine.getSize());
    _fitness.resize(_evalEngine.getSize());
}

void SNGPWorker::setNumTimesToRun(int times)
{
    _times = times;
}

void SNGPWorker::setNumMaxGenerations(int maxGenerations)
{
    _maxGenerations = maxGenerations;
}

void SNGPWorker::reset()
{
    QMutexLocker lock(&_mutex);
    _stats.reset();
    _evalEngine.init();
    resetFitness();
}

void SNGPWorker::pause()
{
    if (_bRunning) {
        _bRunning = false;
        wait();
    }
}

void SNGPWorker::resume()
{
    if (!_bRunning) {
        _bRunning = true;
        start();
    }
}

bool SNGPWorker::isRunning()
{
    return _bRunning;
}

void SNGPWorker::step()
{
    QMutexLocker lock(&_mutex);
    runGeneration();
}

void SNGPWorker::runGeneration()
{
    // Apply a new mutation or revert the previous
    if (_stats.generation == 0) {
        // Calculate fitness values for all test cases
        resetFitness();
        _evalEngine.init();

        _problem->evaluate(_evalEngine.getNodes(), _fitness);

        // Calculate total scores
        int totalScore = 0;
        int bestScore = _fitness[_problem->getNumInputs()];
        for (size_t i = _problem->getNumInputs(); i < _fitness.size(); ++i) {
            totalScore += _fitness[i];
            if (_fitness[i] > bestScore) {
                bestScore = _fitness[i];
            }
        }

        // First time just record the stats, this handles the case
        // where the test is retuning negative values.
        _stats.lastAvgScore = totalScore;
        _stats.avgScore = totalScore;
        _stats.bestScoreEver = totalScore;
        if (_stats.runs == 0) {
            _stats.bestIndividualScore = bestScore;
            _stats.bestIndividualScoreEver = bestScore;
        }
    } else {
        if (_stats.avgScore < _stats.lastAvgScore) {
            _evalEngine.restore();
            _stats.avgScore = _stats.lastAvgScore;
        }
        _evalEngine.mutate();

        _problem->evaluate(_evalEngine.getNodes(),
                           _evalEngine.getChangedNodes(), _fitness);

        _evalEngine.clearChanged();

        // Calculate total scores
        int64_t totalScore = 0;
        int bestScore = _fitness[_problem->getNumInputs()];
        for (size_t i = _problem->getNumInputs(); i < _fitness.size(); ++i) {
            totalScore += _fitness[i];
            if (_fitness[i] > bestScore) {
                bestScore = _fitness[i];
            }
        }

        _stats.lastAvgScore = _stats.avgScore;
        _stats.avgScore = totalScore;
        if (_stats.bestScoreEver < totalScore) {
            _stats.bestScoreEver = totalScore;
        }
        _stats.bestIndividualScore = bestScore;
        if (_stats.bestIndividualScoreEver < bestScore) {
            _stats.bestIndividualScoreEver = bestScore;
        }
    }
    _stats.generation++;
}

void SNGPWorker::resetFitness()
{
    for (size_t i = 0; i < _fitness.size(); ++i) {
         _fitness[i] = 0.0;
    }
}

QString SNGPWorker::getProgramAsText(int i)
{
    QString text;
    QTextStream stream(&text);
    QVector<int> indices;
    QVector<bool> toCheck(i+1, false);
    toCheck[i] = true;
    for (int j = i; j >= 0; -- j) {
        if (toCheck[j]) {
            SNode& node = _nodesCopy[j];
            indices.push_back(j);
            if (node.op != SNode::ValOp &&
                node.op != SNode::InputOp) {
                for (int k = 0; k < node.getNumParams(); ++k) {
                    toCheck[node.param[k]] = true;
                }
            }
        }
    }

    std::reverse(indices.begin(), indices.end());

    std::map<int, int> remapping;
    for (int j = 0; j < indices.size(); ++j) {
        remapping[indices[j]] = j;
    }

    for (int j = 0; j < indices.size(); ++j) {
        SNode& node = _nodesCopy[indices[j]];
        stream << j << " (" << indices[j] << "): ";
        stream << SNode::OpAsString(node.op);
        if (node.op == SNode::ValOp) {
            stream << " " << node.param[0];
        } else if (node.op == SNode::InputOp) {
            stream << " " << indices[j];
        } else {
            stream << " (";
            for (int k = 0; k < node.getNumParams(); ++k) {
                stream << remapping[node.param[k]];
                if (k < node.getNumParams() - 1) {
                    stream << ", ";
                }
            }
            stream << ")";
        }
        stream << endl;
    }
    return text;
}

void SNGPWorker::run()
{
    // qsrand(1);// Set the seed to a fixed value when testing.
    qsrand((uint)QDateTime::currentMSecsSinceEpoch());

    _stats.startTimeMilliseconds = QDateTime::currentMSecsSinceEpoch();

    while (_bRunning) {
        QMutexLocker lock(&_mutex);
        runGeneration();
        bool doneRun = false;
        if (_problem->hitTargetFitness(_fitness)) {
            _stats.hits++;
            doneRun = true;
        } else if (_stats.generation >= _maxGenerations) {
            doneRun = true;
        }
        if (doneRun) {
            _stats.runs++;
            if (_times <= _stats.runs) {
                _bRunning = false;
            } else {
                _evalEngine.init();
                _stats.generation = 0;
            }
        }
    }

    _stats.timeTakenMilliseconds +=
        QDateTime::currentMSecsSinceEpoch() - _stats.startTimeMilliseconds;
}

const std::vector<SNode> &SNGPWorker::getNodes()
{
   if (!_bRunning) {
       _nodesCopy = _evalEngine.getNodes();
   }
   return _nodesCopy;
}

const std::vector<int> &SNGPWorker::getFitness()
{
   if (!_bRunning) {
       _fitnessCopy = _fitness;
   }
   return _fitnessCopy;
}
