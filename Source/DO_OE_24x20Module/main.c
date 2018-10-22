#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>


int main(void)
{
	DDRA = 1 << PORTA6;
	
	/* Replace with your application code */
	while (1)
	{
		PORTA = 1 << PORTA6;
		_delay_ms(500);
		PORTA = 0;
		_delay_ms(500);
	}
}