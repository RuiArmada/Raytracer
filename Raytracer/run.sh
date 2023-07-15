#!/usr/bin/env bash

(cd ./cmake-build-release && ninja)
./cmake-build-release/vi-rt "$@"
