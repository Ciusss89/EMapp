# name of your application
APPLICATION = gtip-em_app

# If no BOARD is found in the environment, use this default:
BOARD ?= nucleo-l476rg

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../RIOT/

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
#CFLAGS += -DDEVELHELP

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

DIRS += $(CURDIR)/energy_meter

# Requirement
FEATURES_REQUIRED += periph_adc

# Modules to include:
USEMODULE += shell
USEMODULE += shell_commands
USEMODULE += ps
USEMODULE += energy_meter
USEMODULE += printf_float
USEMODULE += xtimer

CFLAGS += -I$(CURDIR)
include $(RIOTBASE)/Makefile.include
