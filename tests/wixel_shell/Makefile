APPLICATION = wixel_shell
include ../Makefile.tests_common
BOARD = cc2511
TOOLCHAIN = mcs51
QUIET = 0
USEMODULE += shell
USEMODULE += shell_commands
USEMODULE += ps

DISABLE_MODULE += auto_init

# chronos is missing a getchar implementation
#BOARD_BLACKLIST += chronos

include $(RIOTBASE)/Makefile.include
