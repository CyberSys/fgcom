/*
 * iaxclient: a cross-platform IAX softphone library
 *
 * Copyrights:
 * Copyright (C) 2003-2006, Horizon Wimba, Inc.
 * Copyright (C) 2007, Wimba, Inc.
 *
 * Contributors:
 * Steve Kann <stevek@stevek.com>
 *
 * This program is free software, distributed under the terms of
 * the GNU Lesser (Library) General Public License.
 */

#ifndef _AUDIO_OPENAL_H
#define _AUDIO_OPENAL_H

#include <al.h>
#include <alc.h>
#include <iaxclient_lib.h>

#ifdef __DEBUG__
#define PRINTDBG(fmt, args...)			g_printf("%s,%d: "fmt, __FUNCTION__,__LINE__,##args)
#define EPRINT(fmt, args...)			g_printf(fmt,##args)
#else
#define PRINTDBG(fmt, args...)
#define EPRINT(fmt, args...)
#endif

int openal_initialize(struct iaxc_audio_driver *driver, int sample_rate);
int openal_finalize();

#endif
