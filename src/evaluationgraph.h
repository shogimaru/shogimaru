#pragma once

#include <QWidget>

class EvaluationGraph : public QWidget
{
    Q_OBJECT
public:
    EvaluationGraph(QWidget *parent = nullptr) : QWidget(parent) {}
    virtual ~EvaluationGraph() {}

    void setScores(const QList<int> scores);
    void addScore(int score);
    void setPercentageDisplayEnabled(bool enable) { _percentage = enable; }
    void clear();

public slots:
    void setCurrentMove(int move);

signals:
    void currentMoveChanged(int move);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    QList<int> _scores;
    QList<QPoint> _points;
    int _current {-1};
    int _percentage {false};  // パーセント表示
};
