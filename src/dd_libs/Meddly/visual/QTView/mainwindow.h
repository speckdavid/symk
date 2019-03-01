#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QPainter>
#include <QFileDialog>
#include <QMessageBox>
#include <QVector>
#include "parser.h"

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

    void on_hideForest1_clicked();

    void on_hideForest2_clicked();

    void on_play_clicked();

    void on_step_clicked();

    void on_actionOpen_triggered();

    void on_f1HorizontalLinearReduction_valueChanged(int value);

    void on_f2HorizontalLinearReduction_valueChanged(int value);

    void on_f1VerticalExpanderSlider_valueChanged(int value);

    void on_f2VerticalExpanderSlider_valueChanged(int value);

    void on_redrawForest1_clicked();

    void on_redrawForest2_clicked();

    void on_replayButton_clicked();

    void on_speedFactorDial_valueChanged(int value);

private:
    Ui::MainWindow *ui;
  //  void paintEvent(QPaintEvent *e);
    QGraphicsScene *forest1Scene;
    QGraphicsScene *forest2Scene;

    QFile *file;
    Parser *parser;

    bool forest1Hidden = true;
    bool forest2Hidden = true;
    bool isPaused = true;
    bool endOfLog = false;
    bool forest1Redrawn = true;
    bool forest2Redrawn = true;
    //bool f2Created = false;

    int f1PenHeight = 0;
    int f2PenHeight = 0;
    int f1HorizontalReductionValue = 1;
    int f2HorizontalReductionValue = 1;
    int speedFactorValue = 1;

    QVector<int> *forest1;
    QVector<int> *forest2;
    QVector<int> isInUpdateForest1;
    QVector<int> isInUpdateForest2;
    QVector<int> Forest1Updates;
    QVector<int> Forest2Updates;

    int f1base = 0;
    int f2base = 0;
    int oldSeconds = 0;
    //int newSeconds = 0;
    int oldMicroseconds = 0;
    //int newMiliseconds = 0;

    QString fileName = NULL;
    QString f1Name = NULL;
    QString f2Name = NULL;


};

#endif // MAINWINDOW_H
