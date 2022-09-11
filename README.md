# mqtt2LoRaWAN Bridge

MQTT to LoRaWAN Bridge using a Hardware Abstraction Layer (HAL) for IBM's LMIC 1.6 communication
stack targeted to RPi and Dragino LoRA/GPS HAT.

The goal was to keep the LMIC 1.6 sourcecode untouched, and just provide a
Hardware Abstraction Layer (HAL) for Raspberry Pi and Dragino LoRa/GPS HAT.

Maintained: [Dominik Kuhn](mailto:dominik.kuhn90@googlemail.com)

## Project idea and description

To be as flexible as possible in the use of LoRa-Phy(MAC) we have developed a simple MQTT2LoRa bridge in modern C++. Messages sent on a specific MQTT topic are subscripted by the bridge and sent via LoRa payload. Conversely, frames received via LoRa are published on another MQTT topic. Thus we are able to connect more or less any application in high level language to a LoRa network without having to deal with hardware related SPI protocols. These hardware related things are done in this case by our bridge in C++.

The idea for the bridge came from the idea to couple a simple textmessager with the LoRa technology. For example, we wanted to be able to send and receive IRC chats, matrix chats, etc. (pure text messages) over long distances. However, the development of this bridge showed that the potential is much higher. Prototypes for various applications can be developed very quickly, e.g. GPS loggers written in Python via TTN or weather balloon trackers.

## GPS logger application

In the case of the GPS-Logger, an application written in Python reads the current position of the GPSd service in Linux every second. Every 60 seconds, the currently read position is then formater encoded via TTN packet and published via MQTT, so that this meassage can be sent through the bridge. 

# Installation of build requirements

Change to the directory */opt* and create a directory named *libraries* for the needed libraries. Change the owner of the directory to *pi* to get unrestricted access to the directory and the directroy under it.

```bash
cd /opt && sudo mkdir libaries && chown -R pi:pi ./libraries 
```

## MQTT Libraries

To build the main  MQTT2LoRa bridge and the required libraries paho.mqtt.c/cpp we have to install some packages on  RaspberryPi. For this we orientate primarily on the dependencies of the MQTT libraries. 

The build process requires the following tools:
  * CMake (http://cmake.org)
  * Ninja (https://martine.github.io/ninja/) or
    GNU Make (https://www.gnu.org/software/make/), and
  * gcc (https://gcc.gnu.org/).

On Debian based systems this would mean that the following packages have to be installed:

```bash
apt-get install build-essential gcc make cmake cmake-gui cmake-curses-gui
```

Also, in order to build a debian package from the source code, the following packages have to be installed

```bash
apt-get install fakeroot fakeroot devscripts dh-make lsb-release
```

Ninja can be downloaded from its github project page in the "releases" section. Optionally it is possible to build binaries with SSL support. This requires the OpenSSL libraries and includes to be available. E. g. on Debian:

```bash
apt-get install libssl-dev
```

The documentation requires doxygen and optionally graphviz:

```bash
apt-get install doxygen graphviz
```

### paho.mqtt.c

For the operation of our MQTT2LoRa bridge we need of course a MQTT library. In this case we use *paho.mqtt.cpp* for C++. For this we first need to build and install *paho.mqtt.c* for plain C as follows.

Change to the newly created directory /opt/libraires and clone the current paho-mqtt-c repo from GitHub:

```bash 
cd /opt/libaries 
```  

<br>

```bash 
git clone https://github.com/eclipse/paho.mqtt.c.git 
``` 


Build the mqtt library without tests but with SSL encryption for a secure connection and install it:

```bash
cd paho.mqtt.c
```  

<br>

```bash
cmake -Bbuild -H. -DPAHO_ENABLE_TESTING=OFF \
      -DPAHO_BUILD_STATIC=ON -DPAHO_WITH_SSL=ON -DPAHO_HIGH_PERFORMANCE=ON 
```

<br>

```bash 
cmake --build build/ --target install
```

### paho.mqtt.cpp

Once we have successfully built the *paho.mqtt.c* library, we can build and install the main C++ library. To do this, we change to the */opt/libraires* directory and clone the corresponding repo from GitHub.

``` 
cd /opt/libraires  
```

<br>

``` 
git clone https://github.com/eclipse/paho.mqtt.cpp 
```

Build the mqtt C++ library statical with tests and documentation:

```bash 
cmake -Bbuild -H. -DPAHO_BUILD_STATIC=ON \
      -DPAHO_BUILD_DOCUMENTATION=TRUE -DPAHO_BUILD_SAMPLES=TRUE
```

<br>

```bash 
cmake --build build/ --target install 
```
 
 
### Boost C++ Libraries

This will download the Boost "superproject" (the master project, without any libraries) and place it into the subdirectory boost of the current directory. To override the directory name, pass it as a second argument instead of boost:

```bash 
cd /opt/libraries 
```

<br>

```bash 
git clone https://github.com/boostorg/boost.git boost 
```

You can now cd into the newly created directory with

```bash 
cd boost
```

Then, download all the libraries:

```bash 
git submodule update  --init
```

This will build Boostdep from source using the default "toolset" (a Boost.Build term meaning "compiler") and if successful, place it into the dist/bin subdirectory. The command assumes that b2 (the Boost.Build executable) is somewhere in your path. If you don't have b2, execute:

```bash
.\bootstrap
```

Change your current directory to the Boost root directory and invoke b2 as follows:

```bash 
.\b2 -j2 --with-timer --with-program_options 
``` 

In particular, to limit the amount of time spent building, you may be interested in:

- List itemreviewing the list of library names with --show-libraries
- limiting which libraries get built with the --with-library-name or --without-library-name options
- choosing a specific build variant by adding release or debug to the command line.

### WiringPi

When we reach this point, we have installed almost all the libraries we need. But the most important library to communicate with the SPI interface of the RaspberryPi is still missing. In this case the bridge is based on the popular *WiringPi* library. If we run the MQTT2LoRa bridge on a RaspberryPi with Debian10 we can install WiringPi directly from the package sources with the following command:

```bash
apt-get install wiringpi
``` 

In Debian 11, the wiringpi package has been removed from the official repository. So we have to build the corresponding library ourselves from the sources. For this we clone the following GitHub repo: 

```bash 
git clone https://github.com/WiringPi/WiringPi.git
``` 

To install wiringPi, the easiest way is to use the supplied *build* script:

```bash 
  ./build
``` 

that should do a complete install or upgrade of wiringPi for you. That will install a dynamic library. Some distributions do not have /usr/local/lib in the default LD_LIBRARY_PATH. To fix this, you need to edit /etc/ld.so.conf and add in a single line:

```bash
  /usr/local/lib
``` 

then run the ldconfig command.

```bash
  sudo ldconfig
```

To un-install wiringPi:

```bash
  ./build uninstall
```

# Installation

After all external dependencies are installed we can build the main bridge. For this we clone this repo to an arbitrary location, e.g. to */home/pi/projects* as in our case.

Execute the following command if no *projects* directory exists in the home. 

```bash
cd /home/pi && mkdir projects 
```


```bash
git clone https://github.com/dokuhn/mqtt2LoRaWAN.git
```

When the current repo is cloned, we change to this directory and build the bridge with the following three commands:

```bash
cd rpi_lora && cmake . && make 
```

After that we have built everything. Currently there is no possibility of specific builds. Everything is always built, the main bridge but also the tests needed by the development. 

# ToDo's

- Documentation in GitHub Repo
- be able to read config file and console parameters
- Topics for configuring the LoRa connection (frequency, bandwidth, spreading factor, etc.)



