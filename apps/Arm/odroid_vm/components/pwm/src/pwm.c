/*
 * Copyright 2017, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */
#include <camkes.h>

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>

#include <platsupport/i2c.h>
#include <platsupport/delay.h>
#include <utils/util.h>

#define PWM_SELF_TEST
//#define DEBUG_PWM
#ifdef DEBUG_PWM
#define DPWM(args...) \
    do { \
        printf("PWM %s(%d):", __func__, __LINE__); \
        printf(args); \
        printf("\n"); \
    } while(0)
#else
#define DPWM(...) do{}while(0)
#endif

#define udelay(x) ps_udelay(x)

#define I2C_PWM       I2C0
#define I2C_PWM_ADDR  0x8E

#define MIN_PWM       225
#define MAX_PWM       410

#define MODE1         0x00
#define MODE2         0x10

#define LED0          0x06
#define LED1          0x0A
#define LED2          0x0E
#define LED3          0x12

#define LED(x)        (LED0 + ((x) * 0x4))

#define ALL_LED_ON_L  0xFA
#define ALL_LED_ON_H  0xFB
#define ALL_LED_OFF_L 0xFC
#define ALL_LED_OFF_H 0xFD
#define PRESCALE      0xFE /* prescale = round(osc_clk / (4096 * update rate )) - 1) */
#define NREGS         0x100
#define NLEDS         16

#define FL 0
#define FR 1
#define BL 2
#define BR 3

#define MODE1_RESTART       BIT(7)
#define MODE1_EXTCLK        BIT(6)
#define MODE1_AUTOINC       BIT(5)
#define MODE1_SLEEP         BIT(4)
#define MODE1_SUBADDR1_EN   BIT(3)
#define MODE1_SUBADDR2_EN   BIT(2)
#define MODE1_SUBADDR3_EN   BIT(1)
#define MODE1_ALLCALL_EN    BIT(0)

#define MODE2_INVERT_OUT    BIT(4)
#define MODE2_OCH_ACK       BIT(3) /* Change state on ACK else, change on STOP */
#define MODE2_OUTDRV_TOTEM  BIT(2)
#define MODE2_OUTNE(x)      (x)
#define MODE2_OUTNE_LOW     MODE2_OUTNE(0b00) /* Low state when OE asserted */
#define MODE2_OUTNE_OSRC    MODE2_OUTNE(0b01) /* Open source when OE asserted */
#define MODE2_OUTNE_HIZ     MODE2_OUTNE(0b10) /* Hi Z when OE asserted */

#define LEDON_COUNT(x)      ((x) & 0xfff)
#define LEDON_FULL          BIT(4 + 8)
#define LEDOFF_COUNT(x)     ((x) & 0xfff)
#define LEDOFF_FULL         BIT(4 + 8)


i2c_bus_t i2c_bus;
i2c_slave_t i2c_pwm;
i2c_kvslave_t pwm_kvslave;

typedef struct {
    uint8_t on_l;
    uint8_t on_h;
    uint8_t off_l;
    uint8_t off_h;
} motor_t;

const static motor_t motors[] = {
    { .on_l = LED0, .on_h = LED0 + 1, .off_l = LED0 + 2, .off_h = LED0 + 3 },
    { .on_l = LED1, .on_h = LED1 + 1, .off_l = LED1 + 2, .off_h = LED1 + 3 },
    { .on_l = LED2, .on_h = LED2 + 1, .off_l = LED2 + 2, .off_h = LED2 + 3 },
    { .on_l = LED3, .on_h = LED3 + 1, .off_l = LED3 + 2, .off_h = LED3 + 3 }
};

static void
i2c_complete_cb(i2c_bus_t *bus, enum i2c_stat status, size_t size, void* token) {
    DPWM("Got callback %d, size %d\n", status, size);
    bus_sem_post();
}

void write_register(uint8_t address, uint8_t value) 
{
    uint8_t data[2] = { address, value };
    int count = i2c_slave_write(&i2c_pwm, data, ARRAY_SIZE(data),
                                false, &i2c_complete_cb, NULL);
    if (count >= 0) {
        bus_sem_wait();
        // Maybe need to pass a status back??
    } else {
        assert(0);
    }
}

uint8_t read_register(uint8_t address)
{
    uint8_t data[1] = { address };
    int count = i2c_slave_write(&i2c_pwm, data, ARRAY_SIZE(data),
                                true, &i2c_complete_cb, NULL);
    if (count >= 0) {
        bus_sem_wait();
    }
    data[0] = 0xEE; // easier to check it was overwritten
    count = i2c_slave_read(&i2c_pwm, data, ARRAY_SIZE(data),
                           false, &i2c_complete_cb, NULL);
    if (count >= 0) {
        bus_sem_wait();
    } else {
        assert(0);
    }
    return data[0];
}

static void UNUSED register_dump(void){
    int i;
    printf("----PCA9685 register dump");
    for(i = 0; i < NREGS; i++){
        uint8_t v;
        v = read_register(i);
        if(i % 16 == 0){
            printf("\n 0x%02x: ", i);
        }
        printf("0x%02x ", v);
    }
    printf("\n--------------\n");
}

void
pwm_set_led(int led, int level)
{
    int count;
    int status;

    if(level > LEDON_FULL){
        level = LEDON_FULL;
    }
    uint8_t data[5] = { LED(led), 0, 0, level, level >> 8 };
    count = i2c_slave_write(&i2c_pwm, data, ARRAY_SIZE(data),
                            false, &i2c_complete_cb, &status);
    if (count >= 0) {
        bus_sem_wait();
    } else {
        assert(0);
    }
}

