# Unmaintained

## Due to the current quick way to change profile which works with dbus, this script is discontinued. I'm leaving this repo open in case someone wants to ask questions about this in case one wants to try using this code for other purposes

----------------

# Tuxedo Control Center (TCC) Profile Changer
An unaffiliated external profile changer for tuxedo-control-center

# What is

This program runs by normal user to read a specified file and runs Tuxedo Control Center's tccd to change to the profile as described in the file read. 

# What is for

1. Safely give direct orders to the backend of TCC to change the profile in use without having to use sudo or be root.
2. Safely change TCC profile based on events (E.g. a keyboard shortcut) without the need for such program to be running as root.

# Security level

1. You can only select a profile to activate.
2. The profile to activate must be defined in a json file in the format TCC uses.
3. The profile selection files must be owned by root and may only be editable by root.

# How to install

1. `git clone https://github.com/brunoais/tuxedo-control-center-profile-changer.git`
2. `sudo make install`

# Usage example

1. Create a profile file
```
sudo cat << EOF > new_profile
{"stateMap":{"power_ac":"Default","power_bat":"Default"}}
EOF
sudo chown root:root new_profile
sudo chmod 755 new_profile
```
2. Then activate it whenever you want
```
power-profile-change new_profile
```

## Note
No effort is made to make sure TCC is installed

## Help needed:
I'm trying to make a .deb file using checkinstall but those efforts are not giving results. If you can find the problems please submit a PR.

# Caveats / known bugs

1. TCC's UI appears to be unaware of changes made by this program and it will show the profile that was previously set.

