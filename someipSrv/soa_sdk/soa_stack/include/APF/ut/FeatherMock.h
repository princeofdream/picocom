#pragma once

// V1.0
// CopyRight : EvenTang MIT License
//
// @todo : how to mock functions with variadic parameter
// @todo : how to mock template parameter


#include <functional>
#include <string>
#include <map>
#include <memory>
#include <algorithm>
#include <deque>
#include "../ara/core/string_view.h"
#include "../ara/core/string.h"
#include "../ara/core/map.h"

class MockObj {
public:
    MockObj(ara::core::StringView _name) : m_name(_name), m_is_called(false) {}

    ara::core::StringView getName(void) const {
        return m_name;
    }

    bool isCalled() const {
        return m_is_called;
    }

    virtual void clearMockStatus(void) {
        m_is_called = false;
        clearSub();
    }

    virtual void clearSub() = 0;

    virtual void getParameters(void* _p) const {}


protected:
    bool m_is_called;
private:
    ara::core::StringView m_name;
};

// default template without implementation
template <typename T>
class FunctionMock;

template <typename R, typename ...Args>
class FunctionMock<R(Args...)> : public MockObj {
public:
    using ParaPack = std::tuple<typename std::remove_const<typename std::remove_reference<Args>::type>::type...>;
public:
    FunctionMock(ara::core::StringView _name) 
        : MockObj(_name) { 

        // Check this!
        m_default_behavior = [&](Args... _args) { return R(); };
    }

    FunctionMock(ara::core::StringView _name, std::function<R(Args...)> _default_behavior) // , R&& _default_return = R())
        : MockObj(_name) , m_default_behavior(_default_behavior) {
    }

    R call(Args ... _args) {
        m_is_called = true;

        if (!m_customize_behaviors.empty()) {
            auto temp = m_customize_behaviors.front();
            m_customize_behaviors.pop_front();

            return temp(_args...);
        }
        
        return m_default_behavior(_args...);
    }

    R operator () (Args ... _args) {
        m_is_called = true;

        if (!m_customize_behaviors.empty()) {
            auto temp = m_customize_behaviors.front();
            m_customize_behaviors.pop_front();

            return temp(_args...);
        }
        
        return m_default_behavior(_args...);
    }

    FunctionMock<R(Args...)>& setBehavior(std::function<R(Args...)> _checker) {
        m_customize_behaviors.push_back(_checker);
        return *this;
    }

    FunctionMock<R(Args...)>& willActLike(std::function<R(Args...)> _checker) {
        return setBehavior(_checker);
    }

    FunctionMock<R(Args...)>& setDefaultBehavior(std::function<R(Args...)> _checker) {
        m_default_behavior = _checker;
        return *this;
    }

    virtual void ClearMockStatus(void) {
        MockObj::clearMockStatus();
        m_customize_behaviors.clear();
    }
    void clearSub() {
        m_customize_behaviors.clear();
    }

private:
    std::deque<std::function<R(Args...)>> m_customize_behaviors;
    std::function<R(Args...)> m_default_behavior;
};

#define WillReturn(rslt) setBehavior([&](auto ...args) { return rslt; })
#define WillDefaultReturn(rslt) setDefaultBehavior([&](auto ...args) { return rslt; })

class MockSystem {
public:
    using MockObjHandle = std::shared_ptr<MockObj>;

public:
    MockSystem() {}

public:

    static MockSystem& getInstance(void) {
        static MockSystem mock_sys;
        return mock_sys;
    }

    void addNewMockObj(ara::core::StringView _name, MockObjHandle _mock_obj) {
        m_mock_objs[_name] = _mock_obj;
    }

    MockObjHandle findMockObjByName(ara::core::StringView _name) {        
        return m_mock_objs[_name];
    }

    void refreshMockSystem() {
        // std::for_each(m_mock_objs.begin(), m_mock_objs.end(), [](auto & _mock_obj) {
        //     _mock_obj.second->clearMockStatus();
        // });
        for (auto & [_, mock_obj] : m_mock_objs) {
            mock_obj->clearMockStatus();
        }
    }

