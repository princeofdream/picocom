#ifndef __MISSIONS_HELPER_H__
#define __MISSIONS_HELPER_H__

#include <memory>
#include <string>
#include <deque>
#include <list>

#include "MissionBase.h"
//#include "ScopeGuard.h"
#include "Timer.h"

namespace apf {
namespace mission {
//using Message = Courier::Message;
template <typename Message>
using ApfJobBase       = Job<Message>;
template <typename Message>
using ApfJobHandle     = std::shared_ptr<ApfJobBase<Message>>;
template <typename Message>
using ApfMissionBase   = MissionBase<Message>;
template <typename Message>
using ApfMissionHandle = std::shared_ptr<ApfMissionBase<Message>>;

template <typename Message, typename T, typename ...Args>
ApfJobHandle<Message> CreateJob(Args&&... args) {
    return std::make_shared<T>(std::forward<Args>(args)...);
}

template <typename Message, typename T, typename ...Args>
ApfMissionHandle<Message> CreateMission(Args&&... args) {
    return std::make_shared<T>(std::forward<Args>(args)...);
}

////////////////////////////

template <typename TaskType, typename TypeOperator, typename Message>
class TaskQueue {
public:
    using TaskQueueContainer = std::deque<TaskType>;

public:
    TaskQueue() {
        m_triggled = false;
    }

    enum TaskQueueStatus
    {
        DOING,
        DONE
    };

    void AddTaskToBack(TaskType const & _task) {
        m_tasks.push_back(_task);
    }

    void AddTaskToBack(TaskType && _task) {
        m_tasks.push_back(std::move(_task));
    }

    void AddTaskToFront(TaskType const& _task) {
        m_tasks.push_front(_task);
    }

    void AddTaskToFront(TaskType&& _task) {
        m_tasks.push_front(std::move(_task));
    }

    void AddTaskQueueToBack(TaskQueue const& _tasks) {
        m_tasks.insert(m_tasks.end(), _tasks.m_tasks.begin(), _tasks.m_tasks.end());
    }

    void AddTaskQueueToBack(TaskQueue && _tasks) {
        m_tasks.insert(m_tasks.end(), 
            std::make_move_iterator(_tasks.m_tasks.begin()), 
            std::make_move_iterator(_tasks.m_tasks.end()));
    }
    void AddTaskQueueToFront(TaskQueue const& _tasks) {
        m_tasks.insert(m_tasks.begin(), _tasks.m_tasks.begin(), _tasks.m_tasks.end());
    }

    void AddTaskQueueToFront(TaskQueue&& _tasks) {
        m_tasks.insert(m_tasks.begin(),
            std::make_move_iterator(_tasks.m_tasks.begin()),
            std::make_move_iterator(_tasks.m_tasks.end()));
    }

    TaskQueueStatus StartRunTasks(Message const* _msg) {
        m_triggled = true;

        if (m_tasks.empty()) {
            FEATHER_LOG("[TaskQueue::StartRunTasks] Mission is empty");
            m_triggled = false;
            return TaskQueueStatus::DONE;
        }

        do {
            auto & top_task = m_tasks.front();
            ENSURE(top_task);
            StartTask(top_task, _msg);
            if (IsTaskEnd(top_task)) {
                m_tasks.pop_front();
                if (m_tasks.empty()) {
                    m_triggled = false;
                    return TaskQueueStatus::DONE;
                }
            }
            else {
                return TaskQueueStatus::DOING;
            }
        } while (!m_tasks.empty());

        return TaskQueueStatus::DOING;
    }
    
    TaskQueueStatus ContinueRunTasks(Message const& _msg) {
        if (m_tasks.empty()) {
            m_triggled = false;
            return TaskQueueStatus::DONE;
        }

        auto & top_task = m_tasks.front();
        ENSURE(top_task);
        ContiuneTask(top_task, _msg);
        if (IsTaskEnd(top_task)) {
            m_tasks.pop_front();
            if (m_tasks.empty()) {
                m_triggled = false;
                return TaskQueueStatus::DONE;
            }
            else {
                return StartRunTasks(&_msg);
            }
        }

        return TaskQueueStatus::DOING;
    }

