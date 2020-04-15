#include "stack_config.h"
#include "libieeeC37_118_platform_includes.h"

#include "ieeeC37_118.h"
#include "pal_socket.h"
#include "pal_time.h"
#include "pal_thread.h"

#ifndef DEBUG_IEEEC37_118
#define DEBUG_IEEEC37_118 0
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define BUFER_LENGTH 10000

static int
encodeUInt32FixedSize(uint32_t value, uint8_t* buffer, int bufPos)
{
	uint8_t* valueArray = (uint8_t*)&value;

#if (ORDER_LITTLE_ENDIAN == 1)
	buffer[bufPos++] = valueArray[3];
	buffer[bufPos++] = valueArray[2];
	buffer[bufPos++] = valueArray[1];
	buffer[bufPos++] = valueArray[0];
#else
	buffer[bufPos++] = valueArray[0];
	buffer[bufPos++] = valueArray[1];
	buffer[bufPos++] = valueArray[2];
	buffer[bufPos++] = valueArray[3];
#endif

	return bufPos;
}

static int
encodeUInt16FixedSize(uint16_t value, uint8_t* buffer, int bufPos)
{
	uint8_t* valueArray = (uint8_t*)&value;

#if (ORDER_LITTLE_ENDIAN == 1)

	buffer[bufPos++] = valueArray[1];
	buffer[bufPos++] = valueArray[0];
#else
	buffer[bufPos++] = valueArray[0];
	buffer[bufPos++] = valueArray[1];

#endif

	return bufPos;
}

struct sC37_118client
{
	uint8_t* bufferSnd;
	uint8_t* bufferRcv;

	Socket socket;

	uint16_t idcode;
	uint16_t numPmu;
	uint32_t timeBase;
	uint16_t dataRate;

	Time configTimeStamp;
	Time headerTimeStamp;
	Time dataTimeStamp;

	Configuration configuration;
	bool running;
};

static uint16_t
computeCRC(const uint8_t *Message, uint32_t MessLen)
{
	uint16_t crc = 0xFFFF;
	uint16_t temp;
	uint16_t quick;
	int i;
	for (i = 0; i < MessLen; i++)
	{
		temp = (crc >> 8) ^ Message[i];
		crc <<= 8;
		quick = temp ^ (temp >> 4);
		crc ^= quick;
		quick <<= 5;
		crc ^= quick;
		quick <<= 7;
		crc ^= quick;
	}
	return crc;
}

C37_118client
c37_118client_connect(PMUConnectionData* parameters, uint32_t timeoutInMs)
{
	C37_118client self = (C37_118client)GLOBAL_CALLOC(1, sizeof(struct sC37_118client));

	self->bufferSnd = (uint8_t*)GLOBAL_MALLOC(BUFER_LENGTH);
	self->bufferRcv = (uint8_t*)GLOBAL_MALLOC(BUFER_LENGTH);

	self->idcode = parameters->pmuIdCode;

	self->socket = TcpSocket_create();
	Socket_setConnectTimeout(self->socket, timeoutInMs);

	printf("Start to establish connection with PMU/PDC Server...\n");
	printf("IP : %s, Port:%u\n\n", parameters->ipAddress_PmuServer, parameters->pmuPortNumber);
	
	int count = 0;
	while (!Socket_connect(self->socket,
		parameters->ipAddress_PmuServer, parameters->pmuPortNumber))
	{
		int newTimeoutInMs = (int)pow((double)2, count++) * timeoutInMs;
		printf("Failed to connect. Try to reconnect in %u seconds.\n", newTimeoutInMs / 1000);
		Socket_setConnectTimeout(self->socket, newTimeoutInMs);
	}

	return self;
}

void
c37_118client_stopTransmission(C37_118client self)
{
	memset(self->bufferSnd, 0, BUFER_LENGTH);

	int bufPos = 0;

	/*SYNC*/
	self->bufferSnd[bufPos++] = 0xAA;
	self->bufferSnd[bufPos++] = 0x41;

	/*FRAMESIZE*/
	self->bufferSnd[bufPos++] = 0x00;
	self->bufferSnd[bufPos++] = 0x12;

	/*IDCODE*/
	uint16_t idCode = self->idcode;

	self->bufferSnd[bufPos++] = idCode / 256;
	self->bufferSnd[bufPos++] = idCode % 256;

	int socPosition = bufPos;

	uint64_t timeStamp = Hal_getTimeInMs();

	uint32_t soc = timeStamp / 1000LL;
	uint32_t fracsec = timeStamp % 1000LL;

	/*SOC*/
	bufPos = encodeUInt32FixedSize(soc, self->bufferSnd, bufPos);

	int fracsecPosition = bufPos;

	/*FRACSEC*/
	bufPos = encodeUInt32FixedSize(fracsec, self->bufferSnd, bufPos);

	int comandPosition = bufPos;

	/*CMD*/
	self->bufferSnd[bufPos++] = 0x00;
	self->bufferSnd[bufPos++] = 0x01;

	uint16_t crc = computeCRC(self->bufferSnd, 16);

	/*CHK*/
	self->bufferSnd[bufPos++] = crc / 256;
	self->bufferSnd[bufPos++] = crc % 256;

	if (DEBUG)
	{
		for (int i = 0; i < 18; i++) {
			printf("%x ", self->bufferSnd[i]);
		}
		printf("\n");
	}

	int sResult = Socket_write(self->socket, self->bufferSnd, 18);

	if (DEBUG)
	{
		if (sResult != 0)
			printf("%d Stop Transmission Command Sent.\n");
	}

	self->running = false;
}

void
c37_118client_destroy(C37_118client self)
{
	Socket_destroy(self->socket);

	free(self->bufferRcv);
	free(self->bufferSnd);

	if (self->configTimeStamp.parsedTime != NULL)
		free(self->configTimeStamp.parsedTime);

	if (self->dataTimeStamp.parsedTime != NULL)
		free(self->dataTimeStamp.parsedTime);


	if (self->numPmu != 0)
	{
		for (int i = 0; i < self->numPmu; i++)
		{
			free(self->configuration[i].phasor);
			free(self->configuration[i].analog);
			free(self->configuration[i].digital);
		}

		free(self->configuration);
	}

	free(self);
}

static bool
crcCheck(uint8_t* buffer, int bufferSize)
{
	uint16_t crc = computeCRC(buffer, bufferSize - 2);

	uint16_t crcRcvd = buffer[bufferSize - 2] * 256 + buffer[bufferSize - 1];

	if (crc == crcRcvd)
		return true;
	else
		return false;
}

