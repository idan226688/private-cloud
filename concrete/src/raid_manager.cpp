#include "raid_manager.hpp"
// #include <vector>
// #include "minion_proxy.hpp" //minion proxy

extern void* data;

RaidManager::RaidManager(std::vector<MinionProxy*> minion_proxies, size_t minion_size, size_t minion_count)
: m_minion_proxies(minion_proxies), m_minion_size(minion_size),
m_minion_count(minion_count)
{
}

std::vector<minion_dest_t> RaidManager::GetDest(size_t offset, size_t len)
{
    minion_dest_t md;
    size_t real_size = m_minion_size / 2;
    minion_id id = offset / real_size;
    size_t minion_offset = offset - (id * real_size);
    std::vector<minion_dest_t> ret;
    size_t remain = real_size - minion_offset;

    md.len = std::min(len, remain);
    md.mp = m_minion_proxies[id];
    md.offset = minion_offset;
    md.is_backup = false;
    std::cout << "id = " << id << " minion_offset = " << minion_offset << std::endl;
    std::cout << "remain = " << remain << " md.len = " << md.len << std::endl;
    std::cout << "real_size = " << real_size << " len = " << len << std::endl;
    std::cout << "offset = " << offset << "m_minion_count = " << m_minion_count << "minion size =" << m_minion_size  << std::endl;
    ret.push_back(md);

    // backup minion
    id = id == (m_minion_count - 1) ? 0 : id + 1; // first minion backups last. others before them.
    md.offset = 0;
    md.offset += real_size;
    md.mp = m_minion_proxies[id];
    md.is_backup = true;
    ret.push_back(md);

    //next minion if not enough space 
    len -= md.len;
    remain = real_size;
    ++id;

    for (int i = 2; len > 0; i += 2, len -= md.len, ++id)
    {
        md.len = std::min(len, remain);
        md.mp = m_minion_proxies[id];
        md.offset = 0;
        md.is_backup = false;
        ret.push_back(md);

        //backup minion
        md.offset = real_size;
        id = id == m_minion_count - 1 ? 0 : id + 1; // first minion backups last. others before them.
        md.mp = m_minion_proxies[id];
        md.is_backup = true;
        ret.push_back(md);
    }

    return ret; // minion at ret[i + 1] backups minion at ret[i] at second half of memory
}