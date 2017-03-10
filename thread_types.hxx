#pragma once
#include <thread>
#include <map>

namespace beam_me_up {}
namespace bmu = beam_me_up;

namespace beam_me_up {
using std::bind;
typedef std::thread thread_type;
namespace this_thread = std::this_thread;
//thread_specific_ptr can be used as non-static thread specific member.
//Behind its functionality is static thread specific map of all instances.
template<typename _T>
class thread_specific_ptr {
	thread_specific_ptr(thread_specific_ptr const&) = delete;
	void operator = (thread_specific_ptr const&) = delete;
public:
	typedef std::function<void(_T*)> deleter_function;
	thread_specific_ptr(deleter_function delfn = std::default_delete<_T>())
		: delfn(delfn ? delfn : std::default_delete<_T>())
	{ }
	~thread_specific_ptr()
	{ }
	_T* get(void)
	{
		std::shared_ptr<_T>& ptr = ptrs_thread[this];
		return ptr.get();
	}
	void reset(_T* const p = nullptr)
	{
		std::shared_ptr<_T>& ptr = ptrs_thread[this];
		if (ptr.get() != p)
			ptr = std::shared_ptr<_T>(p, delfn);
	}
private:
	deleter_function                 delfn;
	thread_local static std::map<thread_specific_ptr*, std::shared_ptr<_T>> ptrs_thread;
};

template<typename _T>
thread_local std::map<thread_specific_ptr<_T>*, std::shared_ptr<_T>> thread_specific_ptr<_T>::ptrs_thread;

template<typename _T>
using tss_ptr = thread_specific_ptr<_T>;

class SharedThreadStr {
public:
	SharedThreadStr(void)
		: str(std::make_shared<tss_ptr<std::wstring>>())
	{
		assert(str);
	}
	std::wstring& getStr(void)
	{
		if (!str->get())
			str->reset(new std::wstring);
		assert(str->get());
		return *str->get();
	}
private:
	std::shared_ptr<tss_ptr<std::wstring>> str;
};
}