static int
fourByteToUint32(uint32_t* value, uint8_t* buffer, int bufPos)
{
	*value = buffer[bufPos++] * 0x1000000;
	*value += buffer[bufPos++] * 0x10000;
	*value += buffer[bufPos++] * 0x100;
	*value += buffer[bufPos++];

	return bufPos;
}

static int
threeByteToUint32(uint32_t* value, uint8_t* buffer, int bufPos)
{
	*value = buffer[bufPos++] * 0x10000;
	*value += buffer[bufPos++] * 0x100;
	*value += buffer[bufPos++];

	return bufPos;
}

static int
twoByteToUint16(uint16_t* value, uint8_t* buffer, int bufPos)
{
	*value = buffer[bufPos++] * 0x100;
	*value += buffer[bufPos++];

	return bufPos;
}

static int
decodeFloatingPointValue(float* value, uint8_t* buffer, int bufPos)
{
	float floatValue;
	uint8_t* valueBuf = (uint8_t*)&floatValue;

	int k = 0;
#if (ORDER_LITTLE_ENDIAN == 1)
	for (int i = 3; i >= 0; i--) {
		valueBuf[i] = buffer[bufPos + k];
		k++;
	}
#else
	for (int i = 0; i < 4; i++) {
		valueBuf[i] = buffer[bufPos + k];
		k++;
	}
#endif

	*value = floatValue;

	return bufPos + 4;
}

static Phasor
preparePhsor(Configuration self, uint8_t* buffer, int bufPos)
{
	Phasor phasorsPtr = (Phasor)GLOBAL_CALLOC(self->numberOfPhasors, sizeof(struct sPhasor));

	int j;
	for (j = 0; j < self->numberOfPhasors; j++)
	{
		memcpy((phasorsPtr + j)->channelName, buffer + bufPos, 16);
		bufPos += 16;
	}

	bufPos += 16 * (self->numberOfAnalogs + 16 * self->numberOfDigitals);

	uint32_t phunit;

	for (j = 0; j < self->numberOfPhasors; j++)
	{
		switch (buffer[bufPos++])
		{
		case 0:
			(phasorsPtr + j)->phasorIsVorI = true;
			break;
		case 1:
			(phasorsPtr + j)->phasorIsVorI = false;
			break;
		default:
			break;
		}

		bufPos = threeByteToUint32(&phunit, buffer, bufPos);

		if (self->phasorIsFloatingPoint)
			(phasorsPtr + j)->conversionFactor = 1;
		else
			(phasorsPtr + j)->conversionFactor = (float)phunit * 0.00001;
	}
	return phasorsPtr;
}

static Analog
prepareAnalog(Configuration self, uint8_t* buffer, int bufPos)
{
	Analog analogsPtr = (Analog)GLOBAL_CALLOC(self->numberOfAnalogs, sizeof(struct sAnalog));

	int j;
	for (j = 0; j < self->numberOfAnalogs; j++)
	{
		memcpy((analogsPtr + j)->channelName, buffer + bufPos, 16);
		bufPos += 16;
	}

	bufPos += (16 * (16 * self->numberOfDigitals) + 4 * self->numberOfPhasors);

	for (j = 0; j < self->numberOfAnalogs; j++)
	{
		(analogsPtr + j)->analogType = buffer[bufPos];
		(analogsPtr + j)->conversionFactor = 1;
		bufPos += 4;
	}
	return analogsPtr;
}

static Digital
prepareDigital(Configuration self, uint8_t* buffer, int bufPos)
{
	Digital digitalsPtr = (Digital)GLOBAL_CALLOC(self->numberOfDigitals, sizeof(struct sDigital));

	int j;
	for (j = 0; j < self->numberOfDigitals; j++)
	{
		for (int k = 0; k < 16; k++)
		{
			memcpy((digitalsPtr + j)->channel[k].channelName, buffer + bufPos, 16);
			bufPos += 16;
		}
	}
	bufPos += (4 * (self->numberOfPhasors + self->numberOfAnalogs));

	uint16_t dgTemp;

	for (j = 0; j < self->numberOfDigitals; j++)
	{
		bufPos = twoByteToUint16(&dgTemp, buffer, bufPos);
		(digitalsPtr + j)->dgMaskWord = dgTemp;

		bufPos = twoByteToUint16(&dgTemp, buffer, bufPos);
		(digitalsPtr + j)->validInputs = dgTemp;
		for (int k = 0; k < 16; k++)
		{
			if (dgTemp & 1 == 1)
				(digitalsPtr + j)->channel[k].isEnabled = true;
			dgTemp >>= 1;
		}
	}
	return digitalsPtr;
}

static Configuration
prepareConfiguration(C37_118client self, uint8_t* buffer, int bufPos)
{
	Configuration configurationPtr = (Configuration)GLOBAL_CALLOC(self->numPmu, sizeof(struct sConfiguration));

	uint16_t idCode;
	uint16_t format;
	uint16_t phnmr;
	uint16_t annmr;
	uint16_t dgnmr;
	char chnam[20];

	uint32_t digunit;
	uint16_t fnom;
	uint16_t cfgcnt;
	int16_t dataRate;

	int i;
	for (i = 0; i < self->numPmu; i++)
	{

		memcpy((configurationPtr + i)->stationName, buffer + bufPos, 16);
		bufPos += 16;

		bufPos = twoByteToUint16(&idCode, buffer, bufPos);

		(configurationPtr + i)->pmuIdcode = idCode;

		bufPos = twoByteToUint16(&format, buffer, bufPos);

		if ((format & 0x1) == 0x1)
			(configurationPtr + i)->phasorIsPolar = true;
		else
			(configurationPtr + i)->phasorIsPolar = false;

		if ((format & 0x2) == 0x2)
			(configurationPtr + i)->phasorIsFloatingPoint = true;
		else
			(configurationPtr + i)->phasorIsFloatingPoint = false;

		if ((format & 0x4) == 0x4)
			(configurationPtr + i)->analogIsFloatingPoint = true;
		else
			(configurationPtr + i)->analogIsFloatingPoint = false;

		if ((format & 0x8) == 0x8)
			(configurationPtr + i)->freqIsFloatingPoint = true;
		else
			(configurationPtr + i)->freqIsFloatingPoint = false;

		bufPos = twoByteToUint16(&phnmr, buffer, bufPos);

		(configurationPtr + i)->numberOfPhasors = phnmr;

		bufPos = twoByteToUint16(&annmr, buffer, bufPos);

		(configurationPtr + i)->numberOfAnalogs = annmr;

		bufPos = twoByteToUint16(&dgnmr, buffer, bufPos);

		(configurationPtr + i)->numberOfDigitals = dgnmr;

		int j;

		if (phnmr != 0)
		{
			(configurationPtr + i)->phasor = preparePhsor(configurationPtr, buffer, bufPos);
		}

		bufPos += 16 * configurationPtr->numberOfPhasors;

		if (annmr != 0)
		{
			(configurationPtr + i)->analog = prepareAnalog(configurationPtr, buffer, bufPos);
		}

		bufPos += 16 * configurationPtr->numberOfAnalogs;

		if (dgnmr != 0)
		{
			(configurationPtr + i)->digital = prepareDigital(configurationPtr, buffer, bufPos);
		}

		bufPos += (16 * 16 * (configurationPtr + i)->numberOfDigitals + 4 * ((configurationPtr + i)->numberOfAnalogs + (configurationPtr + i)->numberOfDigitals + (configurationPtr + i)->numberOfPhasors));

		bufPos = twoByteToUint16(&fnom, buffer, bufPos);

		switch (fnom & 0x1)
		{
		case 0:
			(configurationPtr + i)->fNom = 60;
			break;

		case 1:
			(configurationPtr + i)->fNom = 50;
			break;

		default:
			break;
		}

		bufPos = twoByteToUint16(&cfgcnt, buffer, bufPos);

		(configurationPtr + i)->cfgCnt = cfgcnt;
	}
	return configurationPtr;

}

