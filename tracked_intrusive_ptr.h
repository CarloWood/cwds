#pragma once

// This header requires submodules
// https://github.com/CarloWood/ai-utils and
// https://github.com/CarloWood/ai-threadsafe to be installed.

#include "utils/InstanceTracker.h"
#include <boost/intrusive_ptr.hpp>

// tracked::intrusive_ptr<T>
//
// A boost::intrusive_ptr<T> that tracks its instances.
//
// Usage:
//
#if EXAMPLE_CODE
// In the header file that defines Foo.
class Foo
{
  ...
};

#ifdef CWDEBUG
DECLARE_TRACKED_BOOST_INTRUSIVE_PTR(Foo)
#endif

#endif // EXAMPLE_CODE
//
// One can then call
//
//  tracked::intrusive_ptr<Foo>::for_each_instance([](tracked::intrusive_ptr<Foo> const* ptr){
//     Dout(dc::notice, print_using(ptr, &tracked::intrusive_ptr<Foo>::print_tracker_info_on)); });
//
// to print out the tracker info of each intrusive_ptr instance.
//

namespace tracked {

// This class is basically a copy of boost::intrusive_ptr,
// but derived from InstanceTracker.

template <class T>
class intrusive_ptr : public utils::InstanceTracker<intrusive_ptr<T>>
{
 private:
  typedef intrusive_ptr this_type;

 public:
  typedef T element_type;

  intrusive_ptr() : px(0), return_address(nullptr) { }

  [[gnu::noinline]] intrusive_ptr(T* p, bool add_ref = true) : px(p), return_address(nullptr)
  {
    if (px != 0)
    {
      return_address = __builtin_return_address(0);
      if (add_ref)
        intrusive_ptr_add_ref(px);
    }
  }

  intrusive_ptr(T* p, void* ra, bool add_ref = true) : px(p), return_address(nullptr)
  {
    if (px != 0)
    {
      return_address = ra;
      if (add_ref)
        intrusive_ptr_add_ref(px);
    }
  }

  template <class U>
  [[gnu::noinline]] intrusive_ptr(intrusive_ptr<U> const& rhs, typename boost::detail::sp_enable_if_convertible<U, T>::type = boost::detail::sp_empty())
      : px(rhs.get()), return_address(nullptr)
  {
    if (px != 0)
    {
      return_address = __builtin_return_address(0);
      intrusive_ptr_add_ref(px);
    }
  }

  template <class U>
  intrusive_ptr(intrusive_ptr<U> const& rhs, void* ra, typename boost::detail::sp_enable_if_convertible<U, T>::type = boost::detail::sp_empty())
      : px(rhs.get()), return_address(nullptr)
  {
    if (px != 0)
    {
      return_address = ra;
      intrusive_ptr_add_ref(px);
    }
  }

  [[gnu::noinline]] intrusive_ptr(intrusive_ptr const& rhs) : px(rhs.px), return_address(nullptr)
  {
    if (px != 0)
    {
      return_address = __builtin_return_address(0);
      intrusive_ptr_add_ref(px);
    }
  }

  intrusive_ptr(intrusive_ptr const& rhs, void* ra) : px(rhs.px), return_address(nullptr)
  {
    if (px != 0)
    {
      return_address = ra;
      intrusive_ptr_add_ref(px);
    }
  }

  ~intrusive_ptr()
  {
    if (px != 0) intrusive_ptr_release(px);
    return_address = (void*)0xdeaddead;
  }

  template <class U>
  [[gnu::noinline]] intrusive_ptr& operator=(intrusive_ptr<U> const& rhs)
  {
    this_type(rhs, __builtin_return_address(0)).swap(*this);
    return *this;
  }

  // Move support
  [[gnu::noinline]] intrusive_ptr(intrusive_ptr&& rhs) BOOST_SP_NOEXCEPT : px(rhs.px), return_address(nullptr)
  {
    if (px != 0) return_address = __builtin_return_address(0);
    rhs.px = 0;
  }