    template <typename T>
    FunctionMock<T>& findMockObjByNameAndType(ara::core::StringView _name) {
        if (m_mock_objs.find(_name) == m_mock_objs.end()) {
            m_mock_objs[_name] = std::make_shared<FunctionMock<T>>(_name);
        }
        return dynamic_cast<FunctionMock<T>&>(*m_mock_objs[_name]);
    }

private:
    ara::core::Map<ara::core::StringView, MockObjHandle> m_mock_objs;
};

template <typename T>
MockSystem::MockObjHandle createMock(ara::core::StringView _name) {
    return std::make_shared<FunctionMock<T>>(_name);
}

#define ADD_NEW_MOCK_OBJ(fun_name, signature)  m_mock_objs[#fun_name] = createMock<signature>(#fun_name)

#define MockFun(fun_name, signature) dynamic_cast<FunctionMock<signature>&>( \
        *MockSystem::GetInstance().findMockObjByName(#fun_name))

// Style 1:
//
// Definition Sample:
// #define MockOf_Api_from_other_module_1 MockFun("Api_from_other_module_1", int(int, int))
// #define MockOf_Api_from_other_module_2 MockFun("Api_from_other_module_2", void(int, int))
// #define MockOf_Api_from_other_module_3 MockFun("Api_from_other_module_3", void())
// #define MockOf_Api_from_other_module_4 MockFun("Api_from_other_module_4", int())
// 




template <typename T>
struct FunctionSignatureTrait {
    using Signature = T;
};

// Sample: struct TypeFor_Api_from_other_module_4 : public TypeTrait<int()> { };
#define DEFINE_MOCK_OBJ(name, signature) struct FunSigFor_##name : public FunctionSignatureTrait<signature> { }

#define Mock(name) MockSystem::getInstance().findMockObjByNameAndType<FunSigFor_##name::Signature>(#name)

// Style 2:
// 
// Definition Sample:
// DEFINE_MOCK_OBJ(Api_from_other_module_1, int(int, int));
// DEFINE_MOCK_OBJ(Api_from_other_module_2, void(int, int));
// DEFINE_MOCK_OBJ(Api_from_other_module_3, void());
// DEFINE_MOCK_OBJ(Api_from_other_module_4, int());
// DEFINE_MOCK_OBJ(Api_from_other_module_5, void(Student const& _stu, int));
// DEFINE_MOCK_OBJ(Api_from_other_module_6, void(Student& _stu, int));
// DEFINE_MOCK_OBJ(Api_from_other_module_7, void(Student _stu, int));
// DEFINE_MOCK_OBJ(Foo_SetName, void(ara::core::String const&));
//
// Usage Sample:
// WHEN("Case 1") {
//     Msg msg;
//     msg.id = 3;
//     ActionOf(CHECK(TestTarget(msg) == 4));
// 
//     Mock(Api_from_other_module_1).willActLike([](auto _p1, auto _p2) {
//         REQUIRE(_p1 == 2);
//         return 4;
//     });
// 
//     Mock(Api_from_other_module_4).willActLike([]() {
//         return 4;
//     });
// }


class PostAction {
public:
    PostAction(std::function<void()> _triggle, ara::core::StringView _description)
        : m_behavior(_triggle), m_str_name(_description)
    {

    }

    ~PostAction() {
        m_behavior();
        MockSystem::getInstance().refreshMockSystem();
        // std::cout << m_str_name << std::endl;
    }

private:
    std::function<void()> m_behavior;
    ara::core::String           m_str_name;

};

#define Action_LINENAME_CAT(name, line) name##line
#define Action_LINENAME(name, line)     Action_LINENAME_CAT(name, line)
#define ActionOf(expr)                  PostAction Action_LINENAME(PostAction, __LINE__)([&]() {expr;}, #expr)

