#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/regmap.h>

#include <sound/soc.h>
#include <sound/pcm.h>
#include <sound/initval.h>
#include <sound/pcm_params.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/delay.h>

#include "tas5805m.h"

#define tas5805m_NAME		"tas5805m"
#define delay_time		255


static int transmit_registers(struct i2c_client *client, cfg_reg *r, int n)
{
	int i = 0;
	int ret=-1;
	int j=0;
	for (j=0;j<3;j++)
	{
	    if(ret != 0)
	    {
    		while (i < n) {
			 if(r[i].command == 255)
			 {
			 	mdelay(5);
				i++;
				continue;
			 }	
		     ret = i2c_smbus_write_byte_data(client, r[i].command, r[i].param);
		     //pr_err("transmit_registers ------ ret --%d\n",ret);
		     if(ret != 0)
				break;
		     i ++;
		}
	    }
 	//    pr_err("transmit_registers -----for ret --%d\n",ret);
	}
	return ret;
}


static int tas5805m_i2c_probe(struct i2c_client *client,
			   const struct i2c_device_id *id)
{
	int enable_gpio;
	int ret;
	struct pinctrl_state *audio_pinctrl;
	struct pinctrl *pinctrl;

	pr_err("tas5805m_i2c_probe audio probe init\n");

	
	pinctrl= devm_pinctrl_get(&client->dev);
	audio_pinctrl = pinctrl_lookup_state(pinctrl,"audio_enable_default");
	ret = pinctrl_select_state(pinctrl,audio_pinctrl);
	if (ret < 0){
	    pr_err("tas5805m  pinctrl state error\n");
	}

	enable_gpio = of_get_named_gpio(client->dev.of_node, "qcom,audio_enable_gpio",0);
	if (enable_gpio < 0)
	{
		pr_err("tas5805m probe get enable gpio failed");
		return enable_gpio;
	}

	mdelay(5);
	gpio_direction_output(enable_gpio, 1);
	mdelay(120);		
	
	ret = transmit_registers(client,registers,sizeof(registers)/sizeof(registers[0]));
	if (ret != 0)
		gpio_direction_output(enable_gpio, 0);
	
	return ret;
}

static int tas5805m_i2c_remove(struct i2c_client *client)
{
	return 0;
}


static int tas5805m_suspend(struct device *dev)
{
	return 0;
}

static int tas5805m_resume(struct device *dev)
{
	return 0;
}

#if CONFIG_PM
static const struct dev_pm_ops tas5805m_dev_pm_ops = {
	.suspend = tas5805m_suspend,
	.resume = tas5805m_resume,
};
#else
static const struct dev_pm_ops tas5805m_dev_pm_ops = {
};
#endif


static const struct i2c_device_id tas5805m_i2c_id[] = {
	{ "tas5805m",0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, tas5805m_i2c_id);

#ifdef CONFIG_OF
static const struct of_device_id tas5805m_of_match[] = {
	{ .compatible = "ti,tas5805m", },
	{ }
};
MODULE_DEVICE_TABLE(of, tas5805m_of_match);
#endif

static struct i2c_driver tas5805m_i2c_driver = {
	.probe 		= tas5805m_i2c_probe,
	.remove 	= tas5805m_i2c_remove,
	.id_table	= tas5805m_i2c_id,
	.driver		= {
		.name	= "tas5805m",
		.of_match_table = tas5805m_of_match,
	},
};

module_i2c_driver(tas5805m_i2c_driver);

MODULE_AUTHOR("Andy Liu <andy-liu@ti.com>");
MODULE_DESCRIPTION("TAS5805M Audio Amplifier Driver");
MODULE_LICENSE("GPL v2");
