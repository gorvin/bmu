#pragma once
#include <string>
#include <iostream>
#include <list>
#include <bmu/single_shared.hxx>

namespace beam_me_up {}
namespace bmu  = beam_me_up;

inline std::wostream& operator<<(std::wostream& os, ::std::string const& s)
{
	return os << s.c_str();
}

inline std::wostream& operator<<(std::wostream& os, ::std::basic_string<std::uint8_t> const& s)
{
	return os << (char const*)s.c_str();
}

namespace beam_me_up {

typedef std::shared_ptr<std::wstreambuf> StdBufPtr;

/// Modifikator na početku ispisivanja svakog reda u
/// bafer formira neku vrijednost koja će biti ispisana prije eksplicitno zadanog stringa.
typedef std::function<void(StdBufPtr)> LogModifierFn;
class SharedThreadStr;

/// Datum i vrijeme u trenutku upotrebe ovog modifikatora.
void logmod_date(StdBufPtr to_out);
/// Vrijeme u trenutku upotrebe ovog modifikatora.
void logmod_time(StdBufPtr to_out);
void logmod_datetime(StdBufPtr to_out);
void logmod_threadid(StdBufPtr to_out);
LogModifierFn make_logmod_fixed_string(std::string const sts);
LogModifierFn make_logmod_tss_string(std::shared_ptr<SharedThreadStr> sts);

enum loglevel_e {
	LERROR, ///< the quietest logging, only errors 
	LWARN, ///< a little bit quieter logging, errors and warnings
	LINFO, ///< (default) standard verbosity
	LTRACE, ///< trace calls, function names, source files names and line numbers
	LDUMP, ///< dump variables, arrays and memory regions
};
typedef std::shared_ptr<loglevel_e> LogLevelPtr;
char const* to_string(loglevel_e lvl);

class LogScope;
typedef std::shared_ptr<LogScope> LogScopePtr;

/// Manipulatora tip s kojim se u runtimeu formira hijerarhija objekata takvih da
/// imaju loglevel
/// parent loglevel ima prioritet (od tekuceg manipulatora se kroz stablo ide do root ili prvog predaka ciji loglevel ponistava loglevel tekuceg manipulatora)
/// efekat zadnjeg manipulatora vazi dok se ne proslijedi novi manipulator
/// efekat root manipulator vazi u startu dok se ne proslijedi novi
class LogScope : public std::enable_shared_from_this<LogScope> {
	LogScope(void) = delete;
	LogScope(LogScope const&) = delete;
	void operator = (LogScope const&) = delete;
	LogScope(LogScopePtr parent);
public:
	static LogScopePtr create(LogScopePtr parent);
	void setLoglevel(loglevel_e newlevel);
	bool isEnabled(loglevel_e wanted);
private:
	LogScopePtr parent;
	LogLevelPtr level;
};

class logmanip {
public:
	enum type_e { indent, unindent };
private:
	friend std::wostream& operator<<(std::wostream& os, logmanip::type_e m);
	friend std::wostream& operator<<(std::wostream& os, LogScopePtr new_scope);
	friend class LoggerSink;
	friend class LogsFactoryImpl;
public:
	static void setThreadName(std::wstring const&);
	static bool isEnabled(loglevel_e wanted);
private:
	static void update(logmanip::type_e);
	static LogModifierFn getLogModifierIndentation(void);
	static LogModifierFn getLogModifierThreadname(void);
	static std::shared_ptr<SharedThreadStr> indentation_str;
	static std::shared_ptr<SharedThreadStr> threadname_str;
	static LogScopePtr                      current_scope;
};

inline std::wostream& operator<<(std::wostream& os, logmanip::type_e m)
{
	logmanip::update(m);
	return os;
}

inline std::wostream& operator<<(std::wostream& os, LogScopePtr new_scope)
{
	logmanip::current_scope = new_scope;
	return os;
}

class LogsFactoryBase {
protected:
	LogsFactoryBase(void);
public:
	~LogsFactoryBase();
	void setModifiers(std::list<LogModifierFn> modifiers);
	void setClogRotationSize(size_t bymax);
	/// Postavlja zadani fajl kao izlaz. Za filename.empty izlaz je terminal
	void setClogOutput(std::wstring const& filename);
	/// Postavlja zadani fajl kao izlaz. Za filename.empty se ponistava i sav ispis ide u std::wclog
	void setTlogOutputPrefix(std::wstring const& filename_prefix);
	std::wostream& getTlogOutput(void);
private:
	std::shared_ptr<LogsFactoryImpl> _impl;
};

typedef bmu::single_shared<LogsFactoryBase> LogsFactory;
typedef std::shared_ptr<LogsFactory> LogsFactoryPtr;

struct tlog_tag { };
extern tlog_tag  tlog;
template<typename _T>
inline std::wostream& operator<<(tlog_tag& os, _T& val)
{
	return LogsFactory::instance()->getTlogOutput() << val;
}

/// Formira string koji opisuju poziciju u source kodu, npr. za ispisivanje u žurnal. \see SOURCE_AT 
std::string srcpos_full(unsigned int line, std::string const& function, std::string const& file);

/// Slično kao \ref srcpos_full samo sto se iz file izdvoji name bez patha.
std::string srcpos_short(unsigned int line, std::string const& function, std::string const& file);

#define LOGMSG(str) std::wclog << str << std::endl;
#define LVLCLOG(lvl, str) if(::bmu::logmanip::isEnabled(lvl)) { std::wclog << ::bmu::to_string(lvl) << str << std::endl; }
#define ERRCLOG(str) LVLCLOG(::bmu::LERROR, str);
#define WARNCLOG(str) LVLCLOG(::bmu::LWARN, str);
#define INFOCLOG(str) LVLCLOG(::bmu::LINFO, str);
#define TRACECLOG(str) LVLCLOG(::bmu::LTRACE, str);
#define DUMPCLOG(str) LVLCLOG(::bmu::LDUMP, str);
#define LVLTLOG(lvl, str) if(::bmu::logmanip::isEnabled(lvl)) { \
	if(::bmu::LogsFactoryPtr lp = ::bmu::LogsFactory::instance()) { \
		lp->getTlogOutput() << str << std::endl; \
	} }
#define ERRTLOG(str) LVLTLOG(::bmu::LERROR, str);
#define WARNTLOG(str) LVLTLOG(::bmu::LWARN, str);
#define INFOTLOG(str) LVLTLOG(::bmu::LINFO, str);
#define TRACETLOG(str) LVLTLOG(::bmu::LTRACE, str);
#define DUMPTLOG(str) LVLTLOG(::bmu::LDUMP, str);
#ifndef NDEBUG
# define TRACE_HERE ::bmu::Tracer __scope_tracer_variable_name__(SOURCE_AT);
# define DBGMSG(str) LVLTLOG(::bmu::LTRACE, str)
# define DBGMSGAT(str) DBGMSG("At " << SOURCE_AT << ": " << str)
#else
# define TRACE_HERE
# define DBGMSG(str) do { } while(0)
# define DBGMSGAT(str) do { } while(0)
#endif
#define SOURCE_AT (::bmu::srcpos_short(__LINE__, __FUNCTION__, __FILE__).c_str())
#define FILE_AT __FILE__ ":" << __LINE__
}
