# Fedora Setup


## Notes

- Tested on Legion Pro 7i 16IRX8H with Fedora Workstation 40 KDE.
- Battery Charging Limiter on Legion Pro 7i 16IRX8H: `/sys/bus/platform/drivers/ideapad_acpi/VPC2004:00/conservation_mode`
- See [Fedora Quick Docs](https://docs.fedoraproject.org/en-US/quick-docs).


## Prerequisites

1. Enable Nvidia Non-Free Repos in SW Center, if needed.
2. Enable & install Google Chrome Repos in SW Center.
3. Enable & install Steam Repos in SW Center.
4. Enable FlatHub Repos in SW Center.
5. **Optional:** Install snapd from SW Center.
6. Add `max_parallel_downloads=10` to `/etc/dnf/dnf.conf`
7. Make sure that Bluetooth is always enabled at start-up. Add `AutoEnable=true`
to `/etc/bluetooth/main.conf`, if needed.


## System Update

```console
sudo dnf check-update
sudo dnf upgrade
```


## Install General-Use Packages

```console
sudo dnf groupinstall "Development Tools"
sudo dnf groupinstall "Development Libraries"
sudo dnf groupinstall "C Development Tools and Libraries"
sudo dnf install htop
sudo dnf install dracut-config-rescue
sudo dnf install efivar
sudo dnf install acpitool
sudo dnf install vlc
sudo dnf install java
sudo dnf install google-chrome-stable
sudo dnf groupinstall Multimedia
sudo dnf install blender
sudo dnf install gimp
sudo dnf install yacreader
sudo dnf install transmission
```

Also install/configure the following applications:
1. Install 1Password from official website (also available as a snap).
3. Install Visual Studio Code from official website (also available as a snap).
4. Setup WhatsApp Web.
5. Setup Libreoffice to be compatible with Microsoft Office to the extent possible.
See [Make LIBREOFFICE more compatible with MICROSOFT OFFICE & 365 - YouTube](https://youtu.be/G0che2Az9hw?si=srUlNr3YHD33ByAd).
To install MS fonts:

    ```console
    sudo dnf install curl cabextract xorg-x11-font-utils fontconfig
    wget https://downloads.sourceforge.net/project/mscorefonts2/rpms/msttcore-fonts-installer-2.6-1.noarch.rpm
    sudo dnf install /path/to/msttcore-fonts-installer-2.6-1.noarch.rpm
    ```

Remove unnecessary apps that came installed by default. Also perform a sanity test
of the HW platform if needed, and configure system settings.


## Setup SSH Server

1. Create an SSH key-pair, and enable SSH server.

    ```console
    ssh-keygen -t rsa -b 4096
    eval "$(ssh-agent -s)"
    sudo systemctl enable ssh
    touch ~/.ssh/authorized_keys    # add client public keys to this file
    sudo touch /etc/ssh/sshd_config.d/${USER}.conf
    ```

2. Add the following into the .conf file to secure SSH access.

    ```console
    PasswordAuthentication no
    PermitRootLogin no
    PubkeyAuthentication yes
    ```

3. Start SSH server.

    ```console
    sudo systemctl start ssh
    ```


## Share WiFi Password For All Users

```console
nmcli con show                                             # GUIDs for the networks
nmcli connection show "WiFi-SSID"                          # connection.permissions must be set to "--"
nmcli con modify Network-GUID connection.permissions ''    # set connection.permissions to "--" if not already so
```


## [Optional] Prepare Signing Keys For SecureBoot

```console
sudo dnf install kmodtool akmods mokutil openssl
sudo kmodgenca -a
sudo mokutil --import /etc/pki/akmods/certs/public_key.der
sudo systemctl reboot    # enroll keys with UEFI upon reboot
```

For details, see [HowTo/SecureBoot - RPM Fusion](https://rpmfusion.org/Howto/Secure%20Boot).


## [Optional] Install Nvidia Drivers

```console
sudo dnf install gcc kernel-headers kernel-devel
sudo dnf install akmod-nvidia    # wait atleast 5-10 minutes before proceeding further
modinfo -F version nvidia        # do not proceed until this prints out the driver version
sudo systemctl reboot

# optional: CUDA support
sudo dnf install xorg-x11-drv-nvidia-cuda
nvidia-smi

sudo grubby --update-kernel=ALL --args="nvidia-drm.modeset=1"
sudo systemctl reboot
```

For details, see [HowTo/Nvidia - RPM Fusion](https://rpmfusion.org/Howto/NVIDIA).


## Set Up The Work Environment

1. **Git:** oracle/misc.
2. **Linux kernel:** oracle/linux-kernel/docs.
3. **Containers:** oracle/containers.
4. **Arduino CLI:** See [Arduino CLI](https://arduino.github.io/arduino-cli/latest).
5. **Zephyr:** See [Getting Started Guide - Zephyr](https://docs.zephyrproject.org/latest/develop/getting_started/index.html).
6. **Optional:** Install LLM chatbot(s). See [ollama/ollama - GitHub](https://github.com/ollama/ollama).
7. Any remaining professional applications.


## Set Up The Gaming Environment

```console
sudo dnf install steam
sudo dnf install lutris    # setup any remaining app stores
```
