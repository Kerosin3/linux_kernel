#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/gpio/consumer.h>
#include <linux/interrupt.h>
#include <linux/of.h>
#include <linux/workqueue.h>
#include <linux/atomic.h>

#define DRIVER_NAME          "gpio-isr-test"
#define DEBOUNCE_INTERVAL_MS 100

struct gpio_isr_test_priv {
    struct gpio_desc    *gpio;
    int                  irq;
    struct delayed_work  dwork;
    atomic_t             press_count;
};

/* bottom half */
static void gpio_isr_test_work_fn(struct work_struct *work)
{
    struct gpio_isr_test_priv *priv =
        container_of(work, struct gpio_isr_test_priv, dwork.work);

	// test is gpio is low after debounce interval
    if (gpiod_get_value_cansleep(priv->gpio) != 0)
        return;

    atomic_inc(&priv->press_count);

    pr_info("[gpio-isr-test] PIN SHORTED!!! press count=#%d\n",
            atomic_read(&priv->press_count));
}

/* top half (cant sleep!) */
static irqreturn_t gpio_isr_test_isr(int irq, void *dev_id)
{
    struct gpio_isr_test_priv *priv = dev_id;

	// shedule work in debounce interval
    mod_delayed_work(system_wq, &priv->dwork,
                     msecs_to_jiffies(DEBOUNCE_INTERVAL_MS));

    return IRQ_HANDLED;
}

static int gpio_isr_test_probe(struct platform_device *pdev)
{
    struct gpio_isr_test_priv *priv;
    int ret;

    priv = devm_kzalloc(&pdev->dev, sizeof(*priv), GFP_KERNEL);
    if (!priv)
        return -ENOMEM;

    atomic_set(&priv->press_count, 0);

	// init worker callback
    INIT_DELAYED_WORK(&priv->dwork, gpio_isr_test_work_fn);

    /* init gpio */
    priv->gpio = devm_gpiod_get(&pdev->dev, NULL, GPIOD_IN);
    if (IS_ERR(priv->gpio))
        return dev_err_probe(&pdev->dev, PTR_ERR(priv->gpio),
                             "Failed to get GPIO\n");

    /* init irq */
    priv->irq = gpiod_to_irq(priv->gpio);
    if (priv->irq < 0)
        return dev_err_probe(&pdev->dev, priv->irq,
                             "Failed to get IRQ\n");

    ret = devm_request_irq(
        &pdev->dev,
        priv->irq,
        gpio_isr_test_isr,
        IRQF_TRIGGER_FALLING,
        DRIVER_NAME,
        priv
    );
    if (ret)
        return dev_err_probe(&pdev->dev, ret,
                             "Failed to request IRQ\n");

    platform_set_drvdata(pdev, priv);

    dev_info(&pdev->dev, "probed OK — IRQ=%d, debounce=%dms\n",
             priv->irq, DEBOUNCE_INTERVAL_MS);
    return 0;
}

static void gpio_isr_test_remove(struct platform_device *pdev)
{
    struct gpio_isr_test_priv *priv = platform_get_drvdata(pdev);

	// disable irq and cancel current work
    disable_irq(priv->irq);
    cancel_delayed_work_sync(&priv->dwork);

    dev_info(&pdev->dev, "removed %s, total presses: %d\n", KBUILD_MODNAME,
             atomic_read(&priv->press_count));
}

static const struct of_device_id gpio_isr_test_of_match[] = {
    { .compatible = "gpio-isr-test" },
    { }
};
MODULE_DEVICE_TABLE(of, gpio_isr_test_of_match);

static struct platform_driver gpio_isr_test_driver = {
    .probe  = gpio_isr_test_probe,
    .remove = gpio_isr_test_remove,
    .driver = {
        .name           = DRIVER_NAME,
        .of_match_table = gpio_isr_test_of_match,
    },
};

module_platform_driver(gpio_isr_test_driver);

MODULE_AUTHOR("AlexVd");
MODULE_DESCRIPTION("GPIO with workqueue");
MODULE_LICENSE("GPL");
