#!/bin/zsh

inc="/usr/include/Python2.7"
inc="c:/cygwin/usr/include/Python2.7"

for file in *.c; do
  ofile="${file%.*}.o"
  if [[ ! -e "$ofile" ]]; then
    echo ">>>> $file"
    if ! g++ -x c++ -w -I"$inc" -w -c "$file" -o "$ofile"; then
      exit
      true
    fi
  fi
done

