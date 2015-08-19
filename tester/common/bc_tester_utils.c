/*
tester - liblinphone test suite
Copyright (C) 2013  Belledonne Communications SARL

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


/* this must be provided at compile time*/
#include BC_CONFIG_FILE

#include "bc_tester_utils.h"

#include <stdlib.h>
#include <time.h>

#include "CUnit/Basic.h"
#include "CUnit/Automated.h"
#ifdef _WIN32
#if defined(__MINGW32__) || !defined(WINAPI_FAMILY_PARTITION) || !defined(WINAPI_PARTITION_DESKTOP)
#define BC_TESTER_WINDOWS_DESKTOP 1
#elif defined(WINAPI_FAMILY_PARTITION)
#if defined(WINAPI_PARTITION_DESKTOP) && WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
#define BC_TESTER_WINDOWS_DESKTOP 1
#endif
#if defined(WINAPI_PARTITION_PHONE_APP) && WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_PHONE_APP)
#define BC_TESTER_WINDOWS_PHONE 1
#endif
#if defined(WINAPI_PARTITION_APP) && WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP)
#define BC_TESTER_WINDOWS_UNIVERSAL 1
#endif
#endif
#endif


static char *bc_tester_resource_dir_prefix = NULL;
static char *bc_tester_writable_dir_prefix = NULL;

int bc_printf_verbosity_info;
int bc_printf_verbosity_error;

static test_suite_t **test_suite = NULL;
static int nb_test_suites = 0;

#ifdef HAVE_CU_CURSES
#include "CUnit/CUCurses.h"
static unsigned char curses = 0;
#endif

char* xml_file = "CUnitAutomated-Results.xml";
int   xml_enabled = 0;
char * suite_name;
char * test_name;
void (*tester_printf_va)(int level, const char *fmt, va_list args);

void bc_tester_printf(int level, const char *fmt, ...) {
	va_list args;
	va_start (args, fmt);
	tester_printf_va(level, fmt, args);
	va_end (args);
}

int bc_tester_run_suite(test_suite_t *suite) {
	int i;

	CU_pSuite pSuite = CU_add_suite(suite->name, suite->init_func, suite->cleanup_func);

	for (i = 0; i < suite->nb_tests; i++) {
		if (NULL == CU_add_test(pSuite, suite->tests[i].name, suite->tests[i].func)) {
			return CU_get_error();
		}
	}

	return 0;
}

const char * bc_tester_suite_name(int suite_index) {
	if (suite_index >= nb_test_suites) return NULL;
	return test_suite[suite_index]->name;
}

int bc_tester_suite_index(const char *suite_name) {
	int i;

	for (i = 0; i < nb_test_suites; i++) {
		if ((strcmp(suite_name, test_suite[i]->name) == 0) && (strlen(suite_name) == strlen(test_suite[i]->name))) {
			return i;
		}
	}

	return -1;
}

int bc_tester_nb_suites(void) {
	return nb_test_suites;
}

const char * bc_tester_test_name(const char *suite_name, int test_index) {
	int suite_index = bc_tester_suite_index(suite_name);
	if ((suite_index < 0) || (suite_index >= nb_test_suites)) return NULL;
	if (test_index >= test_suite[suite_index]->nb_tests) return NULL;
	return test_suite[suite_index]->tests[test_index].name;
}

int bc_tester_nb_tests(const char *suite_name) {
	int i = bc_tester_suite_index(suite_name);
	if (i < 0) return 0;
	return test_suite[i]->nb_tests;
}

void bc_tester_list_suites(void) {
	int j;
	for(j=0;j<nb_test_suites;j++) {
		bc_tester_printf(bc_printf_verbosity_info, "%s", bc_tester_suite_name(j));
	}
}

void bc_tester_list_tests(const char *suite_name) {
	int j;
	for( j = 0; j < bc_tester_nb_tests(suite_name); j++) {
		const char *test_name = bc_tester_test_name(suite_name, j);
		bc_tester_printf(bc_printf_verbosity_info, "%s", test_name);
	}
}

