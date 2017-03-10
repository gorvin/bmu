#pragma once
#include <string>
#include <sstream>
#include <boost/shared_ptr.hpp>

namespace beam_me_up {}
namespace bmu = beam_me_up;

namespace beam_me_up{

// The typedefs for 8-bit, 16-bit and 32-bit unsigned integers
// You may need to change them to match your system.
typedef unsigned char  u8unit_t;
typedef unsigned short u16unit_t;
typedef unsigned int   u32char_t;//codepoint utf-32

/** UTF-8:
 U+00000000 - U+0000007F: 0xxxxxxx (only here code unit coresponds to codepoint)
 U+00000080 - U+000007FF: 110xxxxx 10xxxxxx
 U+00000800 - U+0000FFFF: 1110xxxx 10xxxxxx 10xxxxxx
 U+00010000 - U+001FFFFF: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
 U+00200000 - U+03FFFFFF: 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
 U+04000000 - U+7FFFFFFF: 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
 UTF-8 predstavlja Unicode znakove (codepoint) sa 1 do 6 okteta. Dakle dužina znaka nije fiksne
 dužine pa nije najpogodnije predstavljanje UTF-8 pomo?u std::basic_string, najviše iz razloga da
 size() ne daje broj karaktera u UTF-8 ve? broj osnovnih jedinica kontejnera npr. okteta ili rijeci.
 Još jedan razlog je što treba izbje?i korištenje standardnih funkcija za promjenu veli?ine slova
 jer Unicode karakteri nemaju 1-1 korespodenciju malih i velikih "slova" mogu?e je:
  - karakter jedne veli?ine se mapira u 2 karaktera pri promjeni veli?ne npr. njema?ko "oštro s" odgovara SS
  - isti karakter se dobija pri promjeni veli?ine više razli?itih karaktera zavisno od lokalizacije npr.
     - u turskom jeziku slovu "I" odgovara malo "i bez ta?kice" a velikom slovu "I sa ta?kicom" odgovara "i"
     - u zapadnim jezicima slovu "I" odgovara malo "i"
 što nije mogu?e ostvariti sa standardnim funkcijama koje podrazumijevaju da su karakteri obrazovani
 od fiksnog broja okteta i da postoji 1-1 korespodencija malih i velikih slova.
 Ipak basic_string po mnogo ?emu odgovara predstavljanju UTF-8 stringa pa ?u ga ipak koristiti ali
 da izbjegnem previde pri radu sa UTF-8 stavi?u da osnovna jedinica nije char ve? unsigned char.
*/
typedef std::basic_string<u8unit_t>  u8vector_t;
typedef u8vector_t::iterator         u8vector_it;

/** Ovo prakti?no i ne koristim jer je UTF-16 bitan samo pri korištenju Xerces- a on ima svoju reprezentaciju */
typedef std::basic_string<u16unit_t> u16vector_t;
typedef u16vector_t::iterator        u16vector_it;

/** Daje kontejner u kome je sadržaj iz zadanog c-stringa. Koristim radi preglednosti. */
inline u8vector_t ascii_utf8(char const* const cstr)
{
	return u8vector_t(cstr, cstr + std::char_traits<char>::length(cstr));
}

/** Daje string u kome je sadržaj iz zadanog utf8 stringa.
Koristim radi preglednosti pri kopiranju kad su imena varijabli preduga?ka.
*/
inline std::string utf8_string(u8vector_t const& vu8)
{
	return std::string(vu8.begin(), vu8.end());
}

/** Daje kontejner u kome je sadržaj iz zadanog c-stringa.
Koristim radi preglednosti pri kopiranju kad su imena varijabli preduga?ka.
*/
inline u8vector_t utf8_vector(std::string const& str)
{
	return u8vector_t(str.begin(), str.end());
}

/** Pretvaranje stringa u broj sa zadanom osnovom npr. std::dec. False ako u stringu nije broj. */
template <typename T> inline
bool from_string(T& t, std::string const& s, std::ios_base& (*f)(std::ios_base&))
{
	std::istringstream ss(s); return !(ss >> f >> t).fail();
}

/** Pretvaranje broja sa zadanom osnovom u string. */
template <class T> inline
std::string to_string(T const& t, std::ios_base& (*f)(std::ios_base&))
{
	return static_cast<std::ostringstream&>(std::ostringstream() << f << t).str();
}

/** Pomo?ni tip koji koristim da bih izbjegao kopiranje gdje ne mogu koristiti referencu na
\ref u8vector npr. pri radu sa libpqxx. */
struct rawcontent_t {
	u8unit_t const* content;
	size_t          length;
};

template<typename _T>
class ExternalOne {
public:
	typedef std::remove_reference<_T> value_type;
	static void set_one(boost::shared_ptr<value_type> external_one) { one = external_one; }
	static value_type* get_one(void) { return one.get(); }
private:
	static boost::shared_ptr<value_type> one;
};

}
