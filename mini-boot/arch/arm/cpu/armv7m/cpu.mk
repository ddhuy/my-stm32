SRCS     += arch/$(ARCH)/cpu/$(CPU)/start.S
SRCS     += arch/$(ARCH)/cpu/$(CPU)/fault.c
LDSCRIPT := arch/$(ARCH)/cpu/$(CPU)/link.ld
