// Based on https://github.com/Eelis/geordi/blob/master/prelude/tracked.hpp
// With major refactoring by Carlo Wood.

#pragma once

#include "debug.h"
#include <new>
#include <ostream>
#include <vector>
#include <boost/range/adaptor/reversed.hpp>

NAMESPACE_DEBUG_CHANNELS_START
extern channel_ct tracked;
NAMESPACE_DEBUG_CHANNELS_END

#define DECLARE_TRACKED(C)                                              \
    namespace { constexpr char const* const name_##C = #C; }            \
    struct C : tracked::Tracked<&name_##C> {                            \
      using tracked::Tracked<&name_##C>::Tracked;                       \
    }

namespace tracked {

// Usage:
//
// DECLARE_TRACKED(B);
//
// after which using B will print debug info about
// construction, destruction, copying, moving, assignment,
// allocation and deletion to dc::tracked.
//
// One may use tracked::mute() and tracked::unmute() to
// verbosely turn dc::tracked on and off.
//
// To track an existing class Foo, derive it from Tracked:
//
// extern char const* name_Foo;
// class Foo : public tracked::Tracked<&name_Foo>
// {
//   using tracked::Tracked<&name_Foo>::Tracked;
//
//   Foo(Foo& orig) : tracked::Tracked<&name_Foo>{orig}, ... { }
//   Foo(Foo const& orig) : tracked::Tracked<&name_Foo>{orig}, ... { }
//   Foo(Foo&& orig) : tracked::Tracked<&name_Foo>{std::move(orig)}, ... { }
//   Foo(Foo const&& orig) : tracked::Tracked<&name_Foo>{std::move(orig)}, ... { }
//   void operator=(Foo const& orig) { tracked::Tracked<&name_Foo>::operator=(orig); ... }
//   void operator=(Foo&& orig) { tracked::Tracked<&name_Foo>::operator=(std::move(orig)); ... }
// ...
//
// and add to Foo.cxx
//
// char const* name_Foo;
//
// To print the instance name of an object, one could
// write static_cast<Foo::Tracked const&>(*this) to an ostream.
//

template<char const* const* NAME>
struct Tracked {
  struct Entry {
   private:
    Tracked const* p;

   public:
    enum Status { fresh, pillaged, destructed, deleted } status;
    Entry(Tracked const* t) : p(t), status(fresh) { }

    void set_status(Status st) { status = st; }

    bool status_destructed() const { return status >= destructed; }
    bool status_deleted() const { return status == deleted; }
    bool status_below(Status st) const { return status < st; }
    bool operator==(Tracked const* t) const { return p == t; }
    bool operator<(void const* vp) const { return p < vp; }

    using Entries = std::vector<Entry>;
    static Entries& entries();
    std::ptrdiff_t id() const { return this - &entries().front(); }

    friend std::ostream& operator<<(std::ostream& os, Entry const& e) { return os << *NAME << e.id(); }
  };

  Tracked()
  {
    make_entry();
    Dout(dc::tracked, *this << "* [" << this << ']');
  }

  Tracked(Tracked const& lvalue)
  {
    lvalue.assert_status_below(Entry::pillaged, "copy");
    make_entry();
    Dout(dc::tracked, *this << "*(" << lvalue << ") [" << this << ']');
  }

  Tracked(Tracked&& rvalue)
  {
    rvalue.assert_status_below(Entry::pillaged, "move");
    make_entry();
    Dout(dc::tracked, rvalue << "=>" << *this << "* [" << this << ']');
    rvalue.entry()->set_status(Entry::pillaged);
  }

  ~Tracked()
  {
    assert_status_below(Entry::destructed, "re-destruct");
    Dout(dc::tracked, *this << "~ [" << this << ']');
    entry()->set_status(Entry::destructed);
  }

  void operator=(Tracked const& r)
  {
    assert_status_below(Entry::destructed, "assign to");
    r.assert_status_below(Entry::pillaged, "assign from");
    Dout(dc::tracked, *this << '=' << r << " [" << this << ']');
    entry()->set_status(Entry::fresh);
  }

  void operator=(Tracked&& r)
  {
    assert_status_below(Entry::destructed, "move-assign to");
    r.assert_status_below(Entry::pillaged, "move");
    entry()->set_status(Entry::fresh);
    Dout(dc::tracked, r << "=>" << *this << " [" << this << ']');
    r.entry()->set_status(Entry::pillaged);
  }

  void refresh()
  {
    assert_status_below(Entry::destructed, "refresh");
    entry()->set_status(Entry::fresh);
    Dout(dc::tracked, "Revived " << *this << " [" << this << ']');
  }

  void* operator new(std::size_t const s)
  {
    void* p = ::operator new(s);
    return static_cast<Tracked*>(p)->op_new(s, false, p);
  }

  void* operator new[](std::size_t const s)
  {
    void* p = ::operator new[](s);
    return static_cast<Tracked*>(p)->op_new(s, true, p);
  }

  void* operator new(std::size_t const s, std::nothrow_t const& t) throw()
  {
    void* p = ::operator new(s, t);
    return static_cast<Tracked*>(p)->op_new(s, false, p);
  }

  void* operator new[](std::size_t const s, std::nothrow_t const& t) throw()
  {
    void* p = ::operator new[](s, t);
    return static_cast<Tracked*>(p)->op_new(s, true, p);
  }

  void* operator new(std::size_t const, void* const p) throw() { return p; }
  void* operator new[](std::size_t const, void* const p) throw() { return p; }

  void operator delete(void* const p, std::size_t const s) throw()
  {
    static_cast<Tracked*>(p)->op_delete(static_cast<Tracked*>(p), s);
  }

