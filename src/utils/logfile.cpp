#include "logfile.h"

#include "confighandler.h"
#include "core/flameshot.h"

#include <QCoreApplication>
#include <QDir>
#include <QMutexLocker>
#include <QStandardPaths>

constexpr const char* LOG_FILE_NAME = "flameshot.log";
constexpr const char* OLD_LOG_FILE_NAME = "flameshot.log.old";

void LogFile::write(const QString* data)
{
    QMutexLocker locker(&m_mutex);
    if (nullptr == m_instance) {
        m_instance = new LogFile(Flameshot::instance());
    }

    m_instance->writeToFile(data);
}

void LogFile::resetLogFile()
{
    QMutexLocker locker(&m_mutex);
    delete m_instance;
    m_instance = nullptr;
}

QString LogFile::defaultLogFilePath()
{
#ifdef QT_STATE_DIR_SUPPORTED
    const auto defaultLocation = QStandardPaths::GenericStateLocation;
#else
    const auto defaultLocation = QStandardPaths::GenericDataLocation;
#endif
    return QDir::toNativeSeparators(
      QStandardPaths::writableLocation(defaultLocation) + "/flameshot");
}

LogFile::LogFile(QObject* parent)
{
    const char* key = "org.flameshot.Flameshot-" APP_VERSION "-logfileroll";
    m_shm = new QSharedMemory(QString(key), parent);

    if (m_shm->create(sizeof(bool))) {
        // We have just created the shared memory, zero it
        // Note also there is a potential race condition here if multiple
        // instances of flameshot start at near the same time (between the
        // create() and lock() calls) but I don't see a way around it using the
        // Qt6 API Also the damage caused by the race condition is minimal:
        // losing a small number of log messages
        m_shm->lock();
        std::memset(m_shm->data(), 0, sizeof(bool));
        m_shm->unlock();
    } else {
        if (QSharedMemory::SharedMemoryError::AlreadyExists == m_shm->error()) {
            if (!m_shm->attach()) {
                throw std::runtime_error(
                  std::format("{}: {} - {}",
                              "Unable to attach to shared memory",
                              (int)m_shm->error(),
                              m_shm->errorString().toStdString()));
            }
        } else {
            throw std::runtime_error(
              std::format("{}: {} - {}",
                          "Unable to create shared memory",
                          (int)m_shm->error(),
                          m_shm->errorString().toStdString()));
        }
    }
    if (!m_shm->isAttached()) {

        throw std::runtime_error(
          std::format("{}: {} - {}",
                      "Shared Memory is not attached",
                      (int)m_shm->error(),
                      m_shm->errorString().toStdString()));
    }

    m_shm->lock();

    auto loggingDirectory = QDir(ConfigHandler().logFilePath());

    bool* logsRolled = static_cast<bool*>(m_shm->data());
    if (!*logsRolled) {

        if (!loggingDirectory.exists()) {
            loggingDirectory.mkpath(".");
        }

        if (loggingDirectory.exists(OLD_LOG_FILE_NAME)) {
            loggingDirectory.remove(OLD_LOG_FILE_NAME);
        }

        if (loggingDirectory.exists(LOG_FILE_NAME)) {
            loggingDirectory.rename(LOG_FILE_NAME, OLD_LOG_FILE_NAME);
        }

        *logsRolled = true;
    }

    m_shm->unlock();

    auto logFilePath = loggingDirectory.filePath(LOG_FILE_NAME);
    m_logFile = std::make_unique<QFile>(logFilePath);
    if (m_logFile->open(QIODeviceBase::WriteOnly | QIODeviceBase::Append)) {
        m_writer = std::make_unique<QTextStream>();
        m_writer->setDevice(m_logFile.get());
    }
}

LogFile::~LogFile()
{
    m_shm->detach();
}

void LogFile::writeToFile(const QString* data)
{

    m_shm->lock();
    if (m_writer) {
        *m_writer << *data;
        m_writer->flush();
    }
    m_shm->unlock();
}

LogFile* LogFile::m_instance = nullptr;
QMutex LogFile::m_mutex;
