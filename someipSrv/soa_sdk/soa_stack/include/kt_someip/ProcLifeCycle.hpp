/*
 * =====================================================================================
 *
 *       Filename:  ProcLifeCycle.hpp
 *
 *    Description:  Description
 *
 *        Version:  1.0
 *        Created:  2023年11月08日 10时15分47秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  lijin (jin), jinli@syncore.space
 *   Organization:  SYNCORE
 *
 * =====================================================================================
 */


#ifndef __PROCE_LIFECYCLE_HEADER__
#define __PROCE_LIFECYCLE_HEADER__ 1

#include <stdio.h>
#include <iostream>
#include <string>
#include <thread>
#include <errno.h>
#include <unistd.h>
#include <utility>
#include <unistd.h>
#include <libgen.h>
#include <ftw.h>
#include <vector>
#include <sstream>
#include <string.h>
#include <mutex>

#if CONFIG_ENABLE_SOMEIPD_HAM
#include <ha/ham.h>
#endif // CONFIG_ENABLE_SOMEIPD_HAM

#include "kt_someip/log/log.hpp"

void *HamHeartbeatThread(void *val);
void *HamRegisterThread(void *param);

class ProcLifeCycle
{
public:
    ProcLifeCycle ();
    virtual ~ProcLifeCycle ();

    std::vector<std::string> TransferStringToVector(const char* val);
    bool initializeHam(const char *pname, const char *top_name, const char *top_path, const char *callup, const char *slay);
    void configureHamDeath(int sentChannelId, pid_t toPid, uint64_t uid);

    int registerHamCondition();
    int registerConditionDeath(const char *cmd);
    int registerConditionHBL(const char *cmd);
    int registerConditionHBH(const char *cmd);

    void releaseHamResources();

    void unregisterHamCondition();
    void unregisterHamConditionHandle();
    void unregisterHamConditionHBL();
    void unregisterHamConditionHBH();

    void unregisterHamAction();
    void unregisterHamActionHandle();
    void unregisterHamActionHBL();
    void unregisterHamActionHBH();

    bool getHamInitStat();
    int DisableHamFeature(const char *val);

    int LocateFilePath(const char *spath, const char *sname, char *val);
    int LocateTopPath(const char *ecmd, const char *keywords);

private:
    bool hamInitialized;

    std::string ham_slay_script;
    std::string ham_warn_script;
    std::string ham_callup_script;

    std::vector<std::string> top_path_list;
    std::vector<std::string> top_dir_name_list;

    std::string ename;
    // std::string exec_cmd;
    std::string project_top;
    std::string app_path;
    std::string app_name;

#if CONFIG_ENABLE_SOMEIPD_HAM
    ham_entity_t *hamEntityHandle;
    ham_condition_t *hamConditionHandle;
    ham_condition_t *hamConditionHBL;
    ham_condition_t *hamConditionHBH;

    ham_action_t *maction_handle;
    ham_action_t *maction_hbl;
    ham_action_t *maction_hbh;
#endif // CONFIG_ENABLE_SOMEIPD_HAM

    std::mutex mlock;
};


struct ProcLifeCycle_t {
    ProcLifeCycle *pLife;
    char exec_cmd[1024];
    pthread_t m_id;
};


#endif /* ifndef __PROCE_LIFECYCLE_HEADER__ */


