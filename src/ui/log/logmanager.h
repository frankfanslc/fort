#ifndef LOGMANAGER_H
#define LOGMANAGER_H

#include <QObject>

#include <util/ioc/iocservice.h>

class LogBuffer;
class LogEntry;

class LogManager : public QObject, public IocService
{
    Q_OBJECT

public:
    explicit LogManager(QObject *parent = nullptr);

    virtual void setActive(bool active);

    QString errorMessage() const { return m_errorMessage; }

    void setUp() override;
    void tearDown() override;

signals:
    void activeChanged();
    void errorMessageChanged();

private slots:
    void processLogBuffer(LogBuffer *logBuffer, bool success, quint32 errorCode);

private:
    void setErrorMessage(const QString &errorMessage);

    qint64 currentUnixTime() const;
    void setCurrentUnixTime(qint64 unixTime);

    void readLogAsync();
    void cancelAsyncIo();

    LogBuffer *getFreeBuffer();
    void addFreeBuffer(LogBuffer *logBuffer);

    void readLogEntries(LogBuffer *logBuffer);

private:
    bool m_active = false;

    QList<LogBuffer *> m_freeBuffers;

    QString m_errorMessage;

    qint64 m_currentUnixTime = 0;
};

#endif // LOGMANAGER_H
