/******************************************************************************
 *
 * Copyright(c) 2007 - 2011 Realtek Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 *
 *
 ******************************************************************************/
#ifndef __RTW_EEPROM_H__
#define __RTW_EEPROM_H__


#define	RTL8712_EEPROM_ID			0x8712
//#define	EEPROM_MAX_SIZE			256

#define	HWSET_MAX_SIZE_128		128
#define	HWSET_MAX_SIZE_256		256
#define	HWSET_MAX_SIZE_512		512

#define	EEPROM_MAX_SIZE			HWSET_MAX_SIZE_512

#define	CLOCK_RATE					50			//100us

//- EEPROM opcodes
#define EEPROM_READ_OPCODE		06
#define EEPROM_WRITE_OPCODE		05
#define EEPROM_ERASE_OPCODE		07
#define EEPROM_EWEN_OPCODE		19      // Erase/write enable
#define EEPROM_EWDS_OPCODE		16      // Erase/write disable

//Country codes
#define USA							0x555320
#define EUROPE						0x1 //temp, should be provided later
#define JAPAN						0x2 //temp, should be provided later

#ifdef CONFIG_SDIO_HCI
#define eeprom_cis0_sz	17
#define eeprom_cis1_sz	50
#endif

//
// Customer ID, note that:
// This variable is initiailzed through EEPROM or registry,
// however, its definition may be different with that in EEPROM for
// EEPROM size consideration. So, we have to perform proper translation between them.
// Besides, CustomerID of registry has precedence of that of EEPROM.
// defined below. 060703, by rcnjko.
//
typedef enum _RT_CUSTOMER_ID
{
	RT_CID_DEFAULT = 0,
	RT_CID_8187_ALPHA0 = 1,
	RT_CID_8187_SERCOMM_PS = 2,
	RT_CID_8187_HW_LED = 3,
	RT_CID_8187_NETGEAR = 4,
	RT_CID_WHQL = 5,
	RT_CID_819x_CAMEO  = 6,
	RT_CID_819x_RUNTOP = 7,
	RT_CID_819x_Senao = 8,
	RT_CID_TOSHIBA = 9,	// Merge by Jacken, 2008/01/31.
	RT_CID_819x_Netcore = 10,
	RT_CID_Nettronix = 11,
	RT_CID_DLINK = 12,
	RT_CID_PRONET = 13,
	RT_CID_COREGA = 14,
	RT_CID_CHINA_MOBILE = 15,
	RT_CID_819x_ALPHA = 16,
	RT_CID_819x_Sitecom = 17,
	RT_CID_CCX = 18, // It's set under CCX logo test and isn't demanded for CCX functions, but for test behavior like retry limit and tx report. By Bruce, 2009-02-17.
	RT_CID_819x_Lenovo = 19,
	RT_CID_819x_QMI = 20,
	RT_CID_819x_Edimax_Belkin = 21,
	RT_CID_819x_Sercomm_Belkin = 22,
	RT_CID_819x_CAMEO1 = 23,
	RT_CID_819x_MSI = 24,
	RT_CID_819x_Acer = 25,
	RT_CID_819x_AzWave_ASUS = 26,
	RT_CID_819x_AzWave = 27, // For AzWave in PCIe, The ID is AzWave use and not only Asus
	RT_CID_819x_HP = 28,
	RT_CID_819x_WNC_COREGA = 29,
	RT_CID_819x_Arcadyan_Belkin = 30,
	RT_CID_819x_SAMSUNG = 31,
	RT_CID_819x_CLEVO = 32,
	RT_CID_819x_DELL = 33,
	RT_CID_819x_PRONETS = 34,
	RT_CID_819x_Edimax_ASUS = 35,
	RT_CID_NETGEAR = 36,
	RT_CID_PLANEX = 37,
	RT_CID_CC_C = 38,
	RT_CID_819x_Xavi = 39,
	RT_CID_LENOVO_CHINA = 40,
	RT_CID_INTEL_CHINA = 41,
	RT_CID_TPLINK_HPWR = 42,
	RT_CID_819x_Sercomm_Netgear = 43,
	RT_CID_819x_ALPHA_Dlink = 44,//add by ylb 20121012 for customer led for alpha
	RT_CID_WNC_NEC = 45,//add by page for NEC
	RT_CID_DNI_BUFFALO = 46,//add by page for NEC
}RT_CUSTOMER_ID, *PRT_CUSTOMER_ID;

struct eeprom_priv
{
	u8		bautoload_fail_flag;
	u8		bloadfile_fail_flag;
	u8		bloadmac_fail_flag;
	u8		EepromOrEfuse;

	u8		mac_addr[6];	//PermanentAddress

	u16		channel_plan;
	u16		CustomerID;

	u8		efuse_eeprom_data[EEPROM_MAX_SIZE]; //92C:256bytes, 88E:512bytes, we use union set (512bytes)
	u8		adjuseVoltageVal;

#ifdef CONFIG_RF_GAIN_OFFSET
		u8		EEPROMRFGainOffset;
		u8		EEPROMRFGainVal;
#endif //CONFIG_RF_GAIN_OFFSET

#ifdef CONFIG_SDIO_HCI
	u8		sdio_setting;
	u32		ocr;
	u8		cis0[eeprom_cis0_sz];
	u8		cis1[eeprom_cis1_sz];
#endif
};


