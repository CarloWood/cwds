// Borrowed from http://eel.is/geordi/ which is Public Domain, yay!
// Specifically, this is https://github.com/Eelis/geordi/blob/master/prelude/tracked.cpp
// Several changes by Carlo Wood to make it more suitable for use in a project with libcwd.

#include "sys.h"
#include "debug.h"
#include "tracked.h"
#include <cassert>
#include <cstdlib>
#include <vector>

namespace tracked {
namespace detail {

enum Status { fresh, pillaged, destructed };

struct Entry {
  Tracked const* p;
  char const* name;
  Status status;
};

typedef std::vector<Entry> Entries;

Entries& entries()
{
  static Entries* p = new Entries;
  return *p;
}

// Keeping track of Trackeds outside of the objects themselves allows us to give
// nice diagnostics for operations on objects that have already perished.
// Invariant: If multiple entries have identical p, then all but the last have status == destructed.
// Todo: not good enough

std::ptrdiff_t id(Entry const& e)
{
  return &e - &entries().front();
}

Entry* entry(Tracked const* const r)
{
  for (Entries::reverse_iterator i(entries().rbegin()); i != entries().rend(); ++i)
    if (i->p == r) return &*i;
  return 0;
}

std::ptrdiff_t id(Tracked const& t)
{
  return id(*entry(&t));
}

std::ostream& operator<<(std::ostream& o, Entry const& e)
{
  return o << e.name << id(e);
}

void make_entry(Tracked const* const r)
{
  if (Entry* const e = entry(r))
    if (e->status != destructed)
      Dout(dc::tracked, "leaked: " << *e << '.');
  Entry const e = {r, "?", fresh};
  entries().push_back(e);
}

void assert_status_below(Tracked const* const r, Status const st, std::string const& s)
{
  Entry* const e = entry(r);
  if (!e)
    Dout(dc::tracked, "tried to " << s << " non-existent object.");
  if (e->status < st)
    return;
  Dout(dc::tracked, "tried to " << s << (e->status == pillaged ? " pillaged " : " destructed ") << *e << '.');
}

void* op_new(std::size_t, bool const array, void* const r, char const* const name)
{
  if (!r)
    return 0;
  Dout(dc::tracked, "new(" << name << (array ? "[]" : "") << ")");
  return r;
}

void op_delete(void* const p, std::size_t const s)
{
  ::operator delete(p);

  for (Entries::const_iterator j = entries().begin(); j != entries().end(); ++j)
    if (p <= j->p && static_cast<void const*>(j->p) <= static_cast<char*>(p) + s)
    {
      Dout(dc::tracked, "delete(" << *j << ")");
      return;
    }
}

void op_array_delete(void* const p, std::size_t const s)
{
  ::operator delete[](p);
  Dout(dc::tracked|continued_cf, "delete[");
  bool first = true;
  for (Entries::const_iterator j = entries().begin(); j != entries().end(); ++j)
    if (p <= j->p && static_cast<void const*>(j->p) <= static_cast<char*>(p) + s)
    {
      if (first)
        first = false;
      else
        Dout(dc::continued, ", ");
      Dout(dc::continued, *j);
    }
  Dout(dc::finish, ']');
}

void Tracked::set_name(char const* const s) const
{
  entry(this)->name = s;
}

Tracked::Tracked()
{
  make_entry(this);
}

Tracked::Tracked(Tracked const& i)
{
  assert_status_below(&i, pillaged, "copy");
  make_entry(this);
}

void Tracked::operator=(Tracked const& r)
{
  assert_status_below(this, destructed, "assign to");
  assert_status_below(&r, pillaged, "assign from");
  entry(this)->status = fresh;
}

#if __cplusplus >= 201103
Tracked::Tracked(Tracked&& r)
{
  assert_status_below(&r, pillaged, "move");
  make_entry(this);
  entry(&r)->status = pillaged;
}

void Tracked::operator=(Tracked&& r)
{
  assert_status_below(this, destructed, "move-assign to");
  assert_status_below(&r, pillaged, "move");
  entry(this)->status = fresh;
  entry(&r)->status   = pillaged;
}
#endif

Tracked::~Tracked()
{
  assert_status_below(this, destructed, "re-destruct");
  entry(this)->status = destructed;
}

}  // namespace detail

void mute()
{
  Dout(dc::tracked, "muted");
  Debug(dc::tracked.off());
}

void unmute()
{
  Debug(dc::tracked.on());
  Dout(dc::tracked, "unmuted");
}

// B:

B::B()
{
  set_name("B");
  Dout(dc::tracked, *this << '*');
}

B::B(B const& b) : Tracked(b)
{
  set_name("B");
  Dout(dc::tracked, *this << "*(" << b << ')');
}

B& B::operator=(B const& b)
{
  Tracked::operator=(b);
  Dout(dc::tracked, *this << '=' << b);
  return *this;
}

B::~B()
{
  assert_status_below(this, detail::destructed, "destruct");
  Dout(dc::tracked, *this << '~');
}

void* B::operator new(std::size_t const s)
{
  return detail::op_new(s, false, ::operator new(s), "B");
}

void* B::operator new[](std::size_t const s)
{
  return detail::op_new(s, true, ::operator new[](s), "B");
}

#if __cplusplus >= 201103
void* B::operator new(std::size_t const s, std::nothrow_t const& t) throw()
{
  return detail::op_new(s, false, ::operator new(s, t), "B");
}

void* B::operator new[](std::size_t const s, std::nothrow_t const& t) throw()
{
  return detail::op_new(s, true, ::operator new[](s, t), "B");
}
#endif

void B::operator delete(void* const p, std::size_t const s) throw()
{
  detail::op_delete(p, s);
}

void B::operator delete[](void* const p, std::size_t const s) throw()
{
  detail::op_array_delete(p, s);
}

void B::f() const
{
  assert_status_below(this, detail::pillaged, "call B::f() on");
  Dout(dc::tracked, *this << ".f()");
}

void B::vf() const
{
  assert_status_below(this, detail::pillaged, "call B::vf() on");
  Dout(dc::tracked, *this << ".vf()");
}

#if __cplusplus >= 201103
B::B(B&& b) : Tracked(std::move(b))
{
  set_name("B");
  Dout(dc::tracked, b << "=>" << *this << '*');
}

B& B::operator=(B&& b)
{
  Tracked::operator=(std::move(b));
  Dout(dc::tracked, b << "=>" << *this);
  return *this;
}
#endif

B& B::operator++()
{
  assert_status_below(this, detail::pillaged, "pre-increment");
  Dout(dc::tracked, "++" << *this);
  return *this;
}

B B::operator++(int)
{
  assert_status_below(this, detail::pillaged, "post-increment");
  B const r(*this);
  Dout(dc::tracked, *this << "++");
  return r;
}

void B::operator*() const
{
  assert_status_below(this, detail::pillaged, "dereference");
  Dout(dc::tracked, '*' << *this);
}

template <typename C, typename Tr>
std::basic_ostream<C, Tr>& operator<<(std::basic_ostream<C, Tr>& o, B const& b)
{
  assert_status_below(&b, detail::pillaged, "read");
  return o << *entry(&b);
}

template std::ostream& operator<<<char, std::char_traits<char>>(std::ostream&, B const&);
//template std::wostream& operator<<<wchar_t, std::char_traits<wchar_t>>(std::wostream&, B const&);

// D:

D::D()
{
  set_name("D");
  Dout(dc::tracked, *this << '*');
}

D::D(D const& d) : B(d)
{
  set_name("D");
  Dout(dc::tracked, *this << "*(" << d << ')');
}

D& D::operator=(D const& d)
{
  B::operator=(d);
  Dout(dc::tracked, *this << '=' << d);
  return *this;
}

D::~D()
{
  assert_status_below(this, detail::destructed, "destruct");
  Dout(dc::tracked, *this << '~');
}

void* D::operator new(std::size_t const s)
{
  return detail::op_new(s, false, ::operator new(s), "D");
}

void* D::operator new[](std::size_t const s)
{
  return detail::op_new(s, true, ::operator new[](s), "D");
}

#if __cplusplus >= 201103
void* D::operator new(std::size_t const s, std::nothrow_t const& t) throw()
{
  return detail::op_new(s, false, ::operator new(s, t), "D");
}

void* D::operator new[](std::size_t const s, std::nothrow_t const& t) throw()
{
  return detail::op_new(s, true, ::operator new[](s, t), "D");
}
#endif

void D::operator delete(void* const p, std::size_t const s) throw()
{
  detail::op_delete(p, s);
}

void D::operator delete[](void* const p, std::size_t const s) throw()
{
  detail::op_array_delete(p, s);
}

void D::f() const
{
  assert_status_below(this, detail::pillaged, "call D::f() on");
  Dout(dc::tracked, *this << ".f()");
}

void D::vf() const
{
  assert_status_below(this, detail::pillaged, "call D::vf() on");
  Dout(dc::tracked, *this << ".vf()");
}

template <typename C, typename Tr>
std::basic_ostream<C, Tr>& operator<<(std::basic_ostream<C, Tr>& o, D const& d)
{
  assert_status_below(&d, detail::pillaged, "read");
  return o << *entry(&d);
}

template std::ostream& operator<<<char, std::char_traits<char>>(std::ostream& , D const&);
//template std::wostream& operator<<<wchar_t, std::char_traits<wchar_t>>(std::wostream&, D const&);

#if __cplusplus >= 201103
D::D(D&& d) : B(std::move(d))
{
  set_name("D");
  Dout(dc::tracked, d << "=>" << *this << '*');
}

D& D::operator=(D&& d)
{
  B::operator=(std::move(d));
  Dout(dc::tracked, d << "=>" << *this);
  return *this;
}
#endif

// In the above, it looks like there is a lot of code duplication for B and D.
// Previous implementations of these tracking facilities used clever CRTP helper
// templates to factor out as much of the common code as possible. However, to
// prevent the cleverness from showing through in gcc diagnostics, small delegators
// had to be put in B/D for all operations (in addition to the ones for the
// constructors which were always there, since constructors cannot be inherited (yet)).
// In the end, the hassle was not worth the gain, so I reverted back to the simple
// straightforward approach.

void atexit()
{
  bool first = true;
  for (detail::Entries::const_iterator i = detail::entries().begin(); i != detail::entries().end(); ++i)
    if (i->status != detail::destructed)
    {
      if (first)
      {
        Dout(dc::tracked|continued_cf, "leaked: ");
        first = false;
      }
      else
        Dout(dc::continued, ", ");

      Dout(dc::continued, *i);
    }

  if (!first)
  {
    Dout(dc::finish, ".");
    abort();
  }
}

} // namespace tracked