    TaskQueueStatus Execute(Message const* _msg) {
        if (!m_triggled) {
            return StartRunTasks(_msg);
        }
        else {
            if (_msg == nullptr) {
                return DOING;
            }
            return ContinueRunTasks(*_msg);
        }
    }

    bool IsAllTaskFinished() {
        return m_tasks.empty();
    }

    bool Empty() const {
        return m_tasks.empty();
    }

    bool IsTriggled() const {
        return m_triggled;
    }

    void Clear() {
        m_tasks.clear();
    }

    TaskQueueContainer const& GetTaskQueue(void) const {
        return m_tasks;
    }

private:
    void StartTask(TaskType const& _task, Message const* _msg) {
        TypeOperator::StartTask(_task, _msg);
    }
    void ContiuneTask(TaskType const& _task, Message const& _msg) {
        TypeOperator::ContiuneTask(_task, _msg);
    }
    bool IsTaskEnd(TaskType const& _task) {
        return TypeOperator::IsTaskEnd(_task);
    }

private:
    TaskQueueContainer  m_tasks;
    bool                m_triggled; // task queue started? be false until all task ended
};

template <typename Message>
class MissionQueue : public TaskQueue<ApfMissionHandle<Message>, MissionQueue<Message>, Message>/*, public LockBeforeUse*/ {
public:
    static void StartTask(ApfMissionHandle<Message> const& _handle, Message const* _msg) {
        _handle->startMission(_msg);
    }
    static void ContiuneTask(ApfMissionHandle<Message> const& _handle, Message const& _msg) {
        _handle->runMission(_msg);
    }
    static bool IsTaskEnd(ApfMissionHandle<Message> const& _handle) {
        return _handle->isMissionEnd();
    }
};

//do multiple task at the same time
template <typename TaskType, typename TypeOperator, typename Message>
class TaskPool {
public:
    using TaskListContainer = std::list<TaskType>;

public:
    TaskPool() {
    }

    void AddTask(TaskType const& _task)
    {
        m_tasks.push_back(_task);
    }

    void Execute(Message const* _msg) {
        for (auto& task : m_tasks)
        {
            if (!IsTaskTriggled(task))
            {
                StartTask(task, _msg);
            }
            else
            {
                ContiuneTask(task, *_msg);
            }
        }
        m_tasks.remove_if([](TaskType const& _x) {
            return TypeOperator::IsTaskEnd(_x);
            });
    }

    bool IsAllTaskFinished() {
        return m_tasks.empty();
    }

    bool Empty() const {
        return m_tasks.empty();
    }

    void Clear() {
        m_tasks.clear();
    }

    TaskListContainer const& GetTaskList(void) const {
        return m_tasks;
    }

private:
    void StartTask(TaskType const& _task, Message const* _msg) {
        TypeOperator::StartTask(_task, _msg);
    }
    void ContiuneTask(TaskType const& _task, Message const& _msg) {
        TypeOperator::ContiuneTask(_task, _msg);
    }
    bool IsTaskEnd(TaskType const& _task) {
        return TypeOperator::IsTaskEnd(_task);
    }
    bool IsTaskTriggled(TaskType const& _task) {
        return TypeOperator::IsTaskTriggled(_task);
    }

private:
    TaskListContainer  m_tasks;
};

template <typename Message>
class MissionPool : public TaskPool<ApfMissionHandle<Message>, MissionPool<Message>, Message>/*, public LockBeforeUse*/ {
public:
    static void StartTask(ApfMissionHandle<Message> const& _handle, Message const* _msg) {
        _handle->startMission(_msg);
    }
    static void ContiuneTask(ApfMissionHandle<Message> const& _handle, Message const& _msg) {
        _handle->runMission(_msg);
    }
    static bool IsTaskEnd(ApfMissionHandle<Message> const& _handle) {
        return _handle->isMissionEnd();
    }
    static bool IsTaskTriggled(ApfMissionHandle<Message> const& _handle) {
        return _handle->isTriggled();
    }
};

////////////////////////////
template <typename Message>
class MessageTimer {
public:
    static bool isTimeOut(const Message& _msg, const uint32_t& _timer_id);
};

template <typename Message>
class ApfJobWithTimer : public ApfJobBase<Message> {
public:
    typename ApfJobBase<Message>::JobStatus doJob(apf::no_ownership_ptr<Message const> _msg) override {
        auto time_for_job = DOWithinTime(_msg);
        if (time_for_job == NOT_WAIT) {
            return ApfJobBase<Message>::JobStatus::DONE;
        }

        m_timer_id = apf::utility::TimerManager::getInstance().setTimerByGenID(time_for_job);
        return ApfJobBase<Message>::JobStatus::WAIT_FOR_RESULT;

    }

