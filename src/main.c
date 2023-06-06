
#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/gpio.h>

#include <sys/printk.h>
#include <sys/util.h>
#include <random/rand32.h>

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS   1000

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)

#if DT_NODE_HAS_STATUS(LED0_NODE, okay)
#define LED0	DT_GPIO_LABEL(LED0_NODE, gpios)
#define PIN	DT_GPIO_PIN(LED0_NODE, gpios)
#define FLAGS	DT_GPIO_FLAGS(LED0_NODE, gpios)
#else
/* A build error here means your board isn't set up to blink an LED. */
#error "Unsupported board: led0 devicetree alias is not defined"
#define LED0	""
#define PIN	0
#define FLAGS	0
#endif

#define SW0_NODE DT_ALIAS(sw0)
#define SW1_NODE DT_ALIAS(sw1)
#define SW2_NODE DT_ALIAS(sw2)
#define SW3_NODE DT_ALIAS(sw3)

#define BUTTON_PIN_1 11
#define BUTTON_PIN_2 12
#define BUTTON_PIN_3 24
#define BUTTON_PIN_4 25

#define LED_PIN_1 13
#define LED_PIN_2 14
#define LED_PIN_3 15
#define LED_PIN_4 16

#define LED_ON 0
#define LED_OFF 1

#define MAX_SEQUENCE_LENGTH 10

struct device *led_pins[4];
struct device *button_pins[4];

uint32_t sequence[MAX_SEQUENCE_LENGTH];
int step = 0;
int checkStep = 0;

void generate_sequence(void)
{
    for (int i = 0; i < MAX_SEQUENCE_LENGTH; i++)
    {
        sequence[i] = (sys_rand32_get() % 4) + 1; //it generates numbers from 1 to 4 
		printk("%d \n", sequence[i]);
    }
}

void turn_on_led(const struct device *dev, int led)
{
	switch(led) {
		case 1: 
			gpio_pin_set(dev, LED_PIN_1, LED_ON);
			break;
		case 2:
			gpio_pin_set(dev, LED_PIN_2, LED_ON);
			break;
		case 3:
			gpio_pin_set(dev, LED_PIN_3, LED_ON);
			break;
		case 4:
			gpio_pin_set(dev, LED_PIN_4, LED_ON);
			break;
	}
}

void turn_off_led(const struct device *dev, int led)
{
    switch(led) {
		case 1: 
			gpio_pin_set(dev, LED_PIN_1, LED_OFF);
			break;
		case 2:
			gpio_pin_set(dev, LED_PIN_2, LED_OFF);
			break;
		case 3:
			gpio_pin_set(dev, LED_PIN_3, LED_OFF);
			break;
		case 4:
			gpio_pin_set(dev, LED_PIN_4, LED_OFF);
			break;
	}
}

void blink_led(const struct device *dev, int led, int delay_time)
{
    turn_on_led(dev, led);
    k_sleep(K_MSEC(delay_time));
    turn_off_led(dev, led);
    k_sleep(K_MSEC(delay_time));
}

void blink_all_leds(const struct device *dev)
{
	for(int i = 1; i < 5; i++)
	{
		turn_on_led(dev, i);
	}

	k_sleep(K_MSEC(300));
	
	for(int i = 1; i < 5; i++)
	{
		turn_off_led(dev, i);
	}
}

void show_sequence(const struct device *dev)
{
    for (int i = 0; i < MAX_SEQUENCE_LENGTH; i++)
    {
        blink_led(dev, sequence[i], 500);
        k_sleep(K_MSEC(500));
    }
}

int get_pressed_button(const struct device *dev)
{
	if (gpio_pin_get(dev, BUTTON_PIN_1) == 1)
		return 1;
	else if (gpio_pin_get(dev, BUTTON_PIN_2) == 1)
		return 2;
	else if (gpio_pin_get(dev, BUTTON_PIN_3) == 1)
		return 3;
	else if (gpio_pin_get(dev, BUTTON_PIN_4) == 1)
		return 4;
    else return -1;
}

void check_button(const struct device *dev, int button)
{
    if (button == sequence[step])
    {
        blink_led(dev, button, 250);
        step++;
        if (step == MAX_SEQUENCE_LENGTH)
        {
			printk("You won!\n");
            generate_sequence();
            step = 0;
            show_sequence(dev);
        }
    }
    else
    {
        printk("Wrong button! Game over! You clicked button number %i.\n", button);
		blink_all_leds(dev);
        step = 0;
        generate_sequence();
        show_sequence(dev);
    }
}

void initialize(const struct device *dev)
{
	/*Leds*/
	gpio_pin_configure(dev, LED_PIN_1, GPIO_OUTPUT);
	gpio_pin_configure(dev, LED_PIN_2, GPIO_OUTPUT);
	gpio_pin_configure(dev, LED_PIN_3, GPIO_OUTPUT);
	gpio_pin_configure(dev, LED_PIN_4, GPIO_OUTPUT);

	gpio_pin_set(dev, LED_PIN_1, LED_OFF);
	gpio_pin_set(dev, LED_PIN_2, LED_OFF);
	gpio_pin_set(dev, LED_PIN_3, LED_OFF);
	gpio_pin_set(dev, LED_PIN_4, LED_OFF);

	/*Buttons*/
	gpio_pin_configure(dev, BUTTON_PIN_1, (GPIO_INPUT | DT_GPIO_FLAGS(SW0_NODE, gpios)));
	gpio_pin_configure(dev, BUTTON_PIN_2, (GPIO_INPUT | DT_GPIO_FLAGS(SW1_NODE, gpios)));
	gpio_pin_configure(dev, BUTTON_PIN_3, (GPIO_INPUT | DT_GPIO_FLAGS(SW2_NODE, gpios)));
	gpio_pin_configure(dev, BUTTON_PIN_4, (GPIO_INPUT | DT_GPIO_FLAGS(SW3_NODE, gpios)));
}

void main(void)
{
	const struct device *dev;

	dev = device_get_binding("GPIO_0");
	if (dev == NULL) {
		return;
	}

	initialize(dev);
	generate_sequence();
    show_sequence(dev);
	k_sleep(K_MSEC(500));

	while (1) {
		/*
		if(gpio_pin_get(dev, button1.pin)){
			gpio_pin_set(dev, LED_PIN_4, LED_ON);
			printk("xd\n");
		}
		else
			gpio_pin_set(dev, LED_PIN_4, LED_OFF);
			*/

		int button = get_pressed_button(dev);
        if (button != -1)
        {
            check_button(dev, button);
            k_sleep(K_MSEC(200));
        }
	}
}