  intrusive_ptr(intrusive_ptr&& rhs, void* ra) BOOST_SP_NOEXCEPT : px(rhs.px), return_address(nullptr)
  {
    if (px != 0) return_address = ra;
    rhs.px = 0;
  }

  [[gnu::noinline]] intrusive_ptr& operator=(intrusive_ptr&& rhs) BOOST_SP_NOEXCEPT
  {
    this_type(std::move(rhs), __builtin_return_address(0)).swap(*this);
    return *this;
  }

  template <class U>
  friend class intrusive_ptr;

  template <class U>
  [[gnu::noinline]] intrusive_ptr(intrusive_ptr<U>&& rhs, typename boost::detail::sp_enable_if_convertible<U, T>::type = boost::detail::sp_empty())
      : px(rhs.px), return_address(nullptr)
  {
    if (px != 0) return_address = __builtin_return_address(0);
    rhs.px = 0;
    rhs.return_address = nullptr;
  }

  template <class U>
  intrusive_ptr(intrusive_ptr<U>&& rhs, void* ra, typename boost::detail::sp_enable_if_convertible<U, T>::type = boost::detail::sp_empty())
      : px(rhs.px), return_address(nullptr)
  {
    if (px != 0) return_address = ra;
    rhs.px = 0;
    rhs.return_address = nullptr;
  }

  template <class U>
  [[gnu::noinline]] intrusive_ptr& operator=(intrusive_ptr<U>&& rhs) BOOST_SP_NOEXCEPT
  {
    this_type(std::move(rhs), __builtin_return_address(0)).swap(*this);
    return *this;
  }

  [[gnu::noinline]] intrusive_ptr& operator=(intrusive_ptr const& rhs)
  {
    this_type(rhs, __builtin_return_address(0)).swap(*this);
    return *this;
  }

  [[gnu::noinline]] intrusive_ptr& operator=(T* rhs)
  {
    this_type(rhs, __builtin_return_address(0)).swap(*this);
    return *this;
  }

  void reset()
  {
    this_type().swap(*this);
  }

  void reset(T* rhs)
  {
    this_type(rhs, __builtin_return_address(0)).swap(*this);
  }

  void reset(T* rhs, bool add_ref)
  {
    this_type(rhs, __builtin_return_address(0), add_ref).swap(*this);
  }

  T* get() const BOOST_SP_NOEXCEPT
  {
    return px;
  }

  T* detach() BOOST_SP_NOEXCEPT
  {
    T* ret = px;
    px     = 0;
    return ret;
  }

  T& operator*() const BOOST_SP_NOEXCEPT_WITH_ASSERT
  {
    BOOST_ASSERT(px != 0);
    return *px;
  }

  T* operator->() const BOOST_SP_NOEXCEPT_WITH_ASSERT
  {
    BOOST_ASSERT(px != 0);
    return px;
  }

// implicit conversion to "bool"
#include <boost/smart_ptr/detail/operator_bool.hpp>

  void swap(intrusive_ptr& rhs) BOOST_SP_NOEXCEPT
  {
    T* tmp = px;
    void* ra = return_address;
    px     = rhs.px;
    return_address = rhs.return_address;
    rhs.px = tmp;
    rhs.return_address = ra;
  }

  void print_tracker_info_on(std::ostream& os) const
  {
    if (return_address)
    {
#ifdef CWDEBUG_LOCATION
      os << libcwd::location_ct((char*)return_address + libcwd::builtin_return_address_offset);
#else
      os << return_address;
#endif
    }
  }

 private:
  T* px;
  void* return_address;
};

} // namespace tracked

