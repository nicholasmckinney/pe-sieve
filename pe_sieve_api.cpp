#include <windows.h>
#include <string>
#include <iostream>

#include "pe_sieve.h"

#define PESIEVE_EXPORTS
#include <pe_sieve_api.h>

#define LIB_NAME "PE-sieve"

using namespace pesieve;


size_t print_report(const pesieve::ReportEx& report, const pesieve::t_params args, char* json_buf, size_t json_buf_size)
{
	if (!report.scan_report) return 0;

	std::string report_str = scan_report_to_json(*report.scan_report, ProcessScanReport::REPORT_SUSPICIOUS_AND_ERRORS, args.json_lvl);
	const size_t report_len = report_str.length();
	if (json_buf && json_buf_size) {
		::memset(json_buf, 0, json_buf_size);
		size_t max_len = report_len <= (json_buf_size - 1) ? report_len : (json_buf_size - 1);
		::memcpy(json_buf, report_str.c_str(), max_len);
	}
	return report_len;
}

PEsieve_report PESIEVE_API_FUNC PESieve_scan_ex(const PEsieve_params args, char *json_buf, size_t json_buf_size, size_t* needed_size)
{
	const pesieve::ReportEx* report = pesieve::scan_and_dump(args);
	if (report == nullptr) {
		pesieve::t_report nullrep = { 0 };
		nullrep.pid = args.pid;
		nullrep.errors = pesieve::ERROR_SCAN_FAILURE;
		return nullrep;
	}
	size_t report_size = 0;
	pesieve::t_report summary = report->scan_report->generateSummary();
	//check the pointers:
	if (json_buf) {
		if (!json_buf_size || IsBadWritePtr(json_buf, json_buf_size)) {
			json_buf = nullptr;
			json_buf_size = 0;
		}
	}
	if (needed_size && IsBadWritePtr(needed_size, sizeof(size_t))) {
		needed_size = nullptr;
	}

	//print the report (only if any valid output buffer was passed)
	if (json_buf || needed_size) {
		report_size = print_report(*report, args, json_buf, json_buf_size);
		if (needed_size) {
			*needed_size = report_size;
		}
	}

	delete report;
	return summary;
}

PEsieve_report PESIEVE_API_FUNC PESieve_scan(const PEsieve_params args)
{
	return PESieve_scan_ex(args, nullptr, 0, nullptr);
}

void PESIEVE_API_FUNC PESieve_help(void)
{
	std::string my_info = pesieve::info();

	std::cout << my_info;
	MessageBox(NULL, my_info.c_str(), LIB_NAME, MB_ICONINFORMATION);
}

extern const DWORD PESIEVE_API PESieve_version = pesieve::PESIEVE_VERSION_ID;
