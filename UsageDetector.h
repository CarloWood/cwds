#pragma once

#include "utils/Array.h"
#include <vector>
#include "debug.h"

NAMESPACE_DEBUG_CHANNELS_START
extern channel_ct usage_detector;
NAMESPACE_DEBUG_CHANNELS_END

NAMESPACE_DEBUG_START

// Usage:
//
// NAMESPACE_DEBUG::UsageDetector<TheType> my_type;
//
template<typename T>
class UsageDetector;

template<typename T>
std::ostream& operator<<(std::ostream& os, UsageDetector<T> const& ud)
{
  return os << ud.base_class();
}

template<class T>
requires std::three_way_comparable<T>
constexpr auto operator<=>(UsageDetector<T> const& lhs, UsageDetector<T> const& rhs)
{
  return lhs.base_class() <=> rhs.base_class();
}

// Specialization for utils::Array.
template<typename T, std::size_t N, typename _Index>
class UsageDetector<utils::Array<T, N, _Index>> : protected utils::Array<T, N, _Index>
{
 private:
  char const* m_debug_name;

 protected:
  using _UDBase = utils::Array<T, N, _Index>;

 public:
  using reference = typename _UDBase::reference;
  using const_reference = typename _UDBase::const_reference;
  using index_type = typename _UDBase::index_type;

  UsageDetector(char const* debug_name) : _UDBase(), m_debug_name(debug_name)
  {
    DoutEntering(dc::usage_detector, libcwd::type_info_of<_UDBase>().demangled_name() << "::Array() [" << debug_name << "] [" << this << "]");
  }

  ~UsageDetector()
  {
    for (_Index i = ibegin(); i != iend(); ++i)
      Dout(dc::always, m_debug_name << "[" << i << "] = " << this->operator[](i));
    DoutEntering(dc::usage_detector, libcwd::type_info_of<_UDBase>().demangled_name() << "::~Array() [" << m_debug_name << "] [" << this << "]");
  }

  reference operator[](index_type __n) _GLIBCXX_NOEXCEPT
  {
    DoutEntering(dc::usage_detector, m_debug_name << "[" << __n << "] [" << this << "]");
    return _UDBase::operator[](__n);
  }

  const_reference operator[](index_type __n) const _GLIBCXX_NOEXCEPT
  {
    DoutEntering(dc::usage_detector, m_debug_name << "[" << __n << "] [" << this << "] READ-ACCESS");
    return _UDBase::operator[](__n);
  }

  reference at(index_type __n)
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".at(" << __n << ") [" << this << "]");
    return _UDBase::at(__n);
  }

  const_reference at(index_type __n) const
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".at(" << __n << ") [" << this << "] READ-ACCESS");
    return _UDBase::at(__n);
  }

  index_type ibegin() const
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".ibegin() [" << this << "] READ-ACCESS");
    return _UDBase::ibegin();
  }

  index_type iend() const
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".iend() [" << this << "] READ-ACCESS");
    return _UDBase::iend();
  }

  _UDBase const& base_class() const
  {
    DoutEntering(dc::usage_detector, "base_class() [" << m_debug_name << "] [" << this << "] READ-ACCESS");
    return *(static_cast<_UDBase const*>(this));
  }
};

// Specialization for std::vector.
template<class T, class Allocator>
class UsageDetector<std::vector<T, Allocator>> : protected std::vector<T, Allocator>
{
 private:
  using _UDBase = std::vector<T, Allocator>;

  char const* m_debug_name;

 public:
  using value_type = _UDBase::value_type;
  using allocator_type = _UDBase::allocator_type;
  using size_type = _UDBase::size_type;
  using difference_type = _UDBase::difference_type;
  using reference = _UDBase::reference;
  using const_reference = _UDBase::const_reference;
  using pointer = _UDBase::pointer;
  using const_pointer = _UDBase::const_pointer;
  using iterator = _UDBase::iterator;
  using const_iterator = _UDBase::const_iterator;
  using reverse_iterator = _UDBase::reverse_iterator;
  using const_reverse_iterator = _UDBase::const_reverse_iterator;

  // Constructors
  constexpr UsageDetector(char const* debug_name) noexcept(noexcept(Allocator())) : _UDBase(), m_debug_name(debug_name)
  {
    DoutEntering(dc::usage_detector, libcwd::type_info_of<_UDBase>().demangled_name() << "::vector() [" << debug_name << "] [" << this << "]");
  }

#if 0
  constexpr explicit UsageDetector(Allocator const& alloc) noexcept : _UDBase(alloc)
  {
  }

  constexpr UsageDetector(size_type count, T const& value, Allocator const& alloc = Allocator()) : _UDBase(count, value, alloc)
  {
  }

  constexpr explicit UsageDetector(size_type count, Allocator const& alloc = Allocator()) : _UDBase(count, alloc)
  {
  }

  template< class InputIt >
  constexpr UsageDetector(InputIt first, InputIt last, Allocator const& alloc = Allocator()) : _UDBase(first, last, alloc)
  {
  }

  constexpr UsageDetector(UsageDetector const& other) : _UDBase(other)
  {
  }

  constexpr UsageDetector(UsageDetector const& other, Allocator const& alloc) : _UDBase(other, alloc)
  {
  }

