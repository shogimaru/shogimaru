#!/bin/bash -x

for lang in "ja"; do
  lrelease src/message_${lang}.ts && mv src/message_${lang}.qm assets/translations/
done
