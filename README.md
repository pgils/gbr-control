# gbr-control
This software is part of the project [ti6-domotica](https://github.com/pgils/ti6-domotica).

## Installation
### 1. Set up operating system
These guides (and software) are tested on these distributions:
- debian 9.6 (vm)
- raspbian (RPi3)

Install packages:
```console
# apt update
# apt install sudo vim git
```
Add user to `sudo` group.
```console
# usermod -a -G sudo <username>
```

### 2. [Install OpenThread Borderrouter](br-setup.md)
### 3. [Install gbr-control](gbr-setup.md)
