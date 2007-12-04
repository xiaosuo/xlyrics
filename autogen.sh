#!/bin/sh
set -x
autopoint -f
aclocal-1.9 -I m4
autoheader
automake-1.9 --add-missing --gnu
autoconf
libtoolize --copy --force
set +x
