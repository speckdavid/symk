#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "miscforestoperations.h"
#include "timefunctions.h"

#include <QString>
#include <QtDebug>

/*
 * This function sets up the initial states of all the
 * buttons. This creates the 2 scenes and attaches
 * them to the graphics views.
 * */
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setCentralWidget(ui->centralWidget);

    MainWindow::forest1Scene = new QGraphicsScene(this);

    ui->forest1->setScene(forest1Scene);
    ui->forest1->scale(1, -1);

    MainWindow::forest2Scene = new QGraphicsScene(this);

    ui->forest2->setScene(forest2Scene);
    ui->forest2->scale(1, -1);

    ui->forest1->hide();
    ui->labelForest1->hide();
    ui->hideForest1->hide();
    ui->forest2->hide();
    ui->labelForest2->hide();
    ui->hideForest2->hide();
    ui->play->hide();
    ui->step->hide();
    ui->f1HLRLabel->hide();
    ui->f1HorizontalLinearReduction->hide();
    ui->f2HLRLabel->hide();
    ui->f2HorizontalLinearReduction->hide();
    ui->verticalExpanderLabel->hide();
    ui->f1PenHeightLabel->hide();
    ui->f2PenHeightLabel->hide();
    ui->f1VerticalExpanderSlider->hide();
    ui->f2VerticalExpanderSlider->hide();
    ui->redrawForest1->hide();
    ui->redrawForest2->hide();
    ui->replayButton->hide();
    ui->speedFactorLabel->hide();
    ui->speedFactorDial->hide();
    ui->timer->hide();

    ui->redrawForest1->setEnabled(false);
    ui->redrawForest2->setEnabled(false);
}

/*
 * This function allows the user to search for a log file and load into the program.
 * It creates the parser to be used for the rest of the lifetime of the log.
 * It checks the first line for "T simple", then starts to set up the forests by
 * calling setupForest in the miscForestOperations.cpp.
 *
 * This functions also activate the relevent buttons and sliders.
 * */
