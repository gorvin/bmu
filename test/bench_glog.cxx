#include "glog/logging.h"
#include <chrono>
#include <iostream>
#include <experimental/filesystem>

int main(int, char* argv[])
{
	std::experimental::filesystem::create_directories("logs");
	FLAGS_logtostderr = 0;
	FLAGS_log_dir = "logs";
	google::InitGoogleLogging(argv[0]);
	int msgcount = 1000000;
	std::cout << "Started glog bench" << std::endl;

	auto now1 = std::chrono::system_clock::now();
	for (int i = 0; i < msgcount; ++i)
		LOG(INFO) << "glog message repetition " << i << ": This is some random log message repeated many times";
	auto now2 = std::chrono::system_clock::now();

	auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(now2 - now1);
	std::cout << "Finished in " << (diff.count() / 1000.0) << " seconds" << std::endl;
	std::cin.get();
	return 0;
}