static int
parseTimeStamp(Time* timeStamp, uint32_t timeBase, uint8_t* buffer, int bufPos)
{
	uint32_t temp, socTemp;

	bufPos = fourByteToUint32(&socTemp, buffer, bufPos);

	timeStamp->secondOfCenctury = socTemp;

	timeStamp->parsedTime = convertEpochToCalendar(socTemp);

	uint8_t timeQualityRcvd = buffer[bufPos++];

	uint8_t timeErrortemp = timeQualityRcvd & 0xF;

	switch (timeErrortemp)
	{
	case 0xF:
		timeStamp->quality.timeIsLocked = false;
		break;

	case 0xB:
		timeStamp->quality.timeError = 10;
		break;

	case 0xA:
		timeStamp->quality.timeError = 1;
		break;

	case 0x9:
		timeStamp->quality.timeError = 0.1;
		break;

	case 0x8:
		timeStamp->quality.timeError = 0.01;
		break;

	case 0x7:
		timeStamp->quality.timeError = 0.001;
		break;

	case 0x6:
		timeStamp->quality.timeError = 0.0001;
		break;

	case 0x5:
		timeStamp->quality.timeError = 0.00001;
		break;

	case 0x4:
		timeStamp->quality.timeError = 0.000001;
		break;

	case 0x3:
		timeStamp->quality.timeError = 0.0000001;
		break;

	case 0x2:
		timeStamp->quality.timeError = 0.00000001;
		break;

	case 0x1:
		timeStamp->quality.timeError = 0.000000001;
		break;

	case 0x0:
		timeStamp->quality.timeError = 0;
		timeStamp->quality.timeIsLocked = true;
		break;

	default:
		break;
	}

	timeQualityRcvd >>= 4;

	if ((timeQualityRcvd & 0x1) == 0x1)
		timeStamp->quality.leapSecondPending = true;
	else
		timeStamp->quality.leapSecondPending = false;

	timeQualityRcvd >>= 1;

	if ((timeQualityRcvd & 0x1) == 0x1)
		timeStamp->quality.leapSecondOccured = true;
	else
		timeStamp->quality.leapSecondOccured = false;

	timeQualityRcvd >>= 1;

	if ((timeQualityRcvd & 0x1) == 0x1)
		timeStamp->quality.leapSecondDirection = -1;
	else
		timeStamp->quality.leapSecondDirection = 1;

	uint32_t fracsecRcvd;

	bufPos = threeByteToUint32(&fracsecRcvd, buffer, bufPos);

	if (timeBase != 0)
	{
		timeStamp->timeValueInMs = socTemp * 1000LL + fracsecRcvd * 1000LL / timeBase;
		timeStamp->fractionOfSecond = (float)fracsecRcvd / (float)timeBase;
	}

	timeStamp->parsedTime->secPlusFraction = (float)timeStamp->parsedTime->second + timeStamp->fractionOfSecond;

	return bufPos;
}

static int
parseConfigurationFrame(C37_118client self, uint8_t* buffer, int bufferSize)
{
	if (crcCheck(buffer, bufferSize) == false)
	{
		if (DEBUG)
			printf("crc Check Error for Received Configuration message");
		return -1;
	}

	int bufPos = 0;

	if (buffer[bufPos++] != 0xAA)
	{
		if (DEBUG)
			printf("The Frame is not started with 0xAA as the first byte of SYNC word.\n");
		return -1;
	}

	if (buffer[bufPos++] != 0x31)
	{
		if (DEBUG)
			printf("The Frame is not Configuration Frame 2 Version 1 (IEEE C37.118-2005).\n");
		return -1;
	}

	uint16_t frameSizeRcdv;

	bufPos = twoByteToUint16(&frameSizeRcdv, buffer, bufPos);

	if (frameSizeRcdv != bufferSize)
	{
		if (DEBUG)
			printf("The FrameSize Word does not match with the received BufferSize.\n");
		return -1;
	}

	uint16_t idCodeRcdv;

	bufPos = twoByteToUint16(&idCodeRcdv, buffer, bufPos);


	if (idCodeRcdv != self->idcode)
	{
		if (DEBUG)
			printf("IDCODE mismatch !\n");
		return -1;
	}

	self->idcode = idCodeRcdv;

	bufPos += 8;

	uint32_t timeBaseRcvd;

	bufPos = fourByteToUint32(&timeBaseRcvd, buffer, bufPos);

	self->timeBase = timeBaseRcvd;

	bufPos -= 12;

	bufPos = parseTimeStamp(&(self->configTimeStamp), 1000, buffer, bufPos);

	bufPos += 4;

	uint16_t numPmuRcvd;

	bufPos = twoByteToUint16(&numPmuRcvd, buffer, bufPos);

	self->numPmu = numPmuRcvd;

	uint16_t dataRateRcvd;

	twoByteToUint16(&dataRateRcvd, buffer, frameSizeRcdv - 4);

	self->dataRate = dataRateRcvd;

	self->configuration = prepareConfiguration(self, buffer, bufPos);
	return 0;
}

