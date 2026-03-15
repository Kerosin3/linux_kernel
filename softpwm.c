#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/gpio.h>
#include <linux/gpio/driver.h>
#include <linux/gpio/consumer.h>

static char *gpio_chip_label = NULL;   // gpiochipname
static int   gpio_pin        = -1;     // pin offset
static int   gpio_num_abs    = -1;     // abs gpio number

module_param(gpio_chip_label, charp, 0444);
MODULE_PARM_DESC(gpio_chip_label, "GPIO chip label");

module_param(gpio_pin, int, 0444);
MODULE_PARM_DESC(gpio_pin, "Pin offset within chip");


#define PERIOD_NS               1000000000ULL  // 1 sec
#define DUTY_STEP_INC   10 // duty cycle increment
#define DUTY_MAX_INC    100 // max 100 %

static struct hrtimer  pwm_timer;
static struct gpio_desc *pwm_gpio;

static int      duty_current_percents = 0;
static bool phase_high  = false;
static unsigned long long period_num  = 0;


// callback
static enum hrtimer_restart pwm_callback(struct hrtimer *timer)
{
        u64 t_high_ns = PERIOD_NS * duty_current_percents / 100; // high phase
        u64 t_low_ns  = PERIOD_NS - t_high_ns; // low phase

        // if low phase
        if (!phase_high) {
                // start phase
                period_num++;
                pr_info("softpwm: period %2llu  duty=%3d%%      HIGH=%7llums  LOW=%7llums\n",
                                period_num, duty_current_percents,
                                t_high_ns / 1000000,
                                t_low_ns  / 1000000);
                // if 0% duty cycle (first tick)
                if (duty_current_percents == 0) {
                        gpiod_set_value(pwm_gpio, 0);
                        // forwart timer to next cycle
                        hrtimer_forward_now(timer, ns_to_ktime(PERIOD_NS));
                        // wait next cycle
                        goto next_duty;
                }
                // high phase
                gpiod_set_value(pwm_gpio, 1);
                phase_high = true;
                // forwart timer
                hrtimer_forward_now(timer, ns_to_ktime(t_high_ns));
                // full duty cycle -> move to next cycle
                if (duty_current_percents == 100) {
                        phase_high = false;
                        goto next_duty;
                }
                //reload timer
                return HRTIMER_RESTART;
        }
        // low phase
        gpiod_set_value(pwm_gpio, 0);
        phase_high = false;
        hrtimer_forward_now(timer, ns_to_ktime(t_low_ns));
        //reload timer
        goto next_duty;

        next_duty:
        duty_current_percents += DUTY_STEP_INC;
        if (duty_current_percents > DUTY_MAX_INC) {
                duty_current_percents = 0;
                pr_info("softpwm: cycle done, restarting\n");
        }
        //reload timer
        return HRTIMER_RESTART;
}

static int __init softpwm_init(void)
{
    struct gpio_device *gdev;
    int gpio_base, ret;

    if (!gpio_chip_label) {
        pr_err("%s: gpio_chip_label arg is required!\n", KBUILD_MODNAME);
        pr_err("%s: usage: insmod/depmod softpwm.ko gpio_chip_label=gpio1 gpio_pin=7\n",
               KBUILD_MODNAME);
        return -EINVAL;
    }
    if (gpio_pin < 0) {
        pr_err("%s: gpio_ping arg is required!\n", KBUILD_MODNAME);
        pr_err("%s: use datasheet...rk3588\n",
               KBUILD_MODNAME);
        return -EINVAL;
    }

    gdev = gpio_device_find_by_label(gpio_chip_label);
    if (!gdev) {
        pr_err("%s: chip '%s' not found\n", KBUILD_MODNAME, gpio_chip_label);
        return -ENODEV;
    }

    gpio_base = gpio_device_get_base(gdev);
    gpio_device_put(gdev);

    if (gpio_base < 0) {
        pr_err("%s: chip has no GPIO number\n", KBUILD_MODNAME);
        return -EINVAL;
    }

    gpio_num_abs = gpio_base + gpio_pin;

    ret = gpio_request(gpio_num_abs, KBUILD_MODNAME);
    if (ret) {
        pr_err("%s: gpio_request(%d) failed: %d\n",
               KBUILD_MODNAME, gpio_num_abs, ret);
        return ret;
    }

    ret = gpio_direction_output(gpio_num_abs, 0);
    if (ret) {
        gpio_free(gpio_num_abs);
        return ret;
    }

    pwm_gpio = gpio_to_desc(gpio_num_abs);
    if (!pwm_gpio || gpiod_cansleep(pwm_gpio)) {
        gpio_free(gpio_num_abs);
        return -EOPNOTSUPP;
    }

    pr_info("%s: chip='%s' base=%d pin=%d → GPIO%d\n",
            KBUILD_MODNAME, gpio_chip_label, gpio_base, gpio_pin, gpio_num_abs);

    // setup timer in hardirq context
    hrtimer_setup(&pwm_timer, pwm_callback, CLOCK_MONOTONIC, HRTIMER_MODE_REL_HARD);
    hrtimer_start(&pwm_timer, ns_to_ktime(1), HRTIMER_MODE_REL_HARD);
    return 0;
}

static void __exit softpwm_exit(void)
{
    //cancel timer
    hrtimer_cancel(&pwm_timer);
    if (pwm_gpio) {
        gpiod_set_value(pwm_gpio, 0);
        gpio_free(gpio_num_abs);
    }
    pr_info("%s: unloaded\n", KBUILD_MODNAME);
}

module_init(softpwm_init);
module_exit(softpwm_exit);

MODULE_AUTHOR("AlexVd");
MODULE_DESCRIPTION("hrtimer soft PWM example");
MODULE_LICENSE("GPL");