static void all_complete_message_handler(const CU_pFailureRecord pFailure) {
#ifdef HAVE_CU_GET_SUITE
	char * results = CU_get_run_results_string();
	bc_tester_printf(bc_printf_verbosity_info,"\n%s",results);
	free(results);
#endif
}

static void suite_init_failure_message_handler(const CU_pSuite pSuite) {
	bc_tester_printf(bc_printf_verbosity_error,"Suite initialization failed for [%s]", pSuite->pName);
}

static void suite_cleanup_failure_message_handler(const CU_pSuite pSuite) {
	bc_tester_printf(bc_printf_verbosity_error,"Suite cleanup failed for [%s]", pSuite->pName);
}

#ifdef HAVE_CU_GET_SUITE
static time_t suite_start_time = 0;
static void suite_start_message_handler(const CU_pSuite pSuite) {
	bc_tester_printf(bc_printf_verbosity_info,"Suite [%s] started\n", pSuite->pName);
	suite_start_time = time(NULL);
}
static void suite_complete_message_handler(const CU_pSuite pSuite, const CU_pFailureRecord pFailure) {
	bc_tester_printf(bc_printf_verbosity_info,"Suite [%s] ended in %lu sec\n", pSuite->pName, time(NULL) - suite_start_time);
}

static time_t test_start_time = 0;
static void test_start_message_handler(const CU_pTest pTest, const CU_pSuite pSuite) {
	bc_tester_printf(bc_printf_verbosity_info,"Suite [%s] Test [%s] started", pSuite->pName,pTest->pName);
	test_start_time = time(NULL);
}

/*derivated from cunit*/
static void test_complete_message_handler(const CU_pTest pTest,
	const CU_pSuite pSuite,
	const CU_pFailureRecord pFailureList) {
	int i;
	char result[2048];
	char buffer[2048];
	CU_pFailureRecord pFailure = pFailureList;
	snprintf(result, sizeof(result), "Suite [%s] Test [%s] %s in %lu secs"
			, pSuite->pName, pTest->pName, pFailure?"failed":"passed",(unsigned long)(time(NULL) - test_start_time));
	if (pFailure) {
		for (i = 1 ; (NULL != pFailure) ; pFailure = pFailure->pNext, i++) {
			snprintf(buffer, sizeof(buffer), "\n    %d. %s:%u  - %s", i,
				(NULL != pFailure->strFileName) ? pFailure->strFileName : "",
				pFailure->uiLineNumber,
				(NULL != pFailure->strCondition) ? pFailure->strCondition : "");
			strncat(result, buffer, strlen(buffer));
		}
	}
	bc_tester_printf(bc_printf_verbosity_info,"%s\n", result);
}
#endif

