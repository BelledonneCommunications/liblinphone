﻿#include <string>

#include "liblinphone_tester_windows.h"

using namespace liblinphone_tester_runtime_component;
using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Storage;
using namespace Windows::System::Threading;

#define MAX_TRACE_SIZE		2048
#define MAX_SUITE_NAME_SIZE	128
#define MAX_WRITABLE_DIR_SIZE 1024

static OutputTraceListener^ sTraceListener;

LibLinphoneTester^ LibLinphoneTester::_instance = ref new LibLinphoneTester();

static void nativeOutputTraceHandler(int lev, const char *fmt, va_list args)
{
	if (sTraceListener) {
		wchar_t wstr[MAX_TRACE_SIZE] = { 0 };
		std::string str;
		str.resize(MAX_TRACE_SIZE);
		vsnprintf((char *)str.c_str(), MAX_TRACE_SIZE, fmt, args);
		mbstowcs(wstr, str.c_str(), MAX_TRACE_SIZE - 1);
		String^ msg = ref new String(wstr);
		String^ l;
		switch (lev) {
		case ORTP_FATAL:
		case ORTP_ERROR:
			l = ref new String(L"Error");
			break;
		case ORTP_WARNING:
			l = ref new String(L"Warning");
			break;
		case ORTP_MESSAGE:
			l = ref new String(L"Message");
			break;
		default:
			l = ref new String(L"Debug");
			break;
		}
		sTraceListener->outputTrace(l, msg);
	}
}

static void libLinphoneNativeOutputTraceHandler(OrtpLogLevel lev, const char *fmt, va_list args)
{
	nativeOutputTraceHandler((int)lev, fmt, args);
}


LibLinphoneTester::LibLinphoneTester()
{
	liblinphone_tester_init(nativeOutputTraceHandler);
	bc_tester_set_resource_dir_prefix("Assets");
}

LibLinphoneTester::~LibLinphoneTester()
{
	liblinphone_tester_uninit();
}

void LibLinphoneTester::setWritableDirectory(StorageFolder^ folder)
{
	char writable_dir[MAX_WRITABLE_DIR_SIZE] = { 0 };
	const wchar_t *wwritable_dir = folder->Path->Data();
	wcstombs(writable_dir, wwritable_dir, sizeof(writable_dir));
	bc_tester_set_writable_dir_prefix(writable_dir);
}

void LibLinphoneTester::setOutputTraceListener(OutputTraceListener^ traceListener)
{
	sTraceListener = traceListener;
}

void LibLinphoneTester::init(bool verbose)
{
	if (verbose) {
		linphone_core_set_log_level_mask((OrtpLogLevel)(ORTP_MESSAGE | ORTP_WARNING | ORTP_ERROR | ORTP_FATAL));
	}
	else {
		linphone_core_set_log_level_mask(ORTP_FATAL);
	}
}

bool LibLinphoneTester::run(Platform::String^ suiteName, Platform::String^ caseName, Platform::Boolean verbose)
{
	std::wstring all(L"ALL");
	std::wstring wssuitename = suiteName->Data();
	std::wstring wscasename = caseName->Data();
	char csuitename[MAX_SUITE_NAME_SIZE] = { 0 };
	char ccasename[MAX_SUITE_NAME_SIZE] = { 0 };
	wcstombs(csuitename, wssuitename.c_str(), sizeof(csuitename));
	wcstombs(ccasename, wscasename.c_str(), sizeof(ccasename));

	init(verbose);
	linphone_core_set_log_handler(libLinphoneNativeOutputTraceHandler);
	return bc_tester_run_tests(wssuitename == all ? 0 : csuitename, wscasename == all ? 0 : ccasename) != 0;
}

void LibLinphoneTester::runAllToXml()
{
	auto workItem = ref new WorkItemHandler([this](IAsyncAction ^workItem) {
		char *xmlFile = bc_tester_file("LibLinphoneWindows10.xml");
		char *logFile = bc_tester_file("LibLinphoneWindows10.log");
		char *args[] = { "--xml-file", xmlFile };
		bc_tester_parse_args(2, args, 0);
		init(true);
		FILE *f = fopen(logFile, "w");
		ortp_set_log_file(f);
		bc_tester_start();
		bc_tester_uninit();
		fclose(f);
		free(xmlFile);
		free(logFile);
	});
	_asyncAction = ThreadPool::RunAsync(workItem);
}

unsigned int LibLinphoneTester::nbTestSuites()
{
	return bc_tester_nb_suites();
}

unsigned int LibLinphoneTester::nbTests(Platform::String^ suiteName)
{
	std::wstring suitename = suiteName->Data();
	char cname[MAX_SUITE_NAME_SIZE] = { 0 };
	wcstombs(cname, suitename.c_str(), sizeof(cname));
	return bc_tester_nb_tests(cname);
}

Platform::String^ LibLinphoneTester::testSuiteName(int index)
{
	const char *cname = bc_tester_suite_name(index);
	wchar_t wcname[MAX_SUITE_NAME_SIZE];
	mbstowcs(wcname, cname, sizeof(wcname));
	return ref new String(wcname);
}

Platform::String^ LibLinphoneTester::testName(Platform::String^ suiteName, int testIndex)
{
	std::wstring suitename = suiteName->Data();
	char csuitename[MAX_SUITE_NAME_SIZE] = { 0 };
	wcstombs(csuitename, suitename.c_str(), sizeof(csuitename));
	const char *cname = bc_tester_test_name(csuitename, testIndex);
	wchar_t wcname[MAX_SUITE_NAME_SIZE];
	mbstowcs(wcname, cname, sizeof(wcname));
	return ref new String(wcname);
}
