/*
 * Project 2
 * Created: 30/03/2020 
 * Author : Rory Brennan
 */ 

void sendmsg (char *s);

// Initialization Functions
void init_USART(void);
void InitPorts(void);
void InitTimer0(void);
void InitTimer2(void);
void InitADC(void);

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>

#define T0_16ms (uint8_t)774
#define BAUD_RATE 16 // 115200
unsigned char qcntr = 0,sndcntr = 0;   /*indexes into the queue*/
unsigned char queue[50];       /*character queue*/
uint32_t adc_reading; // ADC reading variable
unsigned int adc_mode = 1; // ADC mode selection variable
unsigned int adc_flag = 0; // ADC reading available flag
unsigned int IsStringSent = 0; // USART control variable

int main(void)
{
	char ch;  /* character variable for received character*/
	char message [50]; // Message buffer
	
	// Initialization functions 
	InitPorts();
	InitTimer0();
	InitTimer2();
	InitADC();
	init_USART();
	
	sei(); /*global interrupt enable */
	
	unsigned int contmode = 0; // Continuous mode flag
	
	while (1) // Continuous Loop
	{
		if (UCSR0A & (1<<RXC0)) /*check for character received*/
		{
			ch = UDR0;    /*get character sent from PC*/
			adc_flag = 0; // Reset flag
			switch (ch)
			{
				case '0':
					OCR2A = 0; // LED Off
				break;
				case '1':
					OCR2A = 26; // 10% Brightness
				break;
				case '2':
					OCR2A = 51; // 20% Brightness
				break;
				case '3':
					OCR2A = 77; // 30% Brightness
				break;
				case '4':
					OCR2A = 102; // 40% Brightness
				break;
				case '5':
					OCR2A = 128; // 50% Brightness
				break;
				case '6':
					OCR2A = 154; // 60% Brightness
				break;
				case '7':
					OCR2A = 179; // 70% Brightness
				break;
				case '8':
					OCR2A = 205; // 80% Brightness
				break;
				case '9':
					OCR2A = 230; // 90% Brightness
				break;
				case 's':
				case 'S':
					sprintf(message, "OCR2A: %i (%i%% Brightness)", OCR2A, (OCR2A*100)/255); // Print value of 0CR2A and its % brightness
					sendmsg(message);
				break;
				case 'A':
				case 'a':			
					sprintf(message, "ADC: %li",adc_reading); // Print value of ADC no matter what source just as is
					sendmsg(message);
				break;
				case 'V':
				case 'v':
					sprintf(message, "ADC: %li mV",(adc_reading*5000)/1023); // Print ADC reading in millivolts no matter what source
					sendmsg(message);
				break;
				case 'L':
				case 'l':
					if(adc_mode == 1) // LDR
					{
						if(adc_reading > 512) // If bright
						{
							sprintf(message, "Bright");
							sendmsg(message);
						}
						else // else dark
						{
							sprintf(message, "Dark");
							sendmsg(message);
						}
					}
					else
					{
						sprintf(message, "LDR not selected!"); // Throw error to show LDR is not selected as input to ADC
						sendmsg(message);
					}
				break;
				case 'T':
				case 't':
					if(adc_mode == 2) // Check if LM35 is selected
					{
						sprintf(message, "Temp: %li °C",(adc_reading*500)/1023); // Print temperature in degrees Celsius
						sendmsg(message);
					}
					else
					{
						sprintf(message, "TempSensor not selected!"); // Throw error showing incorrect ADC input
						sendmsg(message);
					}
				break;
				case 'C':
				case 'c':					
					sprintf(message,"CM:"); // Begin continuous mode
					sendmsg(message);
					contmode = 1; // Set continuous mode flag													
				break;				
				case 'E':
				case 'e':
					contmode = 0; // Clear continuous mode flag	
					sprintf(message, "Stopping continuous mode...."); // Stop printing values
					sendmsg(message);
				break;
				case 'P':
				case 'p': // ADC0
					contmode = 0;
					adc_mode = 0; 
					sprintf(message, "Enabling Potentiometer....");
					sendmsg(message);
				break;
				case 'N':
				case 'n': // ADC1
					contmode = 0;
					adc_mode = 1;
					sprintf(message, "Enabling LDR....");
					sendmsg(message);
				break;
				case 'M':
				case 'm': // ADC2
					contmode = 0;
					adc_mode = 2;				
					sprintf(message, "Enabling Temperature sensor....");
					sendmsg(message);
				break;
			}
		}
		
		if(adc_flag == 1 && contmode == 1) // Loop for continuous mode
		{
			if(IsStringSent == 1) // Check if last string is fully sent
			{
				if(adc_mode == 0) // ADC0
				{
					sprintf(message, "ADC: %li mV",(adc_reading*5000)/1023); // Output result in millivolts
					sendmsg(message);	
				}
				else if(adc_mode == 1) // ADC1
				{
					if(adc_reading > 512) // If bright
					{
						sprintf(message, "Bright"); // Output result (Bright or Dark)
						sendmsg(message);
					}
					else // else dark
					{
						sprintf(message, "Dark");
						sendmsg(message);
					}
				}
				else if(adc_mode == 2) // ADC2
				{
					sprintf(message, "Temp: %li °C",(adc_reading*500)/1023); // Output result in °C
					sendmsg(message);					
				}
			}
			adc_flag = 0; // Get new ADC value
		}
	}
	return 1; // End
}

