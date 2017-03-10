#include <bmu/tydefs.h>
#include <vector>
#include <algorithm>
#include <functional>
#include <assert.h>

namespace beam_me_up{
	
template<typename _Tp>
 class match_one_of_ranges : public std::unary_function<_Tp, bool> {
    std::vector<std::pair<_Tp, _Tp> > const bounds;

public:
    bool operator()(_Tp const c) const {
        for(size_t i=0; i<bounds.size(); ++i) {
            if(bounds[i].first<=c && c<=bounds[i].second) return true;
        }
        return false;
    }

    match_one_of_ranges(std::pair<_Tp, _Tp> const* bounds_arr, size_t count)
     : bounds(bounds_arr, bounds_arr + count)
    {
#ifndef NDEBUG
        for(size_t i=0; i<bounds.size(); ++i) assert(bounds[i].first < bounds[i].second);
#endif
    }

    template<typename _InputIterator>
    match_one_of_ranges(_InputIterator beg, _InputIterator end)
     : bounds(beg, end)
    {
#ifndef NDEBUG
        for(size_t i=0; i<bounds.size(); ++i) assert(bounds[i].first < bounds[i].second);
#endif
    }
};


template<typename _Tp>
 class match_one_of_chars : public std::unary_function<_Tp, bool> {
    std::vector<_Tp> const acceptable;

public:
    bool operator()(_Tp const c) const
    {
        return std::find(acceptable.begin(), acceptable.end(), c) != acceptable.end();
    }

    match_one_of_chars(_Tp const* bounds_arr, size_t count)
     : acceptable(bounds_arr, bounds_arr + count)
     { }

    template<typename _InputIterator>
    match_one_of_chars(_InputIterator beg, _InputIterator end)
     : acceptable(beg, end)
     { }
};


/*NCName ::= Name - (Char* ':' Char*)	; An XML Name, minus the ":"
	Char   ::= #x9 | #xA | #xD | [#x20-#xD7FF] | [#xE000-#xFFFD] | [#x10000-#x10FFFF]
	Name   ::= NameStartChar (NameChar)*
  NameStartChar ::= ":" | [A-Z] | "_" | [a-z] | [#xC0-#xD6] | [#xD8-#xF6] | [#xF8-#x2FF]
                 | [#x370-#x37D] | [#x37F-#x1FFF] | [#x200C-#x200D] | [#x2070-#x218F]
                 | [#x2C00-#x2FEF] | [#x3001-#xD7FF] | [#xF900-#xFDCF] | [#xFDF0-#xFFFD]
                 | [#x10000-#xEFFFF]
  NameChar ::= NameStartChar | NCNameRestChar ::= "-" | "." | [0-9]
           | #xB7 | [#x0300-#x036F] | [#x203F-#x2040]
*/
///Ovaj predikat koristi samo kao privremenu varijablu!!!
template <typename charT, charT _opening, charT _closing>
 class match_closing : public std::unary_function<charT, bool> {
    mutable int count_opened;
public:
    bool operator()(charT const c) const {
        if(count_opened<0) return false; //bilo je vise zatvorenih nego otvorenih, znaci los je bio pocetak
        switch(c) {
        case _opening: count_opened++; break;
        case _closing: count_opened--; break;
        }
        return count_opened == 0;
    }
    match_closing(void) : count_opened(0) { }
};


template<typename _Tp>
 class match_range : public std::unary_function<_Tp, bool> {
    _Tp _l;
    _Tp _r;

public:
    bool operator()(_Tp const c) const
    {
        return _l<=c && c<=_r;
    }

    match_range(_Tp left, _Tp right)
     : _l(std::min(left, right))
     , _r(std::max(left, right))
     { }

    match_range(void)
     : _l(0)
     , _r(0)
     { }
};


/* NameStartChar ::= ":" | [A-Z] | "_" | [a-z] | [#xC0-#xD6] | [#xD8-#xF6] | [#xF8-#x2FF]
       | [#x370-#x37D] | [#x37F-#x1FFF] | [#x200C-#x200D] | [#x2070-#x218F] | [#x2C00-#x2FEF]
       | [#x3001-#xD7FF] | [#xF900-#xFDCF] | [#xFDF0-#xFFFD]
       | [#x10000-#xEFFFF]
  NOTE: Nema ":" za NCName*/
class match_NCNameStartChar : public std::unary_function<u32char_t, bool> {
    static match_one_of_ranges<u32char_t>  make_ranges(void);

    match_one_of_ranges<u32char_t>  const in_ranges;

public:
    bool operator()(u32char_t const c) const
    {
        return c == '_' || in_ranges(c);
    }

    match_NCNameStartChar(void)
     : in_ranges(make_ranges())
     { }
};


/* NameChar ::= NameStartChar | NCNameRestChar
   NCNameRestChar ::= "-" | "." | [0-9] | #xB7 | [#x0300-#x036F] | [#x203F-#x2040]
   NOTE: Nema ":" za NCName
 */
class match_NCNameChar : public std::unary_function<u32char_t, bool> {
    static match_one_of_ranges<u32char_t>  make_ranges(void);
    static match_one_of_chars<u32char_t>  make_chars(void);

