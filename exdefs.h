#pragma once
//#include <boost/exception/error_info.hpp>
#include <boost/exception_ptr.hpp>
#include <string>

#define ERRINFO_HERE \
       boost::errinfo_at_line(__LINE__) \
    << boost::errinfo_file_name(__FILE__) \
    << boost::errinfo_api_function(__FUNCTION__)

namespace beam_me_up{
//exception data
typedef boost::error_info<struct tag_message_info, std::string> errinfo_message;
typedef boost::error_info<struct tag_extern_api_function_info, std::string> errinfo_extern_api_function;
typedef boost::error_info<struct tag_target_filename_info, std::string> errinfo_target_filename;
typedef boost::error_info<struct tag_nested_diagnostic_info, std::string> errinfo_nested_diagnostic;

struct exception_t : virtual std::exception, virtual boost::exception { };

struct assert_error : virtual exception_t { };
struct stdc_error : virtual exception_t { };//c library error
}