#!/usr/bin/env bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

file_out="${SCRIPT_DIR}/WiFiConsts.h"
if [ ! -f "${file_out}" ] || [ "$1" = "forceWifi" ] || [ "$1" = "forceAll" ]; then
    echo "Creating file '${file_out}', please update with your wifi credentials"
    echo "#define WIFI_SSID \"<your ssid>\"" > "${file_out}"
    echo "#define WIFI_PASSWORD \"<your wifi password>\"" >> "${file_out}"
else
    echo "File '${file_out}' exists, skipping"
fi

file_out="${SCRIPT_DIR}/data/channel_names.js"
if [ ! -f "${file_out}" ] || [ "$1" = "forceJs" ] || [ "$1" = "forceAll" ]; then
    echo "Creating file '${file_out}', please update with your channel names"
    echo "var channelNames = {" > "${file_out}"
    {
        echo "    1: \"ROOM #1\","
        echo "    2: \"ROOM #2\""
        echo "};"
    } >> "${file_out}"
else
    echo "File '${file_out}' exists, skipping"
fi