int bc_tester_run_tests(const char *suite_name, const char *test_name) {
	int i;

	/* initialize the CUnit test registry */
	if (CUE_SUCCESS != CU_initialize_registry())
		return CU_get_error();

	for (i = 0; i < nb_test_suites; i++) {
		bc_tester_run_suite(test_suite[i]);
	}
#ifdef HAVE_CU_GET_SUITE
	CU_set_suite_start_handler(suite_start_message_handler);
	CU_set_suite_complete_handler(suite_complete_message_handler);
	CU_set_test_start_handler(test_start_message_handler);
	CU_set_test_complete_handler(test_complete_message_handler);
#endif
	CU_set_all_test_complete_handler(all_complete_message_handler);
	CU_set_suite_init_failure_handler(suite_init_failure_message_handler);
	CU_set_suite_cleanup_failure_handler(suite_cleanup_failure_message_handler);

	if( xml_enabled != 0 ){
		CU_automated_run_tests();
	} else {

#ifndef HAVE_CU_GET_SUITE
		if( suite_name ){
			bc_tester_printf(bc_printf_verbosity_info, "Tester compiled without CU_get_suite() function, running all tests instead of suite '%s'", suite_name);
		}
#else
		if (suite_name){
			CU_pSuite suite;
			suite=CU_get_suite(suite_name);
			if (!suite) {
				bc_tester_printf(bc_printf_verbosity_error, "Could not find suite '%s'. Available suites are:", suite_name);
				bc_tester_list_suites();
				return -1;
			} else if (test_name) {
				CU_pTest test=CU_get_test_by_name(test_name, suite);
				if (!test) {
					bc_tester_printf(bc_printf_verbosity_error, "Could not find test '%s' in suite '%s'. Available tests are:", test_name, suite_name);
					// do not use suite_name here, since this method is case sensitive
					bc_tester_list_tests(suite->pName);
					return -2;
				} else {
					CU_ErrorCode err= CU_run_test(suite, test);
					if (err != CUE_SUCCESS) bc_tester_printf(bc_printf_verbosity_error, "CU_basic_run_test error %d", err);
				}
			} else {
				CU_run_suite(suite);
			}
		}
		else
#endif
		{
#ifdef HAVE_CU_CURSES
			if (curses) {
			/* Run tests using the CUnit curses interface */
				CU_curses_run_tests();
			}
			else
#endif
			{
				/* Run all tests using the CUnit Basic interface */
				CU_run_all_tests();
			}
		}
	}
	return CU_get_number_of_tests_failed()!=0;

}


void bc_tester_helper(const char *name, const char* additionnal_helper) {
	bc_tester_printf(bc_printf_verbosity_info,"%s --help\n"
		"\t\t\t--list-suites\n"
		"\t\t\t--list-tests <suite>\n"
		"\t\t\t--suite <suite name>\n"
		"\t\t\t--test <test name>\n"
#ifdef HAVE_CU_CURSES
		"\t\t\t--curses\n"
#endif
		"\t\t\t--xml\n"
		"\t\t\t--xml-file <xml file name>\n"
		"And additionally:\n"
		"%s"
		, name
		, additionnal_helper);
}

void bc_tester_init(void (*ftester_printf)(int level, const char *fmt, va_list args), int iverbosity_info, int iverbosity_error) {
#if defined(BC_TESTER_WINDOWS_PHONE) || defined(BC_TESTER_WINDOWS_UNIVERSAL)
	bc_tester_set_resource_dir_prefix("Assets");
#elif defined(__QNX__)
	bc_tester_set_resource_dir_prefix("./app/native/assets/");
#else
	bc_tester_set_resource_dir_prefix(".");
#endif

#ifdef ANDROID
	bc_tester_set_writable_dir_prefix("/data/data/org.linphone.tester/cache");
#elif defined(__QNX__)
	bc_tester_set_writable_dir_prefix("./tmp");
#else
	bc_tester_set_writable_dir_prefix(".");
#endif

	tester_printf_va = ftester_printf;
	bc_printf_verbosity_error = iverbosity_error;
	bc_printf_verbosity_info = iverbosity_info;
}

int bc_tester_parse_args(int argc, char **argv, int argid)
{
	int i = argid;
	if (strcmp(argv[i],"--help")==0){
		return -1;
	} else if (strcmp(argv[i],"--test")==0){
		CHECK_ARG("--test", ++i, argc);
		test_name=argv[i];
	}else if (strcmp(argv[i],"--suite")==0){
		CHECK_ARG("--suite", ++i, argc);
		suite_name=argv[i];
	} else if (strcmp(argv[i],"--list-suites")==0){
		bc_tester_list_suites();
		return 0;
	} else if (strcmp(argv[i],"--list-tests")==0){
		CHECK_ARG("--list-tests", ++i, argc);
		suite_name = argv[i];
		bc_tester_list_tests(suite_name);
		return 0;
	} else if (strcmp(argv[i], "--xml-file") == 0){
		CHECK_ARG("--xml-file", ++i, argc);
		xml_file = argv[i];
		xml_enabled = 1;
	} else if (strcmp(argv[i], "--xml") == 0){
		xml_enabled = 1;
	}else {
		bc_tester_printf(bc_printf_verbosity_error, "Unknown option \"%s\"\n", argv[i]);
		return -1;
	}

	if( xml_enabled && (suite_name || test_name) ){
		bc_tester_printf(bc_printf_verbosity_error, "Cannot use both XML and specific test suite\n");
		return -1;
	}

	/* returns number of arguments read + 1 */
	return i - argid + 1;
}

