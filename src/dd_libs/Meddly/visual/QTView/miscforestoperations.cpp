#include "miscforestoperations.h"
#include "drawfunctions.h"
#include "timefunctions.h"

#include <QDebug>
#include <QMessageBox>

/*
 * This function recieves the begining values
 * from the log files and returns a new forest
 * QVector.
 */
QVector<int> *setuplist(QStringList &list)
{
    QVector<int> *forest = new QVector<int>();

    int value = 0;
    QString temp = NULL;

    for (int var = 5; var < list.size(); ++var)
    {
        temp = list.at(var);
        value = temp.toInt();
        forest->append(value);
    }
    return forest;
}

/*
 * This function sets up forest 1. It returns a
 * new forest QVector. This function calls
 * setupSet or setupRelation depending on the
 * value of f1base.
 */
QVector<int> *setupForest1(Parser *parser
                           , int &f1base
                           , int &f1PenHeight
                           , QString &f1Name)
{
    //The first F line
    QString line = parser->readLine();

    //Split the F line
    // "[,\[ ]"
    QRegExp rx("[,\[ \\]]");// match a comma or a space or square brackets
    QStringList list = line.split(rx, QString::SkipEmptyParts);

    //Setting the forest number and name
    f1Name = "Forest 1 " + list.at(2);
    QString n = list.at(3);
    f1base = n.toInt();
    if(f1base > 0)
    {
        //Call generatePenHeight
        QString d = list.at(4);
        f1PenHeight = generatePenHeight(d.toInt());

        return setuplist(list);
    }else
    {
        //Call generatePenHeight
        QString d = list.at(4);
        f1PenHeight = generatePenHeight(d.toInt() * 2);

        return setuplist(list);
    }
}

/*
 * This does the same thing as setupForest2 now.
 * This needs to be combined with setupForest1 in the future.
 */