// Definition of convenience macro.
#define DECLARE_TRACKED_BOOST_INTRUSIVE_PTR(T) \
  namespace boost { \
  template<> \
  class intrusive_ptr<T> : public tracked::intrusive_ptr<T> \
  { \
   private: \
    using this_type = intrusive_ptr; \
    \
   public: \
    intrusive_ptr() = default; \
    \
    [[gnu::noinline]] intrusive_ptr(T* p, bool add_ref = true) \
        : tracked::intrusive_ptr<T>(p, __builtin_return_address(0), add_ref) { } \
    \
    intrusive_ptr(T* p, void* ra, bool add_ref = true) \
        : tracked::intrusive_ptr<T>(p, ra, add_ref) { } \
    \
    template <class U> \
    [[gnu::noinline]] intrusive_ptr(intrusive_ptr<U> const& rhs, typename boost::detail::sp_enable_if_convertible<U, T>::type = boost::detail::sp_empty()) \
        : tracked::intrusive_ptr<T>(rhs, __builtin_return_address(0)) { } \
    \
    [[gnu::noinline]] intrusive_ptr(intrusive_ptr const& rhs) \
        : tracked::intrusive_ptr<T>(rhs, __builtin_return_address(0)) { } \
    \
    intrusive_ptr(intrusive_ptr const& rhs, void* ra) \
        : tracked::intrusive_ptr<T>(rhs, ra) { } \
    \
    [[gnu::noinline]] intrusive_ptr(intrusive_ptr&& rhs) BOOST_SP_NOEXCEPT \
        : tracked::intrusive_ptr<T>(std::move(rhs), __builtin_return_address(0)) { } \
    \
    intrusive_ptr(intrusive_ptr&& rhs, void* ra) BOOST_SP_NOEXCEPT \
        : tracked::intrusive_ptr<T>(std::move(rhs), ra) { } \
    \
    template <class U> \
    [[gnu::noinline]] intrusive_ptr(intrusive_ptr<U>&& rhs, typename boost::detail::sp_enable_if_convertible<U, T>::type = boost::detail::sp_empty()) \
        : tracked::intrusive_ptr<T>(std::move(rhs), __builtin_return_address(0)) { } \
    \
    template <class U> \
    intrusive_ptr(intrusive_ptr<U>&& rhs, void* ra, typename boost::detail::sp_enable_if_convertible<U, T>::type = boost::detail::sp_empty()) \
        : tracked::intrusive_ptr<T>(std::move(rhs), ra) { } \
    \
    template <class U> \
    [[gnu::noinline]] intrusive_ptr& operator=(intrusive_ptr<U> const& rhs) \
    { \
      this_type(rhs, __builtin_return_address(0)).swap(*this); \
      return *this; \
    } \
    \
    [[gnu::noinline]] intrusive_ptr& operator=(intrusive_ptr&& rhs) BOOST_SP_NOEXCEPT \
    { \
      this_type(std::move(rhs), __builtin_return_address(0)).swap(*this); \
      return *this; \
    } \
    \
    template <class U> \
    [[gnu::noinline]] intrusive_ptr& operator=(intrusive_ptr<U>&& rhs) BOOST_SP_NOEXCEPT \
    { \
      this_type(std::move(rhs), __builtin_return_address(0)).swap(*this); \
      return *this; \
    } \
    \
    [[gnu::noinline]] intrusive_ptr& operator=(intrusive_ptr const& rhs) \
    { \
      this_type(rhs, __builtin_return_address(0)).swap(*this); \
      return *this; \
    } \
    \
    [[gnu::noinline]] intrusive_ptr& operator=(T* rhs) \
    { \
      this_type(rhs, __builtin_return_address(0)).swap(*this); \
      return *this; \
    } \
    \
    void reset() \
    { \
      this_type().swap(*this); \
    } \
    \
    void reset(T* rhs) \
    { \
      this_type(rhs, __builtin_return_address(0)).swap(*this); \
    } \
    \
    void reset(T* rhs, bool add_ref) \
    { \
      this_type(rhs, __builtin_return_address(0), add_ref).swap(*this); \
    } \
  }; \
  } // namespace boost
