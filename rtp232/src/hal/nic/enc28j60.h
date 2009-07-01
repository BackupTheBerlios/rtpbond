#ifndef _enc28j60_H_
#define _enc28j60_H_

#include <stdint.h> //Compiler Independent integers
#include <avr/io.h>   //Used for SPI Commands

// Definition of th RX and TX Buffers
#define ENC_ETXST  0x1A00
#define ENC_ETXND  0x1FFF
#define ENC_ERXST  0x0000
#define ENC_ERXND  0x19FF

// Op Codes send via the SPI Bus  
#define ENC_OP_BFC 0xA0
#define ENC_OP_BFS 0x80
#define ENC_OP_RCR 0x00
#define ENC_OP_RBM 0x3A
#define ENC_OP_WBM 0x7A
#define ENC_OP_WCR 0x40

// an enc register address is 5bit, a bank address 2bit. As the atmel 
// ... is a byte machine, we have 1 spare bit. It is used to
// ... mark the common registers, which are need no bank select.

#define ENC_MASK_BSEL_ALL 0x80
#define ENC_MASK_BSEL     0x60
#define ENC_MASK_REGADR   0x1F

#define ENC_BSEL_0   0x00
#define ENC_BSEL_1   0x20
#define ENC_BSEL_2   0x40
#define ENC_BSEL_3   0x60
#define ENC_BSEL_ALL 0x80


// BANK 0
#define ENC_REG_ERDPTL   (ENC_BSEL_0 + 0x00)   // Read Pointer Low Byte
#define ENC_REG_ERDPTH   (ENC_BSEL_0 + 0x01)   // Read Pointer High Byte
#define ENC_REG_EWRPTL   (ENC_BSEL_0 + 0x02)   // Write Pointer Low Byte
#define ENC_REG_EWRPTH   (ENC_BSEL_0 + 0x03)   // Write Pointer High Byte  
#define ENC_REG_ETXSTL   (ENC_BSEL_0 + 0x04)   // TX Start Low Byte
#define ENC_REG_ETXSTH   (ENC_BSEL_0 + 0x05)   // TX Start High Byte
#define ENC_REG_ETXNDL   (ENC_BSEL_0 + 0x06)   // TX End Low Byte
#define ENC_REG_ETXNDH   (ENC_BSEL_0 + 0x07)   // TX End High Byte
#define ENC_REG_ERXSTL   (ENC_BSEL_0 + 0x08)   // RX Start Low Byte
#define ENC_REG_ERXSTH   (ENC_BSEL_0 + 0x09)   // RX Start High Byte
#define ENC_REG_ERXNDL   (ENC_BSEL_0 + 0x0A)   // RX End Low Byte
#define ENC_REG_ERXNDH   (ENC_BSEL_0 + 0x0B)   // RX End High Byte
#define ENC_REG_ERXRDPTL (ENC_BSEL_0 + 0x0C)   // RX RD Pointer Low Byte
#define ENC_REG_ERXRDPTH (ENC_BSEL_0 + 0x0D)   // RX RD Pointer High Byte
#define ENC_REG_ERXWRPTL (ENC_BSEL_0 + 0x0E)   // RX WR Pointer Low Byte
#define ENC_REG_ERXWRPTH (ENC_BSEL_0 + 0x0F)   // RX WR Pointer High Byte

// BANK 1
#define ENC_REG_EPKTCNT  (ENC_BSEL_1 + 0x19)   // Ethernet Packet Count

// BANK 2
#define ENC_REG_MACON1   (ENC_BSEL_2 + 0x00)
  #define ENC_REG_MACON1_MARXEN   (1<<0)
  #define ENC_REG_MACON1_PASSALL  (1<<1)
  #define ENC_REG_MACON1_RXPAUS   (1<<2)
  #define ENC_REG_MACON1_TXPAUS   (1<<3)
  #define ENC_REG_MACON1_LOOPBK   (1<<4)
#define ENC_REG_MACON2   (ENC_BSEL_2 + 0x01)
  #define ENC_REG_MACON2_MARST    (1<<7)        // MAC Module Reset Bit
#define ENC_REG_MACON3   (ENC_BSEL_2 + 0x02)
  #define ENC_REG_MACON3_FULDPX   (1<<0)        // Full-Duplex Enable Bit (depends on PHCON1.PDPXMD)
  #define ENC_REG_MACON3_FRMLEN   (1<<1)        // Frame Length Checking Enabled Bit
  #define ENC_REG_MACON3_HFRMEN   (1<<2)        // Huge Frame Enable Bit
  #define ENC_REG_MACON3_PHDRLEN  (1<<3)        // Proprietary Header Bit
  #define ENC_REG_MACON3_TXCRCEN  (1<<4)        // Transmit CRC Enable Bit
  #define ENC_REG_MACON3_PADCFG0  (1<<5)
  #define ENC_REG_MACON3_PADCFG1  (1<<6)
  #define ENC_REG_MACON3_PADCFG2  (1<<7)
  #define ENC_REG_MACON3_PADCFG_MASK 0xE0       //Automatic Pad and CRC Configuration Bits
  
#define ENC_REG_MABBIPG  (ENC_BSEL_2 + 0x04)  // Back to Back Inter Packet Gap
#define ENC_REG_MAIPGL   (ENC_BSEL_2 + 0x06)  // Non Back to Back Inter Packet Gap Low Byte
#define ENC_REG_MAIPGH   (ENC_BSEL_2 + 0x07)  // Non Back to Back Inter Packet Gap High Byte
#define ENC_REG_MAMXFLL  (ENC_BSEL_2 + 0x0A)  // Maximum Frame Length Low Byte 
#define ENC_REG_MAMXFLH  (ENC_BSEL_2 + 0x0B)  // Maximum Frame Length High Byte 
#define ENC_REG_MIREGADR (ENC_BSEL_2 + 0x14)
#define ENC_REG_MIWRL    (ENC_BSEL_2 + 0x16)  // MII Write Data Low Byte
#define ENC_REG_MIWRH    (ENC_BSEL_2 + 0x17)  // MII Write Data High Byte
#define ENC_REG_MIRDL    (ENC_BSEL_2 + 0x18)  // MII Read Data Low Byte 
#define ENC_REG_MIRDH    (ENC_BSEL_2 + 0x19)  // MII Read Data High Byte

// BANK 3
#define ENC_REG_MAADR1   (ENC_BSEL_3 + 0x00)  // MAC Address Byte 1
#define ENC_REG_MAADR0   (ENC_BSEL_3 + 0x01)  // MAC Address Byte 0
#define ENC_REG_MAADR3   (ENC_BSEL_3 + 0x02)  // MAC Address Byte 0
#define ENC_REG_MAADR2   (ENC_BSEL_3 + 0x03)  // MAC Address Byte 0
#define ENC_REG_MAADR5   (ENC_BSEL_3 + 0x04)  // MAC Address Byte 0
#define ENC_REG_MAADR4   (ENC_BSEL_3 + 0x05)  // MAC Address Byte 4
#define ENC_REG_MISTAT   (ENC_BSEL_3 + 0x0A)
 #define ENC_REG_MISTAT_BUSY  (1<<0)
#define ENC_REG_EREVID    (ENC_BSEL_3 + 0x12)  // Ethernet Revision ID


// The registers above 0x1B are in all Banks the same
// threrefore we are using ENC_BSEL_ALL to mark them as special
#define ENC_REG_EEIE  (ENC_BSEL_ALL + 0x1B)
#define ENC_REG_EIR   (ENC_BSEL_ALL + 0x1C)   // Ethernet Interrupt Request Registers
 #define ENC_REG_EIR_PKTIF      (1<<6)        // Receive Packet Pending Interrupt Flag
#define ENC_REG_ESTAT (ENC_BSEL_ALL + 0x1D)
  #define ENC_REG_ESTAT_CLKRDY  (1<<0)
  #define ENC_REG_ESTAT_TXABRT  (1<<1)
  #define ENC_REG_ESTAT_LATECOL (1<<4)
#define ENC_REG_ECON2 (ENC_BSEL_ALL + 0x1E)
 #define ENC_REG_ECON2_PWRSV    (1<<5)
 #define ENC_REG_ECON2_PKTDEC   (1<<6)
 #define ENC_REG_ECON2_AUTOINC  (1<<7)
#define ENC_REG_ECON1 (ENC_BSEL_ALL + 0x1F) 
 #define ENC_REG_ECON1_BSEL0    (1<<0) // Bank Select Bit 0
 #define ENC_REG_ECON1_BSEL1    (1<<1) // Bank Select Bit 1
 #define ENC_REG_ECON1_RXEN     (1<<2) // Receive Enable bit
 #define ENC_REG_ECON1_TXRTS    (1<<3) // Transmit Request to Send bit
 #define ENC_REG_ECON1_RXRST    (1<<6) // Receive Logic Reset Bit
 #define ENC_REG_ECON1_TXRST    (1<<7) // Transmit Logic Reset bit




// Read a char from a Control Register
uint8_t enc28j60_crgetc(uint8_t address);
// Read a word from a Control Register
uint16_t enc28j60_crgetw(uint8_t address);
// Read a string from a Control Register
void* enc28j60_crgets(uint8_t address, void* buffer, uint16_t len);

// Write a char to control Registers
void enc28j60_crputc(uint8_t address, uint8_t buffer);
// Write a word to control Registers
void enc28j60_crputw(uint8_t address, uint16_t buffer);
// Write a string to control Registers
void enc28j60_crputs(uint8_t address, const void* buffer, uint16_t len);
void enc28j60_crput(uint8_t address, const void* buffer, uint16_t len);

// Bit Field Set
void enc28j60_crbfs(uint8_t address, char flags);
// Bit Field Clear
void enc28j60_crbfc(uint8_t address, char flags);

// Read a char from Buffer Memory
uint8_t  enc28j60_bmgetc();
// Read a word from Buffer Memory
uint16_t enc28j60_bmgetw();
// Read a string from Buffer Memory
void* enc28j60_bmgets(void* buffer, uint16_t len);

// Write Buffer Register
void enc28j60_bmputs(const void* buffer, uint16_t len);

// Write to Phy Registers
void enc28j60_prputw(uint8_t address, uint16_t buffer);

void enc28j60_init();

// Soft Reset
void enc28j60_reset();

#endif
