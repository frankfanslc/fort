#include "addressespage.h"

#include <QCheckBox>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QMenu>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTabBar>
#include <QVBoxLayout>

#include <conf/addressgroup.h>
#include <conf/confmanager.h>
#include <conf/firewallconf.h>
#include <driver/drivercommon.h>
#include <form/controls/controlutil.h>
#include <form/controls/plaintextedit.h>
#include <form/controls/textarea2splitter.h>
#include <form/controls/textarea2splitterhandle.h>
#include <form/opt/optionscontroller.h>
#include <fortmanager.h>
#include <fortsettings.h>
#include <model/zonelistmodel.h>
#include <user/iniuser.h>
#include <util/iconcache.h>
#include <util/net/netutil.h>
#include <util/textareautil.h>

#include "addresses/addressescolumn.h"

AddressesPage::AddressesPage(OptionsController *ctrl, QWidget *parent) : OptBasePage(ctrl, parent)
{
    setupUi();

    setupAddressGroup();
    setupZones();
}

AddressGroup *AddressesPage::addressGroup() const
{
    return addressGroupByIndex(addressGroupIndex());
}

void AddressesPage::setAddressGroupIndex(int v)
{
    if (m_addressGroupIndex != v) {
        m_addressGroupIndex = v;
        emit addressGroupChanged();
    }
}

void AddressesPage::onSaveWindowState(IniUser *ini)
{
    ini->setOptWindowAddrSplit(m_splitter->saveState());
}

void AddressesPage::onRestoreWindowState(IniUser *ini)
{
    m_splitter->restoreState(ini->optWindowAddrSplit());
}

void AddressesPage::onRetranslateUi()
{
    m_tabBar->setTabText(0, tr("Internet Addresses"));
    m_tabBar->setTabText(1, tr("Allowed Internet Addresses"));

    m_includeAddresses->labelTitle()->setText(tr("Include"));
    m_includeAddresses->cbUseAll()->setText(tr("Include All"));
    m_includeAddresses->retranslateUi();

    m_excludeAddresses->labelTitle()->setText(tr("Exclude"));
    m_excludeAddresses->cbUseAll()->setText(tr("Exclude All"));
    m_excludeAddresses->retranslateUi();

    auto splitterHandle = m_splitter->handle();
    splitterHandle->btMoveAllFrom1To2()->setToolTip(tr("Move All Lines to 'Exclude'"));
    splitterHandle->btMoveAllFrom2To1()->setToolTip(tr("Move All Lines to 'Include'"));
    splitterHandle->btInterchangeAll()->setToolTip(tr("Interchange All Lines"));
    splitterHandle->btMoveSelectedFrom1To2()->setToolTip(tr("Move Selected Lines to 'Exclude'"));
    splitterHandle->btMoveSelectedFrom2To1()->setToolTip(tr("Move Selected Lines to 'Include'"));
    m_btAddLocals->setToolTip(tr("Add Local Networks"));

    retranslateAddressesPlaceholderText();
}

void AddressesPage::retranslateAddressesPlaceholderText()
{
    const auto placeholderText = tr("# Examples:") + '\n' + localNetworks();

    m_excludeAddresses->editIpText()->setPlaceholderText(placeholderText);
}

void AddressesPage::setupUi()
{
    auto layout = new QVBoxLayout();

    // Tab Bar
    m_tabBar = new QTabBar();
    m_tabBar->setShape(QTabBar::TriangularNorth);
    layout->addWidget(m_tabBar);

    m_tabBar->addTab(IconCache::icon(":/icons/global_telecom.png"), QString());
    m_tabBar->addTab(IconCache::icon(":/icons/filter.png"), QString());

    // Address Columns
    setupIncludeAddresses();
    setupExcludeAddresses();

    setupAddressesUseAllEnabled();

    // Splitter
    setupSplitter();
    layout->addWidget(m_splitter, 1);

    // Splitter Buttons
    setupSplitterButtons();

    this->setLayout(layout);
}