/****************************/
/* Initialization functions */ 
/****************************/

void init_USART()
{
	UCSR0A	= (1<<U2X0); // Double speed
	UCSR0B	= (1<<RXEN0) | (1<<TXEN0) | (1<<TXC0);  /*enable receiver, transmitter and transmit interrupt*/
	UCSR0C =  (3<<UCSZ00) | (0<<UPM00); /* disable parity, set to 8 bits */
	UBRR0 = BAUD_RATE;  /*baud rate = 115200*/
}
void InitPorts(void)
{
	DDRB = (8<<PORTB); // Set PORTB bit 3 as output for PWM
}
void InitTimer0(void)
{
	TCCR0B = (5<<CS00);	    /* Set T0 Source = Clock (16MHz)/1024 and put Timer in Normal mode	*/
	TCCR0A = 0;
	TCNT0 = T0_16ms;			/* Recall: 1024-774 = 250 & 250*64us = 16 ms*/
	TIMSK0 = (0<<TOIE0);    /* Disable Timer 0 interrupt (Already initial value of 0) */	
}
void InitTimer2(void)
{
	TCCR2B = (4<<CS00);	    /* Set T2 Source = Clock (16MHz)/256 and put Timer in Normal mode	*/
	TCCR2A = ((2<<COM2A0)|(1<<WGM20)); /* Enable 8 bit PWM (TOP = 0xFF), Enable OC2A clear on match while up counting */	
	OCR2A = 0; // Default Value = LED Off 
}
void InitADC(void)
{
	ADMUX = ((1<<REFS0)|(1<<MUX0)|(0<<ADLAR));  /* AVCC selected for VREF, ADC1 as initial ADC input, 10 bit mode  */
	ADCSRA = ((1<<ADEN)|(1<<ADATE)|(1<<ADIE)|(7<<ADPS0));  /* Enable ADC, Start Conversion, Auto Trigger enabled,
																		Interrupt enabled, Prescale = 128  */
	ADCSRB = (4<<ADTS0); /* Select AutoTrigger Source to Timer 0 Overflow */
}

/*************************************/
/* USART sendmsg function			 */
/* this function loads the queue and */
/* starts the sending process		 */
/*************************************/

void sendmsg (char *s)
{
	qcntr = 0;    /*preset indices*/
	sndcntr = 1;  /*set to one because first character already sent*/
	
	queue[qcntr++] = 0x0d;   /*put CRLF into the queue first*/
	queue[qcntr++] = 0x0a;
	while (*s)
	queue[qcntr++] = *s++;   /*put characters into queue*/
	
	UDR0 = queue[0];  /*send first character to start process*/
}


/*****************************/
/* Interrupt Service Routine */
/*****************************/

/*this interrupt occurs whenever the */
/*USART has completed sending a character*/
ISR(USART_TX_vect)
{
	/*send next character and increment index*/
	if(qcntr != sndcntr)
	{
		UDR0 = queue[sndcntr++];
		IsStringSent = 0; // String is not finished sending
	}
	else if(qcntr == sndcntr) // Check if string is at end
	{
		IsStringSent = 1; // String is finished sending, move onto next
	}	
}

ISR(ADC_vect) // handles ADC interrupts
{
	if(adc_mode == 0) // ADC0
	{
		ADMUX = ((1<<REFS0)|(0<<MUX0)|(0<<ADLAR)); // Setting input to ADC0
		adc_flag = 0; // Reset flag
	}
	else if(adc_mode == 1) // ADC1
	{
		ADMUX = ((1<<REFS0)|(1<<MUX0)|(0<<ADLAR)); // Setting input to ADC1
		adc_flag = 0; // Reset flag
	}
	else if(adc_mode == 2) // ADC2
	{
		ADMUX = ((1<<REFS0)|(2<<MUX0)|(0<<ADLAR)); // Setting input to ADC2
		adc_flag = 0; // Reset flag
	}	
	if(adc_flag == 0) // Check if new ADC data is available
	{
		adc_reading = ADC; // Read in new ADC value		 			
		adc_flag = 1; // Set reading available flag			
	}		
	TIFR0 = 0b001; // Clear Timer0 Overflow flag
}



