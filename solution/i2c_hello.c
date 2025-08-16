/* Benötigter Header */
#include <linux/i2c.h>
#include <linux/kobject.h>

/* Globale Variable weil ich zu faul zum dynamischen allokieren bin */
struct kobject * my_kobj;
/* Globaler i2c_client für Zugriff auf I2C Bus in store and show Funktionen */
struct i2c_client *my_dev;

/* Benenne alle kompatiblen Geräte */
static struct i2c_device_id my_ids[] = {
	{"rgb_brd"},
	{} /* leeres Element signalisiert das Ende der Liste */
};
MODULE_DEVICE_TABLE(i2c, my_ids);

/**
 * Store wird aufgerufen, wenn die Datei /rgb_brd/led geschrieben wird */
static ssize_t mydev_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
	u8 val;
	int i;

	/* Ich brauche mindestens drei zeichen für die drei LEDs, z.B. 010 */
	if (count < 3)
		return -EINVAL;

	val = 0xff; /* default: alle LEDs aus */
	/* Berechnen wir den zu schreibenden Wert */
	for(i = 0; i<3;i++) {
		if (buffer[i] =='1') /* Falls Char 1, schalte i-te LED an */
			val &= ~(1 << (i + 1)); /* Update val */
	}

	/* Schreiben wir den Wert über I2C zum Device */
	i2c_smbus_write_byte(my_dev,val);
	return count;
}

/**
 * Show wird aufgerufen, wenn die Datei /rgb_brd/button gelesen wird */
static ssize_t mydev_show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
	int status;

	/* Lese alle 8 IO Pins ein */
	status = i2c_smbus_read_byte(my_dev);
	if (status < 0) { 
		printk("Error reading from I2C device!\n");
		return -ENODEV;
	}
	/* Für den Taster ist nur Bit 0 wichtig! */
	return sprintf(buffer, "Button is %spressed\n", status & 0x1 ? "not " : "");
}

/* Hier legen wir die Attribute an. */
static struct kobj_attribute led_attr = __ATTR(led, 0660, NULL, mydev_store); /* led ist nur schreibbar */
static struct kobj_attribute button_attr = __ATTR(button, 0660, mydev_show, NULL); /* button ist nur lesbar */

/* Funktion wird aufgerufen, wenn ein kompatibles I2C Gerät hinzugefügt wird */
static int my_probe(struct i2c_client *client)
{
	int status;
	my_dev = client;

	printk("i2c_hello - Hello device with addr. %x\n", client->addr);

	/* Prüfe, ob der Zugriff auf das I2C Gerät klappt, indem ich von den Gerät lese */
	status = i2c_smbus_read_byte(client);
	if (status < 0) {
		printk("Error reading from I2C device!\n");
		return -ENODEV;
	}

	/* Zugriff hat geklappt, dann legen wir jetzt das Kernel Objekt und die Attribute an... */

	my_kobj = kobject_create_and_add("rgb_brd", my_kobj);
	if (!my_kobj) {
		printk("Error creating kernel object\n");
		return -ENOMEM;
	}

	/* Erstelle erstes sysfs file für LED */
	status = sysfs_create_file(my_kobj, &led_attr.attr);
	if (status) {
		printk("Error creating /sys/my_kobj/led\n");
		/* Aufräumen nicht vergessen */
		kobject_put(my_kobj);
		return status;
	}

	/* Und dann das zweite für den Taster */
	status = sysfs_create_file(my_kobj, &button_attr.attr);
	if (status) {
		printk("Error creating /sys/my_kobj/button\n");
		/* Aufräumen nicht vergessen */
		sysfs_remove_file(my_kobj, &led_attr.attr);
		kobject_put(my_kobj);
		return status;
	}

	return 0;
}

/* Funktion wird aufgerufen, wenn ein kompatibles I2C Gerät entfernt wird */
static void my_remove(struct i2c_client *client)
{
	printk("i2c_hello - Goodbye device with addr. %x\n", client->addr);
	/* Schalte die LED wieder aus */
	i2c_smbus_write_byte(client, 0xff);
	/* Aufräumen nicht vergessen */
	sysfs_remove_file(my_kobj, &button_attr.attr);
	sysfs_remove_file(my_kobj, &led_attr.attr);
	kobject_put(my_kobj);
}

/* Fasse kompatible Geräte, Probe- und Remove-Funktionen in Treiber zusammen */
static struct i2c_driver my_driver = {
	.probe = my_probe,
	.remove = my_remove,
	.id_table = my_ids,
	.driver = {
		.name = "my-i2c-driver",
	}
};

/* Registriere Treiber */
module_i2c_driver(my_driver);

/* Informationen über Treiber */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Johannes Roith");
MODULE_DESCRIPTION("Ein Treiber fuer unser RGB Board"); /* TODO: Ergänze hier eine Beschreibung */
