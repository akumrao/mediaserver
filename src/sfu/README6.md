*Usefull Linux command*

To disable screen rotation
xrandr -o normal

root:~# systemctl stop iio-sensor-proxy.service
root:~# systemctl disable  iio-sensor-proxy.service

Disabling the service with systemctl, you can try to mask it, to prevent other services to accidentally enabling screen rotation:

root:~# systemctl mask  iio-sensor-proxy.service

root:~# settings set org.gnome.settings-daemon.peripherals.touchscreen orientation-lock true

root:~# gsettings set org.gnome.settings-daemon.peripherals.touchscreen orientation-lock true
root:~# gsettings set org.gnome.settings-daemon.plugins.orientation active false


https://github.com/FreedomBen/rtl8188ce-linux-driver


 1 down vote

Run this command in your terminal (Applications > Accessories > Terminal):

# show files on desktop
gsettings set org.gnome.desktop.background show-desktop-icons true

# show shares on desktop
gsettings set org.gnome.nautilus.desktop volumes-visible true

or 

better way

mv root root1
when  desktop icons are not vissible,

addusr -d /root ( recreate root directorhy which will recreate the gnome desktop)

then open tweak 
tweak->destop> enable trash icon on desktop
scale to 1.5 for better dispaly of icons 


# restart nautilus
nautilus -q
nautilus

dconf update
usermod -U root
https://superuser.com/questions/481454/how-can-i-login-as-root-so-i-can-use-gdm


https://fedoraproject.org/wiki/Enabling_Root_User_For_GNOME_Display_Manager

$ sudo apt-get update && sudo apt-get upgrade
$ sudo apt-get install gnome-session-flashback
apt-get install gnome-session-flashback
https://www.technhit.in/enable-root-user-ubuntu-16-04-1-lts/
lshw -C network

adduser herman
usermod -aG sudo herman



sudo lshw -C network

*-network UNCLAIMED
       description: Network controller
       product: RTL8192EE PCIe Wireless Network Adapter
       vendor: Realtek Semiconductor Co., Ltd.
       physical id: 0
       bus info: pci@0000:02:00.0
       version: 00
       width: 64 bits
       clock: 33MHz
       capabilities: pm msi pciexpress bus_master cap_list
       configuration: latency=0
       resources: ioport:5000(size=256) memory:f2400000-f2403fff


/lib/modules/3.13.0-153-generic/kernel/drivers/net/wireless

cp gdm-*  192.168.0.102:/etc/pam.d/
root@192.168.0.102's password: 
/root/.bashrc: line 76: DISPLAY: unbound variable
gdm-autologin                                                                                                       100% 1076     1.1KB/s   00:00    
gdm-launch-environment                                                                                              100%  383     0.4KB/s   00:00    
gdm-password  


This is very important
http://www.kurento.org/blog/interoperating-webrtc-and-ip-cameras

addgroup --system pulse
adduser --system --ingroup pulse --home /var/run/pulse pulse
addgroup --system pulse-access

# Some distributions restrict access to the sound devices to a group audio
adduser pulse audio

# Add a user to the pulse-access group
adduser root pulse-access

apt-get install  gdm3
dpkg-reconfigure gdm3
apt-get install ubuntu-gnome-desktop

create new user arvind, delete ther previous one


In this approach the cloud hosted STUN servers are only used to create a network path between peers but real media goes P2P. and in case of no relay, we work via TURN servers.

https://webrtc.org/start/
https://www.html5rocks.com/en/tutorials/webrtc/infrastructure/

snap list
 snap remove core

apt remove snapd

root@arvindumrao:~# umount /snap/core/4917 
umount: /snap/core/4917: not mounted.
root@arvindumrao:~# rmdir /snap/core/4917/  

apt purge snapd ubuntu-core-launcher squashfs-tools



Disable IPv6 in APT
Sometimes, you need to disable IPv6 in the APT package manage only and other program can continue use IPv6 if needed. To disable IPv6 in APT, run the following command to create a configuration file for APT.

sudo nano /etc/apt/apt.conf.d/99force-ipv4
Copy and paste the following line into the file.

 Acquire::ForceIPv4 "true";
Save and close the file. From now on, APT will use IPv4 only.

Disable IPv6 on Ubuntu Altogether
If you want to completely disable IPv6 on your Ubuntu Linux system, then you need to make some changes to Linux kernel parameter.

Edit the 99-sysctl.conf file.

sudo nano /etc/sysctl.d/99-sysctl.conf
Copy and paste the following 3 lines at the bottom of the file.

net.ipv6.conf.all.disable_ipv6 = 1
net.ipv6.conf.default.disable_ipv6 = 1
net.ipv6.conf.lo.disable_ipv6 = 1
Save and close the file. Then execute the following command to load the above changes.

sudo sysctl -p
Now run the following command. You should see 1, which means IPv6 has been successfully disabled.

cat /proc/sys/net/ipv6/conf/all/disable_ipv6



Type sudo apt-get install openssh-server
Enable the ssh service by typing sudo systemctl enable ssh
Start the ssh service by typing sudo systemctl start ssh
PAM no
permitRootLogin=yes
publicAutentication=yes









apt-get install default-jdk

udo apt-key adv --keyserver hkp://keyserver.ubuntu.com:80 --recv EA312927
Add the MongoDB repository to your sources.list.d directory:

