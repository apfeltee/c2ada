#!/bin/bash

sourcedir="../"
c2ada="$sourcedir/c2ada"

if [[ ! -x "$c2ada" ]]; then
  echo "c2ada hasn't been built yet! can't continue."
  exit 1
fi

if $c2ada hello.c; then
  pushd bindings
  gnat make hello -aI"$sourcedir"
  popd
else
  echo "c2ada failed, can't continue."
  exit 1
fi