  constexpr UsageDetector(UsageDetector&& other) noexcept : _UDBase(std::move(other))
  {
  }

  constexpr UsageDetector(UsageDetector&& other, Allocator const& alloc) : _UDBase(std::move(other), alloc)
  {
  }

  constexpr UsageDetector(std::initializer_list<T> init, Allocator const& alloc = Allocator()) : _UDBase(init, alloc)
  {
  }
#endif

  // Destructor
  constexpr ~UsageDetector()
  {
    DoutEntering(dc::usage_detector, libcwd::type_info_of<_UDBase>().demangled_name() << "::~vector() [" << m_debug_name << "] [" << this << "]");
  }

#if 0
  constexpr UsageDetector& operator=(UsageDetector const& other)
  {
    _UDBase::operator=(other);
    return *this;
  }

  constexpr UsageDetector& operator=(UsageDetector&& other) noexcept(std::allocator_traits<Allocator>::propagate_on_container_move_assignment::value)
  {
    _UDBase::operator=(std::move(other));
    return *this;
  }

  constexpr UsageDetector& operator=(std::initializer_list<T> ilist)
  {
    _UDBase::operator=(ilist);
    return *this;
  }
#else
  UsageDetector& operator=(UsageDetector const& other) = delete;
#endif

  constexpr void assign(size_type count, T const& value)
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".assign(" << count << ", " << value << ") [" << this << "]");
    _UDBase::assign(count, value);
  }

  template<class InputIt>
  constexpr void assign(InputIt first, InputIt last)
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".assign(" << first << ", " << last << ") [" << this << "]");
    _UDBase::assign(first, last);
  }

  constexpr void assign(std::initializer_list<T> ilist)
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".assign(" << ilist << ") [" << this << "]");
    _UDBase::assign(ilist);
  }

  constexpr allocator_type get_allocator() const noexcept
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".get_allocator() [" << this << "] READ-ACCESS");
    return _UDBase::get_allocator();
  }

  constexpr reference at(size_type pos)
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".at(" << pos << ") [" << this << "]");
    return _UDBase::at(pos);
  }

  constexpr const_reference at(size_type pos) const
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".at(" << pos << ") [" << this << "] READ-ACCESS");
    return _UDBase::at(pos);
  }

  constexpr reference operator[](size_type pos)
  {
    DoutEntering(dc::usage_detector, m_debug_name << "[" << pos << "] [" << this << "]");
    return _UDBase::operator[](pos);
  }

  constexpr const_reference operator[](size_type pos) const
  {
    DoutEntering(dc::usage_detector, m_debug_name << "[" << pos << "] [" << this << "] READ-ACCESS");
    return _UDBase::operator[](pos);
  }

  constexpr reference front()
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".front() [" << this << "]");
    return _UDBase::front();
  }

  constexpr const_reference front() const
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".front() [" << this << "] READ-ACCESS");
    return _UDBase::front();
  }

  constexpr reference back()
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".back() [" << this << "]");
    return _UDBase::back();
  }

  constexpr const_reference back() const
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".back() [" << this << "] READ-ACCESS");
    return _UDBase::back();
  }

  constexpr T* data() noexcept
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".data() [" << this << "]");
    return _UDBase::data();
  }

  constexpr T const* data() const noexcept
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".data() [" << this << "] READ-ACCESS");
    return _UDBase::data();
  }

  constexpr iterator begin() noexcept
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".begin() [" << this << "]");
    return _UDBase::begin();
  }

  constexpr const_iterator begin() const noexcept
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".begin() [" << this << "] READ-ACCESS");
    return _UDBase::begin();
  }

  constexpr const_iterator cbegin() const noexcept
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".cbegin() [" << this << "] READ-ACCESS");
    return _UDBase::cbegin();
  }

  constexpr iterator end() noexcept
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".end() [" << this << "]");
    return _UDBase::end();
  }

  constexpr const_iterator end() const noexcept
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".end() [" << this << "] READ-ACCESS");
    return _UDBase::end();
  }

  constexpr const_iterator cend() const noexcept
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".cend() [" << this << "] READ-ACCESS");
    return _UDBase::cend();
  }

  constexpr reverse_iterator rbegin() noexcept
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".rbegin() [" << this << "]");
    return _UDBase::rbegin();
  }

  constexpr const_reverse_iterator rbegin() const noexcept
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".rbegin() [" << this << "] READ-ACCESS");
    return _UDBase::rbegin();
  }

  constexpr const_reverse_iterator crbegin() const noexcept
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".crbegin() [" << this << "] READ-ACCESS");
    return _UDBase::crbegin();
  }

  constexpr reverse_iterator rend() noexcept
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".rend() [" << this << "]");
    return _UDBase::rend();
  }

  constexpr const_reverse_iterator rend() const noexcept
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".rend() [" << this << "] READ-ACCESS");
    return _UDBase::rend();
  }

  constexpr const_reverse_iterator crend() const noexcept
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".crend() [" << this << "] READ-ACCESS");
    return _UDBase::crend();
  }

  [[nodiscard]] constexpr bool empty() const noexcept
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".empty() [" << this << "] READ-ACCESS");
    return _UDBase::empty();
  }

  constexpr size_type size() const noexcept
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".size() [" << this << "] READ-ACCESS");
    return _UDBase::size();
  }

  constexpr size_type max_size() const noexcept
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".max_size() [" << this << "] READ-ACCESS");
    return _UDBase::max_size();
  }

  constexpr void reserve(size_type new_cap)
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".reserve(" << new_cap << ") [" << this << "]");
    _UDBase::reserve(new_cap);
  }

  constexpr size_type capacity() const noexcept
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".capacity() [" << this << "] READ-ACCESS");
    return _UDBase::capacity();
  }

  constexpr void shrink_to_fit()
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".shrink_to_fit() [" << this << "]");
    _UDBase::shrink_to_fit();
  }

  constexpr void clear() noexcept
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".clear() [" << this << "]");
    _UDBase::clear();
  }

  constexpr iterator insert(const_iterator pos, T const& value)
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".insert(" << pos << ", " << value << ") [" << this << "]");
    return _UDBase::insert(pos, value);
  }

  constexpr iterator insert(const_iterator pos, T&& value)
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".insert(" << pos << ", " << value << ") [" << this << "]");
    return _UDBase::insert(pos, std::move(value));
  }

  template<class InputIt>
  constexpr iterator insert(const_iterator pos, InputIt first, InputIt last)
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".insert(" << pos << ", " << first << ", " << last << ") [" << this << "]");
    return _UDBase::insert(pos, first, last);
  }

  constexpr iterator insert(const_iterator pos, std::initializer_list<T> ilist)
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".insert(" << pos << ", " << ilist << ") [" << this << "]");
    return _UDBase::insert(pos, ilist);
  }

  template<class... Args>
  constexpr iterator emplace(const_iterator pos, Args&&... args)
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".emplace(" << pos << ", " << join(", ", args...) << ") [" << this << "]");
    return _UDBase::emplace(pos, std::forward<Args>(args)...);
  }

  constexpr iterator erase(const_iterator pos)
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".erase(" << pos << ") [" << this << "]");
    return _UDBase::erase(pos);
  }

  constexpr iterator erase(const_iterator first, const_iterator last)
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".erase(" << first << ", " << last << ") [" << this << "]");
    return _UDBase::erase(first, last);
  }

  constexpr void push_back(T const& value)
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".push_back(" << value << ") [" << this << "]");
    _UDBase::push_back(value);
  }

  constexpr void push_back(T&& value)
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".push_back(" << value << ") [" << this << "]");
    _UDBase::push_back(std::move(value));
  }

  template< class... Args >
  constexpr reference emplace_back(Args&&... args)
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".emplace_back(" << join(", ", args...) << ") [" << this << "]");
    return _UDBase::emplace_back(std::forward<Args>(args)...);
  }

  constexpr void pop_back()
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".pop_back() [" << this << "]");
    _UDBase::pop_back();
  }

  constexpr void resize(size_type count)
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".resize(" << count << ") [" << this << "]");
    _UDBase::resize(count);
  }

  constexpr void resize(size_type count, const value_type& value)
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".resize(" << count << ", " << value << ") [" << this << "]");
    _UDBase::resize(count, value);
  }

  constexpr void swap(UsageDetector& other) noexcept(std::allocator_traits<Allocator>::propagate_on_container_move_assignment::value)
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".swap(" << other << ") [" << this << "]");
    _UDBase::swap(other);
  }

  _UDBase const& base_class() const
  {
    DoutEntering(dc::usage_detector, "base_class() [" << m_debug_name << "] [" << this << "] READ-ACCESS");
    return *(static_cast<_UDBase const*>(this));
  }
};

// Specialization for utils::Vector.
template<typename T, typename _Index, typename _Alloc>
class UsageDetector<utils::Vector<T, _Index, _Alloc>> : protected utils::Vector<T, _Index, _Alloc>
{
 private:
  using _UDBase = utils::Vector<T, _Index, _Alloc>;

  char const* m_debug_name;

 public:
  using value_type = _UDBase::value_type;
  using allocator_type = _UDBase::allocator_type;
  using size_type = _UDBase::size_type;
  using difference_type = _UDBase::difference_type;
  using reference = _UDBase::reference;
  using const_reference = _UDBase::const_reference;
  using pointer = _UDBase::pointer;
  using const_pointer = _UDBase::const_pointer;
  using iterator = _UDBase::iterator;
  using const_iterator = _UDBase::const_iterator;
  using reverse_iterator = _UDBase::reverse_iterator;
  using const_reverse_iterator = _UDBase::const_reverse_iterator;
  using index_type = _UDBase::index_type;

  // Constructors
  constexpr UsageDetector(char const* debug_name) noexcept(noexcept(_Alloc())) : _UDBase(), m_debug_name(debug_name)
  {
    DoutEntering(dc::usage_detector, libcwd::type_info_of<_UDBase>().demangled_name() << "::Vector() [" << debug_name << "] [" << this << "]");
  }

#if 0
  constexpr explicit UsageDetector(_Alloc const& alloc) noexcept : _UDBase(alloc)
  {
  }

  constexpr UsageDetector(size_type count, T const& value, _Alloc const& alloc = _Alloc()) : _UDBase(count, value, alloc)
  {
  }

  constexpr explicit UsageDetector(size_type count, _Alloc const& alloc = _Alloc()) : _UDBase(count, alloc)
  {
  }

