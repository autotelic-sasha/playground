#pragma once
#include <bitset>
#include <type_traits>
#include <initializer_list>
#include <set>
#include "asserts.h"
// much of this was inspired by https://m-peko.github.io/craft-cpp/posts/different-ways-to-define-binary-flags/


namespace autotelica {
	// if you start assigning values to your enum fields, you're on your own
	// this relies on the values being 0, 1, 2, ... when converted to size_t
	// 
	// if you make your enum have none as the last element, everything becomes easier too
	namespace enum_bitsets {
		namespace ops {
			// when you start optimising a lot, it is handy to have just functions
			// that operate on an untyped bitset using an enum for access
			// that way you can store plain bitsets somewhere and only do conversions
			// when you really got to
			template<typename enum_t>
			constexpr size_t enum_size(enum_t last = enum_t::none) {
				using underlying_unsigned_t = typename std::make_unsigned_t<typename std::underlying_type_t<enum_t>>;
				return static_cast<underlying_unsigned_t>(last) + 1;
			}
			template<typename enum_t>
			constexpr enum_t to_enum_value(size_t i) {
				return static_cast<enum_t>(i);
			}

			template<typename enum_t, size_t _size = enum_size(enum_t::none)>
			inline std::bitset<_size>& set(std::bitset<_size>& bits, enum_t bit, bool value = true) {
				bits.set(static_cast<size_t>(bit), value);
				return bits;
			}
			template<typename enum_t, size_t _size = enum_size(enum_t::none)>
			inline std::bitset<_size>& reset(std::bitset<_size>& bits, enum_t bit) {
				return set(bits, bit, false);
			}
			template<typename enum_t, size_t _size = enum_size(enum_t::none)>
			inline std::bitset<_size>& flip(std::bitset<_size>& bits, enum_t bit) {
				return bits.flip(static_cast<size_t>(bit));
			}
			template<typename enum_t, size_t _size = enum_size(enum_t::none)>
			inline bool test(std::bitset<_size> const& bits, enum_t bit) {
				return bits[static_cast<size_t>(bit)];
			}

			template<typename enum_t, size_t _size = enum_size(enum_t::none)>
			inline bool get(std::bitset<_size> const& bits, enum_t bit) {
				return test(bits, bit);
			}
			template<typename enum_t, size_t _size = enum_size(enum_t::none)>
			inline typename std::bitset<_size>::reference get_reference(std::bitset<_size>& bits, enum_t bit) {
				return bits[static_cast<size_t>(bit)];
			}
			template<typename enum_t, size_t _size = enum_size(enum_t::none)>
			inline std::bitset<_size>& set(std::bitset<_size>& bits, std::initializer_list<enum_t> set_bits) {
				for(auto const b : set_bits)
					bits.set(static_cast<size_t>(b));
				return bits;
			}
			template<typename enum_t, size_t _size = enum_size(enum_t::none)>
			inline std::set<enum_t> to_enum_set(std::bitset<_size> const& bits) {
				std::set<enum_t> ret;
				for (size_t i = 0; i < bits.size(); ++i)
					if (bits[i])
						ret.insert(to_enum_value<enum_t>(i));
				return ret;
			}
		}

		template <typename enum_t, size_t _size = ops::enum_size(enum_t::none)>
		class enum_bitset_ref {
			static_assert(std::is_enum<enum_t>::value, "Enum bitsets can only be implemented for enum types.");

			std::bitset<_size>& _bits;
			using this_type_t = enum_bitset_ref<enum_t, _size>;
			using reference_t = typename std::bitset<_size>::reference;
		public:
			enum_bitset_ref(std::bitset<_size>& bits_):_bits(bits_){}
			
			inline std::bitset<_size>& bits() {
				return _bits;
			}
			inline std::bitset<_size> const& bits() const{
				return _bits;
			}
			inline operator std::bitset<_size> const& () const {
				return bits();
			}
			inline operator std::bitset<_size> () const {
				return bits();
			}

			inline bool operator ==(std::bitset<_size> const& rhs) const {
				return _bits == rhs;
			}
			inline bool operator ==(enum_bitset_ref<enum_t, _size> const& rhs) const {
				return _bits == rhs._bits;
			}
			inline bool test(std::size_t pos) const {
				return _bits[pos];
			}
			inline bool test(enum_t pos) const {
				return ops::test(_bits, pos);
			}

