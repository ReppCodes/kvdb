#!/usr/bin/env bash
make clean
make

kvdb set keyone valuevaluevalue
kvdb get keyone
kvdb ts keyone
kvdb del keyone
kvdb get keyone
kvdb ts keyone
kvdb set keyone somuchvalue
sleep 3
kvdb set keyone wherehashallmyvaluegone
kvdb get keyone
kvdb ts keyone
