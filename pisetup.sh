#!/bin/bash
# Super Mario 64 PC on Raspberry Pi
# Find latest updates and code on https://www.github.com/sm64pc/sm64ex
# NOTE: If you clone the repo, source must be copied in ~/src/sm64pc/sm64ex/
# ToDo: Test on more Pi models with fresh Raspbian and allow existing src folders to be updated.
#
clear
echo "This script will assist with compiling Super Mario 64 on Raspbian 10"
echo "Note that accelerated OpenGL (vc4_drm) is required for maximum performance"
echo "Checking Raspberry Pi model..."
lowmem=0
pi=0
pimodel=$(uname -m 2>/dev/null || echo unknown)
pitype=$(tr -d '\0' < /sys/firmware/devicetree/base/model)
repo_url="https://github.com/sm64pc/sm64ex"

if [[ $pimodel =~ "armv6" ]]
then
   echo -e "\nRaspberry Pi Model 1/0(W) detected (LOWMEM)"
   echo "Warning: Additional steps may be required to safely compile and maximize performance"
   pi=1;
   lowmem=1;
   exp=1;
fi

if [[ $pimodel =~ "armv7" ]]
then
   echo -e "\nRaspberry Pi Model 2/3/4 detected (32bit)"
   pi=2;
   lowmem=0;
fi

if [[ $pimodel =~ "aarch64" && $pitype =~ "4" ]]
then
   echo -e "\nRaspberry Pi Model 4 (64 bits) detected"
   echo "Audio errors reported"
   echo "Fixing audio config, reboot after compilation completes to activate"
   sudo sed -i 's/load-module module-udev-detect/load-module module-udev-detect tsched=0/' /etc/pulse/default.pa
	#load-module module-udev-detect tsched=0
   pi=4;
   lowmem=0;
   exp=1;
fi


if [[ $exp == 1 ]]
then
   echo ""
   echo "Notice: Due to detected Pi version, compilation and execution of Super Mario 64 (RPi) is experimental."
   echo "Further steps may be required and software / driver compatibility is not guaranteed."
   read -p "Continue setup & compilation (Y/N): " exp

	if [[ $exp =~ [Yy] ]]
	then
        echo ""
        else
 	exit
	fi

   echo "Please report any problems encountered to ${repo_url} issue tracker."
   echo ""
   sleep 7
fi

#//////////////////////////////////////////////////////////////////////////////////////////////////////////
#//////////////////////////////////////////////////////////////////////////////////////////////////////////
clear
echo "Super Mario 64 RPi Initial Setup"

if [[ $pi != 4 ]]
then #Dumb idea, but quick hack. 
     #We CANNOT enable VC4 for Pi4 as it uses VC6

inxinf=$(inxi -Gx)
echo "Checking for pre-enabled VC4 acceleration (inxi -Gx)"

if [[ $inxinf =~ "not found" ]]
then
echo "Error: inxi not installed. Installing..."
sudo apt-get update
sudo apt-get install -y inxi
inxi=$(inxi -Gx)

	if [[ $inxinf =~ "not found" ]]
	then
	echo "Warning: Setup will not continue unless inxi is installed"
	echo "Please ensure your Pi is in a state to download and install packages"
	sleep 3
	exit
	fi
fi

if [[ $inxinf =~ "vc4_drm" ]]
then
echo "Success: VC4 OpenGL acceleration found!"
echo ""
sleep 4

	else
	echo ""
	echo "OpenGL driver not found. opening Raspi-Config..."
	echo "Please enable raspi-config -> ADV Opt -> OpenGL -> Enable FullKMS Renderer"
	echo ""
	sleep 5
	sudo raspi-config
	vc4add=$(cat /boot/config.txt | grep -e "dtoverlay=vc4-kms-v3d")

		if [[ $vc4add =~ "vc4" ]]
		then
		echo "OGL driver now enabled on reboot"
		fi
fi

