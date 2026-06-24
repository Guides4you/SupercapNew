/* main.c - Application main entry point */

/*
 * Copyright (c) 2015-2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/types.h>
#include <stddef.h>
#include <zephyr/sys/util.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/hci_vs.h>
#include <zephyr/sys/reboot.h>
#include <zephyr/toolchain.h>
#include <zephyr/pm/pm.h>
#include <zephyr/pm/policy.h>
#include <zephyr/pm/device.h>
#include "../../zephyr/subsys/bluetooth/controller/include/ll.h"
#include <zephyr/drivers/i2c.h>
#include <errno.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>

#include "NfcTag.h"
#include "GDEW0154M10.h"
#include "epaper.h"
#include "main.h"

//*** After writing send the command nrfjprog --reset

#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)

#define NFC_GPO_WAIT_TIME 10000

struct bt_le_adv_param ble_par;

uint8_t buffer_URL[URL_BUFFER] = {0};
uint8_t buffer_check_URL[URL_BUFFER] = {0};
uint8_t adv, epaper;
uint8_t check_adv, check_epaper, power, check_power;

int8_t bt_power;

/*
 * Set Advertisement data. Based on the Eddystone specification:
 * https://github.com/google/eddystone/blob/master/protocol-specification.md
 * https://github.com/google/eddystone/tree/master/eddystone-url
 */

struct gpio_dt_spec pwr_pin = GPIO_DT_SPEC_GET(DT_NODELABEL(pwrgpios), gpios);
struct gpio_dt_spec nfc_gpo_pin = GPIO_DT_SPEC_GET(DT_NODELABEL(nfcgpo), gpios);

void set_tx_power(uint8_t handle_type, uint16_t handle, int8_t tx_pwr_lvl);

static struct gpio_callback nfc_gpo_cb;
K_SEM_DEFINE(nfc_gpo_sem, 0, 1);

#define Power_EN()                                \
	gpio_pin_configure_dt(&pwr_pin, GPIO_OUTPUT); \
	gpio_pin_set_dt(&pwr_pin, 0);                 \
	k_msleep(100);
// #define Power_DI() gpio_pin_set_dt(&pwr_pin, 1)

#define Power_DI() \
	k_msleep(20);  \
	gpio_pin_configure_dt(&pwr_pin, GPIO_DISCONNECTED)

uint8_t beacon_data[URL_BUFFER + 6] = {0};

static struct bt_data ad[3] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_NO_BREDR),
	BT_DATA_BYTES(BT_DATA_UUID16_ALL, 0xaa, 0xfe),
	{0}};

/* Set Scan Response data */
static struct bt_data sd[] = {
	BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
};

/* Configure GPIO pads as pPin Reset pin if Pin Reset capabilities desired. If CONFIG_GPIO_AS_PINRESET is not
	defined, pin reset will not be available. One GPIO (see Product Specification to see which one) will then be
	reserved for PinReset and not available as normal GPIO. */