void init_pwm_driver(void)
{
    UNUSED int mode1 , mode2;
    DPWM("Start init led device\n");
    mode1 = read_register(MODE1);
    DPWM("Mode1 is %x\n", mode1);
    write_register(MODE2, MODE2_INVERT_OUT);
    mode2 = read_register(MODE2);
    DPWM("Mode2 is %x\n", mode2);
    write_register(PRESCALE, 135);
    write_register(MODE1, MODE1_AUTOINC);
    udelay(2000); // wait for internal clock to stablise

    DPWM("Finished init");
}

/**
 * Called on every I2C interrupt, direct control to driver.
 */
void
i2c0_int_handle(void)
{
    DPWM(".");
    i2c_handle_irq(&i2c_bus);
    i2c0_int_acknowledge();
}

void set_motor(int motor_id, double speed)
{
    int out = MIN_PWM + ((double)(MAX_PWM - MIN_PWM)) * speed;
    DPWM("Setting motors to %d, got speed in %f", out, speed);
    write_register(motors[motor_id].on_l, 0);
    write_register(motors[motor_id].on_h, 0);
    write_register(motors[motor_id].off_l, out);
    write_register(motors[motor_id].off_h, out >> 8);
}   

static double m[ARRAY_SIZE(motors)];
/* Take in speed values of -1 to 1 */
void pwm_set_motors(double fl, double fr, double bl, double br)
{
    DPWM("got set motors");

    // Save the desired speeds so the run thread can access them
    m[FL] = fl;
    m[FR] = fr;
    m[BL] = bl;
    m[BR] = br;

    // Trigger the run thread to set the speeds
    set_motors_unlock(); 
}

void pwm__init(void)
{
    int err;
    //DPWM("waiting...");
    //udelay(10000000);
    DPWM("Starting pwm driver\n");
    bus_sem_wait(); // Prime the semaphore so the first wait blocks

    DPWM("i2c0 enabled\n");
    err = exynos_i2c_init(I2C_PWM, i2c0, NULL, &i2c_bus);
    assert(!err);

    // Initalise slave
    err = i2c_slave_init(&i2c_bus, I2C_PWM_ADDR,
                           I2C_SLAVE_ADDR_7BIT, I2C_SLAVE_SPEED_FAST,
                           0, &i2c_pwm);
    assert(!err);

    err = i2c_kvslave_init(&i2c_pwm, BIG8, BIG8, &pwm_kvslave);
    if (err) {
        ZF_LOGF("Failed to initialize lib I2C-KVSlave instance.");
    }
 
    // Scan the bus 
    int count, addr;
    int start = 0;
    int found = 0;
    do {
        count = i2c_scan(&i2c_bus, start, &addr, 1);
        if(count){
            printf("Slave @ 0x%x\n", addr);
            if(addr == I2C_PWM_ADDR){
                found = 1;
                break;
            }
        }
    } while(count);
    // Check that the address responded 
    if (found) {
        printf("Received response from PWM address\n");
    } else {
        printf("No reponse from PWM\n");
    } 

    set_motors_lock(); // Prime the lock so the first call to lock blocks
    DPWM("Finished init");
}

int counter = 0;
void pwm_signal(void *arg)
{
	int level;

//	printf("Signal from Pilot!\n");

	if (counter % 2) {
		level = 0;
	} else {
		level = 4095;
	}

	sig_lock();
	for (int led = 2; led < NLEDS - 1; led += 3) {
		pwm_set_led(led, level);
	}
	sig_unlock();

	if (++counter == 16) {
		counter = 0;
	}

	signal_reg_callback(pwm_signal, NULL);
}

unsigned int cnt = 0;
void timer_update_callback(void *arg)
{
	int level;

	if (cnt % 2) {
		level = 0;
	} else {
		level = 4095;
	}

	sig_lock();
	for (int led = 0; led < NLEDS - 1; led += 3) {
		pwm_set_led(led, level);
	}
	sig_unlock();

	if (++cnt == 16) {
		cnt = 0;
	}

	timer_update_reg_callback(timer_update_callback, NULL);
}

void pwm_vmsig(int val)
{
	if (!val) {
		timer_update_reg_callback(timer_update_callback, NULL);
	} else {
		val = val < 0 ? 0 : 4095;
		sig_lock();
		for (int led = 1; led < NLEDS - 1; led += 3) {
			pwm_set_led(led, val);
		}
		sig_unlock();
	}
}

/* This should become a self test, for now does init as well */
int run ()
{
    // A hack since the interrupts in camkes init functions don't seem to trigger
    init_pwm_driver();

    for (int led = 0; led < NLEDS; led++) {
	    pwm_set_led(led, 4095);
    }

    signal_reg_callback(pwm_signal, NULL);

    return 0;
#ifdef PWM_SELF_TEST
    while(true){
        int led;
        int level;
        /* increasing */
        for(level = 0; level < LEDON_FULL; level += LEDON_FULL/100){
            for(led = 0; led < NLEDS; led++){
                pwm_set_led(led, level);
            }
        }
        ps_mdelay(100);
        /* decreasing */
        for(level = LEDON_FULL; level >= 0; level -= LEDON_FULL/100){
            for(led = 0; led < NLEDS; led++){
                pwm_set_led(led, level);
            }
        }
        ps_mdelay(100);

    }
#endif
    return 0;
}