    match_one_of_ranges<u32char_t> const in_ranges;
    match_one_of_chars<u32char_t> const  in_chars;
    match_NCNameStartChar const           is_ncstart;

public:
    bool operator()(u32char_t const c) const
    {
        return is_ncstart(c) || in_chars(c) || in_ranges(c);
    }
    match_NCNameChar(void)
     : in_ranges(make_ranges())
     , in_chars(make_chars())
     , is_ncstart()
     { }
};


class match_NameStartChar : public std::unary_function<u32char_t, bool> {
    match_NCNameStartChar const is_ncstart;

public:
	//needed for microsoft cl.exe
	match_NameStartChar(void) { }

    bool operator()(u32char_t const c) const
    {
        return c == ':' || is_ncstart(c);
    }
};


class match_NameChar : public std::unary_function<u32char_t, bool> {
    match_NCNameChar const is_ncrest;

public:
	//needed for microsoft cl.exe
	match_NameChar(void) { }

    bool operator()(u32char_t const c) const
    {
        return c == ':' || is_ncrest(c);
    }
};


//S ::= ' ' | '\t' | 0x0D | 0x0A
class match_White : public std::unary_function<u32char_t, bool> {
    static match_one_of_chars<u32char_t> make_chars(void);

    match_one_of_chars<u32char_t> in_chars;

public:
    bool operator()(u32char_t const c) const
    {
        return in_chars(c);
    }

    match_White(void)
     : in_chars(make_chars())
     { }
};


class match_BadAttValueChar : public std::unary_function<u32char_t, bool> {
    static match_one_of_chars<u32char_t> make_chars(u32char_t end_ch);

    match_one_of_chars<u32char_t> in_chars;

    explicit match_BadAttValueChar(void); //NE

public:
    bool operator()(u32char_t const c) const
    {
        return in_chars(c);
    }

    match_BadAttValueChar(u32char_t const end_ch)
     : in_chars(make_chars(end_ch))
     { }
};


class match_DecDigit : public std::unary_function<u32char_t, bool> {
    match_range<u32char_t> is_decimal;

public:
    bool operator()(u32char_t const c) const
    {
        return is_decimal(c);
    }
    match_DecDigit(void)
     : is_decimal('0', '9')
     { }

};


class match_HexDigit : public std::unary_function<u32char_t, bool> {
    static match_one_of_ranges<u32char_t>  make_ranges(void);

    match_one_of_ranges<u32char_t> const in_ranges;

public:
    bool operator()(u32char_t const c) const
    {
        return in_ranges(c);
    }
    match_HexDigit(void)
     : in_ranges(make_ranges())
     { }
};


class match_NoPercent : public std::unary_function<u8unit_t, bool> {
    static match_one_of_ranges<u8unit_t>  make_ranges(void);
    static match_one_of_chars<u8unit_t> make_chars(void);

    match_one_of_ranges<u8unit_t> const in_ranges;
    match_one_of_chars<u8unit_t> const in_chars;

public:
    bool operator()(u8unit_t const c) const
    {
        return in_chars(c) || in_ranges(c);
    }

    match_NoPercent(void)
     : in_ranges(make_ranges())
     , in_chars(make_chars())
     { }
};


/** Kolekcija funkcija i predikata koji rade sa UTF-8 i uzimaju u obzir da je dužina karaktera u
  oktetima varijabila. Većinom se predikati odnose na ispravne karaktere XCAP URIja a time i XMLa.
  */
struct LexerChars {
    match_NCNameStartChar const               is_NCNameStartChar;
    std::unary_negate<match_NCNameChar> const not_NCNameChar;
    match_NameStartChar const                 is_NameStartChar;
    std::unary_negate<match_NameChar> const   not_NameChar;
    std::unary_negate<match_DecDigit> const   not_dec_digit;
    std::unary_negate<match_HexDigit> const   not_hex_digit;
    match_NoPercent const                     is_NoPercent;

    /** Da li je između granica gramatički ispravan pozicioni predikat XPatha. */
    bool is_valid_pospredicate(u8vector_it itbeg, u8vector_it const itend) const;

    /** Da li je između granica gramatički ispravna vrijednost atributa. */
    bool is_valid_attvalue(u8vector_it itbeg, u8vector_it const itend, u32char_t const end_ch) const;

    /** Da li je između granica gramatički ispravna vrijednost atributa. */
    bool is_valid_attvalue(u8vector_t const& str) const
    {
        return is_valid_attvalue(
            const_cast<u8vector_t&>(str).begin()
            , const_cast<u8vector_t&>(str).end()
            , '\''
        );
    }

    /** Da li je između granica gramatički ispravan atributski predikat XPatha. */
    bool is_valid_attpredicate(
        u8vector_it itbeg
        , u8vector_it const itend
        , u8vector_t& attprefix
        , u8vector_t& attname
        , u8vector_t& attvalue
    ) const;

    LexerChars(void)
     : is_NCNameStartChar()
     , not_NCNameChar(std::not1(match_NCNameChar()))
     , is_NameStartChar()
     , not_NameChar(std::not1(match_NameChar()))
     , not_dec_digit(std::not1(match_DecDigit()))
     , not_hex_digit(std::not1(match_HexDigit()))
     , is_NoPercent()
     { }
};


}