QVector<int> *setupForest2(Parser *parser
                            , int &f2base
                            , int &f2PenHeight
                            , QString &f2Name)
{
    //The second F line
    QString line = parser->readLine();

    //Split the F line
    // "[,\[ ]"
    QRegExp rx("[,\[ \\]]");// match a comma or a space or square brackets
    QStringList list = line.split(rx, QString::SkipEmptyParts);

    //Setting the forest number and name
    f2Name = "Forest 2 " + list.at(2);
    QString n = list.at(3);
    f2base = n.toInt();
    if(f2base > 0)
    {
        //Call generatePenHeight
        QString d = list.at(4);
        f2PenHeight = generatePenHeight(d.toInt());

        return setuplist(list);
    }else
    {
        //Call generatePenHeight
        QString d = list.at(4);
        f2PenHeight = generatePenHeight(d.toInt() * 2);

        return setuplist(list);
    }
}

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
                 , int &oldMicroseconds
                 , QString f1Name
                 , QString f2Name
                 , bool &endOfFile
                 , int f1HorizontalReductionValue
                 , int f2HorizontalReductionValue
                 , int speedFactorValue)
{

    bool continueLoop = true;
    while(continueLoop)
    {
        QString line = parser->readLine();

        //End of File.
        if (line.length() <1)
        {
            //QMessageBox::information(NULL,"File Name",
            //                            "End of file");
            endOfFile = true;

            //Empty the last of the updates
            drawUpdates(forest1
                        , isInForest1Updates
                        , Forest1Updates
                        , forest1Scene
                        , f1PenHeight
                        , f1base
                        , f1HorizontalReductionValue);

            //This checks to see if forest 2 exist before
            //emptying it and deleting it.
            if(forest2->size() > 1)
            {
                drawUpdates(forest2
                            , isInForest2Updates
                            , Forest2Updates
                            , forest2Scene
                            , f2PenHeight
                            , f2base
                            , f2HorizontalReductionValue);

                delete forest2;
            }

            delete parser;
            delete forest1;


            break;
        }
        QChar t = line.at(0);

        int u = t.toLatin1();

        switch (u)
        {        
        /*
         * This is the p line
         * This case sets the lables with the correct message
         */
        case 112:
        {
            QChar fNumber = line.at(2);
            if(fNumber == '1')
            {
                QString phase = line.remove(0, 3);
                labelForest1->setText(f1Name+ phase);
            }

            if(fNumber == '2')
            {
                QString phase = line.remove(0, 3);
                labelForest2->setText(f2Name + phase);
            }
            continueLoop = false;

            break;
        }
        /*
         * This is the 'a' line
         * This function uses boolean arrays to indicate if an update
         * is already in the forest update list.
         */
        case 97:
        {
            QStringList updates = line.split(" ", QString::SkipEmptyParts);
            QString whichForest = NULL;
            QString sexpectedLevel = NULL;
            QString schangeAmount = NULL;

            int indexFromLog = 0;
            for (int var = 1; var < updates.size(); ++var)
            {
                whichForest = updates.at(var);

                ++var;
                sexpectedLevel = updates.at(var);
                indexFromLog = sexpectedLevel.toInt();

                ++var;
                schangeAmount = updates.at(var);
                int changeAmount = schangeAmount.toInt();

                if(whichForest == "1")
                {
                    int indexOfForest = indexFromLog - f1base;
                    if(isInForest1Updates.at(indexOfForest) == 0)
                    {
                        Forest1Updates.append(indexOfForest);
                        isInForest1Updates.replace(indexOfForest, 1);
                    }

                    changeAmount = changeAmount + forest1->at(indexOfForest);
                    forest1->replace(indexOfForest, changeAmount);
                }else
                {
                    int indexOfForest = indexFromLog - f2base;
                    if(isInForest2Updates.at(indexOfForest) == 0)
                    {
                        Forest2Updates.append(indexOfForest);
                        isInForest2Updates.replace(indexOfForest, 1);
                    }

                    changeAmount = changeAmount + forest2->at(indexOfForest);
                    forest2->replace(indexOfForest, changeAmount);
                }

            }
            //continueLoop = false;
            break;
        }

        /*
         * This is the t line.
         * This function will calculate the time to wait in microseconds
         * then convert it to miliseconds.
         */
        case 116:
        {
            QStringList times = line.split(" ", QString::SkipEmptyParts);

            QString sseconds = times.at(1);
            int newSeconds = sseconds.toInt();
            QString sMicro = times.at(2);
            int newMicroseconds = sMicro.toInt();

            int tempSeconds = newSeconds - oldSeconds;
            int tempMicroseconds = newMicroseconds - oldMicroseconds;

            int tempTimeToWait = tempSeconds * 1000000 + tempMicroseconds;

            //The idea here is that when you slow the speed of the graph
            //display, you lower the time to wait. This will increase the
            //resolution of the data being displayed. You will see more
            //of the udates separately.
            if(tempTimeToWait >= (100000 / speedFactorValue))
            {
                oldSeconds = newSeconds;

                delaySec(tempSeconds);

                //Calculate speed factor slowdown
                //tempMiliseconds = tempMiliseconds*speedFactorValue;
                oldMicroseconds = newMicroseconds;

                //convert mili to micro
                int tempMiliseconds = tempMicroseconds/1000;

                //This will slow down the progress through the log
                //when you increase the speedFactorValue.
                delayMili(tempMiliseconds*speedFactorValue);

                drawUpdates(forest1
                            , isInForest1Updates
                            , Forest1Updates
                            , forest1Scene
                            , f1PenHeight
                            , f1base
                            , f1HorizontalReductionValue);

                //This first checks to see if
                if(forest2->size() > 1)
                {
                    drawUpdates(forest2
                                , isInForest2Updates
                                , Forest2Updates
                                , forest2Scene
                                , f2PenHeight
                                , f2base
                                , f2HorizontalReductionValue);
                }




            }
            continueLoop = false;
            break;
        }
        default:
            QMessageBox::information(NULL,"File Name", line);
            continue;
        }//end switch
    }//end while loop
}

/*
 * This function clears the forestScene, then calls either
 * drawSet or drawRelation in drawfunctions.cpp.
 */
void redrawForest(QGraphicsScene *forestScene
                  , QVector<int> *forest, int base
                  , int penHeight, int reductionValue)
{
    forestScene->clear();

    if (base > 0)
    {
        drawSet(forestScene, forest
                , penHeight, reductionValue);
    }else
    {
        drawRelation(forestScene, forest
                     , penHeight, reductionValue);
    }

}
