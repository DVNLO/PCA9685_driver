#include "PCA9685_driver.h"

/*
Construction of a PCA9685 object when initialization paramaters are void.
post-condition: PCA9685 object initialized with private data members == 0.
*/
PCA9685::PCA9685() {
	bus_file = 0;
	bus_filename = 0;
	bus_funcs = 0;
	slave_addr = 0;
	printf("Log: PCA9685 void construction.\n");
}

/*
Construction of a PCA9685 object when initialization paramaters are specified.
A followup call to setup_i2c() is unecessary. Object setup is complete.
post-condition: PCA9685 object initialized with private data members assigned.
*/
PCA9685::PCA9685(char* bus_filename_in, int slave_addr_in) {
	bus_file = 0;
	bus_funcs = 0;
	bus_filename = bus_filename_in;
	slave_addr = slave_addr_in;
	printf("Log: PCA9685 parameter construction.\n");
	setup();
}

/*
Assignment of a PCA9685 object from source PCA9685 object. Initialization
paramaters are assigned from the source object. Returned object setup is complete.
post-condition: PCA9685 object assigned and initialized with private data members
from the source object.
*/
PCA9685 PCA9685::operator=(const PCA9685& source) {
	PCA9685 product;
	product.setup(source.bus_filename, source.slave_addr);
	return product;
	printf("Log: PCA9685 assignment via operator=.\n");
}


/*
Opens and intializes the i2c bus for communication with a slave device when an
object was constructed with void parameters.
*/
void PCA9685::setup(char* bus_filename_in, int slave_addr_in) {
	if (bus_file == 0 && bus_filename == 0 && slave_addr == 0 &&
		bus_funcs == 0)
	{
		bus_filename = bus_filename_in;
		slave_addr = slave_addr_in;
		setup();
	}
	else {
		throw std::runtime_error("Error: invalid pre-condition for object setup().\n");
	}
}

/*
Opens, verifies, and intializes the i2c bus for communication with a slave device.
*/
void PCA9685::setup() {
	if (bus_file == 0 && bus_funcs == 0) {
		bus_file = i2c::open_bus(bus_filename);
		bus_funcs = i2c::probe_bus_funcs(bus_file);
		if (!(i2c::verify_bus_func(bus_funcs, I2C_FUNC_I2C))) {
			throw std::runtime_error("Error: I2C_FUNC_I2C not supported.\n");
		}
		i2c::init_comms(bus_file, slave_addr);
	}
	printf("Log: PCA9685 setup().\n");
}

/*
post-condition: returns the calling object's bus_file. Warns if bus_file member
== 0; implies object setup incomplete.
*/
int PCA9685::get_bus_file() const {
	if (bus_file == 0) {
		printf("Warning: from get_bus_file();  setup() incomplete.\n");
	}
	return bus_file;
}

/*
post-condition: returns the calling object's bus_filename. Warns if bus_filename
member == 0; implies object setup incomplete.
*/
char* PCA9685::get_bus_filename() const {
	if (bus_filename == 0) {
		printf("Warning: from get_bus_filename(); setup() incomplete.\n");
	}
	return bus_filename;
}

/*
post-condition: returns the calling object's bus_funcs. Warns if the bus_funcs
member == 0; implies object setup incomplete.
*/
unsigned long long PCA9685::get_bus_funcs() const {
	if (bus_funcs == 0) {
		printf("Warning: from get_bus_funcs(); setup() incomplete.\n");
	}
	return bus_funcs;
}


/*
post-condition: returns the calling object's slave_addr. Warns if the slave_addr
member == 0; implies object setup incomplete.
*/
int PCA9685::get_slave_addr() const {
	if (slave_addr == 0) {
		printf("Warning: from get_slave_addr(); setup incomplete.\n");
	}
	return slave_addr;
}

/*
Sets object pwm frequency to new_frequency. Set frequency may deviate slightly from
new_frequency paramater due to limitations of the type cast for new_prescale_value.
If absolute accuracy is necessary use set_pwm_prescale() directly.
*/
void PCA9685::set_pwm_frequency(const double& new_frequency) {
	if (new_frequency >= PCA9685_FREQ_MIN
		&& new_frequency <= PCA9685_FREQ_MAX)
	{
		unsigned char new_prescale_value{ std::round((double{ PCA9685_OSC_CLOCK_MHZ } / (4096 * new_frequency)) + 1) };
		printf("Log: new_prescale_value == %X.\n", new_prescale_value);
		set_pwm_prescale(new_prescale_value);
	}
	else {
		throw std::runtime_error("Error: new_frequency paramater outside bounds.\n");
	}
}

