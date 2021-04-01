#pragma once

#include "utils/InstanceTracker.h"
#include <boost/intrusive_ptr.hpp>

// tracked::intrusive_ptr<T>
//
// A boost::intrusive_ptr<T> that tracks its instances.
//
// Usage:
//
#if EXAMPLE_CODE

// Specialize boost::intrusive_ptr<Foo>.
namespace boost {

template<>
class intrusive_ptr<Foo> : public tracked::intrusive_ptr<Foo>
{
  using tracked::intrusive_ptr<Foo>::intrusive_ptr;
};

} // namespace boost

#endif // EXAMPLE_CODE
//
// or simply
//
#if EXAMPLE_CODE

DECLARE_TRACKED_BOOST_INTRUSIVE_PTR(Foo)

#endif // EXAMPLE_CODE
//
// One can then call tracked::intrusive_ptr<Foo>::for_each([](tracked::intrusive_ptr<Foo> const* ptr){ ... })
// to run over all currently existing instances of this type.
//

// Definition of convenience macro.
#define DECLARE_TRACKED_BOOST_INTRUSIVE_PTR(T) \
  namespace boost { template<> class intrusive_ptr<T> : public tracked::intrusive_ptr<T> { using tracked::intrusive_ptr<T>::intrusive_ptr; }; }

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

  BOOST_CONSTEXPR intrusive_ptr() BOOST_SP_NOEXCEPT : px(0) {}

  intrusive_ptr(T* p, bool add_ref = true) : px(p)
  {
    if (px != 0 && add_ref) intrusive_ptr_add_ref(px);
  }

  template <class U>
  intrusive_ptr(intrusive_ptr<U> const& rhs, typename boost::detail::sp_enable_if_convertible<U, T>::type = boost::detail::sp_empty())
      : px(rhs.get())
  {
    if (px != 0) intrusive_ptr_add_ref(px);
  }

  intrusive_ptr(intrusive_ptr const& rhs) : px(rhs.px)
  {
    if (px != 0) intrusive_ptr_add_ref(px);
  }

  ~intrusive_ptr()
  {
    if (px != 0) intrusive_ptr_release(px);
  }

  template <class U>
  intrusive_ptr& operator=(intrusive_ptr<U> const& rhs)
  {
    this_type(rhs).swap(*this);
    return *this;
  }

  // Move support
  intrusive_ptr(intrusive_ptr&& rhs) BOOST_SP_NOEXCEPT : px(rhs.px) { rhs.px = 0; }

  intrusive_ptr& operator=(intrusive_ptr&& rhs) BOOST_SP_NOEXCEPT
  {
    this_type(static_cast<intrusive_ptr&&>(rhs)).swap(*this);
    return *this;
  }

  template <class U>
  friend class intrusive_ptr;

  template <class U>
  intrusive_ptr(intrusive_ptr<U>&& rhs, typename boost::detail::sp_enable_if_convertible<U, T>::type = boost::detail::sp_empty())
      : px(rhs.px)
  {
    rhs.px = 0;
  }

  template <class U>
  intrusive_ptr& operator=(intrusive_ptr<U>&& rhs) BOOST_SP_NOEXCEPT
  {
    this_type(static_cast<intrusive_ptr<U>&&>(rhs)).swap(*this);
    return *this;
  }

  intrusive_ptr& operator=(intrusive_ptr const& rhs)
  {
    this_type(rhs).swap(*this);
    return *this;
  }

  intrusive_ptr& operator=(T* rhs)
  {
    this_type(rhs).swap(*this);
    return *this;
  }

  void reset() { this_type().swap(*this); }

  void reset(T* rhs) { this_type(rhs).swap(*this); }

  void reset(T* rhs, bool add_ref) { this_type(rhs, add_ref).swap(*this); }

  T* get() const BOOST_SP_NOEXCEPT { return px; }

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
    px     = rhs.px;
    rhs.px = tmp;
  }

 private:
  T* px;
};

} // namespace tracked
