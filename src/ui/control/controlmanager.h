#ifndef CONTROLMANAGER_H
#define CONTROLMANAGER_H

#include <QObject>
#include <QVariant>

#include <util/classhelpers.h>
#include <util/ioc/iocservice.h>

#include "control.h"

QT_FORWARD_DECLARE_CLASS(QLocalServer)
QT_FORWARD_DECLARE_CLASS(QLocalSocket)

class ControlWorker;

struct ProcessCommandArgs
{
    ControlWorker *worker = nullptr;
    Control::Command command = Control::CommandNone;
    const QVariantList &args;
    QString &errorMessage;
};

class ControlManager : public QObject, public IocService
{
    Q_OBJECT

public:
    explicit ControlManager(QObject *parent = nullptr);
    ~ControlManager() override;
    CLASS_DELETE_COPY_MOVE(ControlManager)

    const QList<ControlWorker *> &clients() const { return m_clients; }

    void setUp() override;

    bool isCommandClient() const;

    ControlWorker *newServiceClient(QObject *parent = nullptr) const;

    bool listen();
    bool postCommand();

private slots:
    void onNewConnection();
    void onDisconnected();

    bool processRequest(Control::Command command, const QVariantList &args);

private:
    bool processCommand(const ProcessCommandArgs &p);
    bool processCommandProg(const ProcessCommandArgs &p);

    void close();

    static QString getServerName(bool isService = false);

private:
    QLocalServer *m_server = nullptr;

    QList<ControlWorker *> m_clients;
};

#endif // CONTROLMANAGER_H
