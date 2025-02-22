#include "logger.h"

#include <fort_version.h>

#include <util/dateutil.h>
#include <util/fileutil.h>
#include <util/osutil.h>

namespace {

constexpr int LOGGER_FILE_MAX_SIZE = 1024 * 1024;
constexpr int LOGGER_KEEP_FILES = 7;

QtMessageHandler g_oldMessageHandler = nullptr;

void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &message)
{
    Logger::LogLevel level = Logger::Info;

    switch (type) {
    case QtWarningMsg:
        level = Logger::Warning;
        break;
    case QtCriticalMsg:
    case QtFatalMsg:
        level = Logger::Error;
        break;
    default:
        break;
    }

    Logger *logger = Logger::instance();

    // Write only errors to log file
    if (logger->debug() || level != Logger::Info) {
        const bool isDefaultCategory = !context.category || !strcmp(context.category, "default");
        const QString text =
                isDefaultCategory ? message : QLatin1String(context.category) + ": " + message;

        logger->writeLog(text, level);
    }

    // Additionally write to console if needed
    if (logger->console()) {
        OsUtil::writeToConsole(context.category, message);
    }

    if (g_oldMessageHandler) {
        g_oldMessageHandler(type, context, message);
    }
}

}

Logger::Logger(QObject *parent) :
    QObject(parent), m_isService(false), m_debug(false), m_console(false), m_writing(false)
{
    g_oldMessageHandler = qInstallMessageHandler(messageHandler);
}

void Logger::setIsService(bool v)
{
    m_isService = v;
}

void Logger::setDebug(bool v)
{
    if (m_debug != v) {
        m_debug = v;

        QLoggingCategory::setFilterRules(debug() ? QString() : "*.debug=false");
    }
}

void Logger::setConsole(bool v)
{
    if (m_console != v) {
        m_console = v;

        if (!isService()) {
            OsUtil::showConsole(console());
        }
    }
}

void Logger::setPath(const QString &path)
{
    m_dir.setPath(path);
}

Logger *Logger::instance()
{
    static Logger *g_instanceLogger = nullptr;

    if (!g_instanceLogger) {
        g_instanceLogger = new Logger();
    }
    return g_instanceLogger;
}

QString Logger::fileNamePrefix() const
{
    return QLatin1String("log_fort_") + (isService() ? "svc_" : QString());
}

QString Logger::fileNameSuffix() const
{
    return QLatin1String(".txt");
}

bool Logger::openLogFile()
{
    const QString fileName = fileNamePrefix() + DateUtil::now().toString("yyyy-MM-dd_HH-mm-ss_zzz")
            + fileNameSuffix();

    if (tryOpenLogFile(m_dir, fileName))
        return true;

    m_dir.setPath(FileUtil::pathSlash(FileUtil::appConfigLocation()) + "logs/");
    if (tryOpenLogFile(m_dir, fileName))
        return true;

    m_dir.setPath(FileUtil::pathSlash(FileUtil::tempLocation()) + APP_NAME);
    if (tryOpenLogFile(m_dir, fileName))
        return true;

    qDebug() << "Cannot open log file:" << m_file.fileName() << m_file.errorString();
    return false;
}

bool Logger::tryOpenLogFile(const QDir &dir, const QString &fileName)
{
    dir.mkpath("./");

    m_file.setFileName(dir.filePath(fileName));

    return m_file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);
}

void Logger::closeLogFile()
{
    m_file.close();
}

void Logger::writeLogLine(Logger::LogLevel level, const QString &dateString, const QString &message)
{
    static const char *const g_levelChars = "IWE";

    const QString line = dateString + ' ' + g_levelChars[int(level)] + ' ' + message + '\n';

    m_file.write(line.toUtf8());
    m_file.flush();
}

void Logger::writeLog(const QString &message, Logger::LogLevel level)
{
    if (m_writing)
        return; // avoid recursive calls

    m_writing = true;

    const QString dateString = DateUtil::now().toString("yyyy-MM-dd HH:mm:ss.zzz");

    // Create file when required to avoid empty files
    if (!m_file.isOpen()) {
        FileUtil::removeOldFiles(
                m_dir.path(), fileNamePrefix(), fileNameSuffix(), LOGGER_KEEP_FILES);

        if (!openLogFile()) {
            m_writing = false;
            return;
        }

        // Write file header
        writeLogLine(Info, dateString, APP_NAME " v" APP_VERSION_STR);
    }

    writeLogLine(level, dateString, message);

    if (m_file.size() > LOGGER_FILE_MAX_SIZE) {
        closeLogFile(); // Too big file
    }

    m_writing = false;
}

void Logger::writeLogList(const QString &message, const QStringList &list, Logger::LogLevel level)
{
    writeLog(message + '{' + list.join(',') + '}', level);
}