int
c37_118client_getConfiguration(C37_118client self)
{
	memset(self->bufferSnd, 0, BUFER_LENGTH);
	memset(self->bufferRcv, 0, BUFER_LENGTH);

	/*Preparing "Send CFG-2 Frame" command*/

	int bufPos = 0;

	/*SYNC*/
	self->bufferSnd[bufPos++] = 0xAA;
	self->bufferSnd[bufPos++] = 0x41;

	/*FRAMESIZE*/
	self->bufferSnd[bufPos++] = 0x00;
	self->bufferSnd[bufPos++] = 0x12;

	/*IDCODE*/
	uint16_t idCode = self->idcode;

	self->bufferSnd[bufPos++] = idCode / 256;
	self->bufferSnd[bufPos++] = idCode % 256;

	int socPosition = bufPos;

	uint64_t timeStamp = Hal_getTimeInMs();

	uint32_t soc = timeStamp / 1000LL;
	uint32_t fracsec = timeStamp % 1000LL;

	/*SOC*/
	bufPos = encodeUInt32FixedSize(soc, self->bufferSnd, bufPos);

	int fracsecPosition = bufPos;

	/*FRACSEC*/
	bufPos = encodeUInt32FixedSize(fracsec, self->bufferSnd, bufPos);

	int comandPosition = bufPos;

	/*CMD*/
	self->bufferSnd[bufPos++] = 0x00;
	self->bufferSnd[bufPos++] = 0x05;

	uint16_t crc = computeCRC(self->bufferSnd, 16);

	/*CHK*/
	self->bufferSnd[bufPos++] = crc / 256;
	self->bufferSnd[bufPos++] = crc % 256;

	int sResult = Socket_write(self->socket, self->bufferSnd, 18);
	if (DEBUG)
	{
		if (sResult)
			printf("Get Configuration Frame 2 Command Sent.\n");
	}


	/*Receiving CFG-2 Frame*/

	do {
		sResult = Socket_read(self->socket, self->bufferRcv, BUFER_LENGTH);
	} while (sResult == 0);

	if (DEBUG)
	{
		if (sResult)
			printf("Configuration Frame Received.\n");
		for (int j = 0; j < sResult; j++) {
			printf("%x ", self->bufferRcv[j]);
		}
		printf("\n");
		printf("%d Bytes received.\n", sResult);
	}


	if (!parseConfigurationFrame(self, self->bufferRcv, sResult))
		return 0;
}

void
c37_118client_getConf_pdc(C37_118client self, uint16_t* num_pmu, uint16_t* data_rate, ParsedTime timestamp)
{
	*num_pmu = self->numPmu;
	*data_rate = self->dataRate;
	timestamp->year = self->configTimeStamp.parsedTime->year;
	timestamp->month = self->configTimeStamp.parsedTime->month;
	timestamp->day = self->configTimeStamp.parsedTime->day;
	timestamp->hour = self->configTimeStamp.parsedTime->hour;
	timestamp->minute = self->configTimeStamp.parsedTime->minute;
	timestamp->second = self->configTimeStamp.parsedTime->second;
	timestamp->secPlusFraction = self->configTimeStamp.parsedTime->secPlusFraction;
}

void
c37_118client_getConf_pmu(C37_118client self, uint16_t* numPhasors, uint16_t* numAnalog, uint16_t* numdigital, float* freqNom, uint16_t* configCount, uint16_t* idCode)
{
	for (int i = 0; i < self->numPmu; i++)
	{
		numPhasors[i] = self->configuration[i].numberOfPhasors;
		numAnalog[i] = self->configuration[i].numberOfAnalogs;
		numdigital[i] = self->configuration[i].numberOfDigitals;
		freqNom[i] = self->configuration[i].fNom;
		configCount[i] = self->configuration[i].cfgCnt;
		idCode[i] = self->configuration[i].pmuIdcode;
	}
}

void
c37_118client_getConf_pmu_stationName(C37_118client self, int pmuNumber, char* stationName)
{
	memcpy(stationName, self->configuration[pmuNumber].stationName, 16);
}

void
c37_118client_getConf_pmu_channelName(C37_118client self, int pmuNumber, int phasor, char* channelName, int* phasorType)
{
	memcpy(channelName, self->configuration[pmuNumber].phasor[phasor].channelName, 16);
	if (self->configuration[pmuNumber].phasor[phasor].phasorIsVorI)
		*phasorType = 0;
	else
		*phasorType = 1;
}

void
c37_118client_getConf_pmu_analogs(C37_118client self, int pmuNumber, int analog, char* channelName, int* analogType)
{
	memcpy(channelName, self->configuration[pmuNumber].analog[analog].channelName, 16);
	*analogType = self->configuration[pmuNumber].analog[analog].analogType;
}

/*void
c37_118client_outputConfiguration(C37_118client self, char*** channelNames, int* num_pmu, int* num_phasors)
{
	int i, j;

	*num_pmu = self->numPmu;

	num_phasors = (int*)GLOBAL_CALLOC(self->numPmu, sizeof(int));
	if (num_phasors == NULL)
	{
		fprintf(stderr, "out of memory\n");
		exit;
	}

	channelNames = (char***)GLOBAL_CALLOC(self->numPmu, sizeof(char**));
	if (channelNames == NULL)
	{
		fprintf(stderr, "out of memory\n");
		exit;
	}

	for (i = 0; i < self->numPmu; i++)
	{
		num_phasors[i] = self->configuration[i].numberOfPhasors;

		channelNames[i] = (char**)GLOBAL_CALLOC((self->configuration[i].numberOfPhasors), sizeof(char*));
		if (channelNames[i] == NULL)
		{
			fprintf(stderr, "out of memory\n");
			exit;
		}

		for (int j = 0; j < self->configuration[i].numberOfPhasors; j++)
		{
			channelNames[i][j] = (char*)GLOBAL_CALLOC(1, 16);
			memcpy(channelNames[i][j], self->configuration[i].phasor[j].channelName, 16);

		}
	}
}
*/