			inline bool operator[](std::size_t pos) const {
				return test(pos);
			}
			inline bool operator[](enum_t pos) const {
				return test(pos);
			}
			inline reference_t operator[](std::size_t pos) {
				return _bits[pos];
			}
			inline reference_t operator[](enum_t pos) {
				return ops::get_reference(_bits, pos);
			}
			inline bool all() const {
				return _bits.all();
			}
			inline bool any() const {
				return _bits.any();
			}
			inline bool none() const {
				return _bits.none();
			}
			inline size_t count() const {
				return _bits.count();
			}
			inline size_t size() const {
				return _bits.size();
			}
			inline this_type_t& operator&=(this_type_t const& other) {
				_bits &= other._bits;
				return *this;
			}
			inline this_type_t& operator|=(this_type_t const& other) {
				_bits |= other._bits;
				return *this;
			}
			inline this_type_t& operator^=(this_type_t const& other) {
				_bits ^= other._bits;
				return *this;
			}
			inline std::bitset<_size> operator~() {
				return ~_bits;
			}

			// binary shifts make little sense for enums - not implementing those
			//this_type_t operator<<(std::size_t pos) const;
			//this_type_t& operator<<=(std::size_t pos);
			//this_type_t operator>>(std::size_t pos) const;
			//this_type_t& operator>>=(std::size_t pos);

			inline this_type_t& set() {
				_bits.set();
				return *this;
			}
			inline this_type_t& set( size_t pos, bool value = true) {
				_bits.set(pos, value);
				return *this;
			}
			inline this_type_t& set(enum_t pos, bool value = true) {
				ops::set(_bits, pos, value);
				return *this;
			}
			inline this_type_t& reset() {
				_bits.reset();
				return *this;
			}
			inline this_type_t& reset(size_t pos) {
				_bits.reset(pos);
				return *this;
			}
			inline this_type_t& reset(enum_t pos) {
				ops::reset(_bits, pos);
				return *this;
			}
			inline this_type_t& flip() {
				_bits.flip();
				return *this;
			}
			inline this_type_t& flip(size_t pos) {
				_bits.flip(pos);
				return *this;
			}
			inline this_type_t& flip(enum_t pos) {
				ops::flip(_bits, pos);
				return *this;
			}

			template<
				class CharT = char,
				class Traits = std::char_traits<CharT>,
				class Allocator = std::allocator<CharT>
			>
			inline std::basic_string<CharT, Traits, Allocator>
					to_string(CharT zero = CharT('0'), CharT one = CharT('1')) const {
				return _bits.to_string(zero, one);
			}
			inline unsigned long to_ulong() const {
				return _bits.to_ulong();
			}

			inline unsigned long long to_ullong() const {
				return _bits.to_ullong();
			}

			inline std::set<enum_t> to_enum_set() const {
				return ops::to_enum_set<enum_t>(_bits);
			}
		};

		template <typename enum_t, size_t _size = ops::enum_size(enum_t::none)>
		class enum_bitset {
			static_assert(std::is_enum<enum_t>::value, "Enum bitsets can only be implemented for enum types.");

			std::bitset<_size> _bits;
			using this_type_t = enum_bitset<enum_t, _size>;
			using reference_t = typename std::bitset<_size>::reference;
		public:
			enum_bitset(){}
			enum_bitset(std::initializer_list<enum_t> bits){
				ops::set(_bits, bits);
			}
			
			enum_bitset(std::bitset<_size> bits_) :_bits(bits_) {}
			enum_bitset(unsigned long val) :_bits(val) {}
			
			template< class CharT, class Traits, class Alloc >
			explicit enum_bitset(
				const std::basic_string<CharT, Traits, Alloc>& str,
				typename std::basic_string<CharT, Traits, Alloc>::size_type pos = 0,
				typename std::basic_string<CharT, Traits, Alloc>::size_type n = std::basic_string<CharT, Traits, Alloc>::npos,
				CharT zero = CharT('0'),
				CharT one = CharT('1')) : _bits(str, pos, n, zero, one) {
			}


			template< class CharT >
			explicit enum_bitset(const CharT* str,
				std::size_t n = std::size_t(-1),
				CharT zero = CharT('0'),
				CharT one = CharT('1')) :_bits(str, n, zero, one) {

			}


			inline std::bitset<_size>& bits() {
				return _bits;
			}
			inline std::bitset<_size> const& bits() const {
				return _bits;
			}
			inline operator std::bitset<_size> const& () const {
				return bits();
			}
			inline operator std::bitset<_size>() const {
				return bits();
			}

			inline bool operator ==(std::bitset<_size> const& rhs) const {
				return _bits == rhs;
			}
			inline bool operator ==(enum_bitset_ref<enum_t, _size> const& rhs) const {
				return _bits == rhs._bits;
			}
			inline bool test(std::size_t pos) const {
				return _bits[pos];
			}
			inline bool test(enum_t pos) const {
				return ops::test(_bits, pos);
			}

