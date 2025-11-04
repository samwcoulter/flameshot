#ifndef LOGFILE_H
#define LOGFILE_H

#include <QFile>
#include <QMutex>
#include <QTextStream>

class LogFile
{
public:
    static void write(const QString* data);
    static void resetLogFile();

    LogFile(LogFile&& other) = delete;
    LogFile(LogFile& other) = delete;
    LogFile operator=(LogFile& other) = delete;
    LogFile operator=(LogFile&& other) = delete;
    ~LogFile() = default;

private:
    LogFile();
    void writeToFile(const QString* data);
    QFile* m_logFile;
    QTextStream m_writer;
    static LogFile* m_instance;
    static QMutex m_mutex;
};

#endif
