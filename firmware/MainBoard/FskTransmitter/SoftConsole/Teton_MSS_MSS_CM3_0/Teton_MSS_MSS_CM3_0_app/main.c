#include <Teton_hw_platform.h>

#include <mss_uart.h>
#include <mss_gpio.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "fsk_tx.h"
#include "joshua.h"


//#define MSS_GPIO_AFE1_T_RN_MASK MSS_GPIO_10_MASK
#define MSS_GPIO_AFE1_SHDN_MASK MSS_GPIO_0_MASK
#define MSS_GPIO_LED1_MASK MSS_GPIO_1_MASK

uint32_t CmdHelp(uint32_t argc, char** argv);
uint32_t CmdLed(uint32_t argc, char** argv);
uint32_t CmdAfe(uint32_t argc, char** argv);
uint32_t CmdFsk(uint32_t argc, char** argv);
uint32_t CmdTx(uint32_t argc, char** argv);
uint32_t CmdAmpl(uint32_t argc, char** argv);
uint32_t CmdJoshua(uint32_t argc, char** argv);
uint32_t CmdIQ(uint32_t argc, char** argv);
uint32_t CmdPath(uint32_t argc, char** argv);
uint32_t CmdIfFreq(uint32_t argc, char** argv);
// MAX 2830 related
uint32_t CmdReg(uint32_t argc, char** argv);
uint32_t CmdFreq(uint32_t argc, char** argv);
uint32_t CmdGain(uint32_t argc, char** argv);
uint32_t CmdMode(uint32_t argc, char** argv);


typedef struct cmd_struct
{
	char* CmdString;
	uint32_t (*CmdFunction)(uint32_t argc, char** argv);
} cmd_t;

// List of command words and associated functions
cmd_t cmd_list[] =
{
	"help", CmdHelp,
	"led",  CmdLed,
	"afe",  CmdAfe,
	"fsk",  CmdFsk,
	"tx",   CmdTx,
	"ampl", CmdAmpl,
	"j",    CmdJoshua,
	"iq",   CmdIQ,
	"path",  CmdPath,
	"if", CmdIfFreq,
	"reg",  CmdReg,
	"freq", CmdFreq,
	"gain", CmdGain,
	"mode", CmdMode,
	NULL,   NULL
};

cmd_t* cmd_list_ptr;
char* argList[10];

#define RX_BUFF_SIZE 64
#define CMD_BUFF_SIZE 256


uint8_t cmd_buff[CMD_BUFF_SIZE];
uint8_t cmd_buff_idx = 0;

char* cmd_token;

uint32_t argc;
uint8_t cmd_parse_result;

void process_cmd_buff(uint8_t* cmd_buff, uint8_t length);
void process_rx_data (uint8_t* rx_buff, uint8_t length)
;


//-----------------------------------------------------------------------------


int main()
{
    uint8_t rx_size;
    uint8_t uart_buff[RX_BUFF_SIZE];

	MSS_GPIO_init();

	MSS_GPIO_config(MSS_GPIO_0, MSS_GPIO_OUTPUT_MODE);
	MSS_GPIO_config(MSS_GPIO_1, MSS_GPIO_OUTPUT_MODE);

	//MSS_GPIO_set_outputs( MSS_GPIO_get_outputs() | MSS_GPIO_LED1_MASK );
	MSS_GPIO_set_outputs( MSS_GPIO_get_outputs() & ~MSS_GPIO_LED1_MASK );
	//MSS_GPIO_set_outputs( MSS_GPIO_get_outputs() | MSS_GPIO_LED1_MASK );
	MSS_GPIO_set_outputs( MSS_GPIO_get_outputs() & ~MSS_GPIO_AFE1_SHDN_MASK );

    MSS_UART_init( &g_mss_uart0, MSS_UART_9600_BAUD,
                   MSS_UART_DATA_8_BITS | MSS_UART_NO_PARITY | MSS_UART_ONE_STOP_BIT );

    FSK_TX_init( FSK_38400_BAUD, 150000, 0 );
	FSK_TX->MUX = 1;

    Joshua_init( );

	while( 1 )
	{
		rx_size = MSS_UART_get_rx( &g_mss_uart0, uart_buff, RX_BUFF_SIZE );
		if ( rx_size > 0 )
		{
			// Echo message
			while ( !MSS_UART_tx_complete(&g_mss_uart0) );
			MSS_UART_polled_tx( &g_mss_uart0, uart_buff, rx_size );

			while ( !MSS_UART_tx_complete(&g_mss_uart0) );
			process_rx_data( uart_buff, rx_size );
		}

	}
}


