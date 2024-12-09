#ifndef _MISSION_BASE_H_
#define _MISSION_BASE_H_

#include <memory>
#include <vector>
#include <string>
#include <map>
#include <functional>

#include "NoOwnershipPtr.h"
#include "EnhancedAssertion.h"
#include "ara/core/vector.h"

namespace apf {
namespace mission {

template <typename Msg>
class Job {
public:
    using MessageType = Msg;
    using JobId       = uint32_t;
public:    
    enum class JobResult : uint8_t {
        SUCCESSED,
        FAILED,       
		TIMEOUT,
		UNKONW,
    };

    enum class JobStatus : uint8_t {
        DONE,
        WAIT_FOR_RESULT,
    };
   
    Job() : m_is_failed(false), m_job_id(0xffff) {
    }

public:
    // JobStatus::DONE if the job is one shot and don't wait for response.
    virtual JobStatus doJob(apf::no_ownership_ptr<Msg const>) = 0;
    virtual JobResult checkResult(Msg const&) {
        return JobResult::SUCCESSED;
    }

	// virtual bool ReDo() { return true ; }    
public: 

    apf::no_ownership_ptr<Job<Msg>> getThis() {
        return this;
    }

    void setToBeFailed() {
        m_is_failed = true;
    }

    bool isJobFailed() const {
        return m_is_failed;
    }

    JobId getJobId() const {
        return m_job_id;
    }

public:
    void setJobId(JobId _id) {
        m_job_id = _id;
    }

private:
    bool  m_is_failed;
    JobId m_job_id;    
};


template <typename Msg>
class IfGrammerHelper; // forward declareration

template <typename Msg>
class ExecutionTreeNode {
public:
    using PreCond     = std::function<bool()>;
	using NodeHandle  = std::shared_ptr<ExecutionTreeNode>;
	using NodeRef     = apf::no_ownership_ptr<ExecutionTreeNode>;
    using Branch      = std::pair<PreCond, NodeHandle>;
    using Branches    = ara::core::Vector<Branch>;
    using JobRef      = apf::no_ownership_ptr<Job<Msg>>;
    using JobHandle   = std::shared_ptr<Job<Msg>>;

private:
    JobHandle  m_job_ptr;
    Branches   m_job_branches;
	
public:
    explicit ExecutionTreeNode(JobHandle _job) : m_job_ptr(_job) {    

    }

    JobRef getJobRef() {
        return m_job_ptr.get();
    }

    Branches & getBranches() {
        return m_job_branches;
    }

	NodeRef addBranch(PreCond _fun, NodeHandle _node) {
		m_job_branches.emplace_back(_fun, _node);

        return m_job_branches.back().second.get();
    }

	IfGrammerHelper<Msg> __WHEN(PreCond _pre_cond);
	IfGrammerHelper<Msg> __ALWAYS();
};

template <typename JobType, typename ...Args>
typename std::shared_ptr<JobType> CreateJob(Args && ..._args) {
    return std::make_shared<JobType>(std::forward<Args>(_args)...);
}

template <typename Msg>
class IfGrammerHelper {
public:
	explicit  IfGrammerHelper(typename ExecutionTreeNode<Msg>::PreCond _pre_cond, typename ExecutionTreeNode<Msg>::NodeRef _cur_node)
		: m_pre_condition(_pre_cond)
		, m_current_node(_cur_node) {}

	typename ExecutionTreeNode<Msg>::NodeRef operator [] (typename ExecutionTreeNode<Msg>::NodeHandle _next_node) {
		m_current_node->addBranch(m_pre_condition, _next_node);
		return m_current_node;
	}

private:
	typename ExecutionTreeNode<Msg>::PreCond  m_pre_condition;
	typename ExecutionTreeNode<Msg>::NodeRef  m_current_node;
};

template <typename Msg>
//inline IfGrammerHelper<Msg> ExecutionTreeNode<Msg>::__WHEN(ExecutionTreeNode::PreCond _pre_cond) {
inline IfGrammerHelper<Msg> ExecutionTreeNode<Msg>::__WHEN(typename ExecutionTreeNode<Msg>::PreCond _pre_cond) {
        return IfGrammerHelper<Msg>(_pre_cond, this);
}

template <typename Msg>
inline IfGrammerHelper<Msg> ExecutionTreeNode<Msg>::__ALWAYS() {
	return IfGrammerHelper<Msg>([]{ return true; }, this);
}


#define If(cond)      ->__WHEN([=] { return cond;})
#define ElseIf(cond)  ->__WHEN([=] { return cond;})
#define Else          ->__ALWAYS()
#define AndThen       ->__ALWAYS()
#define AnyWay        ->__ALWAYS()
#define DO_JOB(job)   apf::mission::CreateNode<decltype(job)::element_type::MessageType>(job)


template <typename Msg>
class ExecutionTree {
public:
	using NodeHandle = std::shared_ptr<ExecutionTreeNode<Msg>>;
	using NodeRef    = apf::no_ownership_ptr<ExecutionTreeNode<Msg>>;

private:
    NodeHandle m_root_node;

public:
	NodeRef addRootNode(NodeHandle _node) {
        ENSURE(_node);
		m_root_node = _node;
        return m_root_node.get();        
    }