  template< class InputIt >
  constexpr UsageDetector(InputIt first, InputIt last, _Alloc const& alloc = _Alloc()) : _UDBase(first, last, alloc)
  {
  }

  constexpr UsageDetector(UsageDetector const& other) : _UDBase(other)
  {
  }

  constexpr UsageDetector(UsageDetector const& other, _Alloc const& alloc) : _UDBase(other, alloc)
  {
  }

  constexpr UsageDetector(UsageDetector&& other) noexcept : _UDBase(std::move(other))
  {
  }

  constexpr UsageDetector(UsageDetector&& other, _Alloc const& alloc) : _UDBase(std::move(other), alloc)
  {
  }

  constexpr UsageDetector(std::initializer_list<T> init, _Alloc const& alloc = _Alloc()) : _UDBase(init, alloc)
  {
  }
#endif

  // Destructor
  constexpr ~UsageDetector()
  {
    DoutEntering(dc::usage_detector, libcwd::type_info_of<_UDBase>().demangled_name() << "::~Vector() [" << m_debug_name << "] [" << this << "]");
  }

#if 0
  constexpr UsageDetector& operator=(UsageDetector const& other)
  {
    _UDBase::operator=(other);
    return *this;
  }

  constexpr UsageDetector& operator=(UsageDetector&& other) noexcept(std::allocator_traits<_Alloc>::propagate_on_container_move_assignment::value)
  {
    _UDBase::operator=(std::move(other));
    return *this;
  }

  constexpr UsageDetector& operator=(std::initializer_list<T> ilist)
  {
    _UDBase::operator=(ilist);
    return *this;
  }
#else
  UsageDetector& operator=(UsageDetector const& other) = delete;
#endif