//-----------------------------------------------------------------------------

void process_cmd_buff(uint8_t* cmd_buff, uint8_t length)
{
	cmd_parse_result = 1;

	// Tokenize command buffer content
	cmd_token = strtok((char *)cmd_buff, " ");
	if (cmd_token != NULL)
	{
		cmd_list_ptr = cmd_list;
		while (cmd_list_ptr->CmdString)
		{
			if (!strcmp(cmd_token, cmd_list_ptr->CmdString))
			{
				// Set the argList pointers to tokens
				argc = 0;

				while ( cmd_token != NULL && argc < 10 )
				{
					argList[argc++] = cmd_token;
					cmd_token = strtok(NULL, " ");
				}

				while ( !MSS_UART_tx_complete(&g_mss_uart0) );
				MSS_UART_polled_tx_string( &g_mss_uart0, "\r\nACK" );

				// Invoke associated command function
				cmd_list_ptr->CmdFunction(argc, argList);

				cmd_parse_result = 0;
				break;
			}

			cmd_list_ptr++;
		}
	}

	// Send NAK if no valid command identified
	if (cmd_parse_result == 1)
	{
		while ( !MSS_UART_tx_complete(&g_mss_uart0) );
		MSS_UART_polled_tx_string( &g_mss_uart0, "\r\nNAK" );
	}

	// Send '>' prompt
	while ( !MSS_UART_tx_complete(&g_mss_uart0) );
	MSS_UART_polled_tx_string( &g_mss_uart0, "\r\n>" );
}

void process_rx_data (uint8_t* rx_buff, uint8_t length)
{
	uint8_t i;
	for ( i = 0; i < length; i++ )
	{
		if (*(rx_buff+i) == '\r')
		{
			cmd_buff[cmd_buff_idx] = '\0';
			process_cmd_buff(cmd_buff, cmd_buff_idx);
			cmd_buff_idx = 0;
		}
		else // if (*(rx_buff+i) != '\n')
		{
			cmd_buff[cmd_buff_idx++] = rx_buff[i];
		}
	}
}

/*-----------------------------------------------------------------------------
 *                              Command functions
 *-----------------------------------------------------------------------------*/


uint32_t CmdHelp(uint32_t argc, char** argv)
{
	char buf[128];
	cmd_t* cmdListItr = cmd_list;

	SystemCoreClockUpdate();
	while ( !MSS_UART_tx_complete(&g_mss_uart0) );
	sprintf(buf, "\r\nClocks:\r\nMSS 0:\t%4u Hz\r\nMSS 1:\t%4u Hz\r\nFPGA:\t%4u Hz\r\n",
			g_FrequencyPCLK0, g_FrequencyPCLK1, g_FrequencyFPGA );
	MSS_UART_polled_tx_string( &g_mss_uart0, buf );

	while ( !MSS_UART_tx_complete(&g_mss_uart0) );
	MSS_UART_polled_tx_string( &g_mss_uart0, "\r\nAvailable commands:\r\n" );

	while (cmdListItr->CmdString)
	{
		MSS_UART_polled_tx_string( &g_mss_uart0, "\r\n  " );
		MSS_UART_polled_tx_string( &g_mss_uart0, cmdListItr->CmdString );
		cmdListItr++;
	}
	MSS_UART_polled_tx_string( &g_mss_uart0, "\r\n" );

	return 0;
}