if [[ $lowmem == 1 ]]
then
fixmem=$(cat /boot/cmdline.txt | grep cma=128M)

	if [[ $fixmem =~ "cma=128M" ]]
		then
		echo ""
		echo "Notice: Low-RAM RasPi model detected, BUT fixes already applied."
		echo "Continuing setup."

		else
		echo ""
		echo "Warning: VC4 enabled, but your RasPi has 512MB or less RAM"
		echo "To ensure VC4_DRM and game compilation is succesful, video memory will be reduced"
		echo "gpu_mem=48M (config.txt) | cma=128M (cmdline.txt) will be written to /boot "
		echo ""
		read -p "Fix mem? (Y/N): " fixmem

			if [[ $fixmem =~ [Yy] ]]
			then
			sudo sh -c "echo 'gpu_mem=48' >> /boot/config.txt"
			sudo sh -c "echo 'cma=128M' >> /boot/cmdline.txt"
			sync
			echo "Wrote configuration changes to SD card."
	       	 	sleep 2
			else
			echo ""
			echo "Warning: Compilation freezes & errors are likely to occur on your Pi"
			echo ""
			sleep 3
		fi
	fi
fi

if [[ $fixmem =~ [Yy] || $vc4add =~ "vc4" ]]
then
clear
echo "System configuration has changed!"
read -p "Reboot to enable changes? (Y/N): " fixstart
	if [[ $fixstart =~ [Yy] ]]
	then
	echo ""
	echo "Rebooting RasPi in 4 seconds! Press Control-C to cancel."
	sleep 4
	sudo reboot
	fi
	fi
fi # "Should never run on a Pi 4" part ends here

#--------------------------------------------------------------------------------
#-------------------------------------------------------------------------------
clear
echo "Beginning installation"
echo ""
echo "Step 1. Installing latest dependencies"
echo "Allow installation & checking of Super Mario 64 compile dependencies?"
read -p "Install? (Y/N): " instdep

if [[ $instdep =~ [Yy] ]]
then
echo ""
sudo apt-get update
sudo apt install -y build-essential git python3 libaudiofile-dev libglew-dev libsdl2-dev
sync
else
echo ""
echo "Super Mario 64 dependencies not installed."
echo "Please manually install if Raspbian is modified from stock"
echo ""
sleep 3
fi

#--------------------------------------------------------------------------------------
#-------------------------------------------------------------------------------------
clear
echo "Optional: Compile SDL2 with 'KMSDRM' for enhanced performance?"
echo "KMSDRM allows Super Mario 64 to be run without GUI/Desktop (Xorg) enabled on boot"
echo ""
echo "Warning: Compile could take up to an hour on older Raspberry Pi models"
read -p "Proceed? (Y/N): " sdlcomp

if [[ $sdlcomp =~ [Yy] ]]
then
echo ""
echo "Installing dependencies for SDL2 compilation"

sudo sed -i '/^#\sdeb-src /s/^#//' "/etc/apt/sources.list"
sudo apt build-dep libsdl2
sudo apt install -y libdrm-dev libgbm-dev
sync

echo ""
echo "Creating folder src in HOME directory for compile"
echo ""

mkdir $HOME/src
cd $HOME/src
mkdir $HOME/src/sdl2
cd $HOME/src/sdl2
sleep 2

echo "Downloading SDL2 from libsdl.org and unzipping to HOME/src/sdl2/SDL2"
wget https://www.libsdl.org/release/SDL2-2.0.10.tar.gz
sync
tar xzf ./SDL2*.gz
sync
cd ./SDL2*

echo "Configuring SDL2 library to enable KMSDRM (Xorg free rendering)"
./configure --enable-video-kmsdrm
echo "Compiling modified SDL2 and installing."
make
sudo make install
fi

#----------------------------------------------------------------------
#---------------------------------------------------------------------
sleep 2
clear
echo "Super Mario 64 RPi preparation & downloader"
echo ""
echo "Checking in current directory and"
echo "checking in "$HOME"/src/sm64pc/sm64ex/ for existing Super Mario 64 PC files"
echo ""
sm64dircur=$(ls ./Makefile)
sm64dir=$(ls $HOME/src/sm64pc/sm64ex/Makefile)

if [[ $sm64dircur =~ "Makefile" ]] #If current directory has a makefile
then
sm64dir=$sm64dircur
curdir=1; #If current directory has a Makefile or is git zip
fi

if [[ $sm64dir =~ "Makefile" ]];
then
    echo "Existing Super Mario 64 PC port files found!"
    echo "Redownload files (fresh compile)?"
    read -p "Redownload? (Y/N): " sm64git

    if [[ $sm64git =~ "N" ]] # Do NOT redownload, USE current directory for compile
    then
    sm64dir=1; # Don't redownload files , use current directory (has sm64 files)
    curdir=1 
    fi