  constexpr void assign(size_type count, T const& value)
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".assign(" << count << ", " << value << ") [" << this << "]");
    _UDBase::assign(count, value);
  }

  template<class InputIt>
  constexpr void assign(InputIt first, InputIt last)
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".assign(" << first << ", " << last << ") [" << this << "]");
    _UDBase::assign(first, last);
  }

  constexpr void assign(std::initializer_list<T> ilist)
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".assign(" << ilist << ") [" << this << "]");
    _UDBase::assign(ilist);
  }

  constexpr allocator_type get_allocator() const noexcept
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".get_allocator() [" << this << "] READ-ACCESS");
    return _UDBase::get_allocator();
  }

  constexpr reference front()
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".front() [" << this << "]");
    return _UDBase::front();
  }

  constexpr const_reference front() const
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".front() [" << this << "] READ-ACCESS");
    return _UDBase::front();
  }

  constexpr reference back()
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".back() [" << this << "]");
    return _UDBase::back();
  }

  constexpr const_reference back() const
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".back() [" << this << "] READ-ACCESS");
    return _UDBase::back();
  }

  constexpr T* data() noexcept
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".data() [" << this << "]");
    return _UDBase::data();
  }

  constexpr T const* data() const noexcept
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".data() [" << this << "] READ-ACCESS");
    return _UDBase::data();
  }

  constexpr iterator begin() noexcept
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".begin() [" << this << "]");
    return _UDBase::begin();
  }

  constexpr const_iterator begin() const noexcept
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".begin() [" << this << "] READ-ACCESS");
    return _UDBase::begin();
  }

  constexpr const_iterator cbegin() const noexcept
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".cbegin() [" << this << "] READ-ACCESS");
    return _UDBase::cbegin();
  }

  constexpr iterator end() noexcept
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".end() [" << this << "]");
    return _UDBase::end();
  }

  constexpr const_iterator end() const noexcept
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".end() [" << this << "] READ-ACCESS");
    return _UDBase::end();
  }

  constexpr const_iterator cend() const noexcept
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".cend() [" << this << "] READ-ACCESS");
    return _UDBase::cend();
  }

  constexpr reverse_iterator rbegin() noexcept
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".rbegin() [" << this << "]");
    return _UDBase::rbegin();
  }

  constexpr const_reverse_iterator rbegin() const noexcept
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".rbegin() [" << this << "] READ-ACCESS");
    return _UDBase::rbegin();
  }

  constexpr const_reverse_iterator crbegin() const noexcept
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".crbegin() [" << this << "] READ-ACCESS");
    return _UDBase::crbegin();
  }

  constexpr reverse_iterator rend() noexcept
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".rend() [" << this << "]");
    return _UDBase::rend();
  }

  constexpr const_reverse_iterator rend() const noexcept
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".rend() [" << this << "] READ-ACCESS");
    return _UDBase::rend();
  }

  constexpr const_reverse_iterator crend() const noexcept
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".crend() [" << this << "] READ-ACCESS");
    return _UDBase::crend();
  }

  [[nodiscard]] constexpr bool empty() const noexcept
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".empty() [" << this << "] READ-ACCESS");
    return _UDBase::empty();
  }

  constexpr size_type size() const noexcept
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".size() [" << this << "] READ-ACCESS");
    return _UDBase::size();
  }

  constexpr size_type max_size() const noexcept
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".max_size() [" << this << "] READ-ACCESS");
    return _UDBase::max_size();
  }

  constexpr void reserve(size_type new_cap)
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".reserve(" << new_cap << ") [" << this << "]");
    _UDBase::reserve(new_cap);
  }

  constexpr size_type capacity() const noexcept
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".capacity() [" << this << "] READ-ACCESS");
    return _UDBase::capacity();
  }

  constexpr void shrink_to_fit()
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".shrink_to_fit() [" << this << "]");
    _UDBase::shrink_to_fit();
  }

  constexpr void clear() noexcept
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".clear() [" << this << "]");
    _UDBase::clear();
  }

  constexpr iterator insert(const_iterator pos, T const& value)
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".insert(" << pos << ", " << value << ") [" << this << "]");
    return _UDBase::insert(pos, value);
  }

  constexpr iterator insert(const_iterator pos, T&& value)
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".insert(" << pos << ", " << value << ") [" << this << "]");
    return _UDBase::insert(pos, std::move(value));
  }

  template<class InputIt>
  constexpr iterator insert(const_iterator pos, InputIt first, InputIt last)
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".insert(" << pos << ", " << first << ", " << last << ") [" << this << "]");
    return _UDBase::insert(pos, first, last);
  }

  constexpr iterator insert(const_iterator pos, std::initializer_list<T> ilist)
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".insert(" << pos << ", " << ilist << ") [" << this << "]");
    return _UDBase::insert(pos, ilist);
  }

  template<class... Args>
  constexpr iterator emplace(const_iterator pos, Args&&... args)
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".emplace(" << pos << ", " << join(", ", args...) << ") [" << this << "]");
    return _UDBase::emplace(pos, std::forward<Args>(args)...);
  }

  constexpr iterator erase(const_iterator pos)
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".erase(" << pos << ") [" << this << "]");
    return _UDBase::erase(pos);
  }

  constexpr iterator erase(const_iterator first, const_iterator last)
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".erase(" << first << ", " << last << ") [" << this << "]");
    return _UDBase::erase(first, last);
  }

  constexpr void push_back(T const& value)
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".push_back(" << value << ") [" << this << "]");
    _UDBase::push_back(value);
  }

  constexpr void push_back(T&& value)
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".push_back(" << value << ") [" << this << "]");
    _UDBase::push_back(std::move(value));
  }

  template< class... Args >
  constexpr reference emplace_back(Args&&... args)
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".emplace_back(" << join(", ", args...) << ") [" << this << "]");
    return _UDBase::emplace_back(std::forward<Args>(args)...);
  }

  constexpr void pop_back()
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".pop_back() [" << this << "]");
    _UDBase::pop_back();
  }

  constexpr void resize(size_type count)
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".resize(" << count << ") [" << this << "]");
    _UDBase::resize(count);
  }

  constexpr void resize(size_type count, const value_type& value)
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".resize(" << count << ", " << value << ") [" << this << "]");
    _UDBase::resize(count, value);
  }

  constexpr void swap(UsageDetector& other) noexcept(std::allocator_traits<_Alloc>::propagate_on_container_move_assignment::value)
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".swap(" << other << ") [" << this << "]");
    _UDBase::swap(other);
  }

  reference operator[](index_type __n) _GLIBCXX_NOEXCEPT
  {
    DoutEntering(dc::usage_detector, m_debug_name << "[" << __n << "] [" << this << "]");
    return _UDBase::operator[](__n);
  }

  const_reference operator[](index_type __n) const _GLIBCXX_NOEXCEPT
  {
    DoutEntering(dc::usage_detector, m_debug_name << "[" << __n << "] [" << this << "] READ-ACCESS");
    return _UDBase::operator[](__n);
  }

  reference at(index_type __n)
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".at(" << __n << ") [" << this << "]");
    return _UDBase::at(__n);
  }

  const_reference at(index_type __n) const
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".at(" << __n << ") [" << this << "] READ-ACCESS");
    return _UDBase::at(__n);
  }

  index_type ibegin() const
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".ibegin() [" << this << "] READ-ACCESS");
    return _UDBase::ibegin();
  }

  index_type iend() const
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".iend() [" << this << "] READ-ACCESS");
    return _UDBase::iend();
  }

  _UDBase const& base_class() const
  {
    DoutEntering(dc::usage_detector, "base_class() [" << m_debug_name << "] [" << this << "] READ-ACCESS");
    return *(static_cast<_UDBase const*>(this));
  }
};

template<class Iter>
struct IbpMap
{
  std::pair<Iter, bool> m_ibp;

  Iter& first{m_ibp.first};
  bool& second{m_ibp.second};

  IbpMap(std::pair<Iter, bool> const& ibp) : m_ibp(ibp)
  {
    Dout(dc::usage_detector, "Creation of an ibp");
  }

  void operator=(std::pair<Iter, bool> const& ibp)
  {
    Dout(dc::usage_detector, "Assignment of an ibp");
    m_ibp = ibp;
  }
};

// Specialization for std::map.
template<class Key, class T, class Compare, class Allocator>
class UsageDetector<std::map<Key, T, Compare, Allocator>> : protected std::map<Key, T, Compare, Allocator>
{
 private:
  using _UDBase = std::map<Key, T, Compare, Allocator>;
  using _ibp_t = IbpMap<typename _UDBase::iterator>;

  char const* m_debug_name;

