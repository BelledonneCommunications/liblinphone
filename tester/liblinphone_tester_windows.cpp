/*
 * Copyright (c) 2010-2022 Belledonne Communications SARL.
 *
 * This file is part of Liblinphone
 * (see https://gitlab.linphone.org/BC/public/liblinphone).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <string>
#include <vector>

#include "liblinphone_tester_windows.h"
#include "tester_utils.h"

using namespace BelledonneCommunications::Linphone::Tester;
using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Storage;
using namespace Windows::System::Threading;
using namespace Windows::UI::Core;
using namespace Windows::ApplicationModel::Core;

#define MAX_TRACE_SIZE 2048
#define MAX_SUITE_NAME_SIZE 128
#define MAX_WRITABLE_DIR_SIZE 1024

static OutputTraceListener ^ sTraceListener;

NativeTester ^ NativeTester::_instance = ref new NativeTester();

static void nativeOutputTraceHandler(int lev, const char *fmt, va_list args) {
	if (sTraceListener) {
		wchar_t wstr[MAX_TRACE_SIZE] = {0};
		std::string str;
		str.resize(MAX_TRACE_SIZE);
		vsnprintf((char *)str.c_str(), MAX_TRACE_SIZE, fmt, args);
		mbstowcs(wstr, str.c_str(), MAX_TRACE_SIZE - 1);
		String ^ msg = ref new String(wstr);
		String ^ l;
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

static void libLinphoneNativeOutputTraceHandler(const char *domain, OrtpLogLevel lev, const char *fmt, va_list args) {
	nativeOutputTraceHandler((int)lev, fmt, args);
}

static void processEvents() { // Snippet
	                          /*
	                          auto currentThread = Windows::UI::Core::CoreWindow::GetForCurrentThread();
	                          if(currentThread && currentThread->Dispatcher){
	                              currentThread->Dispatcher->ProcessEvents(Windows::UI::Core::CoreProcessEventsOption::ProcessAllIfPresent);
	                          }else{// Not in GUI thread
	                              auto mainView = CoreApplication::MainView;
	                              if( mainView && mainView->Dispatcher){
	                                  auto myDispatchedHandler = ref new DispatchedHandler([&](){
	                                      mainView->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessOneAndAllPending);
	                                  });
	                                  mainView->Dispatcher->RunAsync(CoreDispatcherPriority::Normal,myDispatchedHandler);
	                      
	                              }else{
	                                  if( CoreApplication::Views->Size > 0 && CoreApplication::Views->First() &&
	                          CoreApplication::Views->First()->Current && CoreApplication::Views->First()->Current->Dispatcher){	                       auto firstView =
	                          CoreApplication::Views->First()->Current->Dispatcher;
	                                      firstView->ProcessEvents(Windows::UI::Core::CoreProcessEventsOption::ProcessAllIfPresent);
	                                  }else{// Do nothing
	                                  }
	                              }
	                          }*/
}

NativeTester::NativeTester() {
}

NativeTester::~NativeTester() {
	liblinphone_tester_uninit();
	for (int i = 0; i < m_commandLine.size(); ++i)
		free(_args[i]);
	free(_args);
}

void NativeTester::setOutputTraceListener(OutputTraceListener ^ traceListener) {
	sTraceListener = traceListener;
}

void NativeTester::initialize(const Platform::Array<Platform::String ^> ^ pParameters,
                              StorageFolder ^ writableDirectory,
                              Platform::Boolean ui,
                              Platform::Boolean verbose) {
	Platform::Array<Platform::String ^> ^ parameters = ref new Platform::Array<Platform::String ^>(pParameters);
	if (ui && sTraceListener) {
		liblinphone_tester_init(nativeOutputTraceHandler);
	} else {
		liblinphone_tester_init(NULL);
	}
	if (verbose)
		linphone_core_set_log_level_mask((OrtpLogLevel)(ORTP_MESSAGE | ORTP_WARNING | ORTP_ERROR | ORTP_FATAL));
	else linphone_core_set_log_level_mask((OrtpLogLevel)(ORTP_WARNING | ORTP_ERROR | ORTP_FATAL));
	char writable_dir[MAX_WRITABLE_DIR_SIZE] = {0};
	const wchar_t *wwritable_dir = writableDirectory->Path->Data();
	wcstombs(writable_dir, wwritable_dir, sizeof(writable_dir));
	bc_tester_set_writable_dir_prefix(writable_dir);

	Platform::String ^ dataPath =
	    Platform::String::Concat(Windows::ApplicationModel::Package::Current->InstalledLocation->Path,
	                             ref new Platform::String(L"\\share\\liblinphone_tester"));

	wwritable_dir = dataPath->Data();
	wcstombs(writable_dir, wwritable_dir, sizeof(writable_dir));
	bc_tester_set_resource_dir_prefix(writable_dir);
	bool_t haveLogFile = FALSE, haveXmlFile = FALSE, forceNoXml = FALSE;

	_args = (char **)malloc(sizeof(char **) * (parameters->Length + 1));
	int countArgs = 0;
	for (unsigned int i = 0; i < parameters->Length; ++i) {
		std::wstring parameter = parameters[i]->Data();
		if (parameter.length() > 0) {
			int length = (int)wcstombs(NULL, parameter.c_str(), 256) + 1;
			_args[countArgs] = (char *)malloc(sizeof(char) * length);
			wcstombs(_args[countArgs++], parameter.c_str(), length);
			if (parameter == L"--log-file") haveLogFile = TRUE;
			else if (parameter == L"--xml-file") haveXmlFile = TRUE;
			else if (parameter == L"--no-xml") forceNoXml = TRUE;
		}
	}
	_args[countArgs] = NULL;
	for (int i = 1; i < countArgs;) {
		i += (size_t)bc_tester_parse_args(countArgs, _args, i);
	}
	if (!haveLogFile) {
		char *logFile = bc_tester_file("LibLinphoneWindows10.log");
		char *logArgs[] = {"--log-file", logFile};
		bc_tester_parse_args(2, logArgs, 0); // logFile memory is passed to tester. Do not free it
	}
	if (!haveXmlFile && !forceNoXml) {
		char *xmlFile = bc_tester_file("LibLinphoneWindows10");
		char *args[] = {"--xml-file", xmlFile};
		bc_tester_parse_args(2, args, 0); // xmlFile memory is passed to tester. Do not free it
	}
	bc_tester_set_process_events_func(processEvents);
	if (flexisip_tester_dns_server != NULL) {
		/*
		 * We have to remove ipv6 addresses because flexisip-tester internally uses a dnsmasq configuration that does
		 * not listen on ipv6.
		 */
		flexisip_tester_dns_ip_addresses = liblinphone_tester_remove_v6_addr(
		    liblinphone_tester_resolve_name_to_ip_address(flexisip_tester_dns_server));
		if (flexisip_tester_dns_ip_addresses == NULL) {
			ms_error("Cannot resolve the flexisip-tester's dns server name '%s'.", flexisip_tester_dns_server);
		}
	}
}

