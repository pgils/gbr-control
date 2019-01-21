## Installing gbr-control
### Disable OpenThread webinterface to prevent conflict
```console
$ sudo systemctl disable otbr-web.service
$ sudo systemctl stop otbr-web.service
```
### Install dependencies
```console
$ sudo apt install sqlite3 libsqlite3-dev apache2
```
### Download packages for recent version of tinyxml2:  
- [libtinyxml2-dev](https://packages.debian.org/sid/libtinyxml2-dev)  
- [libtinyxml2](https://packages.debian.org/sid/libtinyxml2-6a)

and install them
```console
$ sudo dpkg -i libtinyxml2-6a_7.0.0+dfsg-1_amd64.deb
$ sudo dpkg -i libtinyxml2-dev_7.0.0+dfsg-1_amd64.deb
```
### Install gbr-control
```console
$ git clone https://github.com/pgils/gbr-control.git
$ cd gbr-control
$ sudo make install
$ sudo cp extra/post.php /var/www/html/
```
### Set up gbr-control
Create the database file and enable gbr-control service files
```console
$ sudo sqlite3 /var/local/gbr-control.db < extra/gbr-control_create.sql
$ sudo systemctl enable gbr-control.service
$ sudo systemctl enable gbr-control.timer
```

now reboot the borderrouter hardware and check gbr-control output:
```console
$ sudo journalctl -u gbr-control.service -f
```
