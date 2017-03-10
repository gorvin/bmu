#include "bmu/LexerChars.h"
#include "bmu/codepoint_iterator.hxx"
#include "bmu/Logger.h"

namespace beam_me_up{

match_one_of_ranges<u32char_t>
 match_NCNameStartChar::make_ranges(void)
{
    static std::pair<u32char_t, u32char_t> const intervals[] = {
        std::pair<u32char_t, u32char_t>('A', 'Z'),
        std::pair<u32char_t, u32char_t>('a', 'z'),
        std::pair<u32char_t, u32char_t>(0x00c0, 0x00d6),
        std::pair<u32char_t, u32char_t>(0x00d8, 0x00f6),
        std::pair<u32char_t, u32char_t>(0x00f8, 0x02ff),
        std::pair<u32char_t, u32char_t>(0x0370, 0x037d),
        std::pair<u32char_t, u32char_t>(0x037f, 0x1fff),
        std::pair<u32char_t, u32char_t>(0x200c, 0x200d),
        std::pair<u32char_t, u32char_t>(0x2070, 0x218f),
        std::pair<u32char_t, u32char_t>(0x2c00, 0x2fef),
        std::pair<u32char_t, u32char_t>(0x3001, 0xd7ff),
        std::pair<u32char_t, u32char_t>(0xf900, 0xfdcf),
        std::pair<u32char_t, u32char_t>(0xfdf0, 0xfffd),
        std::pair<u32char_t, u32char_t>(0x10000, 0xeffff)
    };
    return match_one_of_ranges<u32char_t>(intervals, sizeof(intervals)/sizeof(std::pair<u32char_t, u32char_t>));
}

match_one_of_chars<u32char_t>
 match_NCNameChar::make_chars(void)
{
    static u32char_t const chars[] = { '-', '.', 0xb7 };
    return match_one_of_chars<u32char_t>(chars, sizeof(chars)/sizeof(u32char_t));
}

match_one_of_ranges<u32char_t>
 match_NCNameChar::make_ranges(void)
{
    static std::pair<u32char_t, u32char_t> const intervals[] = {
        std::pair<u32char_t, u32char_t>('0', '9'),
        std::pair<u32char_t, u32char_t>(0x0300, 0x036f),
        std::pair<u32char_t, u32char_t>(0x203f, 0x2040)
    };
    return match_one_of_ranges<u32char_t>(intervals, sizeof(intervals)/sizeof(std::pair<u32char_t, u32char_t>));
}

match_one_of_chars<u32char_t>
 match_White::make_chars(void)
{
    static u32char_t const chars[] = { ' ', '\t', 0x0d, 0x0a };
    return match_one_of_chars<u32char_t>(chars, sizeof(chars)/sizeof(u32char_t));
}

match_one_of_ranges<u32char_t>
 match_HexDigit::make_ranges(void)
{
    static std::pair<u32char_t, u32char_t> const intervals[] = {
        std::pair<u32char_t, u32char_t>('0', '9'),
        std::pair<u32char_t, u32char_t>('a', 'f'),
        std::pair<u32char_t, u32char_t>('A', 'F')
    };
    return match_one_of_ranges<u32char_t>(intervals, sizeof(intervals)/sizeof(std::pair<u32char_t, u32char_t>));
}

match_one_of_chars<u32char_t>
 match_BadAttValueChar::make_chars(u32char_t const end_ch)
{
    u32char_t const chars[] = { '<', '&', end_ch };
    return match_one_of_chars<u32char_t>(chars, sizeof(chars)/sizeof(u32char_t));
}

/*
node-selector          = step *( "/" step) ["/" terminal-selector]
terminal-selector      = "@" QName | "namespace::*" | extension-selector
step                   = NameorAny |
                         NameorAny "[" position "]" |
                         NameorAny "[" attr-test "]" |
                         NameorAny "[" position "]" "[" attr-test "]" |
                         extension-selector
NameorAny              = QName | "*"   ; QName from XML Namespaces
position               = 1*DIGIT
attr-test              = "@" QName "=" AttValue ; AttValue is from XML specification
extension-selector     = 1*( %x00-2e | %x30-ff )  ; anything but "/"

QName  ::= NCName ':' NCName | NCName
NCName ::= Name - (Char* ':' Char*)	; An XML Name, minus the ":"
Name   ::= NameStartChar (NameChar)*

AttValue	   ::=   	'"' ([^<&"] | Reference)* '"'	|  "'" ([^<&'] | Reference)* "'"
Reference	   ::=   	'&' Name ';' | '&#' [0-9]+ ';'	| '&#x' [0-9a-fA-F]+ ';'

NOTE: node-selector zavrsava sa (taget-selector):
   step | terminal-selector ::= "@" QName |
                              "namespace::*" |
                              NameorAny |
                              NameorAny "[" position "]" |
                              NameorAny "[" attr-test "]" |
                              NameorAny "[" position "]" "[" attr-test "]" |
                              extension-selector
*/
bool LexerChars::is_valid_pospredicate(u8vector_it itbeg, u8vector_it const itend) const
{
    DBGMSGAT("Validating positional predicate: " << std::string(itbeg, itend).c_str());
    if(itbeg == itend) return false;
    codepoint_iterator<u8vector_it> it(itbeg, itbeg, itend);
    codepoint_iterator<u8vector_it> const end(itend, itbeg, itend);
    //NOTE: *it za it == end daje INVALID_CODEPOINT
    return std::find_if(it, end, not_dec_digit) == end;
}

//AttValue	::= '"' ([^<&"] | Reference)* '"'	| "'" ([^<&'] | Reference)* "'"
//Reference	::= '&' Name ';' | '&#' [0-9]+ ';' | '&#x' [0-9a-fA-F]+ ';'
bool LexerChars::is_valid_attvalue(u8vector_it itbeg, u8vector_it const itend, u32char_t const end_ch) const
{
    DBGMSGAT("Validating value in attributive predicate: " << std::string(itbeg, itend).c_str());
    if(itbeg == itend) {
        DBGMSGAT("Atribute value OK - empty");
        return true;//prazna vrijednost
    }
    match_BadAttValueChar const is_attr_bad(end_ch);
    codepoint_iterator<u8vector_it> it(itbeg, itbeg, itend);
    codepoint_iterator<u8vector_it> const end(itend, itbeg, itend);
    //NOTE: *it za it == end daje INVALID_CODEPOINT
    if(*it == '&') {// Reference
        DBGMSGAT("It's reference");
        if(*++it == '#' ) {
            if(*++it == 'x') {
                DBGMSGAT("Hexadecimal");
                it = std::find_if(++it, end, not_hex_digit);
            } else {
                DBGMSGAT("Decimal");
                it = std::find_if(it, end, not_dec_digit);
            }
        } else if(is_NameStartChar(*it)) {
            DBGMSGAT("Name");
            it = std::find_if(it, end, not_NameChar);
        } else {
            DBGMSGAT("Invalid atribute value - bad reference");
            return false;
        }
        return *it == ';' && ++it == end;//Ok je ako je ';' i nema vise karaktera
    }
    //must be attr legal characters
    it = std::find_if(it, end, is_attr_bad);
    DBGMSGAT("It's quoted string " << std::string(itbeg, itend).c_str());

    return it == end;//Ok je ako je ';' i nema vise karaktera
}

//attr-test ::= "@" QName "=" AttValue ; AttValue is from XML specification
//AttValue	::= '"' ([^<&"] | Reference)* '"'	| "'" ([^<&'] | Reference)* "'"
bool LexerChars::is_valid_attpredicate(u8vector_it itbeg
                                                , u8vector_it const itend
                                                , u8vector_t& attprefix
                                                , u8vector_t& attname
                                                , u8vector_t& attvalue) const
{
    DBGMSGAT("Validating attributive predicate: " << std::string(itbeg, itend).c_str());
    codepoint_iterator<u8vector_it> it(itbeg, itbeg, itend);
    codepoint_iterator<u8vector_it> const end(itend, itbeg, itend);
    //NOTE: *it za it == end daje INVALID_CODEPOINT
    if(*it != '@') {
        DBGMSGAT("Invalid attributive predicate - no attribute name");
        return false;
    }
    if(!is_NCNameStartChar(*++it)) {
        DBGMSGAT("Invalid attributive predicate - no at least QName ::= NCName");
        return false; //nema QName
    }
    u8vector_it prev(it.base());
    attprefix.clear();
    it = std::find_if(it, end, not_NCNameChar);
    if(*it == ':' ) { //QName ::= NCName ':' NCName
        attprefix.assign(prev, it.base());
        if(!is_NCNameStartChar(*++it)) {
            DBGMSGAT("Invalid attributive predicate - not found expected QName ::= NCName ':' NCName");
            return false;
        }
        prev = it.base();
        it = std::find_if(it, end, not_NCNameChar);
    }
    attname.assign(prev, it.base());
    if(*it != '=') {
        DBGMSGAT("Invalid attributive predicate - no '='");
        return false;
    }
    u32char_t const end_ch(*++it);
    if(end_ch != '\"' && end_ch != '\'') {
        DBGMSGAT("Invalid attributive predicate - no start of attribute value: \' or \"");
        return false;
    }
    u8vector_it const attvalue_start((++it).base());
    it = std::find(it, end, end_ch);
    if(*it != end_ch) {
        DBGMSGAT("Invalid attributive predicate - no end of attribute value: \' or \"");
        return false;
    }
    attvalue.assign(attvalue_start, it.base());
    return is_valid_attvalue(attvalue_start, it.base(), end_ch);
}

match_one_of_ranges<u8unit_t>
 match_NoPercent::make_ranges(void)
{
    static std::pair<u8unit_t, u8unit_t> const intervals[] = {
        std::pair<u8unit_t, u8unit_t>('a', 'z'),
        std::pair<u8unit_t, u8unit_t>('A', 'Z'),
        std::pair<u8unit_t, u8unit_t>('0', '9'),
    };
    return match_one_of_ranges<u8unit_t>(intervals, sizeof(intervals)/sizeof(std::pair<u8unit_t, u8unit_t>));
}

match_one_of_chars<u8unit_t>
 match_NoPercent::make_chars(void)
{
    u8unit_t const chars[] = {
        ':', '@', '!', '$', '&', '\'', '(', ')'
        , '*', '+', ',', ';', '=', '-', '.', '_', '~'
    };
    return match_one_of_chars<u8unit_t>(chars, sizeof(chars)/sizeof(u8unit_t));
}

}
