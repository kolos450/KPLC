#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>


int main(void)
{
	DDRD = 1 << PORTD6;
	
	/* Replace with your application code */
	while (1)
	{
		PORTD = 1 << PORTD6;
		_delay_ms(500);
		PORTD = 0;
		_delay_ms(500);
	}
}

