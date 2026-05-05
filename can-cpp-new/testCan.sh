#!/bin/bash


for v in 00 10 20 30 40 50 60 70 80 90 A0 B0 C0 D0 E0 FF; do
  echo "Byte0 = $v"
  cansend can0 000#0000${v}0000000000
  sleep 1
done
