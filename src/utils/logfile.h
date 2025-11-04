#ifndef LOGFILE_H
#define LOGFILE_H

#include <QFile>
#include <QMutex>
#include <QTextStream>

class LogFile
{
public:
    static void write(const QString* data);

private:
    LogFile();
    void writeToFile(const QString* data);
    QFile* m_logFile;
    QTextStream m_writer;
    static LogFile* m_instance;
};

#endif