static void bt_ready(int err)
{
	char addr_s[BT_ADDR_LE_STR_LEN];
	bt_addr_le_t addr = {0};
	size_t count = 1;
	if (err)
	{
		return;
	}

	/* Start advertising */
	err = bt_le_adv_start(&ble_par, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
	if (err)
	{
		return;
	}

	/* For connectable advertising you would use
	 * bt_le_oob_get_local().  For non-connectable non-identity
	 * advertising an non-resolvable private address is used;
	 * there is no API to retrieve that.
	 */

	bt_id_get(&addr, &count);
	bt_addr_le_to_str(&addr, addr_s, sizeof(addr_s));
	set_tx_power(BT_HCI_VS_LL_HANDLE_TYPE_ADV, 0, bt_power);
}

void set_tx_power(uint8_t handle_type, uint16_t handle, int8_t tx_pwr_lvl)
{
	struct bt_hci_cp_vs_write_tx_power_level *cp;
	struct bt_hci_rp_vs_write_tx_power_level *rp;
	struct net_buf *buf, *rsp = NULL;
	int err;

	buf = bt_hci_cmd_create(BT_HCI_OP_VS_WRITE_TX_POWER_LEVEL,
							sizeof(*cp));
	if (!buf)
	{
		return;
	}

	cp = net_buf_add(buf, sizeof(*cp));
	cp->handle = sys_cpu_to_le16(handle);
	cp->handle_type = handle_type;
	cp->tx_power_level = tx_pwr_lvl;

	err = bt_hci_cmd_send_sync(BT_HCI_OP_VS_WRITE_TX_POWER_LEVEL, buf, &rsp);
	if (err)
	{
		return;
	}

	rp = (void *)rsp->data;
	net_buf_unref(rsp);
}

void nfc_gpo_event(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	ARG_UNUSED(dev);
	ARG_UNUSED(cb);
	ARG_UNUSED(pins);

	k_sem_give(&nfc_gpo_sem);
}

int main(void)
{

	uint16_t adv_temp;
	k_msleep(5000);
	Power_EN();
	k_msleep(1000);
	NfcTag_init();

	//	uint8_t pwd[4]={0};
	// NfcTag_submitPassWd(pwd);

	//	NfcTag_clearSectorProtect();

	if (NFCGetData(&buffer_URL[0], &power, &adv, &epaper) == NFC_ERROR_READ)
	{
		strcpy(&buffer_URL[0], "URL_ERROR.com");
	}
	else
	{
		NfcTag_HideConfig(strlen(buffer_URL));
	}

	if (epaper == 1)
	{
		Epaper_InitQR(buffer_URL);
		Epaper_Init();
		Epaper_PrintQR();
	}

	switch (power)
	{
	case 0:
		bt_power = RADIO_TXPOWER_TXPOWER_Neg40dBm;
		break;

	case 1:
		bt_power = RADIO_TXPOWER_TXPOWER_Neg20dBm;
		break;

	case 2:
		bt_power = RADIO_TXPOWER_TXPOWER_Neg16dBm;
		break;

	case 3:
		bt_power = RADIO_TXPOWER_TXPOWER_Neg12dBm;
		break;

	case 4:
		bt_power = RADIO_TXPOWER_TXPOWER_Neg8dBm;
		break;

	case 5:
		bt_power = RADIO_TXPOWER_TXPOWER_Neg4dBm;
		break;

	case 6:
		bt_power = RADIO_TXPOWER_TXPOWER_0dBm;
		break;

	case 7:
		bt_power = RADIO_TXPOWER_TXPOWER_Pos3dBm;
		break;

	case 8:
		bt_power = RADIO_TXPOWER_TXPOWER_Pos4dBm;
		break;

	default:
		bt_power = RADIO_TXPOWER_TXPOWER_0dBm;
		break;
	}

	beacon_data[0] = 0xaa;	   /* Eddystone UUID MSB*/
	beacon_data[1] = 0xfe;	   /* Eddystone UUID LSB*/
	beacon_data[2] = 0x10;	   /* Eddystone-URL frame type */
	beacon_data[3] = bt_power; /* Calibrated Tx power in db */
	beacon_data[4] = 0x00;	   /* URL Scheme Prefix http://www. */
	strcpy(&beacon_data[5], buffer_URL);

	ad[2].data = (const uint8_t *)&beacon_data[0];
	ad[2].type = BT_DATA_SVC_DATA16;
	ad[2].data_len = strlen(buffer_URL) + 5;

	/* Initialize the Bluetooth Subsystem */

	ble_par.id = BT_ID_DEFAULT;
	ble_par.sid = 0;
	ble_par.secondary_max_skip = 0;
	ble_par.options = BT_LE_ADV_OPT_USE_IDENTITY;
	ble_par.peer = NULL;

	adv_temp = adv * 160;
	if (adv_temp < 0x0020)
	{
		adv_temp = 0x0020;
	}
	if (adv_temp > 0x4000)
	{
		adv_temp = 0x4000;
	}

	ble_par.interval_max = adv_temp;
	adv_temp = adv_temp - 10;
	if (adv_temp < 0x0020)
	{
		adv_temp = 0x0020;
	}
	ble_par.interval_min = adv_temp;

	bt_enable(bt_ready);

	set_tx_power(BT_HCI_VS_LL_HANDLE_TYPE_ADV, 0, bt_power);

	NfcTag_setEnergyHarvesting(true);
	NfcTag_GoSleep();
	if (epaper == 1)
	{
		Epaper_Gosleep();
	}
	Power_DI();

	gpio_pin_configure_dt(&nfc_gpo_pin, GPIO_INPUT);
	gpio_init_callback(&nfc_gpo_cb, nfc_gpo_event, BIT(nfc_gpo_pin.pin));
	gpio_add_callback(nfc_gpo_pin.port, &nfc_gpo_cb);
	gpio_pin_interrupt_configure_dt(&nfc_gpo_pin, GPIO_INT_EDGE_FALLING);

	while (1)
	{
		if (k_sem_take(&nfc_gpo_sem, K_MSEC(NFC_GPO_WAIT_TIME)) == 0)
		{
			Power_EN();
			NfcTag_Resume();
			NfcTag_setEnergyHarvesting(false);
			if (NfcTag_wasRfWriteEvent() &&
				(NFCGetData(&buffer_check_URL[0], &check_power, &check_adv, &check_epaper) != NFC_ERROR_READ))
			{
				if ((strcmp(buffer_check_URL, buffer_URL) != 0) || (check_power != power) || (check_adv != adv) || (check_epaper != epaper))
				{
					NfcTag_setEnergyHarvesting(true);
					sys_reboot(SYS_REBOOT_COLD);
				}
				else
				{
					buffer_check_URL[0] = 0;
					check_power = 0xFF;
					check_adv = 0xFF;
					check_epaper = 0xFF;
				}
			}
			NfcTag_setEnergyHarvesting(true);
			NfcTag_GoSleep();
			Power_DI();
		}
	}
}