void
c37_118client_printConfiguration(C37_118client self)
{
	printf("PDC Configuration Data :\n");
	printf("========================\n\n");
	printf("1_Configuration Time Stamp: \n");
	printf("---------------------------\n\n");
	printf("Configuration Time Stamp: quality ->Leap Second Occured =  %s\n", self->configTimeStamp.quality.leapSecondOccured ? "true" : "false");
	printf("Configuration Time Stamp: quality ->Leap Second Pending =  %s\n", self->configTimeStamp.quality.leapSecondPending ? "true" : "false");
	printf("Configuration Time Stamp: quality ->Time is Locked =       %s\n", self->configTimeStamp.quality.timeIsLocked ? "true" : "false");
	printf("Configuration Time Stamp: quality ->Time error =           %f\n\n", self->configTimeStamp.quality.timeError);
	printf("Configuration Time Stamp: timeValueInMs =                  %llu\n", self->configTimeStamp.timeValueInMs);
	printf("Configuration Time Stamp: %d-%d-%d , %d:%d:%9.6f\n\n", self->configTimeStamp.parsedTime->year, self->configTimeStamp.parsedTime->month, self->configTimeStamp.parsedTime->day, self->configTimeStamp.parsedTime->hour, self->configTimeStamp.parsedTime->minute, self->configTimeStamp.parsedTime->secPlusFraction);

	printf("2_PDC Data: \n");
	printf("---------------------------\n");
	printf("PDC IDCODE =                %u\n", self->idcode);
	printf("Number of PMUs =            %d\n", self->numPmu);
	printf("Time Base =                 %d\n", self->timeBase);
	if (self->dataRate > 0)
		printf("Data rate =                 %d frames per second\n", self->dataRate);
	else
		printf("Data rate =                 1 frames per %d seconds\n", self->dataRate*-1);

	printf("\n3_PMUs Data: :\n");
	printf("---------------------------\n");
	for (int i = 0; i < self->numPmu; i++)
	{
		printf("3_%d_PMU %d :\n", i + 1, i + 1);
		printf(".............\n");
		printf("Configuration count =   %u\n", self->configuration[i].cfgCnt);
		printf("PMU Name =              %s\n", self->configuration[i].stationName);
		printf("PMU IDCODE =            %u\n\n", self->configuration[i].pmuIdcode);

		printf("Number of Phasors =     %u\n", self->configuration[i].numberOfPhasors);
		printf("Phasors are sent with type %s in %s format.\n\n", self->configuration[i].phasorIsFloatingPoint ? "FLOATING-POINT" : "16-BIT INTEGER", self->configuration[i].phasorIsPolar ? "POLAR" : "RECTANGULAR");

		printf("Number of Analogs =     %u\n", self->configuration[i].numberOfAnalogs);
		printf("Analogs are sent with type %s.\n\n", self->configuration[i].analogIsFloatingPoint ? "FLOATING-POINT" : "16-BIT INTEGER");

		printf("Number of Digitals =    %u\n", self->configuration[i].numberOfDigitals);

		printf("Nominal frequency =     %f\n", self->configuration[i].fNom);
		printf("Frequency & ROCOF are sent with type %s.\n\n", self->configuration[i].freqIsFloatingPoint ? "FLOATING-POINT" : "16-BIT INTEGER");

		for (int j = 0; j < self->configuration[i].numberOfPhasors; j++)
		{
			printf("Phasor %d of PMU %d -> Channel Name =           %s\n", j + 1, i + 1, self->configuration[i].phasor[j].channelName);
			printf("Phasor %d of PMU %d -> conversion factor =      %f\n", j + 1, i + 1, self->configuration[i].phasor[j].conversionFactor);
			printf("Phasor %d of PMU %d -> is Voltage =             %s\n", j + 1, i + 1, self->configuration[i].phasor[j].phasorIsVorI ? "true" : "false");
		}
		printf("\n\n");

		for (int j = 0; j < self->configuration[i].numberOfAnalogs; j++)
		{
			printf("Analog %d of PMU %d -> Channel Name =           %s\n", j + 1, i + 1, self->configuration[i].analog[j].channelName);
			printf("Analog %d of PMU %d -> Analog type =            %u\n", j + 1, i + 1, self->configuration[i].analog[j].analogType);
			printf("Analog %d of PMU %d -> conversion factor =      %f\n", j + 1, i + 1, self->configuration[i].analog[j].conversionFactor);
		}

		printf("\n\n");

		for (int j = 0; j < self->configuration[i].numberOfDigitals; j++)
		{
			for (int k = 0; k < 16; k++)
			{
				printf("Digital %d of PMU %d -> Channel Name %d =       %s\n", j + 1, i + 1, k, self->configuration[i].digital[j].channel[k].channelName);

			}
			printf("Digital %d of PMU %d -> Valid Inputs =          %u\n", j + 1, i + 1, self->configuration[i].digital[j].validInputs);
			printf("Digital %d of PMU %d -> Mask Word =             %f\n", j + 1, i + 1, self->configuration[i].digital[j].dgMaskWord);
		}
	}
}

void
c37_118client_startTransmission(C37_118client self)
{
	memset(self->bufferSnd, 0, BUFER_LENGTH);

	int bufPos = 0;

	/*SYNC*/
	self->bufferSnd[bufPos++] = 0xAA;
	self->bufferSnd[bufPos++] = 0x41;

	/*FRAMESIZE*/
	self->bufferSnd[bufPos++] = 0x00;
	self->bufferSnd[bufPos++] = 0x12;

	/*IDCODE*/
	uint16_t idCode = self->idcode;

	self->bufferSnd[bufPos++] = idCode / 256;
	self->bufferSnd[bufPos++] = idCode % 256;

	int socPosition = bufPos;

	uint64_t timeStamp = Hal_getTimeInMs();

	uint32_t soc = timeStamp / 1000LL;
	uint32_t fracsec = timeStamp % 1000LL;

	/*SOC*/
	bufPos = encodeUInt32FixedSize(soc, self->bufferSnd, bufPos);

	int fracsecPosition = bufPos;

	/*FRACSEC*/
	bufPos = encodeUInt32FixedSize(fracsec, self->bufferSnd, bufPos);

	int comandPosition = bufPos;

	/*CMD*/
	self->bufferSnd[bufPos++] = 0x00;
	self->bufferSnd[bufPos++] = 0x02;

	uint16_t crc = computeCRC(self->bufferSnd, 16);

	/*CHK*/
	self->bufferSnd[bufPos++] = crc / 256;
	self->bufferSnd[bufPos++] = crc % 256;

	int sResult = Socket_write(self->socket, self->bufferSnd, 18);

	if (DEBUG)
	{
		if (sResult != 0)
			printf("%d Start Transmission Command Sent.\n");
	}

	self->running = true;
}
/*
static void
resetConfiguration(void* threadParameter)
{
	C37_118client self = (C37_118client)threadParameter;
	c37_118client_stopTransmission(self);

	c37_118client_getConfiguration(self);

}

void
check_configuration(C37_118client c37_118Object, Iec61850_90_5Message iec61850_90_5Object)
{
	if (c37_118Object->configuration->configToChange)
		Thread thread = Thread_create((ThreadExecutionFunction)resetConfiguration, (void*)c37_118Object, true);
}
*/

static void
c37_118ReceiverLoop(void* threadParameter)
{
	C37_118client self = (C37_118client)threadParameter;

	c37_118client_startTransmission(self);

	while (self->running)
	{
		if (c37_118client_getDataStream(self) == 0)
			c37_118client_printDataStream(self);
		else
			printf("\n\n\n...........................Frame missed..............................\n\n\n");
		Thread_sleep(1);
	}

	c37_118client_stopTransmission(self);
}

