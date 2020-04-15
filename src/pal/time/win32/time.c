/*
 *  time.c
 *
 *  Copyright 2013, 2014 Michael Zillgith
 *
 *  This file is taken from libIEC61850.
 *
 */


#include "libieeeC37_118_platform_includes.h"

#include "stack_config.h"

#include <time.h>

#include <windows.h>

uint64_t
Hal_getTimeInMs()
{
	FILETIME ft;
	uint64_t now;

	static const uint64_t DIFF_TO_UNIXTIME = 11644473600000LL;

	GetSystemTimeAsFileTime(&ft);

	now = (LONGLONG)ft.dwLowDateTime + ((LONGLONG)(ft.dwHighDateTime) << 32LL);

	return (now / 10000LL) - DIFF_TO_UNIXTIME;
}

ParsedTime
convertEpochToCalendar(uint32_t timeValue)
{
	time_t time = (time_t)timeValue;

	struct tm tm;

	ParsedTime dateTime = (ParsedTime)GLOBAL_CALLOC(1, sizeof(struct sParsedTime));

	tm = *localtime(&time);

	dateTime->year = tm.tm_year + 1900;
	dateTime->month = tm.tm_mon + 1;
	dateTime->day = tm.tm_mday;
	dateTime->hour = tm.tm_hour;
	dateTime->minute = tm.tm_min;
	dateTime->second = tm.tm_sec;

	return dateTime;
}