 public:
  using key_type = _UDBase::key_type;
  using mapped_type = _UDBase::mapped_type;
  using key_compare = _UDBase::key_compare;
  using node_type = _UDBase::node_type;
  using insert_return_type = _UDBase::insert_return_type;
  using value_type = _UDBase::value_type;
  using allocator_type = _UDBase::allocator_type;
  using size_type = _UDBase::size_type;
  using difference_type = _UDBase::difference_type;
  using reference = _UDBase::reference;
  using const_reference = _UDBase::const_reference;
  using pointer = _UDBase::pointer;
  using const_pointer = _UDBase::const_pointer;
  using iterator = _UDBase::iterator;
  using const_iterator = _UDBase::const_iterator;
  using reverse_iterator = _UDBase::reverse_iterator;
  using const_reverse_iterator = _UDBase::const_reverse_iterator;

  // Constructors
  UsageDetector(char const* debug_name) : _UDBase(), m_debug_name(debug_name)
  {
    DoutEntering(dc::usage_detector, libcwd::type_info_of<_UDBase>().demangled_name() << "::map() [" << debug_name << "] [" << this << "]");
  }

  // Destructor
  constexpr ~UsageDetector()
  {
    DoutEntering(dc::usage_detector, libcwd::type_info_of<_UDBase>().demangled_name() << "::~map() [" << m_debug_name << "] [" << this << "]");
  }

#if 0
  constexpr UsageDetector& operator=(UsageDetector const& other)
  {
    _UDBase::operator=(other);
    return *this;
  }

  constexpr UsageDetector& operator=(UsageDetector&& other) noexcept(std::allocator_traits<Allocator>::is_always_equal::value
&& std::is_nothrow_move_assignable<Compare>::value)
  {
    _UDBase::operator=(std::move(other));
    return *this;
  }

  constexpr UsageDetector& operator=(std::initializer_list<T> ilist)
  {
    _UDBase::operator=(ilist);
    return *this;
  }
#else
  UsageDetector& operator=(UsageDetector const& other) = delete;
#endif

