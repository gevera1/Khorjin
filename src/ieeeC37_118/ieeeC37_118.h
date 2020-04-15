#ifndef IEEEC37_118_H_
#define IEEEC37_118_H_

#include "libieeeC37_118_platform_includes.h"

#ifdef __cplusplus
extern "C" {
#endif

	typedef struct sPMUConnectionData {
		char ipAddress_PmuServer[20];
		int32_t pmuPortNumber;
		int32_t pmuIdCode;
	}PMUConnectionData;

	typedef struct sStatus{
		uint8_t dataError; /*1. Good Measurement data, No Error,
						   2. PMU Error (No Information About Data)
						   3. PMU in test mode (do not use values)
						   4. PMU error (do not use values)*/
		uint8_t pmuIsNOTSync;
		uint8_t pmuIsTriggered; /* 0 = No Trigger.*/
		uint8_t dataModified; /*1 = Data Modified by Post Processing, 0 = Otherwise*/
		uint8_t pmuTimeQuality;
		uint8_t unlockTime;
		uint8_t triggerReason;
	}Status;

	typedef struct sTimeQuality {
		int32_t leapSecondDirection;
		bool leapSecondOccured;
		bool leapSecondPending;
		bool timeIsLocked;
		float timeError;
	}TimeQuality;

	typedef struct sTime {
		ParsedTime parsedTime;
		uint64_t timeValueInMs;
		uint32_t secondOfCenctury;
		float fractionOfSecond;
		TimeQuality quality;
	}Time;

	struct sPhasor {
		char channelName[20];
		float magnitude;
		float angle;
		float conversionFactor;
		bool phasorIsVorI; /*True = Voltage, False = Current*/
	};

	typedef struct sPhasor* Phasor;

	struct sAnalog {
		char channelName[20];
		float value;
		uint8_t analogType;
		float conversionFactor;
	};

	typedef struct sAnalog* Analog;

	struct sDigital {
		struct {
			char channelName[20];
			bool status;
			bool isEnabled;
		}channel[16];
		uint16_t digitalWord;
		uint16_t dgMaskWord;
		uint16_t validInputs;
		bool dgInputIsNormal;
	};

	typedef struct sDigital* Digital;

	struct sConfiguration
	{
		uint16_t pmuIdcode;
		char stationName[20];
		bool freqIsFloatingPoint;
		bool analogIsFloatingPoint;
		bool phasorIsFloatingPoint;
		bool phasorIsPolar;
		uint16_t numberOfPhasors;
		uint16_t numberOfAnalogs;
		uint16_t numberOfDigitals;
		float fNom;
		float frequency;
		float rocof;
		uint16_t cfgCnt;

		Phasor phasor;
		Analog analog;
		Digital digital;

		//STAT
		uint8_t dataError; /*1. Good Measurement data, No Error,
						   2. PMU Error (No Information About Data)
						   3. PMU in test mode (do not use values)
						   4. PMU error (do not use values)*/
		bool pmuIsNOTSync;
		bool dataSortingByArrivalorTime; /* True: by Arrival False: by Time Stamp*/
		bool pmuIsTriggered; /* False = No Trigger.*/
		bool configToChange; /*Set to 1 for 1 min to advise configuration will change, and clear to 0 when change effected*/
		bool dataModified; /*True = Data Modified by Post Processing, False = Otherwise*/
		uint8_t pmuTimeQuality;
		uint8_t unlockTime;
		uint8_t triggerReason;
		uint16_t statWord;
	};

	typedef struct sConfiguration* Configuration;

	typedef struct sSecurityInformation
	{
		uint32_t timeOfCurrentKey;
		uint16_t timeToNextKey;
		uint16_t securityAlgorithm;
		uint32_t keyId;
	}SecurityInformation;

	typedef struct sC37_118client* C37_118client;

	C37_118client
		c37_118client_connect(PMUConnectionData* parameters, uint32_t timeoutInMs);

	void
		c37_118client_stopTransmission(C37_118client self);

	void
		c37_118client_startReceiveDataStram(C37_118client self);

	void
		c37_118client_startTransmission(C37_118client self);

	int
		c37_118client_getConfiguration(C37_118client self);

	int
		c37_118client_getDataStream(C37_118client self);

	void
		c37_118client_printDataStream(C37_118client self);

	void
		c37_118client_printConfiguration(C37_118client self);

	/*void
		c37_118client_outputDataStream(C37_118client self, ParsedTime timestamp, float* freqBuffer, float* rocofBuffer, float** phasorsBuffer);
		*/
	void
		c37_118client_outputDataStream(C37_118client self, ParsedTime timestamp, float* freqBuffer, float* rocofBuffer, float* phasorsMagBuffer, float* phasorsAngBuffer, float* analogBuffer);

	void
		c37_118client_getConf_pdc(C37_118client self, uint16_t* num_pmu, uint16_t* data_rate, ParsedTime timestamp);

	void
		c37_118client_getConf_pmu(C37_118client self, uint16_t* numPhasors, uint16_t* numAnalog, uint16_t* numdigital, float* freqNom, uint16_t* configCount, uint16_t* idCode);

	void
		c37_118client_getConf_pmu_stationName(C37_118client self, int pmuNumber, char* stationName);

	void
		c37_118client_getConf_pmu_channelName(C37_118client self, int pmuNumber, int phasor, char* channelName, int* phasorType);

	void
		c37_118client_getConf_pmu_analogs(C37_118client self, int pmuNumber, int analog, char* channelName, int* analogType);

	void
		c37_118client_outputDataStream_status(C37_118client self, int pmuNumber, Status* status);

	/*void
		c37_118client_outputConfiguration(C37_118client self, char*** channelNames, int* num_pmu, int* num_phasors);
		*/
	void
		c37_118client_destroy(C37_118client self);

#ifdef __cplusplus
}
#endif

#endif /* IEEEC37_118_H_ */