int bc_tester_start(void) {
	int ret;
	if( xml_enabled ){
		size_t size = strlen(xml_file) + strlen(".tmp") + 1;
		char * xml_tmp_file = malloc(sizeof(char) * size);
		snprintf(xml_tmp_file, size, "%s.tmp", xml_file);
		CU_set_output_filename(xml_tmp_file);
		free(xml_tmp_file);
	}

	ret = bc_tester_run_tests(suite_name, test_name);

	return ret;
}
void bc_tester_add_suite(test_suite_t *suite) {
	if (test_suite == NULL) {
		test_suite = (test_suite_t **)malloc(10 * sizeof(test_suite_t *));
	}
	test_suite[nb_test_suites] = suite;
	nb_test_suites++;
	if ((nb_test_suites % 10) == 0) {
		test_suite = (test_suite_t **)realloc(test_suite, (nb_test_suites + 10) * sizeof(test_suite_t *));
	}
}

void bc_tester_uninit(void) {
	/* Redisplay list of failed tests on end */
	if (CU_get_number_of_failure_records()){
		CU_basic_show_failures(CU_get_failure_list());
	}
	CU_cleanup_registry();
	/*add missing final newline*/
	bc_tester_printf(bc_printf_verbosity_info,"");

	if( xml_enabled ){
		/*create real xml file only if tester did not crash*/
		size_t size = strlen(xml_file) + strlen(".tmp-Results.xml") + 1;
		char * xml_tmp_file = malloc(sizeof(char) * size);
		snprintf(xml_tmp_file, size, "%s.tmp-Results.xml", xml_file);
		rename(xml_tmp_file, xml_file);
		free(xml_tmp_file);
	}

	if (test_suite != NULL) {
		free(test_suite);
		test_suite = NULL;
		nb_test_suites = 0;
	}

	if (bc_tester_resource_dir_prefix != NULL) {
		free(bc_tester_resource_dir_prefix);
		bc_tester_resource_dir_prefix = NULL;
	}
	if (bc_tester_writable_dir_prefix != NULL) {
		free(bc_tester_writable_dir_prefix);
		bc_tester_writable_dir_prefix = NULL;
	}
}

static void bc_tester_set_dir_prefix(char **prefix, const char *name) {
	size_t len = strlen(name);
	if (*prefix != NULL) free(*prefix);
	*prefix = malloc(len + 1);
	strncpy(*prefix, name, len);
	(*prefix)[len] = '\0';
}

const char * bc_tester_get_resource_dir_prefix(void) {
	return bc_tester_resource_dir_prefix;
}

void bc_tester_set_resource_dir_prefix(const char *name) {
	bc_tester_set_dir_prefix(&bc_tester_resource_dir_prefix, name);
}

const char * bc_tester_get_writable_dir_prefix(void) {
	return bc_tester_writable_dir_prefix;
}

void bc_tester_set_writable_dir_prefix(const char *name) {
	bc_tester_set_dir_prefix(&bc_tester_writable_dir_prefix, name);
}

static char * bc_tester_path(const char *prefix, const char *name) {
	char* file = NULL;
	if (name) {
		size_t len = strlen(prefix) + 1 + strlen(name) + 1;
		file = malloc(len);
		snprintf(file, len, "%s/%s", prefix, name);
		file[strlen(file)] = '\0';
	}
	return file;
}

char * bc_tester_res(const char *name) {
	return bc_tester_path(bc_tester_resource_dir_prefix, name);
}

char * bc_tester_file(const char *name) {
	return bc_tester_path(bc_tester_writable_dir_prefix, name);
}
