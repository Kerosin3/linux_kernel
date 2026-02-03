#!/bin/sh

cat > /mnt/testfilenames <<EOF
apple.txt
banana.log
cherry.py
date.json
elderberry.md
fig.sh
grape.csv
honeydew.ini
iceberg.bat
jackfruit.exe
kiwi.zip
lemon.dll
mango.pdf
nectarine.jpg
orange.mp3
pear.png
quince.txt
raspberry.bin
strawberry.ko
tomato.toml
EOF

mkdir -p /mnt/testdir

while read -r filename; do
    touch "/mnt/testdir/$filename"
done < /mnt/testfilenames

echo "Created 20 diverse test files in /mnt/testdir. Filenames listed in /mnt/testfilenames."