void AddressesPage::setupIncludeAddresses()
{
    m_includeAddresses = new AddressesColumn();

    connect(m_includeAddresses->cbUseAll(), &QCheckBox::toggled, this, [&](bool checked) {
        if (addressGroup()->includeAll() == checked)
            return;

        addressGroup()->setIncludeAll(checked);

        ctrl()->setFlagsEdited();
    });
    connect(m_includeAddresses->editIpText(), &QPlainTextEdit::textChanged, this, [&] {
        const auto ipText = m_includeAddresses->editIpText()->toPlainText();

        if (addressGroup()->includeText() == ipText)
            return;

        addressGroup()->setIncludeText(ipText);

        ctrl()->setOptEdited();
    });
}

void AddressesPage::setupExcludeAddresses()
{
    m_excludeAddresses = new AddressesColumn();

    connect(m_excludeAddresses->cbUseAll(), &QCheckBox::toggled, this, [&](bool checked) {
        if (addressGroup()->excludeAll() == checked)
            return;

        addressGroup()->setExcludeAll(checked);

        ctrl()->setFlagsEdited();
    });
    connect(m_excludeAddresses->editIpText(), &QPlainTextEdit::textChanged, this, [&] {
        const auto ipText = m_excludeAddresses->editIpText()->toPlainText();

        if (addressGroup()->excludeText() == ipText)
            return;

        addressGroup()->setExcludeText(ipText);

        ctrl()->setOptEdited();
    });
}

void AddressesPage::setupAddressesUseAllEnabled()
{
    const auto refreshUseAllEnabled = [&] {
        auto cbIncludeAll = m_includeAddresses->cbUseAll();
        auto cbExcludeAll = m_excludeAddresses->cbUseAll();

        const bool includeAll = cbIncludeAll->isChecked();
        const bool excludeAll = cbExcludeAll->isChecked();

        cbIncludeAll->setEnabled(includeAll || !excludeAll);
        cbExcludeAll->setEnabled(!includeAll || excludeAll);
    };

    refreshUseAllEnabled();

    connect(m_includeAddresses->cbUseAll(), &QCheckBox::toggled, this, refreshUseAllEnabled);
    connect(m_excludeAddresses->cbUseAll(), &QCheckBox::toggled, this, refreshUseAllEnabled);
}

void AddressesPage::setupSplitter()
{
    m_splitter = new TextArea2Splitter();

    Q_ASSERT(!m_splitter->handle());

    m_splitter->addWidget(m_includeAddresses);
    m_splitter->addWidget(m_excludeAddresses);

    auto splitterHandle = m_splitter->handle();
    Q_ASSERT(splitterHandle);

    splitterHandle->setTextArea1(m_includeAddresses->editIpText());
    splitterHandle->setTextArea2(m_excludeAddresses->editIpText());
}

void AddressesPage::setupSplitterButtons()
{
    m_btAddLocals = ControlUtil::createSplitterButton(":/icons/drive_network.png", [&] {
        auto area = m_splitter->handle()->currentTextArea();
        TextAreaUtil::appendText(area, localNetworks());
    });

    const auto layout = m_splitter->handle()->buttonsLayout();
    layout->addWidget(m_btAddLocals, 0, Qt::AlignHCenter);
}

void AddressesPage::updateGroup()
{
    m_includeAddresses->cbUseAll()->setChecked(addressGroup()->includeAll());
    m_includeAddresses->editIpText()->setText(addressGroup()->includeText());

    m_excludeAddresses->cbUseAll()->setChecked(addressGroup()->excludeAll());
    m_excludeAddresses->editIpText()->setText(addressGroup()->excludeText());

    updateZonesTextAll();
}

void AddressesPage::setupAddressGroup()
{
    connect(this, &AddressesPage::addressGroupChanged, this, &AddressesPage::updateGroup);

    const auto refreshAddressGroup = [&] {
        const int tabIndex = m_tabBar->currentIndex();
        setAddressGroupIndex(tabIndex);
    };

    refreshAddressGroup();

    connect(m_tabBar, &QTabBar::currentChanged, this, refreshAddressGroup);
}

void AddressesPage::clearZonesMenu()
{
    m_menuZones->close();
    m_menuZones->clear();
}