bool NativeTester::run(Platform::String ^ suiteName, Platform::String ^ caseName, Platform::Boolean verbose) {
	std::wstring all(L"ALL");
	std::wstring wssuitename = suiteName->Data();
	std::wstring wscasename = caseName->Data();
	char csuitename[MAX_SUITE_NAME_SIZE] = {0};
	char ccasename[MAX_SUITE_NAME_SIZE] = {0};
	wcstombs(csuitename, wssuitename.c_str(), sizeof(csuitename));
	wcstombs(ccasename, wscasename.c_str(), sizeof(ccasename));

	if (verbose) {
		linphone_core_set_log_level_mask((OrtpLogLevel)(ORTP_MESSAGE | ORTP_WARNING | ORTP_ERROR | ORTP_FATAL));
	} else {
		linphone_core_set_log_level_mask(ORTP_FATAL);
	}
	linphone_core_set_log_handler(libLinphoneNativeOutputTraceHandler);
	return bc_tester_run_tests(wssuitename == all ? 0 : csuitename, wscasename == all ? 0 : ccasename, NULL) != 0;
}

void NativeTester::runAllToXml() {
	auto workItem = ref new WorkItemHandler([this](IAsyncAction ^ workItem) {
		bc_tester_start(NULL);
		if (flexisip_tester_dns_ip_addresses) {
			bctbx_list_free_with_data(flexisip_tester_dns_ip_addresses, bctbx_free);
			flexisip_tester_dns_ip_addresses = NULL;
		}
		bc_tester_uninit();
	});
	_asyncAction = ThreadPool::RunAsync(workItem);
}

unsigned int NativeTester::nbTestSuites() {
	return bc_tester_nb_suites();
}

unsigned int NativeTester::nbTests(Platform::String ^ suiteName) {
	std::wstring suitename = suiteName->Data();
	char cname[MAX_SUITE_NAME_SIZE] = {0};
	wcstombs(cname, suitename.c_str(), sizeof(cname));
	return bc_tester_nb_tests(cname);
}

Platform::String ^ NativeTester::testSuiteName(int index) {
	const char *cname = bc_tester_suite_name(index);
	wchar_t wcname[MAX_SUITE_NAME_SIZE];
	mbstowcs(wcname, cname, sizeof(wcname));
	return ref new String(wcname);
}

Platform::String ^ NativeTester::testName(Platform::String ^ suiteName, int testIndex) {
	std::wstring suitename = suiteName->Data();
	char csuitename[MAX_SUITE_NAME_SIZE] = {0};
	wcstombs(csuitename, suitename.c_str(), sizeof(csuitename));
	const char *cname = bc_tester_test_name(csuitename, testIndex);
	if (cname == NULL) return ref new String();
	wchar_t wcname[MAX_SUITE_NAME_SIZE];
	mbstowcs(wcname, cname, sizeof(wcname));
	return ref new String(wcname);
}

void NativeTester::parseArgs(Platform::String ^ commandLine, std::vector<std::string> *argv) {
	auto data = commandLine->Data();
	bool inside = false;
	std::string currentParameter;
	for (int i = 0; data[i] != 0; ++i) {
		if (data[i] == '"') {
			inside = !inside;
			// currentParameter += data[i];
		} else if (data[i] == ' ') {
			if (!inside) {
				argv->push_back(currentParameter);
				currentParameter = "";
			} else {
				currentParameter += (char)data[i];
			}
		} else currentParameter += (char)data[i];
	}
	if (currentParameter != "") argv->push_back(currentParameter);
}
