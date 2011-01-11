#!/bin/sh


VERSION=0.4.1
RELEASE=kunquat-$VERSION


mkdir -p release
bzr export release/$RELEASE
cd release/$RELEASE
rm release.sh
cd ..
tar cpzf $RELEASE.tar.gz $RELEASE

exit 0


