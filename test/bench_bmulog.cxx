#include "bmu/Logger.h"
#include <experimental/filesystem>

int main(int argc, char* argv[])
{
	std::experimental::filesystem::create_directories("logs");
	bmu::LogsFactoryPtr logger_scope(bmu::LogsFactory::create());
	logger_scope->setClogOutput("logs/bmulog-bench");
	// log to a file with rotation at size 5MB
	logger_scope->setClogRotationSize(5 * 1024 * 1024);
	logger_scope->setModifiers({ bmu::logmod_time, bmu::logmod_threadid });
	int msgcount = 1000000;
	std::cout << "Started bmulog bench" << std::endl;
	
	auto now1 = std::chrono::system_clock::now();
	for (int i = 0; i < msgcount; ++i)
		std::clog << FILE_AT << " bmulog message repetition " << i << ": This is some random log message repeated many times" << std::endl;
	auto now2 = std::chrono::system_clock::now();

	auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(now2 - now1);
	std::cout << "Finished in " << (diff.count() / 1000.0) << " seconds" << std::endl;
	std::cin.get();
	return 0;
}