//void
//c37_118client_startReceiveDataStram(C37_118client self)
//{
//	Thread thread = Thread_create((ThreadExecutionFunction)c37_118ReceiverLoop, (void*)self, true);
//
//	if (thread != NULL)
//	{
//		if (DEBUG)
//			printf("C37_118 Receiver : Started receiving");
//		Thread_start(thread);
//	}
//	else
//	{
//		if (DEBUG)
//			printf("C37_118 Receiver : Failed to start receiving");
//	}
//}

static int
parseSTATWord(Configuration self, uint8_t* buffer, int bufPos)
{
	uint16_t tempSTAT;
	bufPos = twoByteToUint16(&tempSTAT, buffer, bufPos);

	self->statWord = tempSTAT;

	self->triggerReason = tempSTAT & 0x1111;
	tempSTAT >>= 4;

	self->unlockTime = tempSTAT & 0x11;
	tempSTAT >>= 2;

	self->pmuTimeQuality = tempSTAT & 0x111;
	tempSTAT >>= 3;

	if ((tempSTAT & 0x1) == 0x1)
		self->dataModified = true;
	tempSTAT >>= 1;

	if ((tempSTAT & 0x1) == 0x1)
		self->configToChange = true;
	tempSTAT >>= 1;

	if ((tempSTAT & 0x1) == 0x1)
		self->pmuIsTriggered = true;
	tempSTAT >>= 1;

	if ((tempSTAT & 0x1) == 0x1)
		self->dataSortingByArrivalorTime = true;
	tempSTAT >>= 1;

	if ((tempSTAT & 0x1) == 0x1)
		self->pmuIsNOTSync = true;
	tempSTAT >>= 1;

	self->dataError = tempSTAT & 0x11;

	return bufPos;

}

static int
parseDataFrame(C37_118client self, uint8_t* buffer, int bufferSize)
{
	if (crcCheck(buffer, bufferSize) == false)
	{
		if (DEBUG)
			printf("crc Check Error for Received Configuration message");
		return -1;
	}

	int bufPos = 0;

	if (buffer[bufPos++] != 0xAA)
	{
		if (DEBUG)
			printf("The Frame is not started with 0xAA as the first byte of SYNC word.\n");
		return -1;
	}

	if (buffer[bufPos++] != 0x01)
	{
		if (DEBUG)
			printf("The Frame is not Data Frame Version 1 (IEEE C37.118-2005).\n");
		return -1;
	}

	uint16_t frameSizeRcdv;

	bufPos = twoByteToUint16(&frameSizeRcdv, buffer, bufPos);

	if (frameSizeRcdv != bufferSize)
	{
		if (DEBUG)
			printf("The FrameSize Word does not match with the received BufferSize.\n");
		return -1;
	}


	uint16_t idCodeRcdv;

	bufPos = twoByteToUint16(&idCodeRcdv, buffer, bufPos);

	if (idCodeRcdv != self->idcode)
	{
		if (DEBUG)
			printf("The received ID CODE Word does not match.\n");
		return -1;
	}

	bufPos = parseTimeStamp(&(self->dataTimeStamp), self->timeBase, buffer, bufPos);

	uint16_t tempUint16_1;
	uint16_t tempUint16_2;
	float tempFloat_1;
	float tempFloat_2;

	for (int i = 0; i < self->numPmu; i++)
	{
		bufPos = parseSTATWord(&(self->configuration[i]), buffer, bufPos);

		for (int j = 0; j < self->configuration[i].numberOfPhasors; j++)
		{
			if (self->configuration[i].phasorIsFloatingPoint)
			{
				if (self->configuration[i].phasorIsPolar)
				{
					bufPos = decodeFloatingPointValue(&tempFloat_1, buffer, bufPos);
					bufPos = decodeFloatingPointValue(&tempFloat_2, buffer, bufPos);

					self->configuration[i].phasor[j].magnitude = tempFloat_1;
					self->configuration[i].phasor[j].angle = tempFloat_2;
				}
				else
				{
					bufPos = decodeFloatingPointValue(&tempFloat_1, buffer, bufPos);
					bufPos = decodeFloatingPointValue(&tempFloat_2, buffer, bufPos);

					self->configuration[i].phasor[j].magnitude = sqrt(pow(tempFloat_1, 2) + pow(tempFloat_2, 2));
					self->configuration[i].phasor[j].angle = atan2(tempFloat_2, tempFloat_1) * 180 / M_PI;
				}
			}
			else
			{
				if (self->configuration[i].phasorIsPolar)
				{
					bufPos = twoByteToUint16(&tempUint16_1, buffer, bufPos);
					bufPos = twoByteToUint16(&tempUint16_2, buffer, bufPos);

					self->configuration[i].phasor[j].magnitude = (float)tempFloat_1 * self->configuration[i].phasor[j].conversionFactor;
					self->configuration[i].phasor[j].angle = (float)tempFloat_2 * self->configuration[i].phasor[j].conversionFactor;
				}
				else
				{
					bufPos = twoByteToUint16(&tempUint16_1, buffer, bufPos);
					bufPos = twoByteToUint16(&tempUint16_2, buffer, bufPos);

					self->configuration[i].phasor[j].magnitude = sqrt(pow((float)tempUint16_1, 2) + pow((float)tempUint16_2, 2))*self->configuration[i].phasor[j].conversionFactor;
					self->configuration[i].phasor[j].angle = atan2((float)tempUint16_2, (float)tempUint16_1) * 180 / M_PI;
				}
			}
		}

		if (self->configuration[i].freqIsFloatingPoint)
		{
			bufPos = decodeFloatingPointValue(&tempFloat_1, buffer, bufPos);
			self->configuration[i].frequency = tempFloat_1;
		}
		else
		{
			bufPos = twoByteToUint16(&tempUint16_1, buffer, bufPos);
			self->configuration[i].frequency = self->configuration[i].fNom + (float)tempUint16_1 * 0.001;
		}

		if (self->configuration[i].freqIsFloatingPoint)
		{
			bufPos = decodeFloatingPointValue(&tempFloat_1, buffer, bufPos);
			self->configuration[i].rocof = tempFloat_1 * 0.01;
		}
		else
		{
			bufPos = twoByteToUint16(&tempUint16_1, buffer, bufPos);
			self->configuration[i].frequency = (float)tempUint16_1 * 0.01;
		}

		for (int j = 0; j < self->configuration[i].numberOfAnalogs; j++)
		{
			if (self->configuration[i].analogIsFloatingPoint)
			{
				bufPos = decodeFloatingPointValue(&tempFloat_1, buffer, bufPos);
				self->configuration[i].analog[j].value = tempFloat_1;
			}
			else
			{
				bufPos = twoByteToUint16(&tempUint16_1, buffer, bufPos);
				self->configuration[i].analog[j].value = (float)tempUint16_1 * self->configuration[i].analog[j].conversionFactor;
			}
		}

		for (int j = 0; j < self->configuration[i].numberOfDigitals; j++)
		{
			bufPos = twoByteToUint16(&tempUint16_1, buffer, bufPos);
			self->configuration[i].digital[j].digitalWord = tempUint16_1;
			for (int k = 0; k < 16; k++)
			{
				if (tempUint16_1 & 1 == 1)
					self->configuration[i].digital[j].channel[k].status = true;
				tempUint16_1 >>= 1;
			}

		}
	}
	return 0;
}

int
c37_118client_getDataStream(C37_118client self)
{
	memset(self->bufferRcv, 0, BUFER_LENGTH);

	int sResult;

	do {
		sResult = Socket_read(self->socket, self->bufferRcv, BUFER_LENGTH);
	} while (sResult == 0);
	if (DEBUG)
	{
		for (int j = 0; j < sResult; j++) {
			printf("%x ", self->bufferRcv[j]);
		}
		printf("\n");
		printf("%d Bytes received.\n", sResult);
	}

	//if (!parseDataFrame(self, self->bufferRcv, sResult))
	//return 0;

	return parseDataFrame(self, self->bufferRcv, sResult);
}

void
c37_118client_printDataStream(C37_118client self)
{
	int i, j;
	printf("\n----------------------------------------------------------\n");
	printf("Data Frame Time Stamp: quality ->Leap Second Occured = %s\n", self->dataTimeStamp.quality.leapSecondOccured ? "TRUE" : "FALSE");
	printf("Data Frame Time Stamp: quality ->Leap Second Pending = %s\n", self->dataTimeStamp.quality.leapSecondPending ? "TRUE" : "FALSE");
	printf("Data Frame Time Stamp: quality ->Time is Locked =      %s\n", self->dataTimeStamp.quality.timeIsLocked ? "TRUE" : "FALSE");
	printf("Data Frame Time Stamp: quality ->Time error =          %f\n\n", self->dataTimeStamp.quality.timeError);
	printf("Data Frame Time Stamp: timeValueInMs =                 %llu\n", self->dataTimeStamp.timeValueInMs);
	printf("Data Time Stamp: %d-%d-%d , %d:%d:%9.6f\n\n", self->dataTimeStamp.parsedTime->year, self->dataTimeStamp.parsedTime->month, self->dataTimeStamp.parsedTime->day, self->dataTimeStamp.parsedTime->hour, self->dataTimeStamp.parsedTime->minute, self->dataTimeStamp.parsedTime->secPlusFraction);

	for (i = 0; i < self->numPmu; i++)
	{
		printf("Data of PMU No.%d :\n", i + 1);
		printf("=====================\n\n");
		printf("Status of Data Frame:\n");
		printf("---------------------\n\n");
		printf("1. Data Error:                     ");
		switch (self->configuration[i].dataError)
		{
		case 0:
		printf("GOOD MEASUREMENT DATA, NO ERRORS\n");
			break;
		case 1:
			printf("PMU ERROR. NO INFORMATION ABOUT DATA\n");
			break;
		case 2:
			printf("PMU IN TEST MODE OR ABSENT DATA TAGS HAVE BEEN INSERTED (DO NOT USE VALUES)\n");
			break;
		case 3:
			printf("PMU ERROR. (DO NOT USE VALUES)\n");
			break;
		default:
			break;
		}

		printf("2. Is PMU syncronised with a UTC:  %s \n", self->configuration[i].pmuIsNOTSync ? "FALSE" : "TRUE");
		printf("3. PMU data sorting metrice:       %s \n", self->configuration[i].dataSortingByArrivalorTime ? "ARRIVAL" : "TIME STAMP");
		printf("4. Is PMU triggered:               %s \n", self->configuration[i].pmuIsTriggered ? "TRUE" : "FALSE");
		printf("5. Is PMU configuration to change: %s \n", self->configuration[i].configToChange ? "TRUE" : "FALSE");
		printf("6. Is PMU data modified:           %s \n", self->configuration[i].dataModified ? "TRUE" : "FALSE");
		printf("7. PMU Time Quality:               ");
		switch (self->configuration[i].pmuTimeQuality)
		{
		case 0:
			printf("NOT USED\n");
			break;
		case 1:
			printf("ESTIMATED MAXIMUM TIME ERROR < 100 ns\n");
			break;
		case 2:
			printf("ESTIMATED MAXIMUM TIME ERROR < 1 micro SECOND\n");
			break;
		case 3:
			printf("ESTIMATED MAXIMUM TIME ERROR < 10 micro SECOND\n");
			break;
		case 4:
			printf("ESTIMATED MAXIMUM TIME ERROR < 100 micro SECOND\n");
			break;
		case 5:
			printf("ESTIMATED MAXIMUM TIME ERROR < 1 ms\n");
			break;
		case 6:
			printf("ESTIMATED MAXIMUM TIME ERROR < 10 ms\n");
			break;
		case 7:
			printf("ESTIMATED MAXIMUM TIME ERROR > 10 ms OR TIME ERROR UNKNOWN\n");
			break;
		default:
			break;
		}
		printf("8. PMU Unlocked time:              ");

		switch (self->configuration[i].unlockTime)
		{
		case 0:
			printf("SYNC LOCKED OR UNLOCKED <10s (BEST QUALITY)\n");
			break;
		case 1:
			printf("10 s =< UNLOCKED TIME < 100 s\n");
			break;
		case 2:
			printf("100 s < UNLOCKED TIME =< 1000 s\n");
			break;
		case 3:
			printf("UNLOCKED TIME > 1000 s\n");
		default:
			break;
		}

		if (self->configuration[i].pmuIsTriggered)
		{
			printf("9. PMU Trigger reason:             ");
			switch (self->configuration[i].pmuTimeQuality)
			{
			case 0:
				printf("MANUAL\n");
				break;
			case 1:
				printf("MAGNITUDE LOW\n");
				break;
			case 2:
				printf("MAGNITUDE HIGH\n");
				break;
			case 3:
				printf("PHASE ANGLE DIFF\n");
				break;
			case 4:
				printf("FREQUENCY HIGH OR LOW\n");
				break;
			case 5:
				printf("df/dt HIGH\n");
				break;
			case 7:
				printf("DIGITAL\n");
				break;

			default:
				printf("USER DEFINED\n");
				break;
			}
		}

		printf("\nValues of Data Frame:\n");
		printf("---------------------\n\n");

		printf("Frequency of PMU %d = %f\n", i + 1, self->configuration[i].frequency);
		printf("ROCOF     of PMU %d = %f\n\n", i + 1, self->configuration[i].rocof);
		for (int j = 0; j < self->configuration[i].numberOfPhasors; j++)
		{
			printf("Phasor %d of PMU %d -> Channel Name = %s\n", j + 1, i + 1, self->configuration[i].phasor[j].channelName);
			printf("Phasor %d of PMU %d -> Magnitude    = %f\n", j + 1, i + 1, self->configuration[i].phasor[j].magnitude);
			printf("Phasor %d of PMU %d -> Angle        = %f\n\n", j + 1, i + 1, self->configuration[i].phasor[j].angle);
		}

		for (int j = 0; j < self->configuration[i].numberOfAnalogs; j++)
		{
			printf("Analog %d of PMU %d -> Channel Name = %s\n", j + 1, i + 1, self->configuration[i].analog[j].channelName);
			printf("Analog %d of PMU %d -> Value = %u\n\n", j + 1, i + 1, self->configuration[i].analog[j].value);
		}

		for (int j = 0; j < self->configuration[i].numberOfDigitals; j++)
		{
			for (int k = 0; k < 16; k++)
			{
				printf("Channel %d of Digital %d of PMU %d -> Channel Name = %s\n", k, j + 1, i + 1, self->configuration[i].digital[j].channel[k].channelName);
				printf("Channel %d of Digital %d of PMU %d -> is Enable    = %s\n", k, j + 1, i + 1, self->configuration[i].digital[j].channel[k].isEnabled ? "TURE" : "FALSE");
				printf("Channel %d of Digital %d of PMU %d -> Status       = %s\n\n", k, j + 1, i + 1, self->configuration[i].digital[j].channel[k].status ? "ON" : "OFF");
			}
		}
	}
}

/*void
c37_118client_outputDataStream(C37_118client self, ParsedTime timestamp, float* freqBuffer, float* rocofBuffer, float** phasorsBuffer)
{
	int i, j;

	freqBuffer = (float*)GLOBAL_CALLOC(self->numPmu, sizeof(float));
	if (freqBuffer == NULL)
		{
			fprintf(stderr, "out of memory\n");
			exit;
		}

	rocofBuffer = (float*)GLOBAL_CALLOC(self->numPmu, sizeof(float));
	if (rocofBuffer == NULL)
	{
		fprintf(stderr, "out of memory\n");
		exit;
	}

	phasorsBuffer = (float**)GLOBAL_CALLOC(self->numPmu, sizeof(float*));
	if (phasorsBuffer == NULL)
	{
		fprintf(stderr, "out of memory\n");
		exit;
	}

	for (i = 0; i < self->numPmu; i++)
	{

		phasorsBuffer[i] = (float*)GLOBAL_CALLOC((self->configuration[i].numberOfPhasors)*2, sizeof(float));
		if (phasorsBuffer[i] == NULL)
		{
			fprintf(stderr, "out of memory\n");
			exit;
		}

		//printf("Frequency of PMU %d = %f\n", i + 1, self->configuration[i].frequency);
		freqBuffer[i] = self->configuration[i].frequency;
		//printf("ROCOF     of PMU %d = %f\n\n", i + 1, self->configuration[i].rocof);
		rocofBuffer[i] = self->configuration[i].rocof;

		for (int j = 0; j < (2 * self->configuration[i].numberOfPhasors); j += 2)
		{
		//	channelNames[i][j] = (char*)GLOBAL_CALLOC(1, 16);
		//	memcpy(channelNames[i][j], self->configuration[i].phasor[j].channelName,16);

			//printf("Phasor %d of PMU %d -> Channel Name = %s\n", j + 1, i + 1, self->configuration[i].phasor[j].channelName);
			phasorsBuffer[i][(j/2)] = self->configuration[i].phasor[(j/2)].magnitude;
			//printf("Phasor %d of PMU %d -> Magnitude    = %f\n", j + 1, i + 1, self->configuration[i].phasor[j].magnitude);
			phasorsBuffer[i][(j/2) + 1] = self->configuration[i].phasor[(j/2)+1].angle;
			//printf("Phasor %d of PMU %d -> Angle        = %f\n\n", j + 1, i + 1, self->configuration[i].phasor[j].angle);
		}
	}
}
*/

void
c37_118client_outputDataStream(C37_118client self, ParsedTime timestamp, float* freqBuffer, float* rocofBuffer, float* phasorsMagBuffer, float* phasorsAngBuffer, float* analogBuffer)
{

	timestamp->year = self->dataTimeStamp.parsedTime->year;
	timestamp->month = self->dataTimeStamp.parsedTime->month;
	timestamp->day = self->dataTimeStamp.parsedTime->day;
	timestamp->hour = self->dataTimeStamp.parsedTime->hour;
	timestamp->minute = self->dataTimeStamp.parsedTime->minute;
	timestamp->second = self->dataTimeStamp.parsedTime->second;
	timestamp->secPlusFraction = self->dataTimeStamp.parsedTime->secPlusFraction;

	int k = 0;
	int m = 0;
	for (int i = 0; i < self->numPmu; i++)
	{
		freqBuffer[i] = self->configuration[i].frequency;
		rocofBuffer[i] = self->configuration[i].rocof;

		for (int j = 0; j < self->configuration[i].numberOfPhasors; j++)
		{
			phasorsMagBuffer[k] = self->configuration[i].phasor[j].magnitude;
			phasorsAngBuffer[k] = self->configuration[i].phasor[j].angle;
			k++;
		}

		for (int j = 0; j < self->configuration[i].numberOfAnalogs; j++)
		{
			analogBuffer[m] = self->configuration[i].analog[j].value;
			m++;
		}

		/*status[i].dataError = self->configuration[i].dataError;
		status[i].dataModified = self->configuration[i].dataModified;
		status[i].pmuIsNOTSync = self->configuration[i].pmuIsNOTSync;
		status[i].pmuIsTriggered = self->configuration[i].pmuIsTriggered;
		status[i].pmuTimeQuality = self->configuration[i].pmuTimeQuality;
		status[i].triggerReason = self->configuration[i].triggerReason;
		status[i].unlockTime = self->configuration[i].unlockTime;*/
	}
}

void
c37_118client_outputDataStream_status(C37_118client self, int pmuNumber, Status* status)
{
	status->dataError = self->configuration[pmuNumber].dataError;
	status->dataModified = self->configuration[pmuNumber].dataModified;
	status->pmuIsNOTSync = self->configuration[pmuNumber].pmuIsNOTSync;
	status->pmuIsTriggered = self->configuration[pmuNumber].pmuIsTriggered;
	status->pmuTimeQuality = self->configuration[pmuNumber].pmuTimeQuality;
	status->triggerReason = self->configuration[pmuNumber].triggerReason;
	status->unlockTime = self->configuration[pmuNumber].unlockTime;
}


