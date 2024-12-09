#ifndef _TABLE_DRIVE_TOOL_H_
#define _TABLE_DRIVE_TOOL_H_
#include <algorithm>

template<typename CON, typename PRED, typename Gettor, typename Default>
auto convert_if(CON& _container, PRED _pred, Gettor _getter, Default _default = Default()) \
-> decltype(_getter(*std::begin(_container)))
{
    auto target = std::find_if(std::begin(_container), std::end(_container), _pred);
    if (target != std::end(_container))
    {
        return _getter(*target);
    }
    return _default;
}

template<typename CON, typename PRED, typename Gettor, typename Default>
auto convert_if_reverse(CON& _container, PRED _pred, Gettor _getter, Default _default = Default()) \
-> decltype(_getter(*std::begin(_container)))
{
    auto target = std::find_if(std::rbegin(_container), std::rend(_container), _pred);
    if (target != std::rend(_container))
    {
        return _getter(*target);
    }
    return _default;
}

#define MatchAndPickupItem(data_set, expre1, expre2)		convert_if(data_set, \
			[&](const std::remove_reference<decltype(*std::begin(std::declval<decltype(data_set)&>()))>::type & _item) {return expre1;}, \
			[&](const std::remove_reference<decltype(*std::begin(std::declval<decltype(data_set)&>()))>::type & _item) {return expre2;})

#define MatchAndPickupItem_withDefault(data_set, expre1, expre2, default_result)		convert_if(data_set, \
			[&](const std::remove_reference<decltype(*std::begin(std::declval<decltype(data_set)&>()))>::type & _item) {return expre1;}, \
			[&](const std::remove_reference<decltype(*std::begin(std::declval<decltype(data_set)&>()))>::type & _item) {return expre2;}, \
			default_result )

#define ReverseMatchAndPickupItem(data_set, expre1, expre2)		convert_if_reverse(data_set, \
			[&](const std::remove_reference<decltype(*std::begin(std::declval<decltype(data_set)&>()))>::type & _item) {return expre1;}, \
			[&](const std::remove_reference<decltype(*std::begin(std::declval<decltype(data_set)&>()))>::type & _item) {return expre2;})

#define ReverseMatchAndPickupItem_withDefault(data_set, expre1, expre2, default_result)		convert_if_reverse(data_set, \
			[&](const std::remove_reference<decltype(*std::begin(std::declval<decltype(data_set)&>()))>::type & _item) {return expre1;}, \
			[&](const std::remove_reference<decltype(*std::begin(std::declval<decltype(data_set)&>()))>::type & _item) {return expre2;}, \
			default_result )

/*****************************************************************/




#if 0
std::vector<UM_DATABLOCK> test;
test.push_back(UM_DATABLOCK(10, (PAYLOAD_DATA)0x10));
test.push_back(UM_DATABLOCK(20, (PAYLOAD_DATA)0x20));
test.push_back(UM_DATABLOCK(30, (PAYLOAD_DATA)0x30));
convertIf_withDefault(test, 20 == item.taskID, item.data, (PAYLOAD_DATA)10);

UM_DATABLOCK test2[10] = {};
convertIf_withDefault(test2, 20 == item.taskID, item.data, (PAYLOAD_DATA)100);

int test3[10] = { 0 };
convertIf(test3, 20 == item, item);

Example1
static bool isValueInRegion(const string& newVal)
{
    const char* regionList[] = { "master", "emea", "us", "kor", "asia", "latam" };

    return MatchAndPickupItem(regionList, newVal == item, true, false);
}

Example2
string bkup_ctrl_convert2PathFormat(const string& path)
{
    pair<string, string> s_PathFormat[] =
    {
        { BKUP_CTRL_PATH_KEY,		BKUP_CTRL_PATH_KEY_SHM },
        { BKUP_CTRL_PATH_CSR,		BKUP_CTRL_PATH_CSR_SHM },
        { BKUP_CTRL_PATH_ROOTCA,	BKUP_CTRL_PATH_ROOTCA_SHM },
        { BKUP_CTRL_PATH_SECURE,	BKUP_CTRL_PATH_SECURE_SHM },
        { BKUP_CTRL_PATH_URL,		BKUP_CTRL_PATH_URL_SHM },
        { BKUP_CTRL_PATH_CERT,		BKUP_CTRL_PATH_CERT_SHM },
        { BKUP_CTRL_PATH_COMMIT,	BKUP_CTRL_PATH_COMMIT_FILE },
        { BKUP_CTRL_PATH_COMMIT_NOERASE, BKUP_CTRL_PATH_COMMIT_NOERASE_FILE },
        { BKUP_CTRL_PATH_DATA,		BKUP_CTRL_PATH_DATA_FILE }
    };

    return MatchAndPickupItem_withDefault(s_PathFormat, path == item.first, item.second, "");
}



Example3
bool bkup_ctrl_chkRstrSts(BkupCtrl_SubIndex sIndex)
{
    BkupCtrl_MainIndex mIndex = BKUP_CTRL_MAIN_INDEX_MAX;

    auto iter = g_bkup_sIndTbl.find(sIndex);
    if (iter != g_bkup_sIndTbl.end())
    {
        mIndex = iter->second.datAttr.mIndex;
    }
    else
    {
        return false;
    }

    return convertIf_withDefault(g_bkup_mIndTbl, item.second.mIndex == mIndex, item.second.chckRslt, false);
}


#endif

#endif