#ifndef GENERIC_URI_H
#define GENERIC_URI_H
#include <string>

namespace beam_me_up{

/** Predstavljanje URIja pomocu osnovnih dijelova (schema, host, port, path).
        URL: '[scheme://host[:port]][/]relative-path' -
        Win path: 'C:\\path\\to\\where' - zamijenu se svi '\\' sa '/' i sve bude path
    Ne provjerava se da li ima nedozvoljenih karaktera u dijelovima URIja.
*/
class GenericURI {
    std::string scheme_;
    std::string host_;
    std::string path_;
    std::string port_;
    bool is_absolute_;
    void parse(const std::string& uri);
    void parse_uri(const std::string& uri);
    std::string::const_iterator parseAuthority(
        const std::string::const_iterator& u
        , const std::string::const_iterator& ue
    );
    void absolutise(GenericURI& relURI);
    void combinePath(const std::string& path);
public:
    GenericURI() { }
    /** Prepoznavaje dijelove URIja u stringu. Za to se koristi samo polozaji karaktera '/' i  ':',
        ne provjerava se ima li nedozvoljenih karaktera u tim dijelovima.
     */
    GenericURI(const std::string& uri);
    GenericURI(const GenericURI& base, const std::string& relative_uri);
    GenericURI(const GenericURI& rhs)
     : scheme_(rhs.scheme_)
     , host_(rhs.host_)
     , path_(rhs.path_)
     , port_(rhs.port_)
     , is_absolute_(rhs.is_absolute_)
    { }
    GenericURI& operator=(const GenericURI& rhs)
    {
        GenericURI t(rhs);
        swap(t);
        return *this;
    }
    bool operator==(const GenericURI& rhs) const
    {
        return scheme_ == rhs.scheme_ && host_ == rhs.host_ && path_ == rhs.path_ &&
               port_ == rhs.port_ && is_absolute_ == rhs.is_absolute_;
    }
    bool operator!=(const GenericURI& rhs) const
    {
        return !(operator==(rhs));
    }
    ~GenericURI() { }
    void swap(GenericURI& rhs)
    {
        std::swap(scheme_, rhs.scheme_);
        std::swap(host_, rhs.host_);
        std::swap(path_, rhs.path_);
        std::swap(port_, rhs.port_);
        std::swap(is_absolute_, rhs.is_absolute_);
    }
    const std::string& scheme() const { return scheme_; }
    const std::string& host() const { return host_; }
    const std::string& port() const;
    const std::string& path() const { return path_; }
    /** Da li je URL ili path apsolutni ili relativni. */
    const bool& is_absolute() const { return is_absolute_; }
    std::string as_string() const;
};

}

#endif //GENERIC_URI_H
