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
#  1: Print the infos about current transformer setup, adc setup
#  2: Print 1 + the sempling debug messages
#  3: Print 1 + last 60 seconds measure.
#
#  PLEASE NOTE: printfs breaks the timings
CFLAGS += -DVERBOSE=3

# Select the Current Transformer Type:
# 0: YHDC TA1020
# 1: YHDC SCT013-000
CFLAGS += -DCT_TYPE=1

# Max RMS current which wants measure
#
# Note: It returns the burden resistor in relation RMS_MAX_CURRENT
CFLAGS += -DRMS_MAX_CURRENT=15

# Sempling Unit:
# - According to the NYQUEST rule a 100Hz sampling frequency should be enough,
#   but this ins't true if you have to rebuild the signal..
#
# - Get at least 12 samples to compute a RMS value. The sampling time is
#   self computed. Using 12 samples you are sampling at 600Hz
CFLAGS += -DSAMPLE_UNIT=12

# ADC resolution:
#   0: use 12 BIT
#   1: use 10 BIT
#   2: use 8 BIT
#
# - Note: If you are using the CT with poor RC circuit as transconductance
#	  amplifier it's needless to set high resolution due to noise...
#	  You will use a high resolution if your probe has goog a 
#	  transconductance amplifier.
CFLAGS += -DBIT=2

# To protect the MCU's GPIO enter the max (mA) current which can sink
#
CFLAGS += -DMCU_MAX_CURRENT_SINK=20

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
