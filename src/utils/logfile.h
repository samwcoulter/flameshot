#ifndef LOGFILE_H
#define LOGFILE_H

#include <QFile>
#include <QMutex>
#include <QSharedMemory>
#include <QTextStream>
#include <memory>

class LogFile
{
public:
    static void write(const QString* data);
    static void resetLogFile();
    static QString defaultLogFilePath();

    LogFile(LogFile&& other) = delete;
    LogFile(LogFile& other) = delete;
    LogFile operator=(LogFile& other) = delete;
    LogFile operator=(LogFile&& other) = delete;
    ~LogFile();

private:
    explicit LogFile(QObject* parent);
    void writeToFile(const QString* data);
    std::unique_ptr<QFile> m_logFile;
    std::unique_ptr<QTextStream> m_writer;
    QSharedMemory* m_shm;
    static LogFile* m_instance;
    static QMutex m_mutex;
};

#endif