void PCA9685::set_pwm_prescale(const unsigned char& new_prescale_value) {
	if (new_prescale_value >= PCA9685_PRESCALE_MIN
		&& new_prescale_value <= PCA9685_PRESCALE_MAX)
	{
		i2c::write_out(bus_file, PCA9685_REG_MODE1, PCA9685_MODE1_SLEEP);
		i2c::write_out(bus_file, PCA9685_REG_PRESCALE, new_prescale_value);

		/*
		500us delay from PCA9685 datasheet required before restart. delay_ms()
		limited to avoid issues with pi cpu.
		Line 2012: https://github.com/WiringPi/WiringPi/blob/master/wiringPi/wiringPi.c
		*/
		delay_ms(1);
		i2c::write_out(bus_file, PCA9685_REG_MODE1, PCA9685_MODE1_RESTART);

		unsigned char set_prescale_value = get_prescale_value();
		if (set_prescale_value != new_prescale_value) {
			printf("Error: set_prescale_value() == %X.\n", set_prescale_value);
			printf("Error: new_prescale_value() == %X.\n", new_prescale_value);
			throw std::runtime_error("Error: set_prescale_value != new_prescale_value.\n");
		}

		printf("Log: set_pwm_frequency().\n");
	}
	else {
		throw std::runtime_error("Error: new_prescale_value outside bounds.\n");
	}
}

unsigned char PCA9685::get_prescale_value() const {
	unsigned char result = i2c::read_in(bus_file, PCA9685_REG_PRESCALE);
	printf("Log: get_prescale_value() == %X.\n", result);
	return result;
}

/*
Sets the duty cycle for a channel assuming no delay.
*/
void PCA9685::set_duty_cycle(const unsigned int& channel, const unsigned int& duty_cycle) {
	set_duty_cycle(channel, 0, duty_cycle);
}

/*
Sets the duty cycle for a channel with delay. Considers full on and off cases.
Additional cases (delay > one period) are considered invalid states, and throw.
The duty_cycle is a percentage of the period when a HIGH signal active.
*/
void PCA9685::set_duty_cycle(const unsigned int& channel, const unsigned int& delay,
	const unsigned int& duty_cycle)
{
	if (channel >= 0 && channel < (unsigned int) { PCA9685_MAXCHAN }) {
		if (delay == 0 && duty_cycle == 0) {
			set_full_off(channel);
		}
		else if (delay == 0 && duty_cycle == 100) {
			set_full_on(channel);
		}
		else if (delay >= 0 && delay < 100 && (delay + duty_cycle) > 0 &&
			(delay + duty_cycle) < 100)
		{
			//compute the delay
			unsigned int scaled_shifted_delay = 0;
			if (delay > 0) {
				scaled_shifted_delay = compute_time(delay) - 1;	//index from 0.
			}

			//send the delay
			i2c::write_out(bus_file, PCA9685_REG_LEDN_ON_L(channel), scaled_shifted_delay & 0xFF); //write 2 least significant bytes
			i2c::write_out(bus_file, PCA9685_REG_LEDN_ON_H(channel), scaled_shifted_delay >> 8); //write most significant byte
			printf("Log: scaled_shifted_delay == %u.\n", scaled_shifted_delay);

			//compute on time
			unsigned int scaled_on_time = compute_time(duty_cycle);
			printf("Log: scaled_on_time == %u.\n", scaled_on_time);

			//compute off time
			unsigned int scaled_shifted_off_time = scaled_shifted_delay + scaled_on_time;
			printf("Log: scaled_shifted_off_time == %u.\n", scaled_shifted_off_time);

			//send the off time
			i2c::write_out(bus_file, PCA9685_REG_LEDN_OFF_L(channel), scaled_shifted_off_time & 0xFF);
			i2c::write_out(bus_file, PCA9685_REG_LEDN_OFF_H(channel), scaled_shifted_off_time >> 8);

			//verify success
			unsigned int set_duty = get_duty_cycle(channel);
			if (set_duty != duty_cycle) {
				printf("Error: set_duty == %X.\n", set_duty);
				printf("Error: duty_cycle == %X.\n", duty_cycle);
				throw std::runtime_error("Error: set_duty_cycle() failed.\n");
			}
			printf("Log: set_duty_cycle().\n");
		}
		else {
			throw std::runtime_error("Error: set_duty_cycle() non-supported parameter case.\n");
		}
	}
	else {
		throw std::runtime_error("Error: channel paramater outside bounds.\n");
	}
}