			inline bool operator[](std::size_t pos) const {
				return test(pos);
			}
			inline bool operator[](enum_t pos) const {
				return test(pos);
			}
			inline reference_t operator[](std::size_t pos) {
				return _bits[pos];
			}
			inline reference_t operator[](enum_t pos) {
				return ops::get_reference(_bits, pos);
			}
			inline bool all() const {
				return _bits.all();
			}
			inline bool any() const {
				return _bits.any();
			}
			inline bool none() const {
				return _bits.none();
			}
			inline size_t count() const {
				return _bits.count();
			}
			inline size_t size() const {
				return _bits.size();
			}
			inline this_type_t& operator&=(this_type_t const& other) {
				_bits &= other._bits;
				return *this;
			}
			inline this_type_t& operator|=(this_type_t const& other) {
				_bits |= other._bits;
				return *this;
			}
			inline this_type_t& operator^=(this_type_t const& other) {
				_bits ^= other._bits;
				return *this;
			}
			inline std::bitset<_size> operator~() {
				return ~_bits;
			}

			// binary shifts make little sense for enums - not implementing those
			//this_type_t operator<<(std::size_t pos) const;
			//this_type_t& operator<<=(std::size_t pos);
			//this_type_t operator>>(std::size_t pos) const;
			//this_type_t& operator>>=(std::size_t pos);

			inline this_type_t& set() {
				_bits.set();
				return *this;
			}
			inline this_type_t& set(size_t pos, bool value = true) {
				_bits.set(pos, value);
				return *this;
			}
			inline this_type_t& set(enum_t pos, bool value = true) {
				ops::set(_bits, pos, value);
				return *this;
			}
			inline this_type_t& reset() {
				_bits.reset();
				return *this;
			}
			inline this_type_t& reset(size_t pos) {
				_bits.reset(pos);
				return *this;
			}
			inline this_type_t& reset(enum_t pos) {
				ops::reset(_bits, pos);
				return *this;
			}
			inline this_type_t& flip() {
				_bits.flip();
				return *this;
			}
			inline this_type_t& flip(size_t pos) {
				_bits.flip(pos);
				return *this;
			}
			inline this_type_t& flip(enum_t pos) {
				ops::flip(_bits, pos);
				return *this;
			}

			template<
				class CharT = char,
				class Traits = std::char_traits<CharT>,
				class Allocator = std::allocator<CharT>
			>
			inline std::basic_string<CharT, Traits, Allocator>
				to_string(CharT zero = CharT('0'), CharT one = CharT('1')) const {
				return _bits.to_string(zero, one);
			}
			inline unsigned long to_ulong() const {
				return _bits.to_ulong();
			}

			inline unsigned long long to_ullong() const {
				return _bits.to_ullong();
			}
			inline std::set<enum_t> to_enum_set() const {
				return ops::to_enum_set<enum_t>(_bits);
			}
		};

		template <typename enum_t, size_t _size = enum_size(enum_t::none)>
		inline enum_bitset<enum_t, _size> operator&(
				const enum_bitset<enum_t, _size>& lhs,
				const enum_bitset<enum_t, _size>& rhs) {
			return enum_bitset<enum_t, _size>(lhs.bits() & rhs.bits());
		}
		template <typename enum_t, size_t _size = enum_size(enum_t::none)>
		inline enum_bitset<enum_t, _size> operator|(
				const enum_bitset<enum_t, _size>& lhs,
				const enum_bitset<enum_t, _size>& rhs) {
			return enum_bitset<enum_t, _size>(lhs.bits() | rhs.bits());
		}
		template <typename enum_t, size_t _size = enum_size(enum_t::none)>
		inline enum_bitset<enum_t, _size> operator^(
				const enum_bitset<enum_t, _size>& lhs,
				const enum_bitset<enum_t, _size>& rhs) {
			return enum_bitset<enum_t, _size>(lhs.bits() ^ rhs.bits());
		}

		template< class char_t, class traits_t, typename enum_t, size_t _size = enum_size(enum_t::none) >
		std::basic_ostream<char_t, traits_t>&
				operator<<(std::basic_ostream<char_t, traits_t>& os, const enum_bitset<enum_t, _size>& x) {	
			return (os << x.bits());
		}
		template< class char_t, class traits_t, typename enum_t, size_t _size = enum_size(enum_t::none) >
		std::basic_istream<char_t, traits_t>&
				operator>>(std::basic_istream<char_t, traits_t>& os, const enum_bitset<enum_t, _size>& x) {
			return (os >> x.bits());
		}
	}
}

