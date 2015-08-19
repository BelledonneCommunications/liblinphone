﻿#pragma once

#include "linphonecore.h"
#include "liblinphone_tester.h"

namespace liblinphone_tester_runtime_component
{
	public interface class OutputTraceListener
	{
	public:
		void outputTrace(Platform::String^ lev, Platform::String^ msg);
	};

    public ref class LibLinphoneTester sealed
    {
    public:
		void setWritableDirectory(Windows::Storage::StorageFolder^ folder);
		void setOutputTraceListener(OutputTraceListener^ traceListener);
		unsigned int nbTestSuites();
		unsigned int nbTests(Platform::String^ suiteName);
		Platform::String^ testSuiteName(int index);
		Platform::String^ testName(Platform::String^ suiteName, int testIndex);
		bool run(Platform::String^ suiteName, Platform::String^ caseName, Platform::Boolean verbose);
		void runAllToXml();

		static property LibLinphoneTester^ Instance
		{
			LibLinphoneTester^ get() { return _instance; }
		}
		property Windows::Foundation::IAsyncAction^ AsyncAction
		{
			Windows::Foundation::IAsyncAction^ get() { return _asyncAction; }
		}
	private:
		LibLinphoneTester();
		~LibLinphoneTester();
		void init(bool verbose);

		static LibLinphoneTester^ _instance;
		Windows::Foundation::IAsyncAction^ _asyncAction;
	};
}