void MainWindow::on_actionOpen_triggered()
{
    MainWindow::fileName = QFileDialog::getOpenFileName(
                this,
                "Open File",
                "c://",
                "Text File (*.txt);;All Files (*,*)"
                );
    if (MainWindow::fileName.length() <1)
    {
        return;
    }

    MainWindow::parser = new Parser(MainWindow::fileName);
    QString newLine = parser->readLine();

    if (newLine.compare("T simple") ==0 )
    {
        //This is to skip the comment line.
        parser->readLine();

        MainWindow::forest1 = setupForest1(
                    MainWindow::parser
                    , MainWindow::f1base
                    , MainWindow::f1PenHeight
                    , MainWindow::f1Name);

        ui->forest1->show();
        ui->labelForest1->setText(MainWindow::f1Name);
        ui->labelForest1->show();
        ui->hideForest1->show();
        ui->play->show();
        ui->step->show();
        ui->f1HLRLabel->show();
        ui->f1HorizontalLinearReduction->show();
        ui->verticalExpanderLabel->show();
        ui->f1PenHeightLabel->show();
        ui->f1VerticalExpanderSlider->show();
        ui->redrawForest1->show();
        ui->speedFactorLabel->show();
        ui->speedFactorDial->show();
        ui->timer->show();

        MainWindow::forest1Hidden = false;

        //This is a boolean list representing whether or not
        //an index is already marked for redraw or not.
        //This means that if ther are more than one update for
        //the same line, it is not added twice to the list.
        //This will avoid the line being redrawn multiple times.
        MainWindow::isInUpdateForest1
                = QVector<int>(MainWindow::forest1->size());

        //This call draws the initial bar graph for the first forest.
        redrawForest(MainWindow::forest1Scene
                     , MainWindow::forest1
                     , MainWindow::f1base
                     , MainWindow::f1PenHeight
                     , MainWindow::f1HorizontalReductionValue);

        QString c = MainWindow::parser->peek();

        if(c == "F") //This checks to see if there is another Forest to setup.
        {
            MainWindow::forest2 = setupForest2(
                        MainWindow::parser
                        , MainWindow::f2base
                        , MainWindow::f2PenHeight
                        , MainWindow::f2Name);

            ui->forest2->show();
            ui->labelForest2->setText(MainWindow::f2Name);
            ui->labelForest2->show();
            ui->hideForest2->show();
            ui->f2HLRLabel->show();
            ui->f2HorizontalLinearReduction->show();
            ui->f2PenHeightLabel->show();
            ui->f2VerticalExpanderSlider->show();
            ui->redrawForest2->show();

            MainWindow::forest2Hidden = false;

            MainWindow::isInUpdateForest2
                    = QVector<int>(MainWindow::forest2->size());

            redrawForest(MainWindow::forest2Scene
                         , MainWindow::forest2
                         , MainWindow::f2base
                         , MainWindow::f2PenHeight
                         , MainWindow::f2HorizontalReductionValue);

        }
    }else
    {
        QMessageBox::information(this,"File Name",
           "The file needs to start with T simple");
        MainWindow::file->close();
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

/*
 * This will hide and show forest one when the show/hide forest1 button is clicked.
 * */
void MainWindow::on_hideForest1_clicked()
{
    if(MainWindow::forest1Hidden)
    {
        ui->hideForest1->setText("Hide Forest 1");
        MainWindow::forest1Hidden = false;
        ui->forest1->show();
        ui->labelForest1->show();
    }else
    {
        ui->hideForest1->setText("Show Forest 1");
        MainWindow::forest1Hidden = true;
        ui->forest1->hide();
        ui->labelForest1->hide();
    }
}

/*
 *
 * This will hide and show forest 2 when the show/hide forest2 button is clicked.
 * */
void MainWindow::on_hideForest2_clicked()
{
    if(MainWindow::forest2Hidden)
    {
        ui->hideForest2->setText("Hide Forest 2");
        MainWindow::forest2Hidden = false;
        ui->forest2->show();
        ui->labelForest2->show();
    }else
    {
        ui->hideForest2->setText("Show Forest 2");
        MainWindow::forest2Hidden = true;
        ui->forest2->hide();
        ui->labelForest2->hide();
    }
}

/*
 * If paused, this function will unpause and if unpaused it will pause.
 * Play works by calling on_step_clicked() in a while loop.
 *
 * */
void MainWindow::on_play_clicked()
{
    if(MainWindow::isPaused)
    {

        //forests are playing, set text to "Pause"
        //isPaused goes to false
        //greys out step
        ui->play->setText("Pause");
        MainWindow::isPaused = false;
        ui->step->setEnabled(false);
        ui->f1HorizontalLinearReduction->setEnabled(false);
        ui->f2HorizontalLinearReduction->setEnabled(false);
        ui->f1VerticalExpanderSlider->setEnabled(false);
        ui->f2VerticalExpanderSlider->setEnabled(false);
        ui->speedFactorDial->setEnabled(false);

        while(!MainWindow::isPaused && !MainWindow::endOfLog)
        {
            MainWindow::on_step_clicked();
            //delayMili(1);
        }
    }else
    {
        //forests are not playing
        //isPaused goes to true
        //enable step button
        ui->play->setText("Play");
        MainWindow::isPaused = true;
        ui->step->setEnabled(true);
        ui->f1HorizontalLinearReduction->setEnabled(true);
        ui->f2HorizontalLinearReduction->setEnabled(true);
        ui->f1VerticalExpanderSlider->setEnabled(true);
        ui->f2VerticalExpanderSlider->setEnabled(true);
        ui->speedFactorDial->setEnabled(true);
    }
}

/*
 * This function calles playForest in miscForestOperation.cpp
 * */
void MainWindow::on_step_clicked()
{
    playForests(MainWindow::parser
                , ui->labelForest1
                , ui->labelForest2
                , MainWindow::forest1
                , MainWindow::forest2
                , MainWindow::forest1Scene
                , MainWindow::forest2Scene
                , MainWindow::isInUpdateForest1
                , MainWindow::isInUpdateForest2
                , MainWindow::Forest1Updates
                , MainWindow::Forest2Updates
                , MainWindow::f1base
                , MainWindow::f2base
                , MainWindow::f1PenHeight
                , MainWindow::f2PenHeight
                , MainWindow::oldSeconds
                , MainWindow::oldMicroseconds
                , MainWindow::f1Name
                , MainWindow::f2Name
                , MainWindow::endOfLog
                , MainWindow::f1HorizontalReductionValue
                , MainWindow::f2HorizontalReductionValue
                , MainWindow::speedFactorValue);
    if(MainWindow::endOfLog)
    {
        ui->play->hide();
        ui->step->hide();
        ui->replayButton->show();
    }
}

/*
 * This function allows the user to manually reduce the length of the lines in Forest 1 in a linear fashion.
 * */
void MainWindow::on_f1HorizontalLinearReduction_valueChanged(int value)
{
    if(value == 1)
    {
        value = 1;
    }else if(value == 2)
    {
        value = 10;
    }else if(value == 3)
    {
        value = 100;
    }else
    {
        value = 1000;
    }

    MainWindow::f1HorizontalReductionValue = value;
    MainWindow::forest1Redrawn = false;

    ui->play->setEnabled(false);
    ui->step->setEnabled(false);
    QString zValue = QString::number(value);
    ui->f1HLRLabel->setText("F1 H. L. R.: " + zValue);

    ui->redrawForest1->setEnabled(true);
}

/*
 * This function allows the user to manually reduce the length of the lines in Forest 2 in a linear fashion.
 * */
void MainWindow::on_f2HorizontalLinearReduction_valueChanged(int value)
{
    if(value == 1)
    {
        value = 1;
    }else if(value == 2)
    {
        value = 10;
    }else if(value == 3)
    {
        value = 100;
    }else
    {
        value = 1000;
    }

    MainWindow::f2HorizontalReductionValue = value;
    MainWindow::forest2Redrawn = false;

    ui->play->setEnabled(false);
    ui->step->setEnabled(false);
    QString zValue = QString::number(value);
    ui->f2HLRLabel->setText("F2 H. L. R.: " + zValue);

    ui->redrawForest2->setEnabled(true);
}

/*
 * This function allows the user to expand or contract the height of the lines for Forest 1.
 * This enables redrawForest1 and disables the play and step button.
 * */
void MainWindow::on_f1VerticalExpanderSlider_valueChanged(int value)
{
    MainWindow::f1PenHeight = value;
    MainWindow::forest1Redrawn = false;

    ui->play->setEnabled(false);
    ui->step->setEnabled(false);

    ui->redrawForest1->setEnabled(true);
}

/*
 * This function allows the user to expand or contract the height of the lines for Forest 2.
 * This enables redrawForest2 and disables the play and step button.
 * */
void MainWindow::on_f2VerticalExpanderSlider_valueChanged(int value)
{
    MainWindow::f2PenHeight = value;
    MainWindow::forest2Redrawn = false;

    ui->play->setEnabled(false);
    ui->step->setEnabled(false);

    ui->redrawForest2->setEnabled(true);
}

/*
 * This redraws Forest 1 by calling redrawForest in miscforestoperation.cpp.
 * Then it disables the redrawForest1 button and checks to see if forest 2 needs to be redrawn.
 * If both forests are ready then it enables the play and step buttons.
 * */
void MainWindow::on_redrawForest1_clicked()
{
    redrawForest(MainWindow::forest1Scene
                 , MainWindow::forest1
                 , MainWindow::f1base
                 , MainWindow::f1PenHeight
                 , MainWindow::f1HorizontalReductionValue);

    ui->redrawForest1->setEnabled(false);

    MainWindow::forest1Redrawn = true;

    if (MainWindow::forest2Redrawn)
    {
        ui->play->setEnabled(true);
        ui->step->setEnabled(true);
    }

}

/*
 * This redraws Forest 2 by calling redrawForest in miscforestoperation.cpp.
 * Then it disables the redrawForest1 button and checks to see if forest 1 needs to be redrawn.
 * If both forests are ready then it enables the play and step buttons.
 * */
void MainWindow::on_redrawForest2_clicked()
{
    redrawForest(MainWindow::forest2Scene
                 , MainWindow::forest2
                 , MainWindow::f2base
                 , MainWindow::f2PenHeight
                 , MainWindow::f2HorizontalReductionValue);

    ui->redrawForest2->setEnabled(false);

    MainWindow::forest2Redrawn = true;

    if (MainWindow::forest1Redrawn)
    {
        ui->play->setEnabled(true);
        ui->step->setEnabled(true);
    }
}

/*
 * This function creates a new parser. Then sets up the forests again
 * by calling setupForest() in the miscforestoperations.cpp.
 * This then redraws the forests by calling redrawForest() in the
 * miscforestoperations.cpp.
 * */
void MainWindow::on_replayButton_clicked()
{
    MainWindow::parser = new Parser(MainWindow::fileName);

    QString newLine = parser->readLine();

    MainWindow::forest1 = setupForest1(
                MainWindow::parser
                , MainWindow::f1base
                , MainWindow::f1PenHeight
                , MainWindow::f1Name);

    ui->labelForest1->setText(MainWindow::f1Name);
    ui->play->show();
    ui->step->show();

    ui->f1HorizontalLinearReduction->setEnabled(true);
    ui->f1VerticalExpanderSlider->setEnabled(true);

    MainWindow::forest1Hidden = false;

    MainWindow::isInUpdateForest1
            = QVector<int>(MainWindow::forest1->size());

    redrawForest(MainWindow::forest1Scene
                 , MainWindow::forest1
                 , MainWindow::f1base
                 , MainWindow::f1PenHeight
                 , MainWindow::f1HorizontalReductionValue);

    QString c = MainWindow::parser->peek();

    if(c == "F")
    {
        MainWindow::forest2 = setupForest2(
                    MainWindow::parser
                    , MainWindow::f2base
                    , MainWindow::f2PenHeight
                    , MainWindow::f2Name);

        ui->labelForest2->setText(MainWindow::f2Name);
        ui->f2HorizontalLinearReduction->setEnabled(true);
        ui->f2VerticalExpanderSlider->setEnabled(true);

        MainWindow::forest2Hidden = false;

        MainWindow::isInUpdateForest2
                = QVector<int>(MainWindow::forest2->size());

        redrawForest(MainWindow::forest2Scene
                     , MainWindow::forest2
                     , MainWindow::f2base
                     , MainWindow::f2PenHeight
                     , MainWindow::f2HorizontalReductionValue);

    }

    MainWindow::oldSeconds = 0;
    MainWindow::oldMicroseconds = 0;
    MainWindow::isPaused = true;
    MainWindow::endOfLog = false;
    ui->play->setText("Play");
    ui->replayButton->hide();
    ui->step->setEnabled(true);
    ui->speedFactorDial->setEnabled(true);
}

/*
 * This function changes the value of the factor multiplied with the time stamp.
 * This needs expanded.
 * */
void MainWindow::on_speedFactorDial_valueChanged(int value)
{
    MainWindow::speedFactorValue = value;
    ui->speedFactorLabel->setText("Speed Factor: 1/"
                                  + QString::number(value));
}
