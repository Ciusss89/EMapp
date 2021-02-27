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

# Increase the messages level:
#  0: normal
#  1: Prints the infos about current transformer setup, adc setup
#  2: Adds the sempling debug messages
#  3: +++ Prints each adc outputs.
#  4: ++++ Print all
#
#  PLEASE NOTE: printfs breaks the timings
CFLAGS += -DVERBOSE=2

# Select the Current Transformer Type:
# 0: YHDC TA1020
# 1: YHDC SCT013-000
CFLAGS += -DCT_TYPE=1

# Max RMS current which wants measure
#
CFLAGS += -DRMS_MAX_CURRENT=15

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
