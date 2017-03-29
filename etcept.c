/*-
 * Copyright (C) 2017  Dridi Boukelmoune
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#define _GNU_SOURCE 1

#ifdef __linux__
#  include <sys/vfs.h>
#endif

#include <sys/stat.h>
#include <sys/types.h>

#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define ETCEPT_NEXT(ret, fn, ...) static ret (*next_##fn)(__VA_ARGS__)

#define ETCEPT_INIT(fn)					\
	do {						\
		next_##fn = dlsym(RTLD_NEXT, #fn);	\
	} while (0)

#define ETCEPT_CONDINIT(fn)			\
	do {					\
		/* XXX: racy */			\
		if (next_##fn == NULL)		\
			ETCEPT_INIT(fn);	\
	} while (0)

ETCEPT_NEXT(int, access, const char *, int);
ETCEPT_NEXT(FILE *, fopen, const char *, const char *);
ETCEPT_NEXT(FILE *, freopen, const char *, const char *, FILE *);
ETCEPT_NEXT(int, open, const char *, int, ...);
ETCEPT_NEXT(int, openat, int, const char *, int, ...);
ETCEPT_NEXT(int, stat, const char *, struct stat *);
#ifdef __linux__
ETCEPT_NEXT(int, statfs, const char *, struct statfs *);
#endif

static void __attribute__((constructor))
__etcept_init(void)
{

	ETCEPT_INIT(access);
	ETCEPT_INIT(fopen);
	ETCEPT_INIT(freopen);
	ETCEPT_INIT(open);
	ETCEPT_INIT(openat);
	ETCEPT_INIT(stat); /* XXX: lstat too? */
#ifdef __linux__
	ETCEPT_INIT(statfs); /* XXX: stat64 too? */
#endif
}

static int
__etcept_stat(const char *path)
{
	struct stat st;
	int retval;

#ifdef __linux__
	struct statfs stfs;
	ETCEPT_CONDINIT(statfs);
	if (next_statfs != NULL) {
		retval = next_statfs(path, &stfs);
		if (retval != 0)
			return (retval);
		/* TODO: check regular file */
		return (0);
	}
#endif

	ETCEPT_CONDINIT(stat);
	if (next_stat == NULL)
		return (-1);

	retval = next_stat(path, &st);
	if (retval != 0)
		return (retval);

	if ((st.st_mode & S_IFREG) != S_IFREG)
		return (-1);

	return (0);
}

static unsigned
__etcept_sane(const char *path)
{
	int dots = 0;

	while (*path) {
		if (*path == '/' && (dots == 1 || dots == 2))
			return (0);
		if (*path == '.')
			dots++;
		else
			dots = 0;
		path++;
	}

	return (dots != 1 && dots != 2);
}

static const char *
__etcept_path(const char *path)
{

	/* NB: goto fail without goto */
	do {
		if (path == NULL)
			break;

		if (strncmp("/etc/", path, sizeof "/etc/" - 1))
			break;

		if (!__etcept_sane(path))
			break;

		if (__etcept_stat(path) != 0)
			break;

		path++; /* etcepted */
	} while (0);

	errno = 0;
	return (path);
}

int
access(const char *path, int mode)
{

	ETCEPT_CONDINIT(access);
	path = __etcept_path(path);
	return (next_access(path, mode));
}

FILE *
fopen(const char *path, const char *mode)
{

	ETCEPT_CONDINIT(fopen);
	path = __etcept_path(path);
	return (next_fopen(path, mode));
}

FILE *
freopen(const char *path, const char *mode, FILE *stream)
{

	ETCEPT_CONDINIT(freopen);
	path = __etcept_path(path);
	return (next_freopen(path, mode, stream));
}


#define ETCEPT_OPEN(fn, ...)				\
do {							\
	va_list ap;					\
	int has_mode;					\
	mode_t mode = -1;				\
							\
	ETCEPT_CONDINIT(fn);				\
							\
	has_mode = flags & (O_CREAT | O_TMPFILE);	\
	if (has_mode) {					\
		va_start(ap, flags);			\
		mode = va_arg(ap, mode_t);		\
		va_end(ap);				\
	}						\
							\
	path = __etcept_path(path);			\
							\
	if (has_mode)					\
		return (next_##fn(__VA_ARGS__, mode));	\
							\
	return (next_##fn(__VA_ARGS__));		\
} while (0)

int
open(const char *path, int flags, ...)
{

	ETCEPT_OPEN(open, path, flags);
}

int
openat(int dirfd, const char *path, int flags, ...)
{

	ETCEPT_OPEN(openat, dirfd, path, flags);
}

int
stat(const char *path, struct stat *buf)
{

	ETCEPT_CONDINIT(stat);
	path = __etcept_path(path);
	return (next_stat(path, buf));
}

#ifdef __linux__
int
statfs(const char *path, struct statfs *buf)
{

	ETCEPT_CONDINIT(statfs);
	path = __etcept_path(path);
	return (next_statfs(path, buf));
}
#endif
