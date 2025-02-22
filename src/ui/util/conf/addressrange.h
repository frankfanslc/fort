#ifndef ADDRESSRANGE_H
#define ADDRESSRANGE_H

#include <QObject>
#include <QVariant>

#include <util/net/ip4range.h>

class AddressRange
{
public:
    explicit AddressRange();

    bool includeAll() const { return m_includeAll; }
    void setIncludeAll(bool includeAll) { m_includeAll = includeAll; }

    bool excludeAll() const { return m_excludeAll; }
    void setExcludeAll(bool excludeAll) { m_excludeAll = excludeAll; }

    bool includeIsEmpty() const { return includeRange().isEmpty(); }
    bool excludeIsEmpty() const { return excludeRange().isEmpty(); }

    quint32 includeZones() const { return m_includeZones; }
    void setIncludeZones(quint32 v) { m_includeZones = v; }

    quint32 excludeZones() const { return m_excludeZones; }
    void setExcludeZones(quint32 v) { m_excludeZones = v; }

    Ip4Range &includeRange() { return m_includeRange; }
    Ip4Range &excludeRange() { return m_excludeRange; }

    const Ip4Range &includeRange() const { return m_includeRange; }
    const Ip4Range &excludeRange() const { return m_excludeRange; }

private:
    bool m_includeAll : 1;
    bool m_excludeAll : 1;

    quint32 m_includeZones = 0;
    quint32 m_excludeZones = 0;

    Ip4Range m_includeRange;
    Ip4Range m_excludeRange;
};

#endif // ADDRESSRANGE_H
