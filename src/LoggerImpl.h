#pragma once
#include "bmu/Logger.h"
#include "bmu/thread_types.hxx"

namespace beam_me_up {

typedef std::shared_ptr<std::wostream> OstreamPtr;

/// Pisanje u stringa u izlazni bafer uzimajući u obzir pridružene modifikatori od kojih se dobijaju
/// vrijednosti za ispisivanje prije svakog reda.
class BufferWriterWithModifers {
protected:
	BufferWriterWithModifers(void)
	{ }
public:
	virtual ~BufferWriterWithModifers()
	{ }
	/// Dodavanje modifikatora koji se uzimaju u obzir pri pisanju u bafer
	void setModifiers(std::list<LogModifierFn> newmodifers);
	virtual std::streamsize write(wchar_t const* s, std::streamsize n) = 0;
protected:
	/// Ispisivanje jednog reda u bafer iz vrijednosti dobijenih od pridruženih modifikatora.
	void do_write_modifiers(StdBufPtr sbuf);
	std::streamsize do_write_string(StdBufPtr sbuf, wchar_t const* s, std::streamsize n);
private:
	std::list<LogModifierFn>    modifiers;
};

typedef std::shared_ptr<BufferWriterWithModifers> BufferWriterWithModifersPtr;

/// no synchronization as appropriate for per-thread ostream /see GetTlogOutput
class DirectWriter : public BufferWriterWithModifers {
public:
	DirectWriter(StdBufPtr sbuf)
		: sbuf(sbuf)
	{ }
	std::streamsize write(wchar_t const* s, std::streamsize n);
private:
	StdBufPtr sbuf;
};

typedef std::shared_ptr<DirectWriter> DirectWriterPtr;

typedef std::function<StdBufPtr(void)> TargetStreambufGetterFn;

/// no synchronization as appropriate for per-thread ostream /see GetTlogOutput
class TargetDirectWriter : public BufferWriterWithModifers {
	explicit TargetDirectWriter(void) = delete;
public:
	TargetDirectWriter(BufferWriterWithModifersPtr fallback, TargetStreambufGetterFn getsbuf)
		: fallback(fallback)
		, getsbuf(getsbuf)
	{ }
	std::streamsize write(wchar_t const* s, std::streamsize n);
private:
	BufferWriterWithModifersPtr fallback;
	TargetStreambufGetterFn     getsbuf;
};

typedef std::shared_ptr<TargetDirectWriter> TargetDirectWriterPtr;

typedef std::function<void(void)> RotateFileFn;

//Ako je jedan ostream zajednicki za sve threadove onda treba queue i worker thread za ispisivanje
class QueueWriter : public BufferWriterWithModifers {
	QueueWriter(QueueWriter const&) = delete;
	void operator = (QueueWriter const&) = delete;
	typedef std::list<std::wstring> MsgQueue;
	typedef std::shared_ptr<MsgQueue> MsgQueuePtr;
public:
	QueueWriter(StdBufPtr sbuf, RotateFileFn rotate, std::shared_ptr<size_t> bymax);
	~QueueWriter(void);
	void WriteAllLogsBeforeFinish(bool all = true) 
	{ 
		allwrite = all; 
	}
	std::streamsize write(wchar_t const* s, std::streamsize n);
private:
	void BackendWorker(void);
	StdBufPtr                     sbuf;
	RotateFileFn                  rotate;
	std::shared_ptr<size_t> const bymax;
	size_t                        bycount;
	bool                          finish;
	bool                          allwrite;
	std::mutex                    mutex;
	MsgQueuePtr                   logs;
	thread_type                   worker;
};

typedef std::shared_ptr<QueueWriter> QueueWriterPtr;

/// no synchronization as appropriate for per-thread ostream /see GetTlogOutput
class NoModifiersWriter : public BufferWriterWithModifers {
public:
	NoModifiersWriter(StdBufPtr sbuf)
		: sbuf(sbuf)
	{ }
	std::streamsize write(wchar_t const* s, std::streamsize n);
protected:
	StdBufPtr sbuf;
};

typedef std::shared_ptr<NoModifiersWriter> NoModifiersWriterPtr;

class LoggerBuf : public std::wstreambuf {
	LoggerBuf(void) = delete;
public:
	LoggerBuf(BufferWriterWithModifersPtr bufwriter, size_t bufsize = 4096);
	~LoggerBuf()
	{
		int dummy = 0;
	}
	int overflow(wchar_t c);
	int sync(void);
	std::wstreambuf* setbuf(wchar_t*, std::streamsize)
	{
		return this;
	}
private:
	BufferWriterWithModifersPtr bufwriter;
	size_t                      bufsize;
	std::shared_ptr<wchar_t>    bufptr;
	wchar_t* const              pbeg;
	wchar_t* const              pend;
};

typedef std::shared_ptr<LoggerBuf> LoggerBufPtr;

/// Pridružuje baferom sa modifikatorima std::wclog streamu sa pri čemu izlaz može biti fajl umjesto terminal
class LogsFactoryImpl {
public:
	LogsFactoryImpl(void);
	~LogsFactoryImpl();
	void setModifiers(std::list<LogModifierFn> const& m);
	void setClogRotationSize(size_t bymax)
	{
		*clog_file_bymax = bymax;
	}
	void setClogOutput(std::wstring const filename);
	std::wostream& getTlogOutput(void) 
	{ 
		assert(tlog_ostream); 
		return *tlog_ostream; 
	}
	/// Novi bafer se pravi za poziv iz svake nove niti i ne vrši se sinhronizacija upisa.
	/// \todo treba pratiti koji su fajlovi vec pridruzeni da ne bi vise niti pisalo u isti fajl.
	void setTlogOutputImpl(std::wstring const& filename);
	void setTlogOutputPrefix(std::wstring const& filename_prefix)
	{ 
		tlogfile_name_prefix  = filename_prefix;
	}
private:
	static void nodeleter(std::wstreambuf* /*p*/) { }
	StdBufPtr getTlogStreambuf(void);
	std::locale              locEnUTF8;
	StdBufPtr const          clog_orig_stdbuf;//backup, reverted in destructor
	QueueWriterPtr           clog_orig_writer; // referenca za update modifikatora
	LoggerBufPtr             clog_orig_logbuf;
	std::shared_ptr<size_t>  clog_file_bymax;
	QueueWriterPtr           clog_file_writer; // referenca za update modifikatora
	LoggerBufPtr             clog_file_logbuf; // mijenja se pri zamjeni fajla
	QueueWriterPtr           prev_clog_file_writer; // referenca za update modifikatora
	LoggerBufPtr             prev_clog_file_logbuf; // mijenja se pri zamjeni fajla
	std::wstring              tlogfile_name_prefix;
	TargetDirectWriterPtr    tlog_writer; // referenca za update modifikatora
	LoggerBufPtr             tlog_logbuf;
	OstreamPtr               tlog_ostream;
	tss_ptr<StdBufPtr>       tlog_stdbuf; // per-thread filebuf
	std::list<LogModifierFn> modifiers;
};

}
