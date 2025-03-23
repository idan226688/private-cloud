#define __INIT_SYMBOL
#include <raid_manager.hpp>
#include <iostream>

int main()
{
    std::vector<MinionProxy*> mp(10);
    for (size_t i = 0; i < 10; i++)
    {
        mp[i] = NULL;
    }

    RaidManager* rm = Singleton<RaidManager, std::vector<MinionProxy*>, size_t, size_t>::GetInstance(mp, 2000, 10);

    std::vector<minion_dest_t> md = rm->GetDest(300, 2800);
    std::cout << md[7].offset << std::endl;

    return 0;
}
