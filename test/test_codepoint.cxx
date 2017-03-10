#include "bmu/codepoint_transform.hxx"
#include <iostream>
#include <Windows.h>
#include <string>
#include <codecvt>
#if 0
#include <clocale>
#include <io.h>
#include <fcntl.h>
#endif
#include <chrono>

#ifdef _MSC_VER
#pragma execution_character_set("utf-8")
#endif

using namespace bmu;

inline std::string stdUTF16ToUTF8(std::wstring const& source)
{
	typedef std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> utf8_utf16;
	return utf8_utf16().to_bytes(source);
}

inline std::wstring stdUTF8ToUTF16(std::string const& source)
{
	typedef std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> utf8_utf16;
	return utf8_utf16().from_bytes(source);
}

inline std::wstring stdUTF8ToUTF16(char const* source)
{
	typedef std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> utf8_utf16;
	return utf8_utf16().from_bytes(source);
}

inline std::basic_string<unsigned __int32> stdUTF8ToCodepoint(std::string const& source)
{
	typedef std::wstring_convert<std::codecvt_utf8<unsigned __int32>, unsigned __int32> utf8;
	return utf8().from_bytes(source);
}

// unsigned __int32 instead of char32_t as workaround for VS 2015 bug with missing codecvt_utf8::id
inline std::basic_string<unsigned __int32> stdUTF8ToCodepoint(char const* source)
{
	typedef std::wstring_convert<std::codecvt_utf8<unsigned __int32>, unsigned __int32> utf8;
	return utf8().from_bytes(source);
}

inline std::string stdCodepointToUTF8(std::basic_string<unsigned __int32> const& source)
{
	typedef std::wstring_convert<std::codecvt_utf8<unsigned __int32>, unsigned __int32> utf8;
	return utf8().to_bytes(source);
}

inline std::string stdCodepointToUTF8(unsigned __int32 const* source)
{
	typedef std::wstring_convert<std::codecvt_utf8<unsigned __int32>, unsigned __int32> utf8;
	return utf8().to_bytes(source);
}

bool isLittleEndian(void)
{
	static unsigned short const s_Val = 0x0102;
	static bool const s_bLittleEndian = (0x02 == reinterpret_cast<char const*>(&s_Val)[0]);
	return s_bLittleEndian;
}

// typedef std::wstring_convert<std::codecvt_utf16<unsigned __int32, 0x10ffff, std::little_endian>, unsigned __int32> utf16;
inline std::basic_string<unsigned __int32> stdUTF16ToCodepoint(std::wstring const& source)
{
	return stdUTF8ToCodepoint(stdUTF16ToUTF8(source));
}

inline std::basic_string<unsigned __int32> stdUTF16ToCodepoint(wchar_t const* source)
{
	return stdUTF8ToCodepoint(stdUTF16ToUTF8(source));
}

class ElaspedTimeDumper {
public:
	ElaspedTimeDumper(void)
		: now1(std::chrono::system_clock::now())
	{ }
	void operator()(std::string const& last_operation)
	{
		using namespace std::chrono;
		auto now2 = system_clock::now();
		auto diff = duration_cast<milliseconds>(now2 - now1).count() / 1000.0;
		std::cout << "Finished " << last_operation << " in " << diff << " seconds" << std::endl;
		now1 = now2;
	}
private:
	std::chrono::time_point<std::chrono::system_clock> now1;
};