uint32_t CmdLed(uint32_t argc, char** argv)
{
	while ( !MSS_UART_tx_complete(&g_mss_uart0) );

	if (argc == 2) {
		if (!strcmp(*(argv+1), "on"))
		{
			MSS_GPIO_set_outputs( MSS_GPIO_get_outputs() | MSS_GPIO_LED1_MASK );
			MSS_UART_polled_tx_string( &g_mss_uart0, (uint8_t*)"\r\nLED is ON");
			return 0;
		}

		if (!strcmp(*(argv+1), "off"))
		{
			MSS_GPIO_set_outputs( MSS_GPIO_get_outputs() & ~MSS_GPIO_LED1_MASK );
			MSS_UART_polled_tx_string( &g_mss_uart0, "\r\nLED is OFF");
			return 0;
		}
	}

	// Send help message
	MSS_UART_polled_tx_string( &g_mss_uart0, "\r\nUsage: led [on | off]");
	return 1;
}

uint32_t CmdAfe(uint32_t argc, char** argv)
{
	while ( !MSS_UART_tx_complete(&g_mss_uart0) );

	if (argc == 2)
	{
		if (!strcmp(*(argv+1), "on"))
		{
			MSS_GPIO_set_outputs( MSS_GPIO_get_outputs() & ~MSS_GPIO_AFE1_SHDN_MASK );
			MSS_UART_polled_tx_string( &g_mss_uart0, "\r\nAFE is ON");
			return 0;
		}

		if (!strcmp(*(argv+1), "off"))
		{
			MSS_GPIO_set_outputs( MSS_GPIO_get_outputs() | MSS_GPIO_AFE1_SHDN_MASK );
			MSS_UART_polled_tx_string( &g_mss_uart0, "\r\nAFE is OFF");
			return 0;
		}

	}

	// Send help message
	MSS_UART_polled_tx_string( &g_mss_uart0, "\r\nUsage: afe [ on | off ]");
	return 1;
}


uint32_t CmdFsk(uint32_t argc, char** argv)
{
	uint32_t dphase;
	//uint32_t payload;
	char parse_buf[128];

	while ( !MSS_UART_tx_complete(&g_mss_uart0) );

	if (argc == 2)
	{
		if (!strcmp(*(argv+1), "on"))
		{
			dphase = FSK_TX->CTRL;
			FSK_TX->CTRL = 1;
			dphase = FSK_TX->CTRL;

			if (FSK_TX->CTRL & 1)
			{
				MSS_UART_polled_tx_string( &g_mss_uart0, "\r\nfsk is ON");
			}
			else
			{
				MSS_UART_polled_tx_string( &g_mss_uart0, "\r\nError: fsk is OFF");
			}
			return 0;
		}

		if (!strcmp(*(argv+1), "off"))
		{
			FSK_TX->CTRL = 0;

			if (FSK_TX->CTRL == 0)
			{
				MSS_UART_polled_tx_string( &g_mss_uart0, "\r\nfsk is OFF");
			}
			else
			{
				MSS_UART_polled_tx_string( &g_mss_uart0, "\r\nError: fsk is ON");
			}
			return 0;
		}

		dphase = atoi(*(argv+1));
		if (dphase || !strcmp(*(argv+1), "0")) // TODO: use errno
		{
			FSK_TX->DPHA = dphase;

			sprintf(parse_buf, "\r\nParsed: %d (0x%08x)", (int)dphase, (uint32_t)dphase);
			MSS_UART_polled_tx_string( &g_mss_uart0, parse_buf );
			return 0;
		}
	}

	while ( !MSS_UART_tx_complete(&g_mss_uart0) );

	sprintf( parse_buf, "\r\nStatus: fsk is %s\r\ndphase = %8d (0x%08x)\r\nampl = %8d (0x%08x)", FSK_TX->CTRL ? "ON" : "OFF",
			(uint32_t)FSK_TX->DPHA, (uint32_t)FSK_TX->DPHA,
			(uint32_t)FSK_TX->AMPL, (uint32_t)FSK_TX->AMPL );

	// Send status
	MSS_UART_polled_tx_string( &g_mss_uart0, parse_buf );
	while ( !MSS_UART_tx_complete(&g_mss_uart0) );

	// Send help message
	MSS_UART_polled_tx_string( &g_mss_uart0, "\r\nUsage: fsk [on | off | <dphase>]");
	return 1;
}



