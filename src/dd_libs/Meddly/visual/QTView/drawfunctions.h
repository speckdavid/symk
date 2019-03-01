#ifndef DRAWFUNCTIONS_H
#define DRAWFUNCTIONS_H

#include <QGraphicsScene>
#include <QVector>

/*
 * This functions will add all lines to the scene for sets.
 * This will go through the entire forest and add the lines
 * left aligned, starting on the 0 axis.
 *
 */
void drawSet(QGraphicsScene *forestScene
             , QVector<int> *forest, int penHeight
             , int reductionValue);

/*
 * This functions will add all lines to the scene for relations.
 * This will go through the entire forest and add the lines left
 *  aligned, starting on the 0 axis.
 *
 */
void drawRelation(QGraphicsScene *forestScene
                  , QVector<int> *forest, int penHeight
                  , int reductionValue);

/*
 * This functions chooses the default pen height
 *  for use later when adding lines to a scene.
 * In future versions this could capture the size of the qgraphicsview portal
 * and calculate a pen height that will maximize the use of the space.
 */
int generatePenHeight(int size);

/*
 * This will update the QGraphicScene forestScene.
 *This will only undate the line indexes that have been modified in the
 *forest. Those forest indexes are stored in QVector ForestUpdates.
 */
void drawUpdates(QVector<int> *&forest
                 , QVector<int> &isInForestUpdates
                 , QVector<int> &ForestUpdates
                 , QGraphicsScene *&forestScene
                 , int fPenHeight, int fbase, int fHorizontalReductionValue
                 );

//Needs to be finished
void drawRulers(QGraphicsScene *forestScene
                , int penHeight, int forestsize, int setOrRelation);

#endif // DRAWFUNCTIONS_H
