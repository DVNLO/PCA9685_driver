#include "PCA9685_driver.h"
#include <iostream>
#include <cstdlib>
#include <ctime>

constexpr char* bus_filename = (char*)"/dev/i2c-1";
constexpr int slave_address = 0x40;

int main() {
	std::srand(std::time(0));
	PCA9685_software_reset(bus_filename);
	PCA9685 board{ bus_filename, slave_address };
	try {
		board.set_pwm_prescale(PCA9685_PRESCALE_MIN);	//min prescale value is max frequency

		for (int i = 0; i < 1000; i++)
		{
			board.set_duty_cycle(0, std::rand() % 101);
			board.set_duty_cycle(1, std::rand() % 101);
			board.delay_ms(100);
		}

		/*
		//Initial testing.
		for (int i = 0; i < 100; i++)
		{
		for (int j = 0; j <= 100; j++)
		{
		printf("j == %d\n", j);
		board.set_duty_cycle(0, j);
		board.set_duty_cycle(1, j);
		board.delay_ms(100);
		}
		for (int k = 100; k >= 0; k--)
		{
		printf("k == %d\n", k);
		board.set_duty_cycle(0, k);
		board.set_duty_cycle(1, k);
		board.delay_ms(100);
		}
		}
		*/
		board.set_full_off(0);
		board.set_full_off(1);
	}
	catch (const std::runtime_error& error) {
		std::cout << error.what() << '\n';
		board.set_full_off(0);
		board.set_full_off(1);
		exit(1);
	}

	/*
	board.set_duty_cycle(0, 99);
	board.get_duty_cycle(0);
	board.delay_ms(10000);
	*/


	/*
	i2c::write_out(board.get_bus_file(), PCA9685_REG_LEDX_ON_L, 0x99);
	i2c::write_out(board.get_bus_file(), PCA9685_REG_LEDX_ON_H, 0x01);
	i2c::write_out(board.get_bus_file(), PCA9685_REG_LEDX_OFF_L, 0xCC);
	i2c::write_out(board.get_bus_file(), PCA9685_REG_LEDX_OFF_H, 0x04);
	*/
	return 0;
}
/*
int bus_file_local = i2c::open_bus(bus_filename);
unsigned long long bus_funcs_local = i2c::probe_bus_funcs(bus_file_local);

if (!(i2c::verify_bus_func(bus_funcs_local, I2C_FUNC_I2C))) {
printf("Error: pca9685_software_reset() i2c commands not supported.\n");
exit(1);
}
i2c::init_comms(bus_file_local, slave_address);

i2c::write_out(bus_file_local, PCA9685_REG_LEDX_ON_L, 0x04);
i2c::write_out(bus_file_local, PCA9685_REG_LEDX_ON_H, 0x05);

char read_byte = i2c::read_in(bus_file_local, PCA9685_REG_LEDX_ON_L);
printf("read_byte L == %X\n", read_byte);
char read_byte1 = i2c::read_in(bus_file_local, PCA9685_REG_LEDX_ON_H);
printf("read_byte H == %X\n", read_byte1);
*/