echo "deb http://repo.mongodb.org/apt/ubuntu xenial/mongodb-org/3.2 multiverse" | sudo tee /etc/apt/sources.list.d/mongodb-org-3.2.list


sudo systemctl start mongod
sudo systemctl restart mongod
sudo systemctl stop mongod
You can also enable MongoDB to start on boot:

sudo systemctl enable mongod



 sudo apt-get purge linux-image-3.19.0-25-generic
$ sudo apt-get purge linux-image-3.19.0-56-generic
$ sudo apt-get purge linux-image-3.19.0-58-generic
$ sudo apt-get purge linux-image-3.19.0-59-generic
$ sudo apt-get purge linux-image-3.19.0-61-generic
$ sudo apt-get purge linux-image-3.19.0-65-generic
When you're done removing the older kernels, you can run this to remove ever packages you won't need anymore:

$ sudo apt-get autoremove
And finally you can run this to update grub kernel list:

$ sudo update-grub



https://certification.ubuntu.com/hardware/201803-26136/
lspci -vvv | grep -A 40 -i audio

Intel Sunrise Point-LP HD Audio

Intel Unknown

aplay -lL output:

driver used:snd_hda_intel

5 mins

vi /etc/systemd/system/network-online.target.wants/networking.service
TimeoutStartSec=5min
root@arvindubuntu:~# vi /etc/network/interfaces
allow-hotplug eth0





sudo passwd root

sudo passwd -u root

sudo nano /etc/gdm3/custom.conf

=====
[security]
AllowRoot=true
=====

sudo nano /etc/pam.d/gdm-password

=====
# auth	required	pam_succeed_if.so user != root quiet_success
=====

sudo nano /root/.profile









udo vim /etc/systemd/system/network-online.targets.wants/networking.service
And changing the following line at the end of the file:

TimeoutStartSec=5min
to:

TimeoutStartSec=30sec
I have then rebooted the system and it works fine.



**********************************************
disable touchpad 

 did it this way:

sudo apt remove xserver-xorg-input-synaptics
sudo apt install xserver-xorg-input-libinput
sudo reboot
On my machine I had both of them, so synaptics was default, deleting it helped me. Do not forget to rebo














sudo apt install g++-5
sudo apt install gcc-5
Change the symlink to point to gcc 5 and g++ 5

sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-5 10
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-5 20
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-5 10
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-5 20
sudo update-alternatives --install /usr/bin/cc cc /usr/bin/gcc 30
sudo update-alternatives --set cc /usr/bin/gcc


sudo update-alternatives --install /usr/bin/c++ c++ /usr/bin/g++ 30
sudo update-alternatives --set c++ /usr/bin/g++



service --status-all
apt purge  cups-browsed cups  bluetooth  unattended-upgrades whoopsie


systemctl disable mongodb 


windows  issues

chage boot order from ubunut

efibootmgr
efibootmgr -o 1,2,0

delete boot

efibootmgr -b 0001 -B

++++++++++++++++++++++++++++++++

from windows do 

check in net how to mount efi partition disk with command DISKPART.

then assign letter to drive 

cd S: C: and D:


bcdedit /enum firmware
Copy UEFI entry of "Windows Boot Manager" to create a new entry for Ubuntu:

bcdedit /copy {bootmgr} /d "Ubuntu Secure Boot"
Set file path for the new Ubuntu entry. Replace {guid} with the returned GUID of the previous command.

bcdedit /set {guid} path \EFI\ubuntu\shimx64.efi
Set optionally Ubuntu as first entry in the boot sequence. Replace {guid} with the returned GUID of the copy command.

bcdedit /set {fwbootmgr} displayorder {guid} /addfirst

fdisk
gparted
parted -l

ls -l $(find /boot/efi -iname "*.efi")
os-prober
grub-probe --target=hints_string /boot/efi/EFI/Microsoft/Boot/bootmgfw.efi

grub-install

update-grub

blkid


dell inspiron 1525

ligth weight desktop
 apt-get install lxde

wireless configuration
lspci -vvnn | grep 14e4
0b:00.0 Network controller [0280]: Broadcom Inc. and subsidiaries BCM4311 802.11a/b/g [14e4:4312] (rev 01)

 iwconfig
lsmod | grep b43
rfkill list all
dmesg | grep b43

pt-get install bcmwl-kernel-source

 wget -N -t 5 -T 10 https://github.com/UbuntuForums/wireless-info/raw/master/wireless-info && chmod +x wireless-info && ./wireless-info
 

https://stackoverflow.com/questions/24975377/kvm-module-verification-failed-signature-and-or-required-key-missing-taintin

http://www.iitk.ac.in/LDP/HOWTO/SCSI-2.4-HOWTO/kconfig.html
https://stackoverflow.com/questions/24975377/kvm-module-verification-failed-signature-and-or-required-key-missing-taintin/32194268

https://unix.stackexchange.com/questions/74022/sign-a-module-after-kernel-compilation


https://wiki.debian.org/bcm43xx#Identification

https://askubuntu.com/questions/55868/installing-broadcom-wireless-drivers


finally I got success by
apt-get purge bcmwl-kernel-source
apt-get install firmware-b43-installer
rfkill unblock all  
dpkg-reconfigure firmware-b43-installer

