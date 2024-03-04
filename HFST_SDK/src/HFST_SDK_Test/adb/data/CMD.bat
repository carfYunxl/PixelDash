adb wait-for-device
adb root
adb shell mkdir /data/httu/
adb shell rm /data/httu/sitronix_client
adb shell rm /data/httu/sitronix_server
adb shell rm /data/httu/HopingTool_CFG.ini

adb push sitronix_client /data/httu/
adb push sitronix_server /data/httu/
adb push HopingTool_CFG.ini /data/httu/

adb shell chmod 777 /data/httu/sitronix_client
adb shell chmod 777 /data/httu/sitronix_server
adb shell chmod 777 /data/httu/HopingTool_CFG.ini

::adb shell ./data/httu/sitronix_server&

adb shell setenforce 0
pause