uint32_t CmdTx(uint32_t argc, char** argv)
{
	uint32_t payload;
	uint8_t parse_buf[128];

	while ( !MSS_UART_tx_complete(&g_mss_uart0) );

	if (argc == 2)
	{
		payload = atoi(*(argv+1));
		if (payload || !strcmp(*(argv+1), "0")) // TODO: use errno
		{
			sprintf((char*)parse_buf, "\r\nParsed: %u (0x%08x)", (unsigned int)payload, (unsigned int)payload);
			MSS_UART_polled_tx_string( &g_mss_uart0, parse_buf );

			FSK_TX_transmit(payload);

			//FSK_TX->DPHA = ((uint64_t)1000 << 32)/g_FrequencyFPGA;;
			return 0;
		}
	}

	while ( !MSS_UART_tx_complete(&g_mss_uart0) );

	// Send help message
	MSS_UART_polled_tx_string( &g_mss_uart0, (uint8_t*)"\r\nUsage: tx <payload>]");
	return 1;
}





uint32_t CmdAmpl(uint32_t argc, char** argv)
{
	uint32_t ampl;
	char buf[128];

	if (argc == 1)
	{
		sprintf(buf, "\r\nAmplitude: %d mVpp", FSK_TX_get_amplitude());
		MSS_UART_polled_tx_string( &g_mss_uart0, buf );
		return 0;
	}

	while ( !MSS_UART_tx_complete(&g_mss_uart0) );

	if (argc == 2)
	{
		ampl = atoi(*(argv+1));
		if (ampl || !strcmp(*(argv+1), "0")) // TODO: use errno instead
		{
			FSK_TX_set_amplitude(ampl);

			//sprintf(buf, "\r\nParsed: %8d", (int)ampl);
			sprintf(buf, "\r\nAmplitude: %8d mVpp", (int)FSK_TX_get_amplitude());
			MSS_UART_polled_tx_string( &g_mss_uart0, (uint8_t*)buf );
			return 0;
		}
	}

	while ( !MSS_UART_tx_complete(&g_mss_uart0) );

	// Send status
	sprintf( buf, "\r\nampl = %8d mVpp", FSK_TX_get_amplitude() );
	MSS_UART_polled_tx_string( &g_mss_uart0, buf );
	while ( !MSS_UART_tx_complete(&g_mss_uart0) );

	// Send help message
	MSS_UART_polled_tx_string( &g_mss_uart0, "\r\nUsage: ampl [<amplitude value in mV>]");
	return 1;
}






uint32_t CmdJoshua(uint32_t argc, char** argv)
{
	while ( !MSS_UART_tx_complete(&g_mss_uart0) );

	if (argc == 2)
	{
		if (!strcmp(*(argv+1), "on"))
		{
			MSS_GPIO_set_outputs( MSS_GPIO_get_outputs() | MSS_GPIO_SHDN_MASK );	// Shutdown
			return 0;
		}

		if (!strcmp(*(argv+1), "off"))
		{
			MSS_GPIO_set_outputs( MSS_GPIO_get_outputs() & ~MSS_GPIO_SHDN_MASK );	// Shutdown
			return 0;
		}
	}

	// Send help message
	MSS_UART_polled_tx_string( &g_mss_uart0, "\r\nUsage: j [on | off | <dphase>]");
	return 1;
}




uint32_t CmdPath(uint32_t argc, char** argv)
{
	char buf[128];
	uint32_t path;

	while ( !MSS_UART_tx_complete(&g_mss_uart0) );

	if (argc == 1)
	{
		sprintf(buf, "\r\nActive path: %s", FSK_TX->MUX ? "CONST" : "FSK");
		MSS_UART_polled_tx_string( &g_mss_uart0, (uint8_t*)buf );
		return 0;
	}


	if (argc == 2)
	{
		//FSK_TX->MUX = (uint32_t)1;
		//return 0;

		path = atoi(*(argv+1));
		if (path == 0)
		{
			FSK_TX->MUX = 0;
			sprintf(buf, "\r\nActive path: %s", FSK_TX->MUX ? "CONST" : "FSK");
			while ( !MSS_UART_tx_complete(&g_mss_uart0) );
			MSS_UART_polled_tx_string( &g_mss_uart0, (uint8_t*)buf );
			return 0;
		}
		else if (path == 1)
		{
			FSK_TX->MUX = 1;
			sprintf(buf, "\r\nActive path: %s", FSK_TX->MUX ? "CONST" : "FSK");
			while ( !MSS_UART_tx_complete(&g_mss_uart0) );
			MSS_UART_polled_tx_string( &g_mss_uart0, (uint8_t*)buf );
			return 0;
		}
	}

	// Send status
	while ( !MSS_UART_tx_complete(&g_mss_uart0) );
	sprintf( buf, "\r\nActive path: %s", FSK_TX->MUX ? "CONST" : "FSK");
	MSS_UART_polled_tx_string( &g_mss_uart0, (uint8_t*)buf );

	// Send help message
	while ( !MSS_UART_tx_complete(&g_mss_uart0) );
	MSS_UART_polled_tx_string( &g_mss_uart0, (uint8_t*)"\r\nUsage: path [ 0 | 1 ]\t0 : FSK\t1 : CONST");

	return 1;
}

uint32_t CmdIQ(uint32_t argc, char** argv)
{
	char buf[128];
	uint32_t i, q;

	while ( !MSS_UART_tx_complete(&g_mss_uart0) );


	if (argc == 1)
	{
		sprintf(buf, "\r\nI : %d\t0x%03x\r\nQ : %d\t0x%03x",
				(int)FSK_TX->I, (unsigned int)FSK_TX->I,
				(int)FSK_TX->Q, (unsigned int)FSK_TX->Q);
		MSS_UART_polled_tx_string( &g_mss_uart0, (uint8_t*)buf );
		return 0;
	}

	while ( !MSS_UART_tx_complete(&g_mss_uart0) );

	if (argc == 3)
	{
		i = atoi(*(argv+1));
		q = atoi(*(argv+2));
		if ((i || !strcmp(*(argv+1), "0")) && (q || !strcmp(*(argv+2), "0")))
		{

			//!
			//FSK_TX->MUX = (uint32_t)1;

			FSK_TX->I = i;
			FSK_TX->Q = q;

			// Readback
			sprintf(buf, "\r\nI : %d\t0x%03x\r\nQ : %d\t0x%03x",					(int)FSK_TX->I, (unsigned int)FSK_TX->I,
					(int)FSK_TX->Q, (unsigned int)FSK_TX->Q);
			MSS_UART_polled_tx_string( &g_mss_uart0, (uint8_t*)buf );
			return 0;
		}
	}

	while ( !MSS_UART_tx_complete(&g_mss_uart0) );

	// Send status
	sprintf( buf, "\r\nActive path: %s", FSK_TX->MUX ? "CONST" : "FSK");
	MSS_UART_polled_tx_string( &g_mss_uart0, (uint8_t*)buf );
	while ( !MSS_UART_tx_complete(&g_mss_uart0) );

	// Send help message
	MSS_UART_polled_tx_string( &g_mss_uart0, (uint8_t*)"\r\nUsage: path [0 | 1]\t where 0 : FSK, 1 : CONST");
	return 1;
}

uint32_t CmdIfFreq(uint32_t argc, char** argv)
{
	char buf[128];
	uint32_t center_freq, separation_freq;

	while ( !MSS_UART_tx_complete(&g_mss_uart0) );


	if (argc == 1)
	{
		sprintf(buf, "\r\nLow : %d\tHigh : %d",	delta_phase_low, delta_phase_high);
		MSS_UART_polled_tx_string( &g_mss_uart0, (uint8_t*)buf );
		return 0;
	}

	while ( !MSS_UART_tx_complete(&g_mss_uart0) );

	if (argc == 3)
	{
		center_freq = atoi(*(argv+1));
		separation_freq = atoi(*(argv+2));
		if ((center_freq || !strcmp(*(argv+1), "0")) && (separation_freq || !strcmp(*(argv+2), "0")))
		{
			FSK_TX_set_if_freq(center_freq, separation_freq);

			// Readback
			sprintf(buf, "\r\nLow : %d\tHigh : %d",	delta_phase_low, delta_phase_high);
			MSS_UART_polled_tx_string( &g_mss_uart0, (uint8_t*)buf );
			return 0;
		}
	}

	while ( !MSS_UART_tx_complete(&g_mss_uart0) );

	// Send help message
	MSS_UART_polled_tx_string( &g_mss_uart0, (uint8_t*)"\r\nUsage: iffreq <center freq> <separation freq>");
	return 1;
}



uint32_t CmdReg(uint32_t argc, char** argv)
{
	uint32_t addr;
	uint32_t data;

	char buf[128];

	while ( !MSS_UART_tx_complete(&g_mss_uart0) );

	if (argc == 2)
	{
		addr = atoi(*(argv+1));
		if (addr || !strcmp(*(argv+1), "0"))
		{
			data = Max2830_read_register(addr);
			sprintf(buf, "\r\nAddr: %2d (0x%02x) Data: %3d (0x%04x) (R)",
					(unsigned int)addr,	(unsigned int)addr,
					(unsigned int)data,	(unsigned int)data);
			MSS_UART_polled_tx_string( &g_mss_uart0, (uint8_t*)buf );
			return 0;
		}
	}

	if (argc == 3)
	{
		addr = atoi(*(argv+1));
		if (addr || !strcmp(*(argv+1), "0"))
		{
			data = atoi(*(argv+2));
			if (data || !strcmp(*(argv+2), "0"))
			{
				Max2830_write_register(addr, data);
				sprintf(buf, "\r\nAddr: %2d (0x%02x) Data: %3d (0x%04x) (W)",
						(unsigned int)addr,	(unsigned int)addr,
						(unsigned int)data,	(unsigned int)data);
				MSS_UART_polled_tx_string( &g_mss_uart0, (uint8_t*)buf );
				return 0;
			}
		}
	}

	while ( !MSS_UART_tx_complete(&g_mss_uart0) );

	// Send help message
	MSS_UART_polled_tx_string( &g_mss_uart0, (uint8_t*)"\r\nUsage: reg <address> [<data>]");
	return 1;
}


uint32_t CmdFreq(uint32_t argc, char** argv)
{
	double freq;
	char buf[128];

	while ( !MSS_UART_tx_complete(&g_mss_uart0) );

	if (argc == 1)
	{
		sprintf(buf, "\r\nFrequency: %12.6f MHz", (double)Max2830_get_frequency()/1e6);
		MSS_UART_polled_tx_string( &g_mss_uart0, (uint8_t*)buf );

		/*
		while ( !MSS_UART_tx_complete(&g_mss_uart0) );
		sprintf(buf, "\r\nFrequency: %u Hz", Max2830_get_frequency());
		MSS_UART_polled_tx_string( &g_mss_uart0, buf );
		*/

		return 0;
	}

	if (argc == 2)
	{
		freq = atof(*(argv+1));
		if (freq || !strcmp(*(argv+1), "0"))
		{
			/*
			sprintf(buf, "\r\nFrequency: %u Hz (passing)", (uint32_t) (freq*1e6));
			MSS_UART_polled_tx_string( &g_mss_uart0, buf );
			sprintf(buf, "\r\nFrequency: %12.6f MHz", (float)Max2830_get_frequency()/1e6);
			*/

			Max2830_set_frequency((uint32_t) (freq*1e6));

			sprintf(buf, "\r\nFrequency: %12.6f MHz", (double)Max2830_get_frequency()/1e6);
			MSS_UART_polled_tx_string( &g_mss_uart0, (uint8_t*)buf );

			/*
			while ( !MSS_UART_tx_complete(&g_mss_uart0) );
			sprintf(buf, "\r\nFrequency: %u Hz (R)", Max2830_get_frequency());
			MSS_UART_polled_tx_string( &g_mss_uart0, buf );
			*/

			return 0;
		}
	}

	while ( !MSS_UART_tx_complete(&g_mss_uart0) );

	// Send help message
	MSS_UART_polled_tx_string( &g_mss_uart0, "\r\nUsage: freq [<frequency value in MHz>]");
	return 1;
}


uint32_t CmdGain(uint32_t argc, char** argv)
{
	float gain;
	char buf[128];

	while ( !MSS_UART_tx_complete(&g_mss_uart0) );

	if (argc == 1)
	{
		sprintf(buf, "\r\nTx gain: %4.1f dB", Max2830_get_tx_gain());
		MSS_UART_polled_tx_string( &g_mss_uart0, buf );
		return 0;
	}

	if (argc == 2)
	{
		gain = atof(*(argv+1));
		if (atof || !strcmp(*(argv+1), "0")) // TODO: use errno
		{
			Max2830_set_tx_gain(gain);

			sprintf(buf, "\r\nTx gain: %4.1f dB", gain);
			MSS_UART_polled_tx_string( &g_mss_uart0, (uint8_t*)buf );
			return 0;
		}
	}

	while ( !MSS_UART_tx_complete(&g_mss_uart0) );

	// Send help message
	MSS_UART_polled_tx_string( &g_mss_uart0, "\r\nUsage: gain [<gain in dB>]");
	return 1;
}


uint32_t CmdMode(uint32_t argc, char** argv)
{
	while ( !MSS_UART_tx_complete(&g_mss_uart0) );

	if (argc == 2)
	{
		if (!strcmp(*(argv+1), "shdn"))
		{
			Max2830_set_mode( MAX2830_SHUTDOWN_MODE );
			return 0;
		}

		if (!strcmp(*(argv+1), "idle"))
		{
			Max2830_set_mode( MAX2830_STANDBY_MODE );
			return 0;
		}

		if (!strcmp(*(argv+1), "rx"))
		{
			Max2830_set_mode( MAX2830_RX_MODE );
			return 0;
		}

		if (!strcmp(*(argv+1), "tx"))
		{
			Max2830_set_mode( MAX2830_TX_MODE );
			return 0;
		}

		if (!strcmp(*(argv+1), "rxc"))
		{
			Max2830_set_mode( MAX2830_RX_CALIBRATION_MODE );
			return 0;
		}

		if (!strcmp(*(argv+1), "txc"))
		{
			Max2830_set_mode( MAX2830_TX_CALIBRATION_MODE );
			return 0;
		}
	}

	// Send help message
	MSS_UART_polled_tx_string( &g_mss_uart0, (uint8_t*)"\r\nUsage: mode [shdn | idle | rx | tx | rxc | txc ]");
	return 1;
}
