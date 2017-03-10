#pragma once
#include <memory>
#include <mutex>
#include <cassert>

namespace beam_me_up {

template<typename _T>
class single_shared : public _T, public std::enable_shared_from_this<single_shared<_T>> {
private:
	template<typename... _Args>
	single_shared(_Args&&... args);
	single_shared(single_shared const&) = delete;
	void operator=(single_shared&) = delete;
public:
	~single_shared();
	/// Gets existing instance or creates new if not yet created. Resulting pointer SHOULD be saved in some
	/// local variable or class member to hold reference and ensure lifetime in variable scope
	template<typename... _Args>
	static std::shared_ptr<single_shared> create(_Args&&... args);
	/// Gets existing instance or creates new if not yet created. Resulting pointer CAN be saved in some
	/// local variable or class member for later use because it's reference counted so it ensures instance
	/// lifetime for whole scope of saved variable. Saving pointer in variable is encouraged because this
	/// way multiple calls to instance() with internal mutex locking are avoided making execution faster.
	static std::shared_ptr<single_shared> instance(void) throw();
private:
	static std::mutex         mutex;
	static single_shared<_T>* one;
};

template<typename _T, typename... _Args>
inline std::shared_ptr<single_shared<_T>> make_single_shared(_Args&&... args)
{
	return single_shared<_T>::create(std::forward<_Args>(args)...);
}

template<typename _T>
std::mutex
single_shared<_T>::mutex;

template<typename _T>
single_shared<_T>*
single_shared<_T>::one(nullptr);

template<typename _T>
template<typename... _Args>
single_shared<_T>::single_shared(_Args&&... args)
	: _T(std::forward<_Args>(args)...)
{
	assert(!single_shared::one);
}

template<typename _T>
single_shared<_T>::~single_shared(void)
{
	std::lock_guard<std::mutex> lock(mutex);
	one = nullptr; // reset one because this was last reference
}

template<typename _T>
template<typename... _Args>
std::shared_ptr<single_shared<_T>>
single_shared<_T>::create(_Args&&... args)
{
	std::lock_guard<std::mutex> lock(mutex);
	if (!one) {
		one = new single_shared<_T>(std::forward<_Args>(args)...);
		assert(one);
		return std::shared_ptr<single_shared>(one)->shared_from_this(); // start reference counting then get second reference
	}
	else {
		assert(one);
		return one->shared_from_this(); // get next reference
	}
}

template<typename _T>
std::shared_ptr<single_shared<_T>>
single_shared<_T>::instance(void) throw()
{
	std::lock_guard<std::mutex> lock(mutex);
	assert(one);
	if(one)
		return one->shared_from_this(); // get next reference
	return std::shared_ptr<single_shared>();
}

}
