#pragma once
#include "bmu/codepoint_iterator.hxx"

namespace beam_me_up {

template<typename _Octet, typename _Word>
size_t trWideToMbyte(_Word it, _Word const itend, _Octet itresultbeg, _Octet const itresultend)
{
	if (!it || !itend || !itresultbeg || !itresultend)
		return -1;
	assert(itresultend - itresultbeg >= 4 * (itend - it));
	_Octet itresult = itresultbeg;
	for (; it != itend; ) {
		typename std::iterator_traits<_Word>::difference_type nWords = 0;
		u32char_t cp = getUTF16Codepoint(it, itend, nWords);
		if (INVALID_CODEPOINT == cp)
			return 0;
		it += nWords;
		size_t const nOctets = putUTF8Octets(cp, itresult, itresultend - itresult);
		if (0 == nOctets)
			return -1;
		itresult += nOctets;
	}
	return itresult - itresultbeg;
}

//U+D800 to U+DFFF
//The Unicode standard permanently reserves these code point values for UTF - 16 encoding of the high and low surrogates,
//and they will never be assigned a character, so there should be no reason to encode them. But it is possible to
//unambiguously encode them in UTF-16 by using a code unit equal to the code point, as long as no sequence of two code 
//units can be interpreted as a legal surrogate pair (that is, as long as a high surrogate is never followed by a low surrogate)
template<typename _Octet, typename _Word>
size_t trMbyteToWide(_Octet it, _Octet const itend, _Word itresultbeg, _Word const itresultend)
{
	if (!it || !itend || !itresultbeg || !itresultend)
		return -1;
	assert(itresultend - itresultbeg >= 2 * (itend - it));
	_Word itresult = itresultbeg;
	for (; it != itend; ) {
		std::iterator_traits<_Word>::difference_type nOctets = 0;
		u32char_t cp = getUTF8Codepoint(it, itend, nOctets);
		if (cp == INVALID_CODEPOINT)
			return -1;
		size_t nWords = putUTF16Words(cp, itresult, itresultend - itresult);
		if (0 == nWords)
			return 0;
		itresult += nWords;
		it += nOctets;
	}
	return itresult - itresultbeg;
}

template<typename _Octet, typename _Dword>
size_t trMbyteToUni(_Octet it, _Octet const itend, _Dword itresultbeg, _Dword itresultend)
{
	if (!it || !itend || !itresultbeg || !itresultend)
		return -1;
	assert(itresultend - itresultbeg >= (itend - it));
	_Dword itresult = itresultbeg;
	for (; it != itend; ++itresult) {
		std::iterator_traits<_Dword>::difference_type nOctets = 0;
		unsigned __int32 cp = getUTF8Codepoint(it, itend, nOctets);
		if (cp == INVALID_CODEPOINT)
			return -1;
		*itresult = cp;
		it += nOctets;
	}
	return itresult - itresultbeg;
}

template<typename _Octet, typename _Dword>
size_t trUniToMbyte(_Dword it, _Dword const itend, _Octet itresultbeg, _Octet const itresultend)
{
	if (!it || !itend || !itresultbeg || !itresultend)
		return -1;
	assert(itresultend - itresultbeg >= 4 * (itend - it));
	_Octet itresult = itresultbeg;
	for (; it != itend; ++it) {
		size_t nOctets = putUTF8Octets(*it, itresult, itresultend - itresult);
		if (!nOctets)
			return -1;
		itresult += nOctets;
	}
	return itresult - itresultbeg;
}

}
