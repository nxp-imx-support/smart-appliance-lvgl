Add all submodules before compilation
-------------------------------------
```
$ git submodule update --init
```

How to build lvgl_demo
-------------------------------------
```
$ source /opt/fsl-imx-wayland/6.6-scarthgap/environment-setup-armv8a-poky-linux
$ make

lvgl_demo will be generated under the same root directory
```

How to build test-vit
-------------------------------------
```
$ cp patch/0001-Add-VIT-to-control-light-and-stand-by-and-send-messa.patch imx-voiceui/
$ cd imx-voiceui/
$ git apply 0001-Add-VIT-to-control-light-and-stand-by-and-send-messa.patch
$ cd vit/platforms/iMX9_CortexA55/ex_app/
$ make VERSION=04_10_00

test_vit will be generated under imx-voiceui/vit/platforms/iMX9_CortexA55/ex_app/build/
```