void AddressesPage::createZonesMenu()
{
    const auto onZoneActionTriggered = [&](bool checked) {
        auto action = qobject_cast<QAction *>(sender());
        const int zoneId = action->data().toInt();

        const bool include = m_includeAddresses->btSelectZones()->isDown();
        auto addrGroup = this->addressGroup();

        if (checked) {
            if (include) {
                addrGroup->addIncludeZone(zoneId);
            } else {
                addrGroup->addExcludeZone(zoneId);
            }
        } else {
            if (include) {
                addrGroup->removeIncludeZone(zoneId);
            } else {
                addrGroup->removeExcludeZone(zoneId);
            }
        }

        ctrl()->setOptEdited();

        updateZonesText(include);
    };

    const int zoneCount = zoneListModel()->rowCount();
    for (int row = 0; row < zoneCount; ++row) {
        const auto zoneRow = zoneListModel()->zoneRowAt(row);

        auto action = new QAction(zoneRow.zoneName, m_menuZones);
        action->setCheckable(true);
        action->setData(zoneRow.zoneId);

        connect(action, &QAction::triggered, this, onZoneActionTriggered);

        m_menuZones->addAction(action);
    }
}

void AddressesPage::updateZonesMenu(bool include)
{
    if (m_menuZones->isEmpty()) {
        createZonesMenu();
    }

    const auto actions = m_menuZones->actions();
    if (actions.isEmpty())
        return;

    const quint32 zonesMask = addressGroupZones(include);

    for (auto action : actions) {
        const int zoneId = action->data().toInt();
        const quint32 zoneMask = (quint32(1) << (zoneId - 1));
        const bool checked = (zonesMask & zoneMask) != 0;

        action->setChecked(checked);
    }
}

void AddressesPage::updateZonesMenuEnabled()
{
    const bool isZoneExist = (zoneListModel()->rowCount() != 0);

    m_includeAddresses->btSelectZones()->setEnabled(isZoneExist);
    m_excludeAddresses->btSelectZones()->setEnabled(isZoneExist);
}

void AddressesPage::updateZonesText(bool include)
{
    if (include) {
        m_includeAddresses->labelZones()->setText(zonesText(true));
    } else {
        m_excludeAddresses->labelZones()->setText(zonesText(false));
    }
}

void AddressesPage::updateZonesTextAll()
{
    updateZonesText(true);
    updateZonesText(false);
}

void AddressesPage::setupZones()
{
    m_menuZones = new QMenu(this);

    const auto refreshZonesMenu = [&] {
        const bool include = (sender() == m_includeAddresses->btSelectZones());

        updateZonesMenu(include);
    };

    connect(m_includeAddresses->btSelectZones(), &QPushButton::pressed, this, refreshZonesMenu);
    connect(m_excludeAddresses->btSelectZones(), &QPushButton::pressed, this, refreshZonesMenu);

    m_includeAddresses->btSelectZones()->setMenu(m_menuZones);
    m_excludeAddresses->btSelectZones()->setMenu(m_menuZones);
    updateZonesMenuEnabled();

    connect(confManager(), &ConfManager::zoneRemoved, this, [&](int zoneId) {
        for (auto addrGroup : addressGroups()) {
            addrGroup->removeIncludeZone(zoneId);
            addrGroup->removeExcludeZone(zoneId);
        }
    });
    connect(zoneListModel(), &ZoneListModel::modelChanged, this, [&] {
        clearZonesMenu();
        updateZonesMenuEnabled();
        updateZonesTextAll();
    });
}

const QList<AddressGroup *> &AddressesPage::addressGroups() const
{
    return conf()->addressGroups();
}

AddressGroup *AddressesPage::addressGroupByIndex(int index) const
{
    return addressGroups().at(index);
}

quint32 AddressesPage::addressGroupZones(bool include) const
{
    return include ? addressGroup()->includeZones() : addressGroup()->excludeZones();
}

QString AddressesPage::zonesText(bool include) const
{
    QStringList list;

    quint32 zonesMask = addressGroupZones(include);
    while (zonesMask != 0) {
        const int zoneIndex = DriverCommon::bitScanForward(zonesMask);
        const int zoneId = zoneIndex + 1;
        const auto zoneName = zoneListModel()->zoneNameById(zoneId);

        list.append(zoneName);

        zonesMask ^= (quint32(1) << zoneIndex);
    }

    return list.join(", ");
}

QString AddressesPage::localNetworks()
{
    return NetUtil::localIpv4Networks().join('\n') + '\n';
}
