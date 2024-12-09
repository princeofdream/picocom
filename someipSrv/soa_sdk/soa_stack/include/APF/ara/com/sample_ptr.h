#ifndef _ARA_COM_SAMPLE_PTR_H_
#define _ARA_COM_SAMPLE_PTR_H_
#include "e2e/e2e_types.h"
#include <memory>
namespace ara {
    namespace com {
        //[SWS_CM_00306]
        template <typename T>
        class SamplePtr {
        public:
            // Default constructor
            constexpr SamplePtr() noexcept : m_value() {}
            // semantically equivalent to Default constructor
            constexpr SamplePtr(std::nullptr_t) noexcept : m_value() {}
            constexpr SamplePtr(T* _Ptr) noexcept : m_value(_Ptr) {}
            // Copy constructor is deleted
            SamplePtr(const SamplePtr&) = delete;
            // Move constructor
            SamplePtr(SamplePtr&& _Right) noexcept {
                m_value = std::move(_Right.m_value);
                m_check_status = _Right.m_check_status;
            }

            // Default copy assignment operator is deleted
            SamplePtr& operator=(const SamplePtr&) = delete;
            // Assignment of nullptr_t
            SamplePtr& operator=(std::nullptr_t) noexcept {
                Reset();
                return *this;
            }
            // Move assignment operator
            SamplePtr& operator=(SamplePtr&&) noexcept = default;
            // Dereferences the stored pointer
            T& operator*() const noexcept {
                return *m_value;
            }
            T* operator->() const noexcept {
                return m_value.get();
            }
            //Checks if the stored pointer is null
            explicit operator bool() const noexcept {
                if (m_value != nullptr)
                    return true;
                return false;
            }
            // Swaps the managed object
            void Swap(SamplePtr& _Right) noexcept {
                m_value.swap(_Right.m_value);
            }
            //Replaces the managed object
            void Reset() noexcept {
                m_value.reset();
            }
            //Returns the stored object
            T* Get() const noexcept {
                return m_value;
            }
            //[SWS_CM_90420]
            // Returns the end 2 end protection check result
            ara::com::e2e::ProfileCheckStatus GetProfileCheckStatus() const noexcept {
                return m_check_status;
            }

            void SetProfileCheckStatus(ara::com::e2e::ProfileCheckStatus check_status)
            {
                m_check_status = check_status;
            }
        private:
            std::unique_ptr<T> m_value;
            ara::com::e2e::ProfileCheckStatus m_check_status = ara::com::e2e::ProfileCheckStatus::Ok;
        };

        template <class _Ty, class... _Types>
        SamplePtr<_Ty> make_sample(_Types&&... _Args) { // make a unique_ptr
            return SamplePtr<_Ty>(new _Ty(::std::forward<_Types>(_Args)...));
        }
    }
}
#endif // !_ARA_COM_SAMPLE_PTR_H_