    NodeRef getRootNode() { 
        return m_root_node.get(); 
    }
};

template <typename Msg>
inline typename ExecutionTree<Msg>::NodeHandle CreateNode(typename ExecutionTreeNode<Msg>::JobHandle _job) {
    ENSURE(_job != nullptr).Msg("Create ExecutionTreeNode by an Empty Job!!!!");
	return std::make_shared<ExecutionTreeNode<Msg>>(_job);
}

template <typename Msg>
inline typename ExecutionTree<Msg>::NodeHandle Node(typename ExecutionTreeNode<Msg>::JobRef _job) {
	return std::make_shared<ExecutionTreeNode<Msg>>(_job);
}



//////////////////////////////////////////////////////////////////////////////////////  
// 	 
//          Sample Code for interface of ExecutionTreeNode & ExecutionTree
//    
//		auto check_heater_sts = make_shared<HeaterStsQuery>();
//		auto check_stop_heating_cond = make_shared<CheckStopHeaterCond>();
//		auto stop_heating = make_shared<StopHeater>();
//
//		auto last_branch = [&] { return true; };
//
//		auto root_node = job_tree.AddRootNode(Node(check_heater_sts));
//		auto check_cond_node = root_node->AddBranch(
//			Condition(check_heater_sts->isHeaterOn()),
//			Node(check_stop_heating_cond));
//		auto end_node1 = root_node->AddBranch(
//			Condition(check_heater_sts->isHeaterOn()),
//			Node(check_heater_sts));
//
//		auto check_cond_node2 = check_cond_node->AddBranch(
//			Condition(check_stop_heating_cond->isStopHeaterOperatable()),
//			Node(stop_heating));
//		auto end_node2 = check_cond_node->AddBranch(
//			Condition(check_stop_heating_cond->isStopHeaterOperatable()),
//			Node(check_heater_sts));
//

template <typename Msg>
class MissionBase {
public:
    using ExecutionTreeNodeHandle = typename ExecutionTree<Msg>::NodeHandle;
    using ExecutionTreeNodeRef    = typename ExecutionTree<Msg>::NodeRef;
    using JobHandle = typename ExecutionTreeNode<Msg>::JobHandle;
    using JobRef    = typename ExecutionTreeNode<Msg>::JobRef;
public:
    virtual ~MissionBase() {};
private:
    bool isLastJob()  {
        return m_current_node->job_branches.empty();
    }

    ExecutionTreeNodeRef findNextNode() {
        if(!m_current_node->getBranches().empty()){
            for (auto & branch : m_current_node->getBranches()) {	
                if (branch.first()) {		
                    return branch.second.get();
                }
            }
        }
        return nullptr;
    }

public:
    MissionBase() = default;

    void runMission(Msg const & _msg) {
        if (!m_current_node) { return; }

        JobRef cur_job = m_current_node->getJobRef();
        if (isMissionEnd() && cur_job) {
            return;
        }

        switch (cur_job->checkResult(_msg)) {
        case Job<Msg>::JobResult::FAILED:
            cur_job->setToBeFailed();
            // no break here 
            // continue to find next job
            // [[fallthrough]]; // from C++17
        case Job<Msg>::JobResult::SUCCESSED:
            while (m_current_node = findNextNode()) {
                cur_job = m_current_node->getJobRef();
                if (Job<Msg>::JobStatus::WAIT_FOR_RESULT == cur_job->doJob(&_msg)) {// false means need to wait for response
                    break;
                }
            } 
            break;
        default:
            break;
        }
    }

    void startMission(apf::no_ownership_ptr<const Msg> _msg) {
        m_istriggled = true;
        m_current_node = m_job_tree.getRootNode();
        if (!m_current_node)
            return;
		JobRef cur_job = m_current_node->getJobRef();
		while (Job<Msg>::JobStatus::DONE == cur_job->doJob(_msg)) {
            m_current_node = findNextNode();
            if (m_current_node != nullptr) {
                cur_job = m_current_node->getJobRef();
            }
            else {
                break;
            }
        }
    }
    
    void endMission() {
        m_current_node = nullptr;
	}

    bool isMissionEnd() const {
        return m_current_node == nullptr;
    }

    ExecutionTreeNodeRef startFromJob(ExecutionTreeNodeHandle _node) {
        m_istriggled = false;
        return m_job_tree.addRootNode(_node);
    }

    bool isTriggled() const {
        return m_istriggled;
    }
    
private:
    ExecutionTree<Msg>   m_job_tree;
    ExecutionTreeNodeRef m_current_node;
    bool m_istriggled;
};

   
}
}

///////////////////////////////////////////////////////////////////////////////////////
#if 0

Sample Code for DSL of ExecutionTreeNode & ExecutionTree

class TestJob1 : public JobBase {
public:
    JobStatus DO(Msg const & _msg) {
        // the action
        // return JobBase::JobStatus::DONE if one shot call
        return WAIT_FOR_RESULT;
    }

    JobResult CheckResult(Msg const & _msg) {
        // check the message
        return JobResult::SUCCESSED;

    }

    bool someConditon() const {
        return true; // if need
    }
}


class TestMission : public MisssionBase {

public:
    TestMission() {

        startFromJob(DO_JOB(job1))
        If(job1->Ask)[
            DO_JOB(job2)
            WHEN(some_function)[
                DO_JOB(job4)
            ]
            WHEN(run_branch)[
                DO_JOB(job5)
            ]
        ]
        ANY_WAY [
            DO_JOB(job3)
        ];

        startFromJob(DO(job1))
        ANY_WAY [
            DO(job2)
            ANY_WAY[
                DO(job3)
            ]
        ];
    }
}


#endif
///////////////////////////////////////////////////////////////////////////////////////

#endif // _MISSION_BASE_H_
