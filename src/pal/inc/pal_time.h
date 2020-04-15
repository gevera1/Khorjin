/*
 *  time.c
 *
 *  Copyright 2013, 2014 Michael Zillgith
 *
 *  This file is taken from libIEC61850.
 */

#ifndef HAL_C_
#define HAL_C_

#ifdef __cplusplus
extern "C" {
#endif

//typedef struct tm* parsedTime;

struct sParsedTime
{
	int year;
	int month;
	int day;
	int hour;
	int minute;
	int second;
	float secPlusFraction;
};

typedef struct sParsedTime* ParsedTime;

//parsedTime convertEpochToCalendar(uint32_t timeValue);


ParsedTime
convertEpochToCalendar(uint32_t timeValue);


/*! \addtogroup hal
   *
   *  @{
   */

/**
 * @defgroup HAL_TIME Time related functions
 *
 * @{
 */

/**
 * Get the system time in milliseconds.
 *
 * The time value returned as 64-bit unsigned integer should represent the milliseconds
 * since the UNIX epoch (1970/01/01 00:00 UTC).
 *
 * \return the system time with millisecond resolution.
 */
uint64_t Hal_getTimeInMs(void);

/*! @} */

/*! @} */

#ifdef __cplusplus
}
#endif


#endif /* HAL_C_ */
