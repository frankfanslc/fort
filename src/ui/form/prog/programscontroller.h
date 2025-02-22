#ifndef PROGRAMSCONTROLLER_H
#define PROGRAMSCONTROLLER_H

#include <QObject>

class AppInfoCache;
class AppListModel;
class ConfManager;
class FirewallConf;
class FortManager;
class FortSettings;
class IniOptions;
class IniUser;
class TranslationManager;
class WindowManager;

class ProgramsController : public QObject
{
    Q_OBJECT

public:
    explicit ProgramsController(QObject *parent = nullptr);

    FortManager *fortManager() const;
    FortSettings *settings() const;
    ConfManager *confManager() const;
    FirewallConf *conf() const;
    IniOptions *ini() const;
    IniUser *iniUser() const;
    TranslationManager *translationManager() const;
    WindowManager *windowManager() const;
    AppInfoCache *appInfoCache() const;
    AppListModel *appListModel() const { return m_appListModel; }

    void initialize();

signals:
    void retranslateUi();

private:
    AppListModel *m_appListModel = nullptr;
};

#endif // PROGRAMSCONTROLLER_H