int main()
{
	if (0)
	{
		u8unit_t szMsg[] = u8"Хеллоу тхере!";
		//auto asWideStd = stdUTF8ToUTF16((char*)szMsg);
		auto asCodepointStd = stdUTF8ToCodepoint((char*)szMsg);
		std::basic_string<u32char_t>  asCodepointMy(_countof(szMsg), 0);
		size_t asCodepointMySize = trMbyteToUni(szMsg, szMsg + _countof(szMsg), &asCodepointMy[0], &asCodepointMy[0] + asCodepointMy.size());
		assert(-1 != asCodepointMySize);
		asCodepointMy.resize(asCodepointMySize);
		assert(asCodepointStd == asCodepointMy);
	}
	if (0) 
	{
		wchar_t wcMsg[] = L"Хеллоу тхере!";
		auto asCodepointStd = stdUTF16ToCodepoint(wcMsg);
		std::string asU8My(4 * asCodepointStd.size(), '\0');
		size_t asU8MySize = trUniToMbyte(&asCodepointStd[0], &asCodepointStd[0] + asCodepointStd.size(), (u8unit_t*)&asU8My[0], (u8unit_t*)(&asU8My[0] + asU8My.size()));
		assert(-1 != asU8MySize);
		asU8My.resize(asU8MySize);
		auto asU8Std = stdUTF16ToUTF8(wcMsg);
		assert(asU8My == asU8Std);
	}
	if(1)
	{
		ElaspedTimeDumper dumpelapsed;
		size_t const CP_MAX(0x0010ffffu);///< Maximum valid value for a Unicode code point
		std::basic_string<unsigned __int32> allcodepoints(CP_MAX, 0);
		for (char32_t i = 0; i < CP_MAX; ++i) {
			if(0xD800u <= i + 1 && i + 1 <= 0xDfffu)
				allcodepoints[i] = 1;
			else
				allcodepoints[i] = i + 1;
		}
		dumpelapsed("AllCodepoints");
		auto asU8Std = stdCodepointToUTF8(allcodepoints);
		dumpelapsed("stdCodepointToUTF8");
		if (1)
		{
			std::string asU8(4 * allcodepoints.size(), '\0');
			size_t asU8Size = trUniToMbyte(&allcodepoints[0], &allcodepoints[0] + allcodepoints.size(), (u8unit_t*)&asU8[0], (u8unit_t*)(&asU8[0] + asU8.size()));
			dumpelapsed("trUniToMbyte");
			assert(-1 != asU8Size);
			asU8.resize(asU8Size);
			assert(asU8Std.size() == asU8.size());
			for (char32_t i = 0; i < asU8.size(); ++i) {
				//char const* p = &asU8Std[i];
				//char const* p2 = &asU8[i];
				assert(asU8Std[i] == asU8[i]);
			}
			dumpelapsed("trUniToMbyte comparison");
		}
		auto asU16Std = stdUTF8ToUTF16(asU8Std);
		dumpelapsed("stdUTF8ToUTF16");
		if (1)
		{
			std::wstring asU16(asU8Std.size() * 2, L'\0');
			size_t asU16Size = trMbyteToWide((u8unit_t*)&asU8Std[0], (u8unit_t*)(&asU8Std[0] + asU8Std.size()), (u16unit_t*)&asU16[0], (u16unit_t*)(&asU16[0] + asU16.size()));
			dumpelapsed("trMbyteToWide");
			assert(-1 != asU16Size);
			asU16.resize(asU16Size);
			assert(asU16Std.size() == asU16.size());
			for (char32_t i = 0; i < asU16.size(); ++i) {
				//wchar_t const* pCP = &asU16Std[i];
				//wchar_t const* pAll = &asU16[i];
				assert(asU16Std[i] == asU16[i]);
			}
			dumpelapsed("trMbyteToWide comparison");
		}
		if(1)
		{
			auto asU8From16Std = stdUTF16ToUTF8(asU16Std);
			dumpelapsed("stdUTF16ToUTF8");
			std::string asU8From16(4 * asU16Std.size(), L'\0');
			size_t asU8From16Size = trWideToMbyte((u16unit_t*)&asU16Std[0], (u16unit_t*)(&asU16Std[0] + asU16Std.size()), (u8unit_t*)&asU8From16[0], (u8unit_t*)(&asU8From16[0] + asU8From16.size()));
			dumpelapsed("trWideToMbyte");
			assert(-1 != asU8From16Size);
			asU8From16.resize(asU8From16Size);
			assert(asU8From16Std.size() == asU8From16.size());
			auto* const itstdbeg = &asU8From16Std[0];
			auto* const itstdend = itstdbeg + asU8From16Std.size();
			auto* itstd = itstdbeg;
			for (size_t i = 0; i < asU8From16.size(); ) {
				std::string::difference_type nOctets = 0;
				char32_t cp = getUTF8Codepoint(itstd, itstdend, nOctets);
				assert(cp <= CP_MAX);
				for (size_t ibase = i; i < ibase + nOctets; ++i, ++itstd) {
					//char const* pCP = &asU8From16Std[i];
					//char const* pAll = &asU8From16[i];
					assert(asU8From16Std[i] == asU8From16[i]);
				}
			}
			dumpelapsed("trWideToMbyte comparison");
		}

		std::basic_string<u32char_t>  asCP(asU8Std.size(), 0);
		size_t asCPSize = trMbyteToUni((u8unit_t*)&asU8Std[0], (u8unit_t*)(&asU8Std[0] + asU8Std.size()), &asCP[0], &asCP[0] + asCP.size());
		dumpelapsed("trMbyteToUni");
		assert(-1 != asCPSize);
		asCP.resize(asCPSize);
		if (1)
		{
			for (char32_t i = 0; i < CP_MAX; ++i) {
				//unsigned __int32 const* pCP = &asCP[i];
				//unsigned __int32 const* pAll = &allcodepoints[i];
				assert(asCP[i] == allcodepoints[i]);
			}
			dumpelapsed("trMbyteToUni comparison");
		}
	}

#ifdef _MSC_VER
	DWORD dwErr = 0;
	if (FALSE == SetConsoleOutputCP(CP_UTF8)) // important on windows
		dwErr = GetLastError();
#endif

#if 0
	_setmode(_fileno(stdout), _O_U16TEXT);
	wprintf(L"%c (wprintf euro sign)\n", L'\u20AC');
	std::wcout << L"\u20AC Хеллоу тхере! (euro sign + cyrilic chars)" << std::endl;
#endif

	std::locale locEnUTF8(std::locale(), ::new std::codecvt_utf8<wchar_t>);
	std::ios_base::sync_with_stdio(false);
	std::wcout.imbue(locEnUTF8);
	wchar_t wcMsg[] = L"Хеллоу тхере!";//NOTE: source code file should be encoded as UTF-8 with BOM

	std::wcout << wcMsg << "\n"
		<< L"Хеллоу тхере!\n" 
		<< L"euro sign \u20AC" << std::endl;
    return 0;
}
