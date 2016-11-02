/*
 * Copyright (C) 2014 Oliver Hahm
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @addtogroup  core_util
 * @{
 *
 * @file
 * @brief       Types used by the kernel
 *
 * @author      Oliver Hahm <oliver.hahm@inria.fr>
 * @author      René Kijewski <rene.kijewski@fu-berlin.de>
 * @author      Ludwig Knüpfer <ludwig.knuepfer@fu-berlin.de>
 */
#include "kernel_types.h"

static int pid_is_valid(kernel_pid_t pid)
{
	return ((1 <= pid) && (pid <= (MAXTHREADS - 1)));
}
