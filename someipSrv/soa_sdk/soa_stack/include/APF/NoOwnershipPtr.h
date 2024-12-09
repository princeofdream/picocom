
#ifndef _NO_OWNER_SHIP_PTR_H
#define _NO_OWNER_SHIP_PTR_H
namespace apf {

// @brief Check Null for Raw Pointer
//        No ownership of the memory pointed by m_ptr
// 
// @todo  Need to be improved much!~    
template <typename T>
class no_ownership_ptr {
public:
    no_ownership_ptr(T* _p) : m_ptr(_p) {

    }

    no_ownership_ptr(no_ownership_ptr const & _right) : m_ptr(_right.m_ptr) {}

    no_ownership_ptr(std::unique_ptr<T> & _right) : m_ptr(_right.get()) {}
    no_ownership_ptr(std::shared_ptr<T> & _right) : m_ptr(_right.get()) {}

    // @brief Default constructor initialize m_ptr to be nullptr
    no_ownership_ptr() : m_ptr(nullptr) {}

    T* operator = (T* _p) {
        m_ptr = _p;
        return m_ptr;
    }

    operator bool() const {
        return m_ptr != nullptr;
    }

    bool operator == (no_ownership_ptr _right) const {
        return m_ptr == _right.m_ptr;
    }

    bool operator != (no_ownership_ptr _right) const {
        return m_ptr != _right.m_ptr;
    }

    bool operator == (T const * const _p) const {
        return m_ptr == _p;
    }

    bool operator != (T const * const _p) const {
        return m_ptr != _p;
    }

    bool operator == (std::nullptr_t) const {
        return m_ptr == nullptr;
    }

    bool operator != (std::nullptr_t) const {
        return m_ptr != nullptr;
    }

    T* get() const {
        return m_ptr;
    }

    T * const operator -> () const {
        //ENSURE(m_ptr).Msg("Use m_ptr when it's null! Throw Exception. Check it!");
        if (!m_ptr) {
            throw "no_ownership_ptr is NULL!!!";
        }
        
        return m_ptr;
    }

private:
    T* m_ptr;
};

template <typename T>
bool operator == (std::nullptr_t, no_ownership_ptr<T> _cnrp) {
    return nullptr == _cnrp.get();
}

template <typename T>
bool operator != (std::nullptr_t, no_ownership_ptr<T> _cnrp) {
    return nullptr != _cnrp.get();
}

template <typename T>
bool operator == (T* _p, no_ownership_ptr<T> _cnrp) {
    return _p == _cnrp.get();
}

template <typename T>
bool operator != (T* _p, no_ownership_ptr<T> _cnrp) {
    return _p != _cnrp.get();
}

}
#endif