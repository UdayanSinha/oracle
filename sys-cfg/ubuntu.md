# Ubuntu Setup


## Notes

- Tested on Legion Pro 7i 16IRX8H with Ubuntu 24.04 LTS.
- Battery Charging Limiter on Legion Pro 7i 16IRX8H: `/sys/bus/platform/drivers/ideapad_acpi/VPC2004:00/conservation_mode`


## System Update

```console
sudo apt update
sudo apt upgrade
```


## Install General-Use Packages

```console
sudo apt install htop
sudo apt install curl
sudo apt install git
sudo apt install gnupg2
sudo apt install efivar
sudo apt install acpitool
sudo apt install pciutils
sudo apt install i2c-tools
sudo apt install openssh-server
sudo apt install gparted
sudo apt install vlc
sudo apt install blender
sudo apt install gimp
```

Also install/configure the following applications:
1. Install Google Chrome from official website.
2. Install 1Password from official website (also available as a snap).
3. Install Visual Studio Code from official website (also available as a snap).
4. Setup WhatsApp Web.
5. Setup Libreoffice to be compatible with Microsoft Office to the extent possible.
See [Make LIBREOFFICE more compatible with MICROSOFT OFFICE & 365 - YouTube](https://youtu.be/G0che2Az9hw?si=srUlNr3YHD33ByAd).

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
sudo apt install steam
sudo apt install lutris    # setup any remaining app stores
```
