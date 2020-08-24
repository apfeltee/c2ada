#!/bin/zsh

if [[ $1 ]]; then
  for file in "$@"; do
    ofile="${file%.*}.o"
    echo ">> $file ..."
    if gcc -w -c "$file" -o "$ofile"; then
      rm -f "$ofile"
    else
      break
    fi
  done
fi

