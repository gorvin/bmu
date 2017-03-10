#include "bmu/single_shared.hxx"
#include "bmu/thread_types.hxx"
#include <iostream>
#include <string>

#if 0
thread_local unsigned int rage = 1;
#else
bmu::tss_ptr<unsigned int> rage_ptr;
#endif

std::mutex cout_mutex;

void increase_rage(const std::string& thread_name)
{
#if 0
	++rage; // modifying outside a lock is okay; this is a thread-local variable
	std::lock_guard<std::mutex> lock(cout_mutex);
	std::cout << "Rage counter for " << thread_name << ": " << rage << '\n';
#else
	assert(!rage_ptr.get());
	rage_ptr.reset(new unsigned int(2));
	std::lock_guard<std::mutex> lock(cout_mutex);
	std::cout << "Rage counter for " << thread_name << ": " << *rage_ptr.get() << '\n';
#endif
}

class Test {
public:
	Test(int arg)
		: arg(arg)
	{ }
	int getArg(void) const 
	{ 
		return arg; 
	}
	void setArg(int argnew)
	{
		arg = argnew;
	}
private:
	int arg;
};

typedef bmu::single_shared<Test> TestSingle;
typedef std::shared_ptr<TestSingle> TestSinglePtr;

int main(int argc, char* argv[])
{
	std::clog << "Hello" << std::endl;
	{
		assert(!rage_ptr.get());
		rage_ptr.reset(new unsigned int(1));
		std::thread a(increase_rage, "a"), b(increase_rage, "b");
		{
			std::lock_guard<std::mutex> lock(cout_mutex);
#if 0
			std::cout << "Rage counter for main: " << rage << '\n';
#else
			std::cout << "Rage counter for main: " << *rage_ptr.get() << '\n';
#endif
		}
		a.join();
		b.join();
	}
	{
		TestSinglePtr ref_initial = TestSingle::create(1);
		TestSinglePtr ref_2 = bmu::make_single_shared<Test>(1);
		assert(ref_2 == ref_initial);
		assert(1 == ref_2->getArg());
		ref_2->setArg(2);
		assert(2 == ref_2->getArg());
		{
			TestSinglePtr ref_3 = bmu::make_single_shared<Test>(1);
			assert(ref_3 == ref_initial);
			assert(2 == ref_3->getArg());
			ref_3->setArg(3);
			assert(3 == ref_2->getArg());
			assert(3 == ref_3->getArg());
		}
		assert(3 == ref_2->getArg());
	}
	std::clog << "Bye" << std::endl;
	std::cin.get();
	return 0;
}