  allocator_type get_allocator() const noexcept
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".get_allocator() [" << this << "] READ-ACCESS");
    return _UDBase::get_allocator();
  }

  T& at(size_type pos)
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".at(" << pos << ") [" << this << "]");
    return _UDBase::at(pos);
  }

  T const& at(size_type pos) const
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".at(" << pos << ") [" << this << "] READ-ACCESS");
    return _UDBase::at(pos);
  }

  T& operator[](Key const& key)
  {
    DoutEntering(dc::usage_detector, m_debug_name << "[" << key << "] [" << this << "]");
    return _UDBase::operator[](key);
  }

  T& operator[](Key&& key)
  {
    DoutEntering(dc::usage_detector, m_debug_name << "[" << key << "] [" << this << "]");
    return _UDBase::operator[](std::move(key));
  }

  iterator begin() noexcept
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".begin() [" << this << "]");
    return _UDBase::begin();
  }

  const_iterator begin() const noexcept
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".begin() [" << this << "] READ-ACCESS");
    return _UDBase::begin();
  }

  const_iterator cbegin() const noexcept
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".cbegin() [" << this << "] READ-ACCESS");
    return _UDBase::cbegin();
  }

  iterator end() noexcept
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".end() [" << this << "]");
    return _UDBase::end();
  }

  const_iterator end() const noexcept
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".end() [" << this << "] READ-ACCESS");
    return _UDBase::end();
  }

  const_iterator cend() const noexcept
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".cend() [" << this << "] READ-ACCESS");
    return _UDBase::cend();
  }

  reverse_iterator rbegin() noexcept
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".rbegin() [" << this << "]");
    return _UDBase::rbegin();
  }

  const_reverse_iterator rbegin() const noexcept
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".rbegin() [" << this << "] READ-ACCESS");
    return _UDBase::rbegin();
  }

  const_reverse_iterator crbegin() const noexcept
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".crbegin() [" << this << "] READ-ACCESS");
    return _UDBase::crbegin();
  }

  reverse_iterator rend() noexcept
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".rend() [" << this << "]");
    return _UDBase::rend();
  }

  const_reverse_iterator rend() const noexcept
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".rend() [" << this << "] READ-ACCESS");
    return _UDBase::rend();
  }

  const_reverse_iterator crend() const noexcept
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".crend() [" << this << "] READ-ACCESS");
    return _UDBase::crend();
  }

  [[nodiscard]] bool empty() const noexcept
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".empty() [" << this << "] READ-ACCESS");
    return _UDBase::empty();
  }

  size_type size() const noexcept
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".size() [" << this << "] READ-ACCESS");
    return _UDBase::size();
  }

  size_type max_size() const noexcept
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".max_size() [" << this << "] READ-ACCESS");
    return _UDBase::max_size();
  }

  void clear() noexcept
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".clear() [" << this << "]");
    _UDBase::clear();
  }

  _ibp_t insert(value_type const& value)
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".insert(" << value << ") [" << this << "]");
    return _UDBase::insert(value);
  }

  template<class P>
  _ibp_t insert(P&& value)
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".insert(" << value << ") [" << this << "]");
    return _UDBase::insert(std::move(value));
  }

  _ibp_t insert(value_type&& value)
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".insert(" << value << ") [" << this << "]");
    return _UDBase::insert(std::move(value));
  }

  iterator insert(const_iterator pos, value_type const& value)
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".insert(" << pos << ", " << value << ") [" << this << "]");
    return _UDBase::insert(pos, value);
  }

  template<class P>
  iterator insert(const_iterator pos, P&& value)
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".insert(" << pos << ", " << value << ") [" << this << "]");
    return _UDBase::insert(pos, std::move(value));
  }

  iterator insert(const_iterator pos, value_type&& value)
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".insert(" << pos << ", " << value << ") [" << this << "]");
    return _UDBase::insert(pos, std::move(value));
  }

  template<class InputIt>
  void insert(InputIt first, InputIt last)
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".insert(" << first << ", " << last << ") [" << this << "]");
    _UDBase::insert(first, last);
  }

  void insert(std::initializer_list<value_type> ilist)
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".insert(" << ilist << ") [" << this << "]");
    _UDBase::insert(ilist);
  }

  insert_return_type insert(node_type&& nh)
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".insert(" << nh << ") [" << this << "]");
    return _UDBase::insert(std::move(nh));
  }

  iterator insert(const_iterator pos, node_type&& nh)
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".insert(" << pos << ", " << nh << ") [" << this << "]");
    return _UDBase::insert(pos, std::move(nh));
  }

  template<class M>
  _ibp_t insert_or_assign(Key const& k, M&& obj)
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".insert_or_assign(" << k << ", " << obj << ") [" << this << "]");
    return _UDBase::insert_or_assign(k, std::move(obj));
  }

  template<class M>
  _ibp_t insert_or_assign(Key&& k, M&& obj)
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".insert_or_assign(" << k << ", " << obj << ") [" << this << "]");
    return _UDBase::insert_or_assign(std::move(k), std::move(obj));
  }

  template<class M>
  iterator insert_or_assign(const_iterator hint, Key const& k, M&& obj)
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".insert_or_assign(" << hint << ", " << k << ", " << obj << ") [" << this << "]");
    return _UDBase::insert_or_assign(hint, k, std::move(obj));
  }

  template<class M>
  iterator insert_or_assign(const_iterator hint, Key&& k, M&& obj)
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".insert_or_assign(" << hint << ", " << k << ", " << obj << ") [" << this << "]");
    return _UDBase::insert_or_assign(hint, std::move(k), std::move(obj));
  }

  template<class... Args>
  _ibp_t emplace(Args&&... args)
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".emplace(" << join(", ", args...) << ") [" << this << "]");
    return _UDBase::emplace(std::forward<Args>(args)...);
  }

  template<class... Args>
  iterator emplace_hint(const_iterator hint, Args&&... args)
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".emplace_hint(" << hint << ", " << join(", ", args...) << ") [" << this << "]");
    return _UDBase::emplace_hint(hint, std::forward<Args>(args)...);
  }

  template<class... Args>
  _ibp_t try_emplace(Key const& k, Args&&... args)
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".try_emplace(" << k << ", " << join(", ", args...) << ") [" << this << "]");
    return _UDBase::try_emplace(k, std::forward<Args>(args)...);
  }

  template<class... Args>
  _ibp_t try_emplace(Key&& k, Args&&... args)
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".try_emplace(" << k << ", " << join(", ", args...) << ") [" << this << "]");
    return _UDBase::try_emplace(std::move(k), std::forward<Args>(args)...);
  }

  template<class... Args>
  iterator try_emplace(const_iterator hint, Key const& k, Args&&... args)
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".try_emplace(" << hint << ", " << k << ", " << join(", ", args...) << ") [" << this << "]");
    return _UDBase::try_emplace(hint, k, std::forward<Args>(args)...);
  }

  template<class... Args>
  iterator try_emplace(const_iterator hint, Key&& k, Args&&... args)
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".try_emplace(" << hint << ", " << k << ", " << join(", ", args...) << ") [" << this << "]");
    return _UDBase::try_emplace(hint, k, std::forward<Args>(args)...);
  }

  iterator erase(iterator pos)
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".erase(" << pos << ") [" << this << "]");
    return _UDBase::erase(pos);
  }

  iterator erase(const_iterator pos)
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".erase(" << pos << ") [" << this << "]");
    return _UDBase::erase(pos);
  }

  iterator erase(const_iterator first, const_iterator last)
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".erase(" << first << ", " << last << ") [" << this << "]");
    return _UDBase::erase(first, last);
  }

  size_type erase(Key const& key)
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".erase(" << key << ") [" << this << "]");
    return _UDBase::erase(key);
  }

  void swap(UsageDetector& other) noexcept(std::allocator_traits<Allocator>::is_always_equal::value && std::is_nothrow_swappable<Compare>::value)
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".swap(" << other << ") [" << this << "]");
    _UDBase::swap(other);
  }

  node_type extract(const_iterator position)
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".extract(" << position << ") [" << this << "]");
    return _UDBase::extract(position);
  }

  node_type extract(Key const& k)
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".extract(" << k << ") [" << this << "]");
    return _UDBase::extract(k);
  }

  template<class C2>
  void merge(UsageDetector<std::map<Key, T, C2, Allocator>>& source)
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".merge(" << source << ") [" << this << "]");
    _UDBase::merge(source.base_class());
  }

  template<class C2>
  void merge(UsageDetector<std::map<Key, T, C2, Allocator>>&& source)
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".merge(" << source << ") [" << this << "]");
    _UDBase::merge(std::move(source).base_class());
  }

