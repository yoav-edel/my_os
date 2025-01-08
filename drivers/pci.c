//
// Created by Yoav on 1/6/2025.
//

#include "pci.h"
#include "io.h"

uint32_t pci_config_read(uint8_t bus, uint8_t device, uint8_t function, uint8_t reg) {
    uint32_t address = pci_config_address(bus, device, function, reg);
    out32(PCI_CONFIG_ADDRESS, address);
    return in32(PCI_CONFIG_DATA);
}

void pci_config_write(uint8_t bus, uint8_t device, uint8_t function, uint8_t reg, uint32_t data) {
    uint32_t address = pci_config_address(bus, device, function, reg);
    out32(PCI_CONFIG_ADDRESS, address);
    out32(PCI_CONFIG_DATA, data);
}


