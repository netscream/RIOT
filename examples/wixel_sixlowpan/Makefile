# name of your application
APPLICATION = wixel_sixlowpan

# If no BOARD is found in the environment, use this default:
BOARD ?= cc2511

# For cc2511 we need to set the toolchain
TOOLCHAIN = mcs51

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../..

## Uncomment to support the XBee module and nomac
#USEMODULE += xbee
#USEMODULE += gnrc_nomac

## set default UART to use in case none was defined
#XBEE_UART ?= "UART_NUMOF-1"

## export UART to params file
#CFLAGS += -DXBEE_UART=$(XBEE_UART)

## add current directory to the include path. Putting it in CFLAGS will make
## it go to the beginning, before the standard includes.
## That way xbee_params.h get's included and auto configuration can pick it up.
#CFLAGS += -I$(CURDIR)

# Include packages that pull up and auto-init the link layer.
# NOTE: 6LoWPAN will be included if IEEE802.15.4 devices are present
#USEMODULE += gnrc_netdev_default
#USEMODULE += auto_init_gnrc_netif
# Specify the minimum networking modules for IPv6
#USEMODULE += gnrc_ipv6
#USEMODULE += gnrc_ndp
# Additional networking modules that can be dropped if not needed
#USEMODULE += gnrc_icmpv6_echo
# Use minimal standard PRNG
#USEMODULE += prng_minstd
USEMODULE += gnrc_sixlowpan
#USEMODULE += gnrc_nettype
# Set a custom 802.15.4 channel if needed
#DEFAULT_CHANNEL ?= 26
#CFLAGS += -DDEFAULT_CHANNEL=$(DEFAULT_CHANNEL)

#CFLAGS += -DGNRC_PKTBUF_SIZE=512 -DGNRC_IPV6_NETIF_ADDR_NUMOF=4 -DGNRC_IPV6_NC_SIZE=1

# Change this to 0 show compiler invocation lines by default:
#QUIET ?= 1

include $(RIOTBASE)/Makefile.include
