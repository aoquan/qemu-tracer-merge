CONFIG_AC97:=$(findstring y,$(CONFIG_AC97)y)
CONFIG_ACPI_CPU_HOTPLUG:=$(findstring y,$(CONFIG_ACPI_CPU_HOTPLUG)y)
CONFIG_ACPI:=$(findstring y,$(CONFIG_ACPI)y)
CONFIG_ACPI_MEMORY_HOTPLUG:=$(findstring y,$(CONFIG_ACPI_MEMORY_HOTPLUG)y)
CONFIG_ACPI_X86:=$(findstring y,$(CONFIG_ACPI_X86)y)
CONFIG_ACPI_X86_ICH:=$(findstring y,$(CONFIG_ACPI_X86_ICH)y)
CONFIG_ADLIB:=$(findstring y,$(CONFIG_ADLIB)y)
CONFIG_AHCI:=$(findstring y,$(CONFIG_AHCI)y)
CONFIG_APIC:=$(findstring y,$(CONFIG_APIC)y)
CONFIG_APM:=$(findstring y,$(CONFIG_APM)y)
CONFIG_APPLESMC:=$(findstring y,$(CONFIG_APPLESMC)y)
CONFIG_CS4231A:=$(findstring y,$(CONFIG_CS4231A)y)
CONFIG_E1000_PCI:=$(findstring y,$(CONFIG_E1000_PCI)y)
CONFIG_EDU:=$(findstring y,$(CONFIG_EDU)y)
CONFIG_EEPRO100_PCI:=$(findstring y,$(CONFIG_EEPRO100_PCI)y)
CONFIG_ES1370:=$(findstring y,$(CONFIG_ES1370)y)
CONFIG_ESP:=$(findstring y,$(CONFIG_ESP)y)
CONFIG_ESP_PCI:=$(findstring y,$(CONFIG_ESP_PCI)y)
CONFIG_FDC:=$(findstring y,$(CONFIG_FDC)y)
CONFIG_GUS:=$(findstring y,$(CONFIG_GUS)y)
CONFIG_HDA:=$(findstring y,$(CONFIG_HDA)y)
CONFIG_HPET:=$(findstring y,$(CONFIG_HPET)y)
CONFIG_I8254:=$(findstring y,$(CONFIG_I8254)y)
CONFIG_I8257:=$(findstring y,$(CONFIG_I8257)y)
CONFIG_I8259:=$(findstring y,$(CONFIG_I8259)y)
CONFIG_I82801B11:=$(findstring y,$(CONFIG_I82801B11)y)
CONFIG_ICC_BUS:=$(findstring y,$(CONFIG_ICC_BUS)y)
CONFIG_IDE_CORE:=$(findstring y,$(CONFIG_IDE_CORE)y)
CONFIG_IDE_ISA:=$(findstring y,$(CONFIG_IDE_ISA)y)
CONFIG_IDE_PCI:=$(findstring y,$(CONFIG_IDE_PCI)y)
CONFIG_IDE_PIIX:=$(findstring y,$(CONFIG_IDE_PIIX)y)
CONFIG_IDE_QDEV:=$(findstring y,$(CONFIG_IDE_QDEV)y)
CONFIG_IOAPIC:=$(findstring y,$(CONFIG_IOAPIC)y)
CONFIG_IOH3420:=$(findstring y,$(CONFIG_IOH3420)y)
CONFIG_IPACK:=$(findstring y,$(CONFIG_IPACK)y)
CONFIG_ISA_DEBUG:=$(findstring y,$(CONFIG_ISA_DEBUG)y)
CONFIG_ISA_TESTDEV:=$(findstring y,$(CONFIG_ISA_TESTDEV)y)
CONFIG_IVSHMEM:=$(findstring y,$(CONFIG_IVSHMEM)$(CONFIG_KVM))
CONFIG_LPC_ICH9:=$(findstring y,$(CONFIG_LPC_ICH9)y)
CONFIG_LSI_SCSI_PCI:=$(findstring y,$(CONFIG_LSI_SCSI_PCI)y)
CONFIG_MC146818RTC:=$(findstring y,$(CONFIG_MC146818RTC)y)
CONFIG_MEGASAS_SCSI_PCI:=$(findstring y,$(CONFIG_MEGASAS_SCSI_PCI)y)
CONFIG_MEM_HOTPLUG:=$(findstring y,$(CONFIG_MEM_HOTPLUG)y)
CONFIG_NE2000_ISA:=$(findstring y,$(CONFIG_NE2000_ISA)y)
CONFIG_NE2000_PCI:=$(findstring y,$(CONFIG_NE2000_PCI)y)
CONFIG_NVME_PCI:=$(findstring y,$(CONFIG_NVME_PCI)y)
CONFIG_PAM:=$(findstring y,$(CONFIG_PAM)y)
CONFIG_PARALLEL:=$(findstring y,$(CONFIG_PARALLEL)y)
CONFIG_PCI:=$(findstring y,$(CONFIG_PCI)y)
CONFIG_PCI_PIIX:=$(findstring y,$(CONFIG_PCI_PIIX)y)
CONFIG_PCI_Q35:=$(findstring y,$(CONFIG_PCI_Q35)y)
CONFIG_PCI_TESTDEV:=$(findstring y,$(CONFIG_PCI_TESTDEV)y)
CONFIG_PCKBD:=$(findstring y,$(CONFIG_PCKBD)y)
CONFIG_PCNET_COMMON:=$(findstring y,$(CONFIG_PCNET_COMMON)y)
CONFIG_PCNET_PCI:=$(findstring y,$(CONFIG_PCNET_PCI)y)
CONFIG_PCSPK:=$(findstring y,$(CONFIG_PCSPK)y)
CONFIG_PFLASH_CFI01:=$(findstring y,$(CONFIG_PFLASH_CFI01)y)
CONFIG_PIIX_PCI:=$(findstring y,$(CONFIG_PIIX_PCI)y)
CONFIG_PVPANIC:=$(findstring y,$(CONFIG_PVPANIC)y)
CONFIG_QXL:=$(findstring y,$(CONFIG_QXL)$(CONFIG_SPICE))
CONFIG_ROCKER:=$(findstring y,$(CONFIG_ROCKER)y)
CONFIG_RTL8139_PCI:=$(findstring y,$(CONFIG_RTL8139_PCI)y)
CONFIG_SB16:=$(findstring y,$(CONFIG_SB16)y)
CONFIG_SD:=$(findstring y,$(CONFIG_SD)y)
CONFIG_SDHCI:=$(findstring y,$(CONFIG_SDHCI)y)
CONFIG_SERIAL:=$(findstring y,$(CONFIG_SERIAL)y)
CONFIG_SERIAL_PCI:=$(findstring y,$(CONFIG_SERIAL_PCI)y)
CONFIG_SGA:=$(findstring y,$(CONFIG_SGA)y)
CONFIG_SMBIOS:=$(findstring y,$(CONFIG_SMBIOS)y)
CONFIG_TPM_TIS:=$(findstring y,$(CONFIG_TPM_TIS)$(CONFIG_TPM))
CONFIG_USB_AUDIO:=$(findstring y,$(CONFIG_USB_AUDIO)y)
CONFIG_USB_BLUETOOTH:=$(findstring y,$(CONFIG_USB_BLUETOOTH)y)
CONFIG_USB_EHCI:=$(findstring y,$(CONFIG_USB_EHCI)y)
CONFIG_USB:=$(findstring y,$(CONFIG_USB)y)
CONFIG_USB_NETWORK:=$(findstring y,$(CONFIG_USB_NETWORK)y)
CONFIG_USB_OHCI:=$(findstring y,$(CONFIG_USB_OHCI)y)
CONFIG_USB_SERIAL:=$(findstring y,$(CONFIG_USB_SERIAL)y)
CONFIG_USB_SMARTCARD:=$(findstring y,$(CONFIG_USB_SMARTCARD)y)
CONFIG_USB_STORAGE_BOT:=$(findstring y,$(CONFIG_USB_STORAGE_BOT)y)
CONFIG_USB_STORAGE_MTP:=$(findstring y,$(CONFIG_USB_STORAGE_MTP)y)
CONFIG_USB_STORAGE_UAS:=$(findstring y,$(CONFIG_USB_STORAGE_UAS)y)
CONFIG_USB_TABLET_WACOM:=$(findstring y,$(CONFIG_USB_TABLET_WACOM)y)
CONFIG_USB_UHCI:=$(findstring y,$(CONFIG_USB_UHCI)y)
CONFIG_USB_XHCI:=$(findstring y,$(CONFIG_USB_XHCI)y)
CONFIG_VGA_CIRRUS:=$(findstring y,$(CONFIG_VGA_CIRRUS)y)
CONFIG_VGA:=$(findstring y,$(CONFIG_VGA)y)
CONFIG_VGA_ISA:=$(findstring y,$(CONFIG_VGA_ISA)y)
CONFIG_VGA_PCI:=$(findstring y,$(CONFIG_VGA_PCI)y)
CONFIG_VIRTIO:=$(findstring y,$(CONFIG_VIRTIO)y)
CONFIG_VIRTIO_PCI:=$(findstring y,$(CONFIG_VIRTIO_PCI)y)
CONFIG_VIRTIO_VGA:=$(findstring y,$(CONFIG_VIRTIO_VGA)y)
CONFIG_VMMOUSE:=$(findstring y,$(CONFIG_VMMOUSE)y)
CONFIG_VMPORT:=$(findstring y,$(CONFIG_VMPORT)y)
CONFIG_VMWARE_VGA:=$(findstring y,$(CONFIG_VMWARE_VGA)y)
CONFIG_VMW_PVSCSI_SCSI_PCI:=$(findstring y,$(CONFIG_VMW_PVSCSI_SCSI_PCI)y)
CONFIG_VMXNET3_PCI:=$(findstring y,$(CONFIG_VMXNET3_PCI)y)
CONFIG_WDT_IB6300ESB:=$(findstring y,$(CONFIG_WDT_IB6300ESB)y)
CONFIG_WDT_IB700:=$(findstring y,$(CONFIG_WDT_IB700)y)
CONFIG_XEN_I386:=$(findstring y,$(CONFIG_XEN_I386)$(CONFIG_XEN))
CONFIG_XIO3130:=$(findstring y,$(CONFIG_XIO3130)y)
