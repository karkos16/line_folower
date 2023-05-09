#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdbool.h>

#define SENSOR1 PC1
#define SENSOR2 PC0
#define SENSOR3 PC2
#define SENSOR4 PC3

#define LED PD0
#define LEFT_MOTOR_FORWARD PD5
#define LEFT_MOTOR_BACKWARD PD4
#define LEFT_MOTOR_SPEED OCR1B
#define RIGHT_MOTOR_SPEED OCR1A
#define RIGHT_MOTOR_FORWARD PD6
#define RIGHT_MOTOR_BACKWARD PD7
// Checking sensors, 1 means that sensor didn't detect black

void setupSensors() {
	ADCSRA |= (
				(1 << ADEN) //Enable ADC
	);

	ADMUX |= (
				(1 << REFS0)
	); // Set ADC Reference as Vcc

	ADCSRA |= (
				(1 << ADPS0) |
				(1 << ADPS1) |
				(1 << ADPS2)
	); // Prescaler 128;

	ADMUX &= ~((1 << MUX3) | (1 << MUX2) | (1 << MUX1) | (1 << MUX0)); // Set bits MUX3:0 to zero

}

bool leftEdgeSensor() {
	ADMUX &= ~((1 << MUX3) | (1 << MUX2) | (1 << MUX1) | (1 << MUX0));
	ADMUX |= (1 << MUX0); // Set PC1 for ADC input
	ADCSRA |= (1<<ADSC); // Start conversion
	while (ADCSRA & (1 << ADSC)) {
		// wait for result
	}
	if (ADC > 300) return true;
	return false;
}

bool leftSensor() {
	ADMUX &= ~((1 << MUX3) | (1 << MUX2) | (1 << MUX1) | (1 << MUX0)); // Set PC0 for ADC input
	ADCSRA |= (1<<ADSC); // Start conversion
	while (ADCSRA & (1 << ADSC)) {
		// wait for result
	}
	if (ADC > 300) return true;
	return false;
}

bool rightEdgeSensor() {
	ADMUX &= ~((1 << MUX3) | (1 << MUX2) | (1 << MUX1) | (1 << MUX0));
	ADMUX |= ( (1 << MUX1) | (1 << MUX0) );// Set PC3 for ADC input
	ADCSRA |= (1<<ADSC); // Start conversion
	while (ADCSRA & (1 << ADSC)) {
		// wait for result
	}
	if (ADC > 300) return true;
	return false;
}

bool rightSensor() {
	ADMUX &= ~((1 << MUX3) | (1 << MUX2) | (1 << MUX1) | (1 << MUX0));
	ADMUX |= (1 << MUX1);// Set PC2 for ADC input
	ADCSRA |= (1<<ADSC); // Start conversion
	while (ADCSRA & (1 << ADSC)) {
		// wait for result
	}
	if (ADC > 300) return true;
	return false;
}


uint8_t readSensors() {
	uint8_t sensors_values = 0b0000;

	if(leftEdgeSensor()) sensors_values |= (1 << 3);
	if(leftSensor()) sensors_values |= (1 << 2);
	if(rightSensor()) sensors_values |= (1 << 1);
	if(rightEdgeSensor()) sensors_values |= (1 << 0);

	return sensors_values;
}

void setupPWMandMOTORS() {

//	Setting pins D4-7 as output and PD0 for LED
	DDRD |= (
				(1 << LED) |
				(1 << LEFT_MOTOR_FORWARD) |
				(1 << LEFT_MOTOR_BACKWARD) |
				(1 << RIGHT_MOTOR_FORWARD) |
				(1 << RIGHT_MOTOR_BACKWARD));

//	Setting pins B1-2 as output for PWM
	DDRB |= (
			(1 << PB1) |
			(1 << PB2) );

//	Motors always enable
	PORTD |= (
				(1 << LEFT_MOTOR_FORWARD) |
				(1 << RIGHT_MOTOR_FORWARD));

//	Setting PWM on Timer/Counter 1 in 8bit mode
	TCCR1A |= (1 << WGM10) |
			  (1 << COM1A1) |
			  (1 << COM1B1);
	TCCR1B |= (1 << CS10) | (1 << WGM12);

//	Enable interrupts
	sei();
}

