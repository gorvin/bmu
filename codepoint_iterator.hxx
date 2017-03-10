#pragma once
#include <assert.h>
#include <cstdint>
#include <iterator>

namespace beam_me_up {}
namespace bmu = beam_me_up;

namespace beam_me_up {

typedef std::uint8_t u8unit_t;
typedef std::uint16_t u16unit_t;
typedef std::uint32_t u32char_t;
const u32char_t INVALID_CODEPOINT(-1);

namespace detail {
	const u32char_t MAX_CODEPOINT(0x0010ffffu);///< Maximum valid value for a Unicode code point
	inline bool isSubOctet(u8unit_t val)
	{
		return ((val & 0xC0) == 0x80);
	}
}

template<typename _OutIt>
size_t putUTF8Octets(u32char_t cp, _OutIt it, size_t const outOctetsCount)
{
	typedef typename std::iterator_traits<_OutIt>::iterator_category iterator_category;
	static_assert(std::is_base_of<std::output_iterator_tag, iterator_category>::value || std::is_base_of<std::random_access_iterator_tag, iterator_category>::value
		, "putUTF8Octets requires output iterator");
	if (cp > detail::MAX_CODEPOINT)
		return 0;
	unsigned n = 0;
	if (cp < 0x80)         // 0xxx xxxxx (septet0), at most 7 bits (0x7f)
		n = 1;
	else if (cp < 0x800)   // 110x xxxx  10xx xxxx (quintet1 sextet0), at most 5 + 6 = 11 bits (0x7ff)
		n = 2;
	else if (cp < 0x10000) // 1110 xxxx  10xx xxxx  10xx xxxx (quartet2 sextet1 sextet0), at most 4 + 2 * 6 = 16 bits (0xffff)
		n = 3;
	else                   // 1111 0xxx  10xx xxxx  10xx xxxx  10xx xxxx (trio3 sextet2 sextet1 sextet0), at most 3 + 3 * 6 = 21 bits (0x1fffff)
		n = 4;
	if (outOctetsCount < n) // do nothing if there's not enough room
		return 0;
	u8unit_t seq[4];
	for (size_t i = n - 1; i != 0; --i) { //from last until 2nd octet
		seq[i] = 0x80 | (cp & 0x3f);
		cp >>= 6;
	}
	static u8unit_t const lead1st[5]{ 0,    0, 0xC0, 0xE0, 0xF0 };
	static u8unit_t const mask1st[5]{ 0, 0xff, 0x1f, 0x0f, 0x07 };
	seq[0] = lead1st[n] | (cp & mask1st[n]); // 1st octet
	for (size_t i = 0; i < n; ++i, ++it) //copy octets to output
		*it = seq[i];
	return n;
}

template<typename _Container>
inline size_t putUTF8Octets(u32char_t cp, std::back_insert_iterator<_Container> it)
{
	return putUTF8Octets(cp, it, 4);
}

//Trail-bytes always start with "10" as the top-bits, with the lower 6 bits being used to store the remaining bits from the Unicode value. 
//A trail-byte must always follow a lead-byte - it cannot ever appear on its own. So, a Unicode value in the range 0-127 is represented as-is. 
//Values outside of this range (0x80 - 0x10FFFF) are represented using a multibyte sequence, comprising exactly one lead-byte and one-or-more trail-bytes. 
template<class _FwdIt>
u32char_t getUTF8Codepoint(_FwdIt it, _FwdIt const end, typename std::iterator_traits<_FwdIt>::difference_type& nOctets)
{
	typedef typename std::iterator_traits<_FwdIt>::iterator_category iterator_category;
	static_assert(std::is_base_of<std::forward_iterator_tag, iterator_category>::value
		, "getUTF8Codepoint requires forward iterator");
	u8unit_t const by0(static_cast<u8unit_t>(*it & 0xFF));
	if (by0 < 0x80)
		nOctets = 1;
	else if ((by0 & 0xE0) == 0xC0)
		nOctets = 2;
	else if ((by0 & 0xF0) == 0xE0)
		nOctets = 3;
	else if ((by0 & 0xF8) == 0xF0)
		nOctets = 4;
	else
		return INVALID_CODEPOINT;
	if (end - it < nOctets)
		return INVALID_CODEPOINT;
	if (1 == nOctets) // 0xxx xxxxx (septet0), at most 7 bits (0x7f)
		return by0;
	u8unit_t const by1(static_cast<u8unit_t>(*++it & 0xFF));
	if (!detail::isSubOctet(by1))
		return INVALID_CODEPOINT;
	if (2 == nOctets) // 110x xxxx  10xx xxxx (quintet1 sextet0), at most 5 + 6 = 11 bits (0x7ff)
		return ((by0 & 0x1f) << 6) | (by1 & 0x3f);
	u8unit_t const by2(static_cast<u8unit_t>(*++it & 0xFF));
	if (!detail::isSubOctet(by2))
		return INVALID_CODEPOINT;
	if (3 == nOctets) // 1110 xxxx  10xx xxxx  10xx xxxx (quartet2 sextet1 sextet0), at most 4 + 2 * 6 = 16 bits (0xffff)
		return (u32char_t(by0 & 0x0f) << 12) | (u32char_t(by1 & 0x3f) << 6) | (by2 & 0x3f);
	u8unit_t const by3(static_cast<u8unit_t>(*++it & 0xFF));
	if (!detail::isSubOctet(by3))
		return INVALID_CODEPOINT;
	if (4 == nOctets) // 1111 0xxx  10xx xxxx  10xx xxxx  10xx xxxx (trio3 sextet2 sextet1 sextet0), at most 3 + 3 * 6 = 21 bits (0x1fffff)
		return (u32char_t(by0 & 0x07) << 18) | (u32char_t(by1 & 0x3f) << 12) | (u32char_t(by2 & 0x3f) << 6) | (by3 & 0x3f);
	return INVALID_CODEPOINT;
}

template<typename _OutIt>
size_t putUTF16Words(u32char_t cp, _OutIt it, size_t const outWordsCount)
{
	typedef typename std::iterator_traits<_OutIt>::iterator_category iterator_category;
	static_assert(std::is_base_of<std::output_iterator_tag, iterator_category>::value || std::is_base_of<std::random_access_iterator_tag, iterator_category>::value
		, "putUTF8Octets requires output iterator");
	static_assert(sizeof(typename std::iterator_traits<_OutIt>::value_type) == 2, "Expected double-byte values");
	if (cp > detail::MAX_CODEPOINT)
		return 0;
	unsigned n = 0;
	if (cp > 0xffff) { //make a surrogate pair
		if (outWordsCount < 2)
			return 0;
		*it = static_cast<u16unit_t>(0xd800u | ((cp - 0x10000) >> 10));
		*++it = static_cast<u16unit_t>(0xdc00u | (cp & 0x3ff));
		return 2;
	}
	if (outWordsCount < 1)
		return 0;
	*it = cp;
	return 1;
}

template<class _FwdIt>
u32char_t getUTF16Codepoint(_FwdIt it, _FwdIt const end, typename std::iterator_traits<_FwdIt>::difference_type& nWords)
{
	typedef typename std::iterator_traits<_FwdIt>::iterator_category iterator_category;
	static_assert(std::is_base_of<std::forward_iterator_tag, iterator_category>::value
		, "getUTF16Codepoint requires forward iterator");
	static_assert(sizeof(typename std::iterator_traits<_FwdIt>::value_type) == 2, "Expected double-byte values");
	u32char_t const w1(*it & 0xFFFF);
	u32char_t cp = w1;
	nWords = 1;
	bool const bIsSurrogatePair = (0xd800u <= w1 && w1 <= 0xdbffu);// Take care of surrogate pairs first
	if (bIsSurrogatePair && (it + 1) != end) {
		u32char_t const w2(*(it + 1) & 0xFFFF); // don't yet increment iterator just peek next value
		if (0xdc00u <= w2 && w2 <= 0xdfffu) { // this is surrogate pair
			static u32char_t const mask10 = (1 << 10) - 1;
			cp = 0x10000 + (((w1 & mask10) << 10) | (w2 & mask10));
			if (cp <= detail::MAX_CODEPOINT) {
				++it; // skip already read low surrogate
				nWords = 2;
			}
			else
				cp = w1;
		}
	}
	return cp;
}

//rezultat je Unicode Codepoint. Rezultat INVALID_CODEPOINT je indikacija greske i tada je it == end
template <typename _OctetIterator>
class codepoint_iterator : public std::iterator <std::bidirectional_iterator_tag, u32char_t> {
	void setInvalid(void) 
	{
		__cp_length = 0;
		__cp = INVALID_CODEPOINT;
		__it = __end;
	}
	void dereferenceCodepoint(void);
	void goToNextCodepoint(void)
	{
		std::advance(__it, __cp_length);
		dereferenceCodepoint();
	}
	void goToPrevCodepoint(void)
	{
		if (__it == __start)
			return setInvalid(); // can't go back
		while (__it != __start && detail::isSubOctet(*(--__it))); // go back skipping suboctets
		if (detail::isSubOctet(*__it))
			return setInvalid();
		dereferenceCodepoint();
	}
public:
	explicit codepoint_iterator()
		: __cp_length(0)
		, __cp(INVALID_CODEPOINT)
		, __it()
		, __start()
		, __end()
	{
		static_assert(sizeof(typename _OctetIterator::value_type) == 1, "Expected single-byte values");
	}
	explicit codepoint_iterator(_OctetIterator __it,
		_OctetIterator __itstart,
		_OctetIterator __itend)
		: __cp_length(0)
		, __cp(INVALID_CODEPOINT)
		, __it(__it)
		, __start(__itstart)
		, __end(__itend)
	{
		assert(__it >= __start);
		assert(__it <= __end);
		dereferenceCodepoint();
	}
	_OctetIterator base() const 
	{ 
		return __it; 
	}
	u32char_t operator * () const
	{
		assert((__cp != INVALID_CODEPOINT && __cp_length != 0 && __it != __end)
			|| (__cp == INVALID_CODEPOINT && __cp_length == 0 && __it == __end));
		return __cp;
	}
	operator bool (void) const
	{
		bool bOK = INVALID_CODEPOINT != __cp && __it != __end;
		return INVALID_CODEPOINT != __cp && __it != __end;
	}
	bool operator ! (void) const
	{
		return INVALID_CODEPOINT == __cp || __it == __end;
	}
	bool operator == (codepoint_iterator const& __other) const
	{
		assert(__start == __other.__start && __end == __other.__end);
		return (__it == __other.__it);
	}
	bool operator != (codepoint_iterator const& __other) const
	{
		return !(operator == (__other));
	}
	codepoint_iterator& operator ++ () // prefix
	{
		goToNextCodepoint();
		return *this;
	}
	codepoint_iterator operator ++ (int) // postfix
	{
		codepoint_iterator const __tmp(*this);
		goToNextCodepoint();
		return __tmp;
	}
	codepoint_iterator& operator -- ()
	{
		goToPrevCodepoint();
		return *this;
	}
	codepoint_iterator operator -- (int)
	{
		codepoint_iterator const __tmp(*this);
		goToPrevCodepoint();
		return __tmp;
	}
private:
	difference_type __cp_length;
	u32char_t       __cp;
	_OctetIterator  __it;
	_OctetIterator  __start;
	_OctetIterator  __end;
};

template <typename _OctetIterator>
void codepoint_iterator<_OctetIterator>::dereferenceCodepoint(void)
{
	if (__it == __end) {
		setInvalid();
		return;
	}
	difference_type cpLen = 0;
	__cp = getUTF8Codepoint<_OctetIterator>(__it, __end, cpLen);
	__cp_length = cpLen;
	if (INVALID_CODEPOINT == __cp)
		return setInvalid();
}

}
