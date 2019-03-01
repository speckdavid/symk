#include "parser.h"
#include <QMessageBox>
Parser::Parser(QString fileName)
{
    Parser::file = new QFile(fileName);
    Parser::file->open(QFile::ReadOnly);
}

Parser::~Parser()
{
    file->close();
}

QString Parser::readLine()
{
    return file->readLine().trimmed();
}

QString Parser::peek()
{
    QString t = file->peek(1);
    return t;
}
