#ifndef CONF_H_
#define CONF_H_

#include <map>

int conf_read(const std::string& filename, std::map<std::string, std::string>& my_map);

template <typename Map, typename Key>
bool conf_contains(const Map& pMap, const Key& pKey)
{
    return pMap.find(pKey) != pMap.end();
}

std::string ReadContents(const std::string& file_name);

#endif /* CONF_H_ */