    typename ApfJobBase<Message>::JobResult checkResult(const Message& _msg) override {
        if (MessageTimer<Message>::isTimeOut(_msg,m_timer_id)){
            return ApfJobBase<Message>::JobResult::FAILED;
        }
        else if (ApfJobBase<Message>::JobResult::SUCCESSED == WaitForResult(_msg)) {
            apf::utility::TimerManager::getInstance().cancelTimer(m_timer_id);
            return ApfJobBase<Message>::JobResult::SUCCESSED;
        }
        return ApfJobBase<Message>::JobResult::UNKONW;
    }

    // @return How much time (in millisecond)
    virtual uint32_t DOWithinTime(apf::no_ownership_ptr<Message const> _msg) = 0;
    virtual typename ApfJobBase<Message>::JobResult WaitForResult(const Message& _msg) = 0;

protected:
    static uint32_t const NOT_WAIT = 0;

private:
    apf::utility::TimerManager::TimerId m_timer_id; // @todo: timer customize
};

template <typename Message>
using JobExecutor           = std::function<void(apf::no_ownership_ptr<Message const> _msg)>;
template <typename Message>
using JobExecutorWithinTime = std::function<uint32_t(apf::no_ownership_ptr<Message const> _msg)>;
template <typename Message>
using JobExecutorWithStatus = std::function<typename ApfJobBase<Message>::JobStatus(apf::no_ownership_ptr<Message const> _msg)>;
template <typename Message>
using JobResultChecker      = std::function<typename ApfJobBase<Message>::JobResult(const Message& _msg)>;
template <typename Message>
using JobTreeMaker          = std::function<void(ApfMissionHandle<Message>)>;

template <typename Message>
class SimpleJobHelper : public ApfJobBase<Message> {
public:
    SimpleJobHelper(JobExecutor<Message> _executor)
        : m_executor(_executor)
    {    }

    typename ApfJobBase<Message>::JobStatus doJob(apf::no_ownership_ptr<Message const> _msg) override {
        m_executor(_msg);
        return ApfJobBase<Message>::JobStatus::DONE;
    }

private:
    JobExecutor<Message>     m_executor;
};

template <typename Message>
class SimpleJobHelper2 : public ApfJobWithTimer<Message> {
public:
    SimpleJobHelper2(JobExecutorWithinTime<Message> _do, JobResultChecker<Message> _checker)
        : m_do(_do), m_checker(_checker)
    {  }

    uint32_t DOWithinTime(apf::no_ownership_ptr<Message const> _msg) override {
        return m_do(_msg);
    }

    typename ApfJobBase<Message>::JobResult WaitForResult(const Message& _msg) override {
        return m_checker(_msg);
    }

private:
    JobExecutorWithinTime<Message>  m_do;
    JobResultChecker<Message>       m_checker;
};

template <typename Message>
class SimpleJobHelper3 : public ApfJobBase<Message> {
public:
    SimpleJobHelper3(JobExecutorWithStatus<Message> _do, JobResultChecker<Message> _checker)
        : m_do(_do), m_checker(_checker)
    {  }

    typename ApfJobBase<Message>::JobStatus doJob(apf::no_ownership_ptr<Message const> _msg) override {
        return m_do(_msg);
    }

