#ifndef MISCFORESTOPERATIONS_H
#define MISCFORESTOPERATIONS_H
#include "parser.h"
#include <QLabel>
#include <QGraphicsScene>

/*
 * This function recieves the begining values
 * from the log files and returns a new forest
 * QVector.
 */
QVector<int> *setuplist(QStringList &list);

/*
 * This function sets up forest 1. It returns a
 * new forest QVector. This function calls
 * setupSet or setupRelation depending on the
 * value of f1base.
 */
QVector<int> *setupForest1(Parser *parser, int &f1base
                           , int &f1PenHeight
                           , QString &f1Name);

QVector<int> *setupForest2(Parser *parser
                           , int &f2base
                           , int &f2PenHeight
                           , QString &f2Name);

/*
 * This function will process a, t, and p lines.
 * it continues on anything else.
 * If it reaches the end of the file then it
 * will draw the current updates and delete
 * the parser and forests QVectors.
 */
void playForests(Parser *parser
                 , QLabel *labelForest1
                 , QLabel *labelForest2
                 , QVector<int> *&forest1
                 , QVector<int> *&forest2
                 , QGraphicsScene *&forest1Scene
                 , QGraphicsScene *&forest2Scene
                 , QVector<int> &isInForest1Updates
                 , QVector<int> &isInForest2Updates
                 , QVector<int> &Forest1Updates
                 , QVector<int> &Forest2Updates
                 , int f1base, int f2base
                 , int f1PenHeight, int f2PenHeight
                 , int &oldSeconds
                 , int &oldMiliseconds
                 , QString f1Name, QString f2Name
                 , bool &endOfFile
                 , int f1HorizontalReductionValue
                 , int f2HorizontalReductionValue
                 , int speedFactorValue);

/*
 * This function clears the forestScene, then calls either
 * drawSet or drawRelation in drawfunctions.cpp.
 */
void redrawForest(QGraphicsScene *forestScene
                   , QVector<int> *forest, int base
                  , int penHeight, int reductionValue);

//void redrawForest2(QGraphicsScene *forest2Scene);
#endif // MISCFORESTOPERATIONS_H
