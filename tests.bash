#!/usr/bin/env bash
make clean
make

# serial tests
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

# parallel tests
kvdb set keytwo valuevaluevalue&
kvdb get keytwo&
kvdb ts keytwo&
kvdb del keytwo&
kvdb get keytwo&
kvdb ts keytwo&
kvdb set keytwo somuchvalue&
sleep 3
kvdb set keytwo wherehashallmyvaluegone&
kvdb get keytwo&
kvdb ts keytwo
