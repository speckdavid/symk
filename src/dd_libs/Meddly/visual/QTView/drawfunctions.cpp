#include "drawfunctions.h"
#include "timefunctions.h"

#include <QMessageBox>
#include <QGraphicsLineItem>
#include <QGraphicsItem>

/*
 * This functions will add all lines to the scene for sets.
 * This will go through the entire forest and add the lines left aligned, starting on the 0 axis.
 *
 */
void drawSet(QGraphicsScene *forestScene
             , QVector<int> *forest
             , int penHeight, int reductionValue)
{
    QPen green(Qt::green);
    green.setWidth(penHeight);
    //int startLine = 0;
    int endLine = 0;

    int top = forest->size();
    int y = 0;

    //This is the call to the function drawRulers. This will start to create
    //the initial x and y axis
    //drawRulers(forestScene, penHeight
    //           , top, 0);

    for (int var = 0; var < top; ++var)
    {
        //endLine = ( (forest->at(var))/2 ) /reductionValue;
        //startLine = 0 - endLine;

        endLine = forest->at(var) /reductionValue;

        y = var *penHeight;

        forestScene->addLine(0//startLine
                             , y
                             , endLine
                             , y
                             , green);
        delayMili(1);

    }
    /*
     * //This a troublshooting tool to see where 0,0 is on the
     * QGraphicScene
    QPen red(Qt::red);
    red.setWidth(1);
    forestScene->addLine(-20,0,-1,0,red);
    */
}

/*
 * This functions will add all lines to the scene for relations.
 * This will go through the entire forest and add the lines left
 *  aligned, starting on the 0 axis.
 *
 */
void drawRelation(QGraphicsScene *forestScene
                  , QVector<int> *forest
                  , int penHeight, int reductionValue)
{
    QPen green(Qt::green);
    green.setWidth(penHeight);

    QPen darkGreen(Qt::darkGreen);
    darkGreen.setWidth(penHeight);
    //int startLine = 0;
    int endLine = 0;

    int posYlevel = 0;
    int negYlevel = 0;

    int top = forest->size() - 1;
    int middle = top/2;

    //This is the call to the function drawRulers. This will start to create
    //the initial x and y axis.
    //drawRulers(forestScene, penHeight
    //           , top, 1);

    for (int var = 0; var < middle; ++var)
    {
        //Positive lines
        endLine = (forest->at(middle + var + 1)) / reductionValue;
        //startLine = 0 - endLine;

        posYlevel = ((var*2)+1) *penHeight;

        forestScene->addLine(0//startLine
                             , posYlevel
                             , endLine
                             , posYlevel
                             , green);

        //Negitive lines

        //endLine = ( (forest->at(middle - var -1))/2 ) / reductionValue;
        //startLine = 0 - endLine;

        endLine = forest->at(middle - var -1) / reductionValue;

        negYlevel = ((var*2)+0) * penHeight;

        forestScene->addLine(0 //startLine
                             , negYlevel
                             , endLine
                             , negYlevel
                             , darkGreen);
        delayMili(1);
    }
    /*
     * //This a troublshooting tool to see where 0,0 is on the
     * QGraphicScene
    QPen red(Qt::red);
    red.setWidth(1);
    forestScene->addLine(-20,0,-1,0,red);
    */
}

//When called this will redraw the x and y axis.
//This doesn't yet have a perameter to redraw the x axis. It
//has a default length for the x axis.
void drawRulers(QGraphicsScene *forestScene
                , int penHeight, int forestsize
                , int setOrRelation)
{
    QPen black(Qt::black);
    black.setWidth(1);

    //vertical line
    forestScene->addLine(-20
                         , -20
                         , -20
                         , penHeight*(forestsize -1) //-20
                         , black);
    //horizontal line
    int x = 300;
    forestScene->addLine(-20
                         , -20
                         , 20 + x
                         , -20
                         , black);

    if(setOrRelation == 0)
    {
        //add code for marks on ruler.

        int height = 0;
        //while
    }else
    {

        //code for markes on rulers and lables for those marks.

    }
}