  void operator delete[](void* const p, std::size_t const s) throw()
  {
    static_cast<Tracked*>(p)->op_array_delete(static_cast<Tracked*>(p), s);
  }

  void make_entry() const;
  Entry* entry() const;

  static void atexit();

  void assert_status_below(typename Entry::Status status, std::string const& s) const;
  void* op_new(std::size_t, bool const array, void* const r);
  void op_delete(void* const p, std::size_t const s);
  void op_array_delete(void* const p, std::size_t const s);

  friend std::ostream& operator<<(std::ostream& os, Tracked const& t) { return os << *t.entry(); }
};

//static
template<char const* const* NAME>
typename Tracked<NAME>::Entry::Entries& Tracked<NAME>::Entry::entries()
{
  static Entries* p = nullptr;
  if (!p)
  {
    p = new Entries;
    std::atexit(&atexit);
  }
  return *p;
}

template<char const* const* NAME>
void Tracked<NAME>::make_entry() const
{
  if (Entry* const e = entry())
    if (!e->status_destructed())
      Dout(dc::tracked, "leaked: " << e << " [" << this << ']');
  Entry::entries().emplace_back(this);
}

template<char const* const* NAME>
typename Tracked<NAME>::Entry* Tracked<NAME>::entry() const
{
  for (auto&& e : boost::adaptors::reverse(Entry::entries()))
    if (e == this) return &e;
  return nullptr;
}

template<char const* const* NAME>
void Tracked<NAME>::assert_status_below(typename Entry::Status status, std::string const& s) const
{
  Entry* const e = entry();
  if (!e)
    Dout(dc::tracked, "Trying to " << s << " non-existent object:");
  if (e->status_below(status))
    return;
  Dout(dc::tracked, "Trying to " << s << (e->status_destructed() ? " destructed " : " pillaged ") << *e << ':');
}

template<char const* const* NAME>
void* Tracked<NAME>::op_new(std::size_t, bool const array, void* const r)
{
  if (!r)
    return 0;
  Dout(dc::tracked, "new(" << *NAME << (array ? "[]" : "") << ") [" << this << "]");
  return r;
}

template<char const* const* NAME>
void Tracked<NAME>::op_delete(void* const p, std::size_t const s)
{
  ::operator delete(p);

PRAGMA_DIAGNOSTIC_PUSH_IGNORED("-Wuse-after-free")
  for (auto&& e : Entry::entries())
    if (!(e < p) && e < static_cast<char*>(p) + s)
    {
      Dout(dc::tracked, "delete(" << e << ") [" << this << ']');
      e.set_status(Entry::deleted);
      return;
    }
PRAGMA_DIAGNOSTIC_POP
}

template<char const* const* NAME>
void Tracked<NAME>::op_array_delete(void* const p, std::size_t const s)
{
  ::operator delete[](p);
  Dout(dc::tracked|continued_cf, "delete[");
  bool first = true;
  for (auto&& e : Entry::entries())
    if (!e.status_deleted() && !(e < p) && e < static_cast<char*>(p) + s)
    {
      if (first)
        first = false;
      else
        Dout(dc::continued, ", ");
      Dout(dc::continued, e);
      e.set_status(Entry::deleted);
    }
  Dout(dc::finish, "] [" << this << "]");
}

//static
template<char const* const* NAME>
void Tracked<NAME>::atexit()
{
  bool first = true;
  for (auto&& e : Entry::entries())
    if (!e.status_destructed())
    {
      if (first)
      {
        Dout(dc::tracked|continued_cf, "leaked: ");
        first = false;
      }
      else
        Dout(dc::continued, ", ");

      Dout(dc::continued, e);
    }

  if (!first)
  {
    Dout(dc::finish, '.');
  }
}

inline void mute()
{
  Dout(dc::tracked, "muted");
  Debug(dc::tracked.off());
}

inline void unmute()
{
  Debug(dc::tracked.on());
  Dout(dc::tracked, "unmuted");
}

} // namespace tracked;

#if 0
DECLARE_TRACKED(B);

int main()
{
  Debug(NAMESPACE_DEBUG::init());

  B b0;                        // B0*
  B* d;
  {
    B b1;                      // B1*
    B b2(b1);                  // B2*(B1)
    B b3(b1);                  // B3*(B1)
    B b4(std::move(b1));       // B1=>B4*
    B b5(b1);                  // Trying to copy pillaged B1: B5*(B1)
    B b6(std::move(b1));       // Trying to move pillaged B1: B1=>B6*
    b4.~B();                   // B4~
    d = &b1;
  }                            // B6~, B5~, Trying to re-destruct destructed B4: B4~, B3~, B2~, B1~

  B b7(*d);                    // Trying to copy destructed B1: B7*(B1)

  b7 = b0;                     // B7=B0
  b7 = *d;                     // Trying to assign from destructed B1: B7=B1
  b7 = std::move(b0);          // B0=>B7
  b7 = b0;                     // Trying to assign from pillaged B0: B7=B0

  B b8;                        // B8*
  Dout(dc::notice, "b8 = " << b8); // b8 = B8

  B* b9 = new B;               // new(B), B9*
  B* b10 = new B(*b9);         // new(B), B10*(B9)
  delete b9;                   // B9~, delete(B9)
  delete b10;                  // B10~, delete(B10)
  B* b11 = new B[3];           // new(B[]), B11*, B12*, B13*
  delete [] b11;               // B13~, B12~, B11~, delete[B11, B12, B13]
}                              // B8~, B7~, B0~
#endif
