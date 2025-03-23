#ifndef __RAID_MANAGER_HPP__
#define __RAID_MANAGER_HPP__

#include <unordered_map> // std::unordered_map
#include <vector> //std::vector
#include <string> //std::string

#include "minion_proxy.hpp" //minion proxy
#include "singleton.hpp" // singleton

typedef unsigned long minion_id;

class MinionProxy;
typedef struct minion_dest
{
    MinionProxy* mp;
    size_t offset;
    size_t len;
    bool is_backup;
} minion_dest_t;

class RaidManager
{
public:
    ~RaidManager() = default;
    
    std::vector<minion_dest_t> GetDest(size_t offset, size_t len);
    

private:
    friend Singleton<RaidManager, std::vector<MinionProxy*>, size_t, size_t>;
    RaidManager(std::vector<MinionProxy*> minion_proxies, size_t minion_size, size_t minion_count);

    std::vector<MinionProxy*> m_minion_proxies;
    size_t m_minion_size;
    size_t m_minion_count;
};

#endif // __RAID_MANAGER_HPP__