void forward() {
	PORTD &= ~(
			(1 << LEFT_MOTOR_BACKWARD) |
			(1 << RIGHT_MOTOR_BACKWARD) |
			(1 << LEFT_MOTOR_FORWARD) |
			(1 << RIGHT_MOTOR_FORWARD)
			);
	PORTD |= (
			(1 << LEFT_MOTOR_FORWARD) |
			(1 << RIGHT_MOTOR_FORWARD)
			);
}

void left() {
	PORTD &= ~(
				(1 << LEFT_MOTOR_BACKWARD) |
				(1 << RIGHT_MOTOR_BACKWARD) |
				(1 << LEFT_MOTOR_FORWARD) |
				(1 << RIGHT_MOTOR_FORWARD)
				);
	PORTD |= (
			(1 << LEFT_MOTOR_BACKWARD) |
			(1 << RIGHT_MOTOR_FORWARD)
			);
}

void right() {
	PORTD &= ~(
				(1 << LEFT_MOTOR_BACKWARD) |
				(1 << RIGHT_MOTOR_BACKWARD) |
				(1 << LEFT_MOTOR_FORWARD) |
				(1 << RIGHT_MOTOR_FORWARD)
				);
	PORTD |= (
			(1 << LEFT_MOTOR_FORWARD) |
			(1 << RIGHT_MOTOR_BACKWARD)
			);
}


int controlMotors(uint8_t sensorsBits) {
	unsigned char STOP = 0;

//  No black - full speed
	if (sensorsBits == 0b1111) {
		forward();
		RIGHT_MOTOR_SPEED = 120;
		LEFT_MOTOR_SPEED = 136;
	}
//	Black on right edge sensor - right motor
	else if (sensorsBits == 0b1110) {
		right();
		RIGHT_MOTOR_SPEED = (160 - 15);
		LEFT_MOTOR_SPEED = (192 - 15);
		_delay_ms(20);
	}
	else if (sensorsBits == 0b1100 || sensorsBits == 0b1000) {
		right();
		RIGHT_MOTOR_SPEED = (165 - 15);
		LEFT_MOTOR_SPEED = (192 - 15);
	}
//	Black on middle right sensor
	else if (sensorsBits == 0b1101) {
		forward();
		RIGHT_MOTOR_SPEED = (130 - 15);
		LEFT_MOTOR_SPEED = (160 - 15);
	}
//	Black on left edge sensor
	else if (sensorsBits == 0b0111) {
		left();
		RIGHT_MOTOR_SPEED = (192 - 15);
		LEFT_MOTOR_SPEED = (160 - 15);
		_delay_ms(20);
	}
	else if (sensorsBits == 0b0011 || sensorsBits == 0b0001) {
		left();
		RIGHT_MOTOR_SPEED = (192 - 15);
		LEFT_MOTOR_SPEED = 165;
	}
//	Black in left sensor
	else if (sensorsBits == 0b1011) {
		forward();
		RIGHT_MOTOR_SPEED = (160 - 15);
		LEFT_MOTOR_SPEED = (130 - 15);
	}
//	Black on all sensors - STOP
	else if (sensorsBits == 0b0000) {
		RIGHT_MOTOR_SPEED = STOP;
		LEFT_MOTOR_SPEED = STOP;
		_delay_ms(500);
		return 1;

	}
	return 0;
}


int main(void)
{

	setupPWMandMOTORS();
	setupSensors();

	while (1)
	{
	   uint8_t sensors = readSensors();

//	   Turn on led if black wasn't detected
	  if (sensors != 0b1111) {
		  PORTD &= ~(1 << PD0);
	  } else PORTD |= (1 << PD0);

	   controlMotors(sensors);
	   _delay_ms(6);
	}
}
