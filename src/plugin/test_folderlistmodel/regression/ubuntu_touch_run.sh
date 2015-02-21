#!/bin/sh

export APP_ID=com.ubuntu.filemanager_filemanager_test

DESKTOP=`ls /usr/share/applications/*.desktop | tail -1`

## any application which uses QApplication crashes without --desktop_file_hint=valid_desktop_file

./regression_folderlilstmodel --desktop_file_hint=$DESKTOP $*