#if 0
  template<class C2>
  void merge(std::multimap<Key, T, C2, Allocator>& source)
  {
    DoutEntering(dc::usage_detector, m_debug_name << "merge(" << source << ") [" << this << "]");
    _UDBase::merge(source);
  }

  template<class C2>
  void merge(std::multimap<Key, T, C2, Allocator>&& source)
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".merge(" << source << ") [" << this << "]");
    _UDBase::merge(std::move(source));
  }
#endif

  size_type count(Key const& key) const
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".count(" << key << ") [" << this << "] READ-ACCESS");
    return _UDBase::count(key);
  }

  template<class K>
  size_type count(K const& x) const
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".count(" << x << ") [" << this << "] READ-ACCESS");
    return _UDBase::count(x);
  }

  iterator find(Key const& key)
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".find(" << key << ") [" << this << "]");
    return _UDBase::find(key);
  }

  const_iterator find(Key const& key) const
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".find(" << key << ") [" << this << "] READ-ACCESS");
    return _UDBase::find(key);
  }

  template<class K>
  iterator find(K const& x)
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".find(" << x << ") [" << this << "]");
    return _UDBase::find(x);
  }

  template<class K>
  const_iterator find(K const& x) const
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".find(" << x << ") [" << this << "] READ-ACCESS");
    return _UDBase::find(x);
  }

  bool contains(Key const& key) const
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".contains(" << key << ") [" << this << "] READ-ACCESS");
    return _UDBase::contains(key);
  }

  template<class K>
  bool contains(K const& x) const
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".contains(" << x << ") [" << this << "] READ-ACCESS");
    return _UDBase::contains(x);
  }

  std::pair<iterator,iterator> equal_range(Key const& key)
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".equal_range(" << key << ") [" << this << "]");
    return _UDBase::equal_range(key);
  }

  std::pair<const_iterator,const_iterator> equal_range(Key const& key) const
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".equal_range(" << key << ") [" << this << "] READ-ACCESS");
    return _UDBase::equal_range(key);
  }

  template<class K>
  std::pair<iterator,iterator> equal_range(K const& x)
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".equal_range(" << x << ") [" << this << "]");
    return _UDBase::equal_range(x);
  }

  template<class K>
  std::pair<const_iterator,const_iterator> equal_range(K const& x) const
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".equal_range(" << x << ") [" << this << "] READ-ACCESS");
    return _UDBase::equal_range(x);
  }

  iterator lower_bound(Key const& key)
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".lower_bound(" << key << ") [" << this << "]");
    return _UDBase::lower_bound(key);
  }

  const_iterator lower_bound(Key const& key) const
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".lower_bound(" << key << ") [" << this << "] READ-ACCESS");
    return _UDBase::lower_bound(key);
  }

  template<class K>
  iterator lower_bound(K const& x)
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".lower_bound(" << x << ") [" << this << "]");
    return _UDBase::lower_bound(x);
  }

  template<class K>
  const_iterator lower_bound(K const& x) const
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".lower_bound(" << x << ") [" << this << "] READ-ACCESS");
    return _UDBase::lower_bound(x);
  }

  iterator upper_bound(Key const& key)
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".upper_bound(" << key << ") [" << this << "]");
    return _UDBase::upper_bound(key);
  }

  const_iterator upper_bound(Key const& key) const
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".upper_bound(" << key << ") [" << this << "] READ-ACCESS");
    return _UDBase::upper_bound(key);
  }

  template<class K>
  iterator upper_bound(K const& x)
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".upper_bound(" << x << ") [" << this << "]");
    return _UDBase::upper_bound(x);
  }

  template<class K>
  const_iterator upper_bound(K const& x) const
  {
    DoutEntering(dc::usage_detector, m_debug_name << ".upper_bound(" << x << ") [" << this << "] READ-ACCESS");
    return _UDBase::upper_bound(x);
  }

  _UDBase const& base_class() const
  {
    DoutEntering(dc::usage_detector, "base_class() [" << m_debug_name << "] [" << this << "] READ-ACCESS");
    return *(static_cast<_UDBase const*>(this));
  }

  char const* debug_name() const
  {
    return m_debug_name;
  }
};

template<class Key, class T, class Compare, class Alloc>
void swap(UsageDetector<std::map<Key, T, Compare, Alloc>>& lhs,
          UsageDetector<std::map<Key, T, Compare, Alloc>>& rhs) noexcept(noexcept(lhs.swap(rhs)))
{
  DoutEntering(dc::usage_detector, "swap(" << lhs.debug_name() << " [" << &lhs << "], " << rhs.debug_name() << " [" << &rhs << "])");
  std::swap(lhs.base_class(), rhs.base_class());
}

template<class Key, class T, class Compare, class Alloc, class Pred>
typename std::map<Key, T, Compare, Alloc>::size_type erase_if(UsageDetector<std::map<Key, T, Compare, Alloc>>& c, Pred pred)
{
  DoutEntering(dc::usage_detector, "erase_if(" << c.debug_name() << " [" << &c << "], pred)");
  return std::erase_if(c.base_class(), pred);
}

NAMESPACE_DEBUG_END
