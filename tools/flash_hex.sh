#!/bin/sh
# Automate the steps of taking a .hex file, converting to a .dfu file and
# upgrading the USB device.

DFU_UTIL=dfu-util;

if [ $# -ne 1 ]; then
  echo "Usage: $0 <hex-file>"
  exit;
fi

hex_file=$1
if [ ! -r $hex_file ]; then
  echo "Can't read file hex $hex_file";
  exit;
fi

tool_dir=$(dirname $0)
hex2dfu="${tool_dir}/hex2dfu"

if [ ! -x $hex2dfu ]; then
  echo "Can't execute $hex2dfu, try running make first";
  exit;
fi

type $DFU_UTIL > /dev/null 2>&1 || { echo "Can't execute $DFU_UTIL"; exit 1; }

model_id=0

if [[ "$hex_file" == *"number1"* ]]; then
  model_id=1;
elif [[ "$hex_file" == *"number8"* ]]; then
  model_id=2;
elif [[ "$hex_file" == *"ethernet_sk2"* ]]; then
  model_id=3;
fi

$hex2dfu -m $model_id $hex_file
if [ $? -ne 0 ]; then
  echo "hex2dfu failed";
  exit;
fi

dfu_file=${hex_file%.hex}.dfu

if [ ! -r $dfu_file ]; then
  echo 'Missing .dfu file';
  exit;
fi

$DFU_UTIL -a 0 -R -D $dfu_file
