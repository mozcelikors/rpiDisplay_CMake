#!/bin/bash

# To run at start create this script at /etc/init.d/rpiDisplay
#   Add to startup: update-rc.d rpiDisplay defaults
#   Remove from startup: update-rc.d -f rpiDisplay remove
#
# The script is present in initscriptrpidisplay recipe in meta-embeddev to automatize the process in Linux.
# But first, deploy the application to /home/root/projects

## Get which directory we're at
SOURCE="${BASH_SOURCE[0]}"
while [ -h "$SOURCE" ]; do # resolve $SOURCE until the file is no longer a symlink
  DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"
  SOURCE="$(readlink "$SOURCE")"
  [[ $SOURCE != /* ]] && SOURCE="$DIR/$SOURCE" # if $SOURCE was a relative symlink, we need to resolve it relative to the path where the symlink file was located
done
DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"

modprobe fbtft_device custom name=fb_ili9341 gpios=reset:25,dc:24,led:18 speed=16000000 rotate=270 bgr=1

export QT_QPA_PLATFORM=linuxfb:fb=/dev/fb1

# If app is not already running, start it..
if  pidof -x "backend_app" > /dev/null ; then
    export running=1
fi
if [ -z ${running+x} ]; then
    LD_LIBRARY_PATH=/home/root/projects /home/root/projects/backend_app -tr &

    LD_LIBRARY_PATH=/home/root/projects /home/root/projects/frontend_app &
fi
