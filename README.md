Texttool
==================

[![License](https://img.shields.io/badge/license-BSD%203--Clause-blue.svg?style=flat-square)](https://github.com/mikaelsundell/texttool/blob/master/README.md)

Introduction
------------

texttool is a utility for creating title images.

![Sample image or figure.](images/image.png 'it8tool')

Building
--------

The texttool app can be built both from commandline or using optional Xcode `-GXcode`.

```shell
mkdir build
cd build
cmake .. -DCMAKE_PREFIX_PATH=<path> -GXcode
cmake --build . --config Release -j 8
```

**Example using 3rdparty on arm64 with Xcode**

```shell
mkdir build
cd build
cmake ..
cmake .. -DCMAKE_PREFIX_PATH=<path>/3rdparty/build/macosx/arm64.debug -GXcode
```

Usage
-----

Print texttool help message with flag ```--help```.

```shell
texttool -- a utility for creating text in images

Usage: overlaytool [options] ...

General flags:
    --help                     Print help message
    -v                         Verbose status messages
    -d                         Debug status messages
Input flags:
    --title                    Set title
    --subtitle                 Set subtitle
    --size SIZE                Set size (default: 1024, 1024)
Output flags:
    --outputfile OUTPUTFILE    Set output file
```

Example title image
--------

```shell
./texttool
--title "Hello, world!"
--subtitle "An image about Hello, world!"
--outputfile title.png 
--size "2350,1000" 
```

Download
---------

Texttool is included as part of pipeline tools. You can download it from the releases page:

* https://github.com/mikaelsundell/pipeline/releases

Dependencies
-------------

| Project     | Description |
| ----------- | ----------- |
| Imath       | [Imath project @ Github](https://github.com/AcademySoftwareFoundation/Imath)
| OpenImageIO | [OpenImageIO project @ Github](https://github.com/OpenImageIO/oiio)
| 3rdparty    | [3rdparty project containing all dependencies @ Github](https://github.com/mikaelsundell/3rdparty)

Project
-------------

* GitHub page   
https://github.com/mikaelsundell/overlaytool
* Issues   
https://github.com/mikaelsundell/overlaytool/issues

Copyright
---------

* Roboto font   
https://fonts.google.com/specimen/Roboto   
Designed by Christian Robertson
