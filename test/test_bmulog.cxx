#include "bmu/Logger.h"

int main(int argc, char* argv[])
{
	printf("%s", "Hello\n");
	{
		bmu::LogsFactoryPtr logger_scope(bmu::LogsFactory::create());
		logger_scope->setModifiers({ bmu::logmod_date, bmu::logmod_time });
		LOGMSG("test");
		std::wclog << "test2" << std::endl;
		bmu::tlog << "Thread log je ovo" << std::endl;
		
		bmu::logmanip::setThreadName(L"##### Ime 1 threada #####");
		std::wclog << bmu::logmanip::indent << "Msg1" << std::endl;
		std::wclog << "Msg2" << std::endl;
		bmu::logmanip::setThreadName(L"##### Ime 2 threada #####");
		std::wclog << "Msg3" << std::endl;
		std::wclog << "Very long long long line\n with new line character" << std::endl;
		bmu::logmanip::setThreadName(L"##### Ime threada promijenjeno #####");
		std::wclog << "Msg " << " about " << " something " << std::endl;

		LVLCLOG(::bmu::LINFO, "Ovo je info" << " ili tako nesto");
		LVLTLOG(::bmu::LINFO, "Ovo je info" << " ili tako nesto");
		{
			std::thread th2([] {
				bmu::logmanip::setThreadName(L"##### Ime th2 #####");
				std::wclog << "Th2 " << " Th2 " << " Th2 " << std::endl;
				}
			);
			th2.join();
		}
		LVLCLOG(::bmu::LWARN, "Ovo je upozorenje" << " ili tako nesto");
		LVLTLOG(::bmu::LWARN, "Ovo je upozorenje" << " ili tako nesto");
		std::wclog << bmu::logmanip::indent << "After indent 1 " << "and line continuation." << std::endl;
		std::wclog << "After indent 2 " << "and line continuation." << std::endl;
		std::wclog << bmu::logmanip::unindent << "After unindent 1 " << "and line continuation." << std::endl;
		std::wclog << "After unindent 2 " << "and line continuation." << std::endl;
		std::wclog << bmu::logmanip::unindent << "After unindent 1 " << "and line continuation." << std::endl;
		std::wclog << "After unindent 2 " << "and line continuation." << std::endl;
		{
			bmu::logmanip::setThreadName(std::wstring());
			bmu::LogScopePtr lsroot(bmu::LogScope::create(nullptr));
				bmu::LogScopePtr lsbranch1(bmu::LogScope::create(lsroot));
					bmu::LogScopePtr lsleaf11(bmu::LogScope::create(lsbranch1));
					bmu::LogScopePtr lsleaf12(bmu::LogScope::create(lsbranch1));
				bmu::LogScopePtr lsbranch2(bmu::LogScope::create(lsroot));
				bmu::LogScopePtr lsleaf21(bmu::LogScope::create(lsbranch2));
				bmu::LogScopePtr lsleaf22(bmu::LogScope::create(lsbranch2));

			std::wclog << lsroot << " LS Root" << " 1 " << std::endl;
			std::wclog << lsroot << " LS Root" << " 2 " << std::endl;
			std::list<bmu::loglevel_e> levels{ 
				bmu::LERROR, 
				bmu::LWARN, 
				bmu::LINFO, 
				bmu::LTRACE, 
				bmu::LDUMP, 
				bmu::LERROR // once more to do all in loop but first time uninitialized
			};
			int curlev = -1;
			std::wclog << lsroot << std::endl;
			for (bmu::loglevel_e lvl : levels) {
				std::wclog << " Current loglevel " << curlev << std::endl;
				if(bmu::logmanip::isEnabled(bmu::LERROR))
					std::wclog << " LS Root" << " ERROR " << std::endl;
				if (bmu::logmanip::isEnabled(bmu::LWARN))
					std::wclog << " LS Root" << " WARNING " << std::endl;
				if (bmu::logmanip::isEnabled(bmu::LINFO))
					std::wclog << " LS Root" << " INFO " << std::endl;
				if (bmu::logmanip::isEnabled(bmu::LTRACE))
					std::wclog << " LS Root" << " TRACE " << std::endl;
				if (bmu::logmanip::isEnabled(bmu::LDUMP))
					std::wclog << " LS Root" << " DUMP " << std::endl;
				lsroot->setLoglevel(lvl);
				curlev = lvl;
			}
		}
		wchar_t wcMsg[] = L"\u0425\u0435\u043B\u043B\u043E\u0443 \u0442\u0445\u0435\u0440\u0435!";//NOTE: source code file should be encoded as UTF-8 with BOM
		std::wclog << "UTF-16 string: " << wcMsg << std::endl;
	}

	printf("%s", "Bye\n");
	std::cin.get();
	return 0;
}
