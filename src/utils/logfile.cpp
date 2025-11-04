#include "logfile.h"

#include "confighandler.h"

#include <QDir>
#include <QMutexLocker>
#include <QStandardPaths>

constexpr const char* LOG_FILE_NAME = "flameshot.log";
constexpr const char* OLD_LOG_FILE_NAME = "flameshot.log.old";

void LogFile::write(const QString* data)
{
    QMutexLocker locker(&m_mutex);
    if (nullptr == m_instance) {
        m_instance = new LogFile();
    }

    m_instance->writeToFile(data);
}

void LogFile::resetLogFile()
{
    QMutexLocker locker(&m_mutex);
    delete m_instance;
    m_instance = nullptr;
}

LogFile::LogFile()
{
    auto stateDirectory = QDir(ConfigHandler().logFilePath());

    if (!stateDirectory.exists()) {
        stateDirectory.mkpath(".");
    }

    if (stateDirectory.exists(OLD_LOG_FILE_NAME)) {
        stateDirectory.remove(OLD_LOG_FILE_NAME);
    }

    if (stateDirectory.exists(LOG_FILE_NAME)) {
        stateDirectory.rename(LOG_FILE_NAME, OLD_LOG_FILE_NAME);
    }

    auto logFilePath = stateDirectory.filePath(LOG_FILE_NAME);
    m_logFile = new QFile(logFilePath);
    m_logFile->open(QIODeviceBase::WriteOnly | QIODeviceBase::Append);
    m_writer.setDevice(m_logFile);
}

void LogFile::writeToFile(const QString* data)
{
    m_writer << *data;
    m_writer.flush();
}

LogFile* LogFile::m_instance = nullptr;
QMutex LogFile::m_mutex;
