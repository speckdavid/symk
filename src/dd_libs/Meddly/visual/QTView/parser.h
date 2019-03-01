#ifndef PARSER_H
#define PARSER_H
#include <QString>
#include <QTextStream>
#include <QFile>

class Parser
{
public:
    Parser(QString fileName);
    ~Parser();
    QString readLine();
    QString peek();
private:
    QFile *file;
    QTextStream *input;
};

#endif // PARSER_H