extern void eeprom_write16(_adapter *padapter, u16 reg, u16 data);
extern u16 eeprom_read16(_adapter *padapter, u16 reg);
extern void read_eeprom_content(_adapter *padapter);
extern void eeprom_read_sz(_adapter * padapter, u16 reg,u8* data, u32 sz);

extern void read_eeprom_content_by_attrib(_adapter *	padapter	);

#ifdef PLATFORM_LINUX
#ifdef CONFIG_ADAPTOR_INFO_CACHING_FILE
extern int isAdaptorInfoFileValid(void);
extern int storeAdaptorInfoFile(char *path, struct eeprom_priv * eeprom_priv);
extern int retriveAdaptorInfoFile(char *path, struct eeprom_priv * eeprom_priv);
#endif //CONFIG_ADAPTOR_INFO_CACHING_FILE
#endif //PLATFORM_LINUX

#if 0 // alic
#else
/*9346CR*/
#define	_VPDIDX_MSK		0xFF00
#define	_VPDIDX_SHT		8
#define	_EEM_MSK		0x00C0
#define	_EEM_SHT		6
#define	_EEM0			BIT(6)
#define	_EEM1			BIT(7)
#define	_EEPROM_EN		BIT(5)
#define	_9356SEL		BIT(4)
#define	_EECS			BIT(3)
#define	_EESK			BIT(2)
#define	_EEDI			BIT(1)
#define	_EEDO			BIT(0)

#define RTL8712_IOBASE_TXPKT		0x10200000	/*IOBASE_TXPKT*/
#define RTL8712_IOBASE_RXPKT		0x10210000	/*IOBASE_RXPKT*/
#define RTL8712_IOBASE_RXCMD		0x10220000	/*IOBASE_RXCMD*/
#define RTL8712_IOBASE_TXSTATUS		0x10230000	/*IOBASE_TXSTATUS*/
#define RTL8712_IOBASE_RXSTATUS		0x10240000	/*IOBASE_RXSTATUS*/
#define RTL8712_IOBASE_IOREG		0x10250000	/*IOBASE_IOREG ADDR*/
#define RTL8712_IOBASE_SCHEDULER	0x10260000	/*IOBASE_SCHEDULE*/

#define RTL8712_IOBASE_TRXDMA		0x10270000	/*IOBASE_TRXDMA*/
#define RTL8712_IOBASE_TXLLT		0x10280000	/*IOBASE_TXLLT*/
#define RTL8712_IOBASE_WMAC		0x10290000	/*IOBASE_WMAC*/
#define RTL8712_IOBASE_FW2HW		0x102A0000	/*IOBASE_FW2HW*/
#define RTL8712_IOBASE_ACCESS_PHYREG	0x102B0000	/*IOBASE_ACCESS_PHYREG*/

#define RTL8712_IOBASE_FF	0x10300000 /*IOBASE_FIFO 0x1031000~0x103AFFFF*/

/*IOREG Offset for 8712*/
#define RTL8712_SYSCFG_		RTL8712_IOBASE_IOREG
#define RTL8712_CMDCTRL_	(RTL8712_IOBASE_IOREG + 0x40)
#define RTL8712_MACIDSETTING_	(RTL8712_IOBASE_IOREG + 0x50)
#define RTL8712_TIMECTRL_	(RTL8712_IOBASE_IOREG + 0x80)
#define RTL8712_FIFOCTRL_	(RTL8712_IOBASE_IOREG + 0xA0)
#define RTL8712_RATECTRL_	(RTL8712_IOBASE_IOREG + 0x160)
#define RTL8712_EDCASETTING_	(RTL8712_IOBASE_IOREG + 0x1D0)
#define RTL8712_WMAC_		(RTL8712_IOBASE_IOREG + 0x200)
#define RTL8712_SECURITY_	(RTL8712_IOBASE_IOREG + 0x240)
#define RTL8712_POWERSAVE_	(RTL8712_IOBASE_IOREG + 0x260)
#define RTL8712_GP_		(RTL8712_IOBASE_IOREG + 0x2E0)
#define RTL8712_INTERRUPT_	(RTL8712_IOBASE_IOREG + 0x300)
#define RTL8712_DEBUGCTRL_	(RTL8712_IOBASE_IOREG + 0x310)
#define RTL8712_OFFLOAD_	(RTL8712_IOBASE_IOREG + 0x2D0)

#define SYS_ISO_CTRL		(RTL8712_SYSCFG_ + 0x0000)
#define SYS_FUNC_EN		(RTL8712_SYSCFG_ + 0x0002)
#define PMC_FSM			(RTL8712_SYSCFG_ + 0x0004)
#define SYS_CLKR		(RTL8712_SYSCFG_ + 0x0008)
#define EE_9346CR		(RTL8712_SYSCFG_ + 0x000A)

#endif

#endif  //__RTL871X_EEPROM_H__

