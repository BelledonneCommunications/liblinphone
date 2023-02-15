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

#pragma once

#include "liblinphone_tester.h"
#include "linphone/core.h"

#include <string>
#include <vector>

namespace BelledonneCommunications {
namespace Linphone {
namespace Tester {
public
interface class OutputTraceListener {
public:
	void outputTrace(Platform::String ^ lev, Platform::String ^ msg);
};

public
ref class NativeTester sealed {
public:
	void setOutputTraceListener(OutputTraceListener ^ traceListener);
	unsigned int nbTestSuites();
	unsigned int nbTests(Platform::String ^ suiteName);
	Platform::String ^ testSuiteName(int index);
	Platform::String ^ testName(Platform::String ^ suiteName, int testIndex);
	void initialize(const Platform::Array<Platform::String ^> ^ args,
	                Windows::Storage::StorageFolder ^ writableDirectory,
	                Platform::Boolean ui,
	                Platform::Boolean verbose);

	bool run(Platform::String ^ suiteName, Platform::String ^ caseName, Platform::Boolean verbose);
	void runAllToXml();

	static property NativeTester ^
	    Instance { NativeTester ^ get() { return _instance; } } property Windows::Foundation::IAsyncAction ^
	    AsyncAction { Windows::Foundation::IAsyncAction ^ get() { return _asyncAction; } } private : NativeTester();
	~NativeTester();
	void parseArgs(Platform::String ^ commandLine, std::vector<std::string> *argv);

	static NativeTester ^ _instance;
	Windows::Foundation::IAsyncAction ^ _asyncAction;
	std::vector<std::string> m_commandLine;
	char _currentProcPath[255];
	char **_args;
};
} // namespace Tester
} // namespace Linphone
} // namespace BelledonneCommunications
