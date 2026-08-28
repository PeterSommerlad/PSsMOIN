#!/bin/sh
rm include/*
cd include
ln ../cevelop-workspace/PSsMOIN/src/pssmoin.h .
ln ../cevelop-workspace/PSsMOINwithStruct/src/moins.h .
ls -l