/*
 * This functions chooses the default pen height
 *  for use later when adding lines to a scene.
 * In future versions this could capture the size of the qgraphicsview portal
 * and calculate a pen height that will maximize the use of the space.
 */
int generatePenHeight(int size)
{
    if(size <20)
    {
        return 15;
    }else if(size >= 20 && size <= 100)
    {
        return 10;
    }else
    {
        return 1;
    }
}

/*
 * This will update the QGraphicScene forestScene.
 *This will only undate the line indexes that have been modified in the
 *forest. Those forest indexes are stored in QVector ForestUpdates.
 */
void drawUpdates(QVector<int> *&forest
                 , QVector<int> &isInForestUpdates
                 , QVector<int> &ForestUpdates
                 , QGraphicsScene *&forestScene
                 , int fPenHeight
                 , int fbase
                 , int fHorizontalReductionValue)
{
    QPen green(Qt::green);
    QPen darkgreen(Qt::darkGreen);

    QGraphicsItem *line = NULL;
    QGraphicsLineItem *lineI = NULL;
    qreal x = 0;
    qreal y = 0;

    //This will process the forest updates.
    //It will decide wheather it is a set or relation
    //then complete the updates to the individual lines.

    //set
    if(fbase > 0)
    {
        while (!ForestUpdates.isEmpty())
        {
            int indexOfForest = ForestUpdates.takeFirst();
            //this will reset the values in boolean list
            //isInForestUpdates back to 0 to indicate that those
            //indexes are no longer contained in the ForestUpdate list.
            isInForestUpdates.replace(indexOfForest, 0);
            y = indexOfForest * fPenHeight;
            line = forestScene->itemAt(x, y, QTransform());
            lineI = qgraphicsitem_cast<QGraphicsLineItem*>(line);

            int endline = forest->at(indexOfForest);
            endline = endline/fHorizontalReductionValue;

            //If you try to grab a 0 to 0 line then the
            //pointer will be null. This checks to see if
            //you tried to pull an empty line from the
            //scene. If you did then it will create a new
            //line to add to the appropriate spot.
            if(lineI == NULL)
            {
                green.setWidth(fPenHeight);
                forestScene->addLine(0//startline
                                      , y
                                      , endline
                                      , y
                                      , green
                                      );
            }else//This changes the positionof the
                //end of the line object.
            {
                lineI->setLine(0 //startline
                               , y
                               , endline
                               , y);
            }
        }
    //Relation
    }else
    {
        while (!ForestUpdates.isEmpty())
        {
            int indexOfForest = ForestUpdates.takeFirst();
            isInForestUpdates.replace(indexOfForest, 0);
            //positive levels
            int middle = forest->size()/2;
            if(indexOfForest > middle)
            {
                int indexOnScene =
                        (indexOfForest - middle) * 2;

                y = indexOnScene * fPenHeight;
                line = forestScene->itemAt(x, y, QTransform());
                forestScene->removeItem(line);
                delete line;
                delayMili(1);

                int endline = forest->at(indexOfForest);
                endline = endline/fHorizontalReductionValue;

                green.setWidth(fPenHeight);
                forestScene->addLine(0//startline
                                      , y
                                      , endline
                                      , y
                                      , green
                                      );

            }else//negitive levels
            {
                int indexOnScene = (forest->size() - 1)
                        - (indexOfForest * 2) -1;
                y = indexOnScene * fPenHeight;
                line = forestScene->itemAt(x, y, QTransform());
                forestScene->removeItem(line);
                delete line;
                delayMili(1);

                int endline = forest->at(indexOfForest);
                endline = endline/fHorizontalReductionValue;

                darkgreen.setWidth(fPenHeight);
                forestScene->addLine(0 //startline
                                      , y
                                      , endline
                                      , y
                                      , darkgreen
                                      );
            }
        }
    }
}
