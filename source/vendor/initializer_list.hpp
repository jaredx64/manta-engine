#pragma once
#include <vendor/config.hpp>

#if USE_OFFICIAL_HEADERS
	#include <vendor/conflicts.hpp>
		#include <initializer_list>
	#include <vendor/conflicts.hpp>
#else
	#include <vendor/stddef.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// initializer_list

	namespace std
	{
	#if PIPELINE_COMPILER_MSVC
		template <class _Elem> class initializer_list
		{
		public:
			using value_type = _Elem;
			using reference = const _Elem &;
			using const_reference = const _Elem &;
			using size_type = size_t;
			using iterator = const _Elem *;
			using const_iterator = const _Elem *;

			constexpr initializer_list() noexcept : _First(nullptr), _Last(nullptr) {}
			constexpr initializer_list(const _Elem *_First_arg, const _Elem *_Last_arg) noexcept :
				_First(_First_arg), _Last(_Last_arg) { }

			constexpr const _Elem *begin() const noexcept { return _First; }
			constexpr const _Elem *end() const noexcept { return _Last; }

			constexpr size_t size() const noexcept { return static_cast<size_t>(_Last - _First); }

		private:
			const _Elem *_First;
			const _Elem *_Last;
		};
	#else
		template <class _E> class initializer_list
		{
		public:
			typedef _E value_type;
			typedef const _E &reference;
			typedef const _E &const_reference;
			typedef size_t size_type;
			typedef const _E *iterator;
			typedef const _E *const_iterator;

		private:
			iterator _M_array;
			size_type _M_len;
			constexpr initializer_list(const_iterator __a, size_type __l) :
				_M_array(__a), _M_len(__l) { }

		public:
			constexpr initializer_list() noexcept : _M_array(0), _M_len(0) {}
			constexpr size_type size() const noexcept { return _M_len; }
			constexpr const_iterator begin() const noexcept { return _M_array; }
			constexpr const_iterator end() const noexcept { return begin() + size(); }
		};

		template <class _Tp>
		constexpr const _Tp *begin(initializer_list<_Tp> __ils) noexcept { return __ils.begin(); }
		template <class _Tp>
		constexpr const _Tp *end(initializer_list<_Tp> __ils) noexcept { return __ils.end(); }
	#endif
	}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif