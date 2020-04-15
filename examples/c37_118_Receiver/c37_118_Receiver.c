//#include <math.h>
//#include <stdint.h>
//#include <stdlib.h>
#include <string.h>
//#include <stdbool.h>
//#include <stdio.h>
//#include <conio.h>
//#include <signal.h>

#include "ieeeC37_118.h"
//#include "pal_thread.h"

int
main(int argc, char** argv)
{
	PMUConnectionData pmuConnectionData;

	strcpy(pmuConnectionData.ipAddress_PmuServer, "130.237.53.177");
	pmuConnectionData.pmuIdCode = 221;
	pmuConnectionData.pmuPortNumber = 33221;

	C37_118client c37_118Object = c37_118client_connect(&pmuConnectionData, 1000);

	c37_118client_stopTransmission(c37_118Object);
	
	c37_118client_getConfiguration(c37_118Object);

	c37_118client_printConfiguration(c37_118Object);

	c37_118client_startTransmission(c37_118Object);

	while(true)
	{
		c37_118client_getDataStream(c37_118Object);
		c37_118client_printDataStream(c37_118Object);
	}
	
	c37_118client_destroy(c37_118Object);

	return 0;
}




