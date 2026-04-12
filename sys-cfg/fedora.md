# Fedora Setup


## Notes

- Tested on an `x86_64` PC with Fedora Workstation 43 KDE.
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
sudo dnf group install development-tools
sudo dnf group install c-development
sudo dnf install htop
sudo dnf install dracut-config-rescue
sudo dnf install efivar
sudo dnf install acpitool
sudo dnf install vlc
sudo dnf install java
sudo dnf install google-chrome-stable
sudo dnf install gparted
sudo dnf install blender
sudo dnf install gimp
sudo dnf install yacreader
sudo dnf install transmission
sudo dnf install curl
sudo dnf install cabextract
sudo dnf install xorg-x11-font-utils
sudo dnf install fontconfig
sudo dnf install neovim
sudo dnf install ripgrep
sudo dnf install fzf
sudo dnf install bat
sudo dnf install tmux
```

Also install/configure the following applications:
1. Install 1Password from official website (also available as a snap).
2. Setup neovim (set `EDITOR=nvim` in environment).
3. Setup tmux.
4. Setup fzf.

    ```console
    # fzf setup
    eval "$(fzf --bash)"    # see https://github.com/junegunn/fzf

    fzf-ff() {
      fzf \
        --preview="bat --color=always {}"
    }

    fzf-fg() {
      fzf \
        --ansi --phony --query "${1:-.}" \
        --bind "change:reload:rg --line-number --no-heading --color=always {q} || true" \
        --delimiter : \
        --preview "bat --style=numbers --color=always {1} --highlight-line {2}"
    }
    ```
5. Setup WhatsApp Web.
6. Setup Libreoffice to be compatible with Microsoft Office to the extent possible.
See [Make LIBREOFFICE more compatible with MICROSOFT OFFICE & 365 - YouTube](https://youtu.be/G0che2Az9hw?si=srUlNr3YHD33ByAd).
7. Copy Microsoft Office fonts (`.ttf` files) to `~/.local/share/fonts/msfonts` (create directory if needed).

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
6. **Optional:** Install LLM chatbot(s).
7. Any remaining professional applications.


## Set Up The Gaming Environment

```console
sudo dnf install mangohud
sudo dnf install goverlay
sudo dnf install steam
sudo dnf install lutris    # setup any remaining app stores
```
