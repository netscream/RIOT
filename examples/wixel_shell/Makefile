# name of your application
APPLICATION = wixel_shell

# If no BOARD is found in the environment, use this default:
BOARD ?= cc2511

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../..

TOOLCHAIN = mcs51
#BOARD_INSUFFICIENT_MEMORY := airfy-beacon chronos msb-430 msb-430h nrf51dongle nrf6310 \
                             nucleo-f334 pca10000 pca10005 stm32f0discovery telosb weio \
                             wsn430-v1_3b wsn430-v1_4 yunjia-nrf51822 z1 nucleo-f072

# Include packages that pull up and auto-init the link layer.
# NOTE: 6LoWPAN will be included if IEEE802.15.4 devices are present
#USEMODULE += posix
#USEMODULE += gnrc_netdev_default
#USEMODULE += auto_init_gnrc_netif
# Specify the mandatory networking modules for socket communication via UDP
#USEMODULE += gnrc_ipv6_default
#USEMODULE += gnrc_udp
#USEMODULE += gnrc_conn_udp
#USEMODULE += posix_sockets
# Add also the shell, some shell commands
USEMODULE += shell
USEMODULE += shell_commands
#USEMODULE += ps

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
#CFLAGS += -DDEVELHELP

# Change this to 0 show compiler invocation lines by default:
#QUIET ?= 1

include $(RIOTBASE)/Makefile.include
