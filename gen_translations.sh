#!/bin/bash -x

export PATH=$HOME/Qt/6.4.2/gcc_64/bin:/usr/bin

for lang in "ja"; do
  lrelease src/message_${lang}.ts && mv src/message_${lang}.qm assets/translations/
done