else #Do a fresh compile in HOME/src/sm64pc/sm64ex/
    sm64dir=0;
    curdir=0;
fi

#------------------------------------------------------------------------------
#------------------------------------------------------------------------------

if [[ $sm64git =~ [Yy] || $sm64dir == 0 || $curdir == 0 ]]  #If user wants to redownload or NOT git-zip execution
then
echo "Step 2. Super Mario 64 PC-Port will now be downloaded from github"
echo "Current folder will NOT be compiled."
read -p "Proceed? (Y/N): " gitins

if [[ $gitins =~ [Yy] ]]
then
echo ""
echo "Creating directory "$HOME"/src/sm64pc"
mkdir $HOME/src/
cd $HOME/src/
mkdir $HOME/src/sm64pc
cd $HOME/src/sm64pc

echo ""
echo "Downloading latest Super Mario 64 PC-port code"
git clone ${repo_url}
cd $HOME/src/sm64pc/sm64ex/
echo "Download complete"
echo ""
sleep 2
fi #End of downloader
fi
sleep 2

#-------------------------------------------------------------------
#------------------------------------------------------------------
clear
echo "Super Mario 64 RPi compilation"
echo ""
echo "Step 3. Compiling Super Mario 64 for the Raspberry Pi"
echo ""
echo "Warning: Super Mario 64 assets are required in order to compile"
if [[ $curdir == 1 ]]
then
echo "Assets will be extracted from "$PWD" "
else
echo "Assets will be extracted from $HOME/src/sm64pc/sm64ex/baserom.(us/eu/jp).z64 "
fi

if [[ $curdir == 1 ]]
then
sm64z64=$(find ./* | grep baserom) #See if current directory is prepped
else
sm64z64=$(find $HOME/src/sm64pc/sm64ex/* | grep baserom) #see if fresh compile directory is prepped
fi

if [[ $sm64z64 =~ "baserom" ]]
then
echo ""
echo "Super Mario 64 assets found in compilation directory"
echo "Continuing with compilation"

else
echo ""
echo "Please satisfy this requirement before continuing."
echo "Exiting Super Mario 64 RasPi setup and compilation script."
echo ""
echo "Note: Re-run script once baserom(s) are inserted into"

if [[ $curdir == 1 ]]
then
echo $PWD
echo ""
else
echo ""
echo $HOME/src/sm64pc/sm64ex/
fi

sleep 5
exit

fi

#-------------------------------------------------------------------------------
#-------------------------------------------------------------------------------
sleep 3
clear
echo ""

if [[ $curdir != 1 ]] # If we're not compiling from a git zip / random directory
then
cd $HOME/src/sm64pc/sm64ex/
fi

echo "Beginning Super Mario 64 RasPi compilation!"
echo ""
echo "Warning: Compilation may take up to an hour on weaker hardware"
echo "At least 300MB of free storage AND RAM is recommended"
echo ""
make clean
sync
if [[ $pimodel =~ "armv6" ]]
then
make TARGET_RPI=1
else
make TARGET_RPI=1 -j4
fi
sync


#---------------------------------------------------------------------------
#--------------------------------------------------------------------------

if [[ $curdir == 1 ]]
then
sm64done=$(find ./build/*/* | grep .arm)
else
sm64done=$(find $HOME/src/sm64pc/sm64ex/build/*/* | grep .arm)
fi

echo ""
if [[ $sm64done =~ ".arm" ]]
then
echo "Super Mario 64 RasPi compilation successful!"
echo "You may find it in"

if [[ $curdir == 1 ]]
then
$sm64loc=$(ls ./build/*pc/*.arm)
else
$sm64loc=$(ls $HOME/src/sm64pc/sm64ex/build/*pc/*.arm)
fi

echo $sm64loc

echo ""
echo "Execute compiled Super Mario 64 RasPi?"
read -p "Run game (Y/N): " sm64run

if [[ $sm64run =~ [Yy] ]]
then
cd
chmod +x $sm64loc
bash $sm64loc
sleep 1
fi

else
echo "Cannot find compiled sm64*.arm binary..."
echo "Please note of any errors during compilation process and report them to"
echo "${repo_url}"
sleep 5
fi

exit
