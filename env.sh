#/bin/sh
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
export TREMO_SDK_PATH="$SCRIPT_DIR"

if ! command -v arm-none-eabi-gcc 2>&1 >/dev/null
then
    echo "ARM GCC was not found, adding to path manually, change path to suit your configuration"

    # adjust this to suit your setup
    export PATH="$PATH:/c/tools/gcc-arm-none-eabi-10.3-2021.10/bin"

    if ! command -v arm-none-eabi-gcc 2>&1 >/dev/null
    then
        echo "ARM GCC still not found, check env.sh and try again"
        exit 1
    else
        echo "ARM GCC now available"
    fi
fi