    typename ApfJobBase<Message>::JobResult checkResult(const Message& _msg) override {
        return m_checker(_msg);
    }

private:
    JobExecutorWithStatus<Message>  m_do;
    JobResultChecker<Message>       m_checker;
};


template <typename Message>
ApfJobHandle<Message> CreateTempJob(JobExecutor<Message> _job_do) {
    return CreateJob<Message, SimpleJobHelper<Message>>(_job_do);
}
template <typename Message>
ApfJobHandle<Message> CreateTempJob(JobExecutorWithinTime<Message> _job_do, JobResultChecker<Message> _job_checker) {
    return CreateJob<Message, SimpleJobHelper2<Message>>(_job_do, _job_checker);
}
template <typename Message>
ApfJobHandle<Message> CreateTempJob(JobExecutorWithStatus<Message> _job_do, JobResultChecker<Message> _job_checker) {
    return CreateJob<Message, SimpleJobHelper3<Message>>(_job_do, _job_checker);
}

template <typename Message>
class SimpleMissionHelper : public ApfMissionBase<Message> {
public:
    SimpleMissionHelper() { ; }
};

template <typename Message>
class SimpleMissionHelper2 : public ApfMissionBase<Message> {
public:
    SimpleMissionHelper2(JobExecutor<Message> _executor) {
        auto temp_job = CreateTempJob<Message>(_executor);

        this->startFromJob(DO_JOB(temp_job));
    }

    SimpleMissionHelper2(JobExecutorWithStatus<Message> _executor, JobResultChecker<Message> _job_checker) {
        auto temp_job = CreateTempJob<Message>(_executor, _job_checker);

        this->startFromJob(DO_JOB(temp_job));
    }
};

template <typename Message>
class SimpleMissionHelper3 : public ApfMissionBase<Message> {
public:
    SimpleMissionHelper3(JobExecutorWithinTime<Message> _executor, JobResultChecker<Message> _job_checker) {
        auto temp_job = CreateTempJob<Message>(_executor, _job_checker);

        this->startFromJob(DO_JOB(temp_job));
    }
};

template <typename Message>
ApfMissionHandle<Message> CreateTempMission(JobTreeMaker<Message> _job_tree_maker) {
    auto mission = CreateMission<Message, SimpleMissionHelper<Message>>();
    _job_tree_maker(mission);
    return mission;
}
template <typename Message>
ApfMissionHandle<Message> CreateTempMission(JobExecutor<Message> _job_do) {
    return CreateMission<Message, SimpleMissionHelper2<Message>>(_job_do);
}
template <typename Message>
ApfMissionHandle<Message> CreateTempMission(JobExecutorWithinTime<Message> _job_do, JobResultChecker<Message> _job_checker) {
    return CreateMission<Message, SimpleMissionHelper3<Message>>(_job_do, _job_checker);
}
template <typename Message>
ApfMissionHandle<Message> CreateTempMission(JobExecutorWithStatus<Message> _job_do, JobResultChecker<Message> _job_checker) {
    return CreateMission<Message, SimpleMissionHelper2<Message>>(_job_do, _job_checker);
}
template <typename Message>
inline uint8_t WaitForSomeTiming(Message const* _msg) { return 0; }

#if 0
void Test() {


    LOGD(PHONE_CDM, "SmsRecvNotification::ProcessNotification SYS__QPOP_PHONE_0003 show\n");
    auto show_sms_pop = [=](const Message& _msg) {
        auto& cmd_manager = PhoneCDM::IAppSwitchControl::CDMManager::GetInstance();
        if (cmd_manager.GetCurrentCDM() != E_CDM_NAME_ECPHONE_INCOMINGCALL) {
            ScreenManager::GetInstance().ShowPop(SYS__QPOP_PHONE_0003, nullptr, sms_item_name_or_number);
            return PhoneCDMJobBase::JobResult::SUCCESSED;
        }
        return PhoneCDMJobBase::JobResult::UNKONW;
    };
    auto show_sms_pop_mission = Mission::CreateTempMission(WaitForSomeTiming, show_sms_pop);


}


Sample:
auto mission = CreateTempMission([&](Courier::Message const* _message) {
    cmd_manager.ResponseLoadToMaster(_msg);
});

Mission::CreateTempMission([&, =_msg](ApfMMissionHandle _mission_handle) {
   auto res_to_master = CreateTempJob([&](Courier::Message const*) {
	   cmd_manager.ResponseLoadToMaster();
    });
   _mission_handle->StartFromJob(DO_JOB(res_to_master));
});

#endif

} // namespace Mission
} // namespace Apf

#endif //