/*
Sets the full on bit for a pwm channel as stated in the datasheet.
*/
void PCA9685::set_full_on(const unsigned int& channel) {
	i2c::write_out(bus_file, PCA9685_REG_LEDN_ON_L(channel), 0x00);
	i2c::write_out(bus_file, PCA9685_REG_LEDN_ON_H(channel), 0x10);
	i2c::write_out(bus_file, PCA9685_REG_LEDN_OFF_L(channel), 0x00);
	i2c::write_out(bus_file, PCA9685_REG_LEDN_OFF_H(channel), 0x00);
	printf("Log: set_full_on().\n");
}

/*
Sets the full on bit for a pwm channel as stated in the datasheet.
*/
void PCA9685::set_full_off(const unsigned int& channel) {
	i2c::write_out(bus_file, PCA9685_REG_LEDN_ON_L(channel), 0x00);
	i2c::write_out(bus_file, PCA9685_REG_LEDN_ON_H(channel), 0x00);
	i2c::write_out(bus_file, PCA9685_REG_LEDN_OFF_L(channel), 0x00);
	i2c::write_out(bus_file, PCA9685_REG_LEDN_OFF_H(channel), 0x10);
	printf("Log: set_full_off().\n");
}

/*
Computes absolute pwm operational time from a percent parameter.
post-condition: returns the absolute unshifted time.
*/
unsigned int PCA9685::compute_time(const unsigned int& percent) {
	return (unsigned int) { std::round(PCA9685_COUNTER_RANGE * (double{ percent } / 100)) };
}

unsigned int PCA9685::get_duty_cycle(const unsigned int& channel) {
	if (channel >= 0 && channel < PCA9685_MAXCHAN) {
		unsigned int on_l{ i2c::read_in(bus_file, PCA9685_REG_LEDN_ON_L(channel)) };	//two least significant bytes
		unsigned int on_h{ i2c::read_in(bus_file, PCA9685_REG_LEDN_ON_H(channel)) };	//most significant byte
		unsigned int off_l{ i2c::read_in(bus_file, PCA9685_REG_LEDN_OFF_L(channel)) };
		unsigned int off_h{ i2c::read_in(bus_file, PCA9685_REG_LEDN_OFF_H(channel)) };
		unsigned int duty{ std::round((double{ ((off_h << 8) + off_l) - ((on_h << 8) + on_l) }
		/ PCA9685_COUNTER_RANGE) * 100) };
		printf("Succcess: get_duty_cycle() == %u.\n", duty);
		return duty;
	}
	else {
		throw std::runtime_error("Error: get_duty_cycle() channel outside bounds.\n");
	}

}

/*
Delays execution for some number of milliseconds. Function sourced from
wiring pi library.
*/
void PCA9685::delay_ms(const unsigned int& interval) {
	struct timespec sleeper, dummy;
	sleeper.tv_sec = (time_t)(interval / 1000);
	sleeper.tv_nsec = (long)(interval % 1000) * 1000000;
	nanosleep(&sleeper, &dummy);
}

/*
Executes a software reset of all pca9685 boards connected on a bus to default
conditions as specified in the pca9685 datasheet. A call to
PCA9685_software_reset() should be made prior to initialization and subsequent
instatiation of PCA9685 objects. PCA9685_software_reset() insures a default
start condition for each pca9685 board.
post-condition: all pca9586 boards connected on the bus are reset to the default
state.
*/
void PCA9685_software_reset(const char* bus_filename) {
	int bus_file_local = i2c::open_bus(bus_filename);
	unsigned long long bus_funcs_local = i2c::probe_bus_funcs(bus_file_local);

	if (!(i2c::verify_bus_func(bus_funcs_local, I2C_FUNC_I2C))) {
		throw std::runtime_error("Error: pca9685_software_reset() i2c commands not supported.\n");
	}

	i2c::init_comms(bus_file_local, GENERAL_CALL_ADDR);
	i2c::write_out(bus_file_local, PCA9685_SWRST);
	printf("Log: pca9685_software_reset().\n");
}
