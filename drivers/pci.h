//
// Created by Yoav on 1/6/2025.
//


#ifndef MYKERNELPROJECT_PCI_H
#define MYKERNELPROJECT_PCI_H

#include "../std/stdint.h"

#define PCI_CONFIG_ADDRESS 0xCF8 // Config Address io port - used to select the PCI bus, device, function and register
#define PCI_CONFIG_DATA 0xCFC // Config Data io port - used to read/write the configuration data

/*
 * Create the address for the PCI configuration space
 * The address is a 32 bit number that contains the bus, device, function and register
 */
inline uint32_t pci_config_address(uint8_t bus, uint8_t device, uint8_t function, uint8_t reg) {
    return ((1 << 31) | (bus << 16) | (device << 11) | (function << 8) | (reg & 0xFC));
}

uint32_t pci_config_read(uint8_t bus, uint8_t device, uint8_t function, uint8_t reg);

void pci_config_write(uint8_t bus, uint8_t device, uint8_t function, uint8_t reg, uint32_t data);

/*
 * Enable the bus mastering of the PCI device - this is needed for the device to be able to use DMA.
 * Theroetically this enable the comunications between two devices without the need of the CPU
 */
void pci_enable_bus_mastering(uint8_t bus, uint8_t device, uint8_t function);


/*
 * Set the base address registers of the PCI device
 */
void pci_set_bars(uint8_t bus, uint8_t device, uint8_t function);


void pci_init();
#endif //MYKERNELPROJECT_PCI_H
