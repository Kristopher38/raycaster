#ifndef PARSEDENUM_H
#define PARSEDENUM_H
#include <unordered_map>
#include <string>
#include <stdexcept>

template<typename Key, typename Val> struct ParsedEnum
{
	const std::unordered_map<Key, Val> mapping;
	Val parse(Key key) const
	{
		if (auto it = mapping.find(key); it != mapping.end())
			return it->second;
		else throw std::invalid_argument("No enum mapping found for the key");
	}
};

#endif // PARSEDENUM_H
