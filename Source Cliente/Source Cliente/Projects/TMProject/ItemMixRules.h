#pragma once
#include <array>

struct ItemRange {
    int start;
    int end;
};

struct MixPairRule {
    int itemIndex;
    int partnerStart;
    int partnerEnd;
};

struct ThirdItemRule {
    ItemRange range;
    int itemIndex;
};

inline bool inRange(int index, ItemRange range) {
    return index >= range.start && index <= range.end;
}

constexpr std::array<ItemRange, 5> SPECIAL_ITEM_RANGES{{
    {1901,1910},
    {1234,1237},
    {1369,1372},
    {1519,1522},
    {1669,1672}
}};
constexpr std::array<int, 1> SPECIAL_SINGLE_ITEMS{{1714}};

constexpr std::array<MixPairRule, 27> MIX_PAIR_RULES{{
    {1901,2491,2494},
    {1902,2551,2554},
    {1903,2611,2614},
    {1904,2671,2674},
    {1905,2731,2734},
    {1906,2791,2794},
    {1907,2859,2862},
    {1908,2863,2866},
    {1909,2895,2898},
    {1910,2935,2938},
    {1234,1221,1221},
    {1235,1222,1222},
    {1236,1223,1223},
    {1237,1224,1224},
    {1369,1356,1356},
    {1370,1357,1357},
    {1371,1358,1358},
    {1372,1359,1359},
    {1519,1506,1506},
    {1520,1507,1507},
    {1521,1508,1508},
    {1522,1509,1509},
    {1669,1656,1656},
    {1670,1657,1657},
    {1671,1658,1658},
    {1672,1659,1659},
    {1714,1711,1711}
}};

constexpr std::array<ThirdItemRule, 6> THIRD_ITEM_RULES{{
    {{1901,1910},772},
    {{1234,1237},542},
    {{1369,1372},542},
    {{1519,1522},542},
    {{1669,1672},542},
    {{1714,1714},772}
}};

inline bool isSpecialMixItem(int index) {
    for (const auto& range : SPECIAL_ITEM_RANGES)
        if (inRange(index, range))
            return true;
    for (const auto& single : SPECIAL_SINGLE_ITEMS)
        if (index == single)
            return true;
    return false;
}

inline bool isValidPair(int item1, int item2) {
    for (const auto& rule : MIX_PAIR_RULES)
        if (item1 == rule.itemIndex && item2 >= rule.partnerStart && item2 <= rule.partnerEnd)
            return true;
    return false;
}

inline bool requiresThirdItem(int item1, int thirdItem) {
    for (const auto& rule : THIRD_ITEM_RULES)
        if (inRange(item1, rule.range))
            return thirdItem == rule.itemIndex;
    return false;
}
