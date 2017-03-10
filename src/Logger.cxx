#include "LoggerImpl.h"
#include <chrono>
#include <sstream>
#include <ctime>
#include <fstream>
#include <codecvt>
#ifdef _MSC_VER
# include <Windows.h>
#endif

namespace beam_me_up {

//NOTE: workaround for thread-unsafe std::localtime
void to_localtime_thread_safe(std::time_t const& time, std::tm& tm_snapshot)
{
#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__))
	localtime_s(&tm_snapshot, &time);
#else
	localtime_r(&time, &tm_snapshot); // POSIX  
#endif
}

void logmod_date(StdBufPtr to_out)
{
	auto now = std::chrono::system_clock::now(); // system time
	auto now_time_t = std::chrono::system_clock::to_time_t(now);
	std::tm local_tm;
	to_localtime_thread_safe(now_time_t, local_tm);
	wchar_t buf[128];
	size_t cch = std::wcsftime(buf, _countof(buf), L"%x ", &local_tm); // %x writes localized date representation
	to_out->sputn(buf, cch);
}

void logmod_time(StdBufPtr to_out)
{
	auto now = std::chrono::system_clock::now(); // system time
	{
		auto now_time_t = std::chrono::system_clock::to_time_t(now);
		std::tm local_tm;
		to_localtime_thread_safe(now_time_t, local_tm);
		wchar_t buf[128];
		size_t cch = std::wcsftime(buf, _countof(buf), L"%X.", &local_tm);// %X writes localized time representation
		to_out->sputn(buf, cch);
	}
	{
		const std::chrono::duration<double> tse = now.time_since_epoch();
		std::chrono::seconds::rep usec = std::chrono::duration_cast<std::chrono::nanoseconds>(tse).count() % 1000000;
		std::wstring usstr(std::to_wstring(usec) + L" ");
		to_out->sputn(&usstr[0], usstr.size());
	}
}

void logmod_datetime(StdBufPtr to_out)
{
	auto now = std::chrono::system_clock::now(); // system time
	auto now_time_t = std::chrono::system_clock::to_time_t(now);
	std::tm local_tm;
	to_localtime_thread_safe(now_time_t, local_tm);
	wchar_t buf[128];
	size_t cch = std::wcsftime(buf, _countof(buf), L"%y%m%d-%H%M%S ", &local_tm); // %x writes time representation in short form
	to_out->sputn(buf, cch);
}

void logmod_threadid(StdBufPtr to_out)
{
	std::wostringstream oss;
	oss << this_thread::get_id() << " ";
	std::wstring const str(oss.str());
	to_out->sputn(&str[0], str.size());
}

void logmod_fixed_string_impl(std::wstring const str, StdBufPtr to_out)
{
	if(!str.empty())
		to_out->sputn(&str[0], str.size());
}

LogModifierFn make_logmod_fixed_string(std::wstring const sts)
{
	return bind(&logmod_fixed_string_impl, sts, std::placeholders::_1);
}

void logmod_tss_string_impl(std::shared_ptr<SharedThreadStr> str, StdBufPtr to_out)
{
	std::wstring const* tmpptr(&str->getStr());
	std::wstring const& tmp(str->getStr());
	if (!tmp.empty())
		to_out->sputn(&tmp[0], tmp.size());
}

LogModifierFn make_logmod_tss_string(std::shared_ptr<SharedThreadStr> sts)
{
	return bind(&logmod_tss_string_impl, sts, std::placeholders::_1);
}

char const* to_string(loglevel_e lvl)
{
	switch (lvl)
	{
	case LERROR: return "ERROR: ";
	case LWARN: return "WARN: ";
	case LINFO: return "";
	case LTRACE: return "TRACE";
	case LDUMP: return "DUMP: ";
	}
	assert(0);
	return "";
}

std::shared_ptr<SharedThreadStr> logmanip::indentation_str(std::make_shared<SharedThreadStr>());
std::shared_ptr<SharedThreadStr> logmanip::threadname_str(std::make_shared<SharedThreadStr>());
LogScopePtr logmanip::current_scope;

std::wstring getDateTimeFilenameSuffix(void)
{
	using namespace std::chrono;
	auto now = std::chrono::system_clock::now(); // system time

	const std::chrono::duration<double> tse = now.time_since_epoch();
	std::chrono::seconds::rep nanosecs = std::chrono::duration_cast<std::chrono::nanoseconds>(tse).count() % 1000000;

	auto now_time_t = std::chrono::system_clock::to_time_t(now);
	std::tm local_tm;
	to_localtime_thread_safe(now_time_t, local_tm);
	wchar_t buf[128];
	//std::strftime(date, sizeof(date), "[%Y-%m-%d]", &local_tm);
	size_t cch = std::wcsftime(buf, _countof(buf), L"%y%m%d-%H%M%S.", &local_tm); // %x writes time representation in short form
	return std::wstring(buf, cch) + std::to_wstring(nanosecs);
}

void logmanip::setThreadName(std::wstring const& name)
{
	threadname_str->getStr() = name;
}

bool logmanip::isEnabled(loglevel_e wanted)
{
	return current_scope
		? current_scope->isEnabled(wanted)
		: wanted <= LINFO;
}

LogModifierFn logmanip::getLogModifierIndentation(void)
{
	return make_logmod_tss_string(indentation_str);
}

LogModifierFn logmanip::getLogModifierThreadname(void)
{
	return make_logmod_tss_string(threadname_str);
}

void logmanip::update(logmanip::type_e m)
{ 
	switch(m)
	{
	case logmanip::indent:
		indentation_str->getStr().push_back('\t');
		break;
	case logmanip::unindent:
		if(!indentation_str->getStr().empty())
			indentation_str->getStr().pop_back();
		break;
	}
}

LogScope::LogScope(LogScopePtr parent)
	: parent(parent)
	, level()
{
	if (parent) {
		LogScopePtr eff(parent);
		while (!eff->level && eff->parent)
			eff = eff->parent;
		if (eff)
			level = eff->level; // inherited level
	}
}

LogScopePtr LogScope::create(LogScopePtr parent)
{
	return LogScopePtr(new LogScope(parent));
}

void LogScope::setLoglevel(loglevel_e newlevel)
{
	level = std::make_shared<loglevel_e>(newlevel);
}

bool LogScope::isEnabled(loglevel_e wanted)
{
	loglevel_e efflevel = level ? *level : LINFO;
	return wanted <= efflevel;
}

void BufferWriterWithModifers::setModifiers(std::list<LogModifierFn> newmodifers)
{
	modifiers.clear();
	for (auto& mod : newmodifers) {
		if (mod)
			modifiers.push_back(mod);
	}
}

void BufferWriterWithModifers::do_write_modifiers(StdBufPtr sbuf)
{
	//for (auto& mod : modifiers)
	//	mod(sbuf); // it's checked in setModifiers if null
	//for (auto i = modifiers.begin(); i != modifiers.end(); ++i) {
	//	auto& mod(*i);
	for (size_t i = 0; i < modifiers.size(); ++i) {
		auto it = modifiers.begin();
		std::advance(it, i);
		auto& mod(*it);
		mod(sbuf); // it's checked in setModifiers if null
	}
}

std::streamsize BufferWriterWithModifers::do_write_string(StdBufPtr sbuf, wchar_t const* s, std::streamsize n)
{
	return sbuf->sputn(s, n);
}

std::streamsize DirectWriter::write(wchar_t const* s, std::streamsize n)
{
	BufferWriterWithModifers::do_write_modifiers(sbuf);
	return BufferWriterWithModifers::do_write_string(sbuf, s, n);
}

std::streamsize TargetDirectWriter::write(wchar_t const* s, std::streamsize n)
{
	StdBufPtr sbuf = getsbuf();
	if (sbuf) {
		BufferWriterWithModifers::do_write_modifiers(sbuf);
		return BufferWriterWithModifers::do_write_string(sbuf, s, n);
	}
	return fallback->write(s, n);
}

QueueWriter::QueueWriter(StdBufPtr sbuf, RotateFileFn rotate, std::shared_ptr<size_t> bymax)
	: sbuf(sbuf)
	, rotate(rotate)
	, bymax(bymax)
	, bycount(0)
	, finish(false)
	, allwrite(true)
	, mutex()
	, logs(std::make_shared<MsgQueue>())
	, worker(bind(&QueueWriter::BackendWorker, this))
{ 
}

QueueWriter::~QueueWriter(void)
{
	finish = true;
	if(worker.get_id() != this_thread::get_id())
		worker.join();
}

std::streamsize QueueWriter::write(wchar_t const* s, std::streamsize n)
{
	std::wstringbuf tmpbuf;
	StdBufPtr sptmpbuf(&tmpbuf, [](std::wstringbuf*p) { });
	BufferWriterWithModifers::do_write_modifiers(sptmpbuf);
	std::wstring prefixstr(tmpbuf.str());
	{
		std::lock_guard<std::mutex> lock(mutex);
		if (!prefixstr.empty())
			logs->push_back({ &prefixstr[0], (size_t)prefixstr.size() });
		logs->push_back({ s, (size_t)n });
	}
	bycount += prefixstr.size() + (size_t)n;
	if (rotate && bymax && bycount >= *bymax) {
		bycount = 0;
		rotate();
	}
	return n;
}

void QueueWriter::BackendWorker(void)
{
#ifndef NDEBUG
	bmu::logmanip::setThreadName(L"##### BackendWorker thread #####");
#endif
	MsgQueue  tmplogs;
	do {
		if (logs->empty()) {
			this_thread::sleep_for(std::chrono::milliseconds(10));
			continue;
		}
		else {
			std::lock_guard<std::mutex> lock(mutex);
			tmplogs = std::move(*logs);
		}
		while (!tmplogs.empty()) {
			if (!allwrite && finish)
				return;
			std::wstring& val(tmplogs.front());
			if (!tmplogs.front().empty()) {
				BufferWriterWithModifers::do_write_string(sbuf, &val[0], val.size());
				tmplogs.pop_front();
			}
		}
	} while ((!logs->empty() && allwrite) || !finish);
	int dummy = 0;
}

std::streamsize NoModifiersWriter::write(wchar_t const* s, std::streamsize n)
{
	return BufferWriterWithModifers::do_write_string(sbuf, s, n);
}

LoggerBuf::LoggerBuf(BufferWriterWithModifersPtr bufwriter, size_t bufsize)
	: bufwriter(bufwriter)
	, bufsize(bufsize)
	, bufptr(new wchar_t[bufsize ? bufsize : 4096], std::default_delete<wchar_t[]>())
	, pbeg(bufptr.get())
	, pend(pbeg + bufsize)
{
	setp(pbeg, pend - 1);
}

int LoggerBuf::overflow(wchar_t c)
{
	if (!pptr())
		setp(pbeg, pbeg);
	int w = pptr() - pbase();
	if (c != EOF) {
		// We always leave space
		*pptr() = c;
		++w;
	}
	if (w == bufwriter->write(pbeg, w)) {
		setp(pbeg, pend - 1);
		return (-1 != c) ? c : 'x'; // return non-eof
	}
	else {
		setp(0, 0);
		return -1; // Indicate error.
	}
}

int LoggerBuf::sync(void)
{
	if (pptr() && pptr() > pbase())
		return overflow(-1);// Flush waiting output
	return (0);
}

LogsFactoryImpl::LogsFactoryImpl(void)
	: locEnUTF8(std::locale(), ::new std::codecvt_utf8<wchar_t>)
	, clog_orig_stdbuf(std::wclog.rdbuf(), &LogsFactoryImpl::nodeleter)
	, clog_orig_writer(std::make_shared<QueueWriter>(clog_orig_stdbuf, std::function<void(void)>(), std::shared_ptr<size_t>()))
	, clog_orig_logbuf(std::make_shared<LoggerBuf>(clog_orig_writer))
	, clog_file_bymax(std::make_shared<size_t>(-1))
	, clog_file_writer()
	, clog_file_logbuf()
	, tlog_writer(std::make_shared<TargetDirectWriter>(clog_orig_writer, bind(&LogsFactoryImpl::getTlogStreambuf, this)))
	, tlog_logbuf(std::make_shared<LoggerBuf>(tlog_writer))
	, tlog_ostream(std::make_shared<std::wostream>(tlog_logbuf.get()))
{
#ifdef _MSC_VER
	DWORD dwErr = 0;
	if (FALSE == SetConsoleOutputCP(CP_UTF8)) // important on windows
		dwErr = GetLastError();
#endif

	std::wcout.imbue(locEnUTF8);
	std::wclog.imbue(locEnUTF8);
	std::wcerr.imbue(locEnUTF8);
	tlog_ostream->imbue(locEnUTF8);

	std::wclog.rdbuf(clog_orig_logbuf.get());
	setModifiers(modifiers);
}

LogsFactoryImpl::~LogsFactoryImpl()
{
    std::wclog.rdbuf(clog_orig_stdbuf.get());
}

void LogsFactoryImpl::setModifiers(std::list<LogModifierFn> const& m)
{
	modifiers = m;
#if 0
	modifiers.push_back(logmod_date);
	modifiers.push_back(logmod_time);
#endif
	modifiers.push_back(logmanip::getLogModifierIndentation()); // indentation modifer is always active
	modifiers.push_back(logmanip::getLogModifierThreadname());
	if (clog_orig_writer)
		clog_orig_writer->setModifiers(modifiers);
	if (clog_file_writer)
		clog_file_writer->setModifiers(modifiers);
	if (tlog_writer)
		tlog_writer->setModifiers(modifiers);
}

inline bool fileExists(std::wstring const& fname)
{
	return nullptr != std::wfilebuf().open(fname.c_str(), std::ios::in);
}

void LogsFactoryImpl::setClogOutput(std::wstring const fnamebase)
{
    if(fnamebase.empty()) {
		std::wclog.rdbuf(clog_orig_logbuf.get());
		clog_file_writer.reset();
		clog_file_logbuf.reset();
		return;
    }
	prev_clog_file_writer = clog_file_writer; // ensure lifetime until clog.rdbuf
	prev_clog_file_logbuf = clog_file_logbuf; // ensure lifetime until clog.rdbuf

	//ako vec postoji bekapuj (koristi time of day za filename sufiks)
	std::wstring fname = fnamebase + L"-" + getDateTimeFilenameSuffix();
	while (fileExists(fname)) {
		fname.push_back('0');
	}
	std::shared_ptr<std::wfilebuf> fb(new std::wfilebuf);
	fb->open(fname.c_str(), std::ios::out | std::ios::trunc);
    if(!fb->is_open()) {
        std::wcerr << "Can't open " << fname << " for clog backend" << std::endl;
        return;
    }
	std::function<void(void)> rotate_fn = bind(&LogsFactoryImpl::setClogOutput, this, fnamebase);
	clog_file_writer.reset(new QueueWriter(fb, rotate_fn, clog_file_bymax));
	clog_file_logbuf = std::make_shared<LoggerBuf>(clog_file_writer);
    if(!clog_file_writer || !clog_file_logbuf) {
        std::wcerr << "Can't create clog backend" << std::endl;
        return;
    }
	clog_file_writer->setModifiers(modifiers);
    std::wclog.rdbuf(clog_file_logbuf.get()); // redirect clog to file through queued buffer
}

void LogsFactoryImpl::setTlogOutputImpl(std::wstring const& filename)
{
	if(filename.empty()) {
        tlog_stdbuf.reset();//everything goes to std::wclog from this thread
		// ako je u std::wclog svakako se vec koristi clog_orig_buf
        return;
    }
    std::shared_ptr<std::wfilebuf> fb(new std::wfilebuf);
    assert(fb.get());
	fb->open(filename.c_str(), std::ios::out | std::ios::trunc);
    if(!fb->is_open()) {
        std::wcerr << "Can't open " << filename << " for thread log" << std::endl;
        return;
    }
    //thread stream with initialized backend of type boostsbuf_t
	tlog_stdbuf.reset(new StdBufPtr(fb));
    if(!tlog_stdbuf.get()) {
        std::wcerr << "Can't create thread log output stream" << std::endl;
        return;
    }
}

StdBufPtr LogsFactoryImpl::getTlogStreambuf(void)
{
	if (!tlogfile_name_prefix.empty())
	{
		if (!tlog_stdbuf.get()) {
			std::wostringstream oss;
			oss << this_thread::get_id();
			setTlogOutputImpl(tlogfile_name_prefix + oss.str());
		}
		if (tlog_stdbuf.get())
			return *tlog_stdbuf.get();
		// fallback to std::wclog
	}
	else {
		if (tlog_stdbuf.get())
			setTlogOutputImpl(std::wstring());
	}
	return StdBufPtr();
}

LogsFactoryBase::LogsFactoryBase(void)
	: _impl{ new LogsFactoryImpl }
{ 
	std::wclog << "**** Created new LogsFactoryBase instance ****" << std::endl;
}

LogsFactoryBase::~LogsFactoryBase(void)
{
	std::wclog << "**** Uninitializing LogsFactoryBase ****" << std::endl;
}

void LogsFactoryBase::setClogRotationSize(size_t bymax)
{
	_impl->setClogRotationSize(bymax);
}

/** Postavlja zadani fajl kao izlaz. \todo za filename.empty treba se koristiti terminal kao
izlaz ali indirektno preko clog_orig_buf */
void LogsFactoryBase::setClogOutput(std::wstring const& filename) 
{ 
	_impl->setClogOutput(filename); 
}

/** Postavlja zadani fajl kao izlaz. Za filename.empty se ponistava i sav ispis ide u std::wclog. */
void LogsFactoryBase::setTlogOutputPrefix(std::wstring const& filename_prefix) 
{ 
	_impl->setTlogOutputPrefix(filename_prefix); 
}

std::wostream& LogsFactoryBase::getTlogOutput(void) 
{ 
	return _impl->getTlogOutput(); 
}

void LogsFactoryBase::setModifiers(std::list<LogModifierFn> modifiers) 
{ 
	return _impl->setModifiers(modifiers); 
}

tlog_tag  tlog;

std::string srcpos_full(unsigned int line, std::string const& function, std::string const& file)
{
	std::ostringstream oss;
	oss << "{" << function << "} in [" + file + "](" << std::dec << line << ")";
	return oss.str();
}

std::string srcpos_short(unsigned int line, std::string const& function, std::string const& file)
{
#ifdef _WIN32
# define PATH_SLASH_CHARACTER '\\'
#else
# define PATH_SLASH_CHARACTER '/'
#endif
	return srcpos_full(line, function, file.substr(file.rfind(PATH_SLASH_CHARACTER) + 1));
}

void EnableLogMessages(bool bEnable)
{
	std::wclog.setstate(bEnable ? std::ios_base::goodbit : std::ios_base::failbit);
}

}
