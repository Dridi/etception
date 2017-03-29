#!/bin/sh
#
# Copyright (C) 2017  Dridi Boukelmoune
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

set -e
set -u

# Fixtures
# ========

check() {
	printf 'Running: %s\n===\n' "$*"
	if ! LD_PRELOAD="$ETCEPT_PRELOAD" "$@"
	then
		printf "Failed: LD_PRELOAD=%s %s\n" "$ETCEPT_PRELOAD" "$*" >&2
		return 1
	fi
	printf '\n'
}

xfail() {
	! "$@"
}

# Init
# ====

readonly SRCDIR=$(dirname "$0")
readonly TMPDIR=$(mktemp -d test.XXXXXX)

readonly LIBDIR=$(cd "$SRCDIR"; pwd)
readonly WRKDIR=$(cd "$TMPDIR"; pwd)

trap 'cd /; rm -rf "$WRKDIR"' EXIT

cd "$WRKDIR"

mkdir etc

readonly SHARED=${SHARED:-libetcept.so}

if [ -n "${LD_PRELOAD:-}" ]
then
	readonly ETCEPT_PRELOAD="$LD_PRELOAD:$LIBDIR/$SHARED"
else
	readonly ETCEPT_PRELOAD="$LIBDIR/$SHARED"
fi

# Checks
# ======

check xfail getent hosts unlikely.etception.domain.name

echo '::1 unlikely.etception.domain.name' > etc/hosts

check getent hosts unlikely.etception.domain.name

# XXX: assuming localhost defined in /etc/hosts
check grep localhost /etc/./hosts
check grep localhost /etc/../etc/hosts
