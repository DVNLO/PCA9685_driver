#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <cmath>
//#include "<path_to>\I2c_dev_interface.h"
#include "../../../I2c_dev_interface/I2c_dev_interface/I2c_dev_interface/I2c_dev_interface.h"

#define PCA9685_REG_MODE1			0x00
#define PCA9685_REG_MODE2			0x01
#define PCA9685_REG_SUBADDR1		0x02
#define PCA9685_REG_SUBADDR2		0x03
#define PCA9685_REG_SUBADDR3		0x04
#define PCA9685_REG_ALLCALLADDR		0x05

#define PCA9685_REG_LEDX_ON_L		0x06
#define PCA9685_REG_LEDX_ON_H		0x07
#define PCA9685_REG_LEDX_OFF_L		0x08
#define PCA9685_REG_LEDX_OFF_H		0x09

#define PCA9685_REG_LEDN_ON_L(N)	((PCA9685_REG_LEDX_ON_L) + (4 * N))
#define PCA9685_REG_LEDN_ON_H(N)	((PCA9685_REG_LEDX_ON_H) + (4 * N))
#define PCA9685_REG_LEDN_OFF_L(N)	((PCA9685_REG_LEDX_OFF_L) + (4 * N))
#define PCA9685_REG_LEDN_OFF_H(N)	((PCA9685_REG_LEDX_OFF_H) + (4 * N))

#define PCA9685_REG_ALL_LED_ON_L	0xFA
#define PCA9685_REG_ALL_LED_ON_H	0xFB
#define PCA9685_REG_ALL_LED_OFF_L	0xFC
#define PCA9685_REG_ALL_LED_OFF_H	0xFD
#define PCA9685_REG_PRESCALE		0xFE

#define PCA9685_SWRST				0x06

#define PCA9685_PRESCALE_MIN		0x03	//max. frequency of 1526 Hz
#define PCA9685_PRESCALE_MAX		0xFF	//min. frequency of 24 Hz
#define PCA9685_FREQ_MIN			24
#define PCA9685_FREQ_MAX			1526

#define PCA9685_COUNTER_RANGE		4096
#define PCA9685_DEFAULT_PERIOD		5000000		//Default period_ns = 1/200 Hz
#define PCA9685_OSC_CLOCK_MHZ		25000000	//Internal oscillator; 25 MHz

#define PCA9685_NUMREGS				0xFF
#define PCA9685_MAXCHAN				0x10	//

#define PCA9685_LED_FULL_ON			(1 << 4) //0000 0001 << 4 = 0001 0000
#define PCA9685_MODE1_RESTART		(1 << 7) //0000 0001 << 7 = 1000 0000
#define PCA9685_MODE1_SLEEP			(1 << 4) 
#define PCA9685_MODE2_INVERT		(1 << 4) 


class PCA9685
{
public:
	PCA9685();
	PCA9685(char* bus_filename_in, int slave_addr_in);
	PCA9685 operator=(const PCA9685& source);

	void setup(char* bus_filename_in, int slave_addr_in);
	void setup();

	int get_bus_file() const;
	char* get_bus_filename() const;
	unsigned long long get_bus_funcs() const;
	int get_slave_addr() const;

	void set_pwm_frequency(const double& new_frequency);
	void set_pwm_prescale(const unsigned char& new_prescale_value);
	unsigned char get_prescale_value() const;

	void set_duty_cycle(const unsigned int& channel, const unsigned int& duty_on_percent);
	void set_duty_cycle(const unsigned int& channel, const unsigned int& delay,
		const unsigned int& duty_cycle);
	void set_full_on(const unsigned int& channel);
	void set_full_off(const unsigned int& channel);
	unsigned get_duty_cycle(const unsigned int& channel);

	void delay_ms(const unsigned int& interval);

	friend int i2c::open_bus(const char* bus_filename);
	friend unsigned long long i2c::probe_bus_funcs(const int& bus_file);
	friend bool i2c::verify_bus_func(const unsigned long long bus_funcs,
		const unsigned long long func_constant);
	friend void i2c::init_comms(const int& bus_file, const int& slave_addr);
	friend void i2c::write_out(const int& bus_file, const unsigned char& byte);
	friend void i2c::write_out(const int& bus_file, const unsigned char& reg, const unsigned char& byte);
	friend unsigned char i2c::read_in(const int& bus_file);
	friend unsigned char i2c::read_in(const int& bus_file, const unsigned char& reg);
	friend void PCA9685_software_reset(const char* i2c_bus_filename);
private:
	unsigned int compute_time(const unsigned int& percent);

	int bus_file;
	char* bus_filename;
	unsigned long long bus_funcs;
	int slave_addr;
};

void PCA9685_software_reset(const char* i2c_bus_filename);

