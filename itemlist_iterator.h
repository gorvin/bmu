#ifndef ITEMLIST_ITERATOR_H
#define ITEMLIST_ITERATOR_H
#include <iterator>

namespace beam_me_up{

/** Specijalizuj ovo za trazeni tip liste */
template <typename _T, typename _C> 
struct itemlist_traits
{
	typedef typename _T list_type;
	typedef typename _C value_type;
	static size_t size(list_type& __c);
	static value_type item(_T& __c, size_t idx);
};


/** Iterator koji omogucuje koristenje standardnih algoritama na listi koja ima interfejse
 za dobijanje duzine liste i elementa na datom indeksu definisane specijalizacijom itemlist_traits
*/
template<typename _ItemList, typename _ValueType = xercesc::DOMNode*>
class itemlist_iterator : public std::iterator<std::input_iterator_tag, _ValueType> {
    _ItemList& __list;
    size_t __idx;
    size_t __length;

    //za formiranje end() iteratora
    explicit itemlist_iterator (_ItemList& list, size_t length)
     : __list(list)
     , __idx(length)
     , __length(length)
    { }

public:
	typedef itemlist_traits<_ItemList, _ValueType> traits_type;
    typedef _ValueType value_type;

    explicit itemlist_iterator (_ItemList& list)
     : __list(list)
     , __idx(0)
     , __length(traits_type::size(list))
    { }

    itemlist_iterator end(void) const { return itemlist_iterator(__list, __length); }

    value_type operator* () const
    {
        assert(__idx < __length);
        return traits_type::item(__list, __idx);
    }

    bool operator == (itemlist_iterator const& __other) const
    {
        assert(&__list == &__other.__list);
        return (__idx == __other.__idx);
    }

    bool operator != (itemlist_iterator const& __other) const
    {
        return !(operator == (__other));
    }

    itemlist_iterator& operator ++ ()//prefix
    {
        ++__idx;
        return *this;
    }

    itemlist_iterator operator ++ (int)//postfix
    {
        itemlist_iterator const __tmp(*this);
        ++__idx;
        return __tmp;
    }
};

}

#endif //ITEMLIST_ITERATOR_H
