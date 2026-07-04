// SPDX-FileCopyrightText: 2016-2019, 2022, 2026 Carlo Wood
// SPDX-License-Identifier: MIT

/**
 * cwds -- Application-side libcwd support code.
 *
 * @file
 * @brief This file contains the declaration of signal_safe_printf.
 */

#pragma once

#include <cstdarg>

void signal_safe_printf(char const* fmt, ...);
