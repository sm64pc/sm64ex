#!/bin/bash

# Directories and Files
LIBDIR=./tools/lib/
LIBAFA=libaudiofile.a
LIBAFLA=libaudiofile.la
AUDDIR=./tools/audiofile-0.3.6

# Command line options
OPTIONS=("Analog Camera" "No Draw Distance" "Text-saves" "Smoke Texture Fix" "Release build" "Clean build")
EXTRA=("BETTERCAMERA=1" "NODRAWINGDISTANCE=1" "TEXTSAVES=1" "TEXTURE_FIX=1" "DEBUG=0" "clean")

# Colors
RED=$(tput setaf 1)
GREEN=$(tput setaf 2)
YELLOW=$(tput setaf 3)
CYAN=$(tput setaf 6)
RESET=$(tput sgr0)

# Checks to see if the libaudio directory and files exist
if [ -d "$LIBDIR" -a -e "${LIBDIR}$LIBAFA" -a -e "${LIBDIR}$LIBAFLA"  ]; then
    printf "\n${GREEN}libaudio files exist, going straight to compiling.${RESET}\n"
else 
    printf "\n${GREEN}libaudio files not found, starting initialization process.${RESET}\n\n"

    printf "${YELLOW} Changing directory to: ${CYAN}${AUDDIR}${RESET}\n\n"
		cd $AUDDIR

    printf "${YELLOW} Executing: ${CYAN}autoreconf -i${RESET}\n\n"
		autoreconf -i

    printf "\n${YELLOW} Executing: ${CYAN}./configure --disable-docs${RESET}\n\n"
		PATH=/mingw64/bin:/mingw32/bin:$PATH LIBS=-lstdc++ ./configure --disable-docs

    printf "\n${YELLOW} Executing: ${CYAN}make -j${RESET}\n\n"
		PATH=/mingw64/bin:/mingw32/bin:$PATH make -j

    printf "\n${YELLOW} Making new directory ${CYAN}../lib${RESET}\n\n"
		mkdir ../lib


    printf "${YELLOW} Copying libaudio files to ${CYAN}../lib${RESET}\n\n"
		cp libaudiofile/.libs/libaudiofile.a ../lib/
		cp libaudiofile/.libs/libaudiofile.la ../lib/

    printf "${YELLOW} Going up one directory.${RESET}\n\n"
		cd ../

		printf "${GREEN}Notepad will now open, please follow the instructions carefully.\n\n"
		printf "${YELLOW}Locate the line: " 
		printf "${CYAN}tabledesign_CFLAGS := -Wno-uninitialized -laudiofile\n"
		printf "${YELLOW}Then add at the end: ${CYAN}-lstdc++\n" 
		printf "${YELLOW}So it reads: "
		printf "${CYAN}tabledesign_CFLAGS := -Wno-uninitialized -laudiofile -lstdc++\n\n"
		notepad "Makefile"
		read -n 1 -r -s -p $'\e[32mPRESS ENTER TO CONTINUE...\e[0m\n'

    printf "${YELLOW} Executing: ${CYAN}make -j${RESET}\n\n"
		PATH=/mingw64/bin:/mingw32/bin:$PATH make -j		

    printf "\n${YELLOW} Going up one directory.${RESET}\n"
		cd ../
fi 

menu() {
		printf "\nAvaliable options:\n"
		for i in ${!OPTIONS[@]}; do 
				printf "%3d%s) %s\n" $((i+1)) "${choices[i]:- }" "${OPTIONS[i]}"
		done
		if [[ "$msg" ]]; then echo "$msg"; fi
		printf "${YELLOW}Please do not select \"Clean build\" with any other option.\n"
		printf "Leave all options unchecked for a Vanilla build.\n${RESET}"
}

prompt="Check an option (again to uncheck, press ENTER):"
while menu && read -rp "$prompt" num && [[ "$num" ]]; do
		[[ "$num" != *[![:digit:]]* ]] &&
		(( num > 0 && num <= ${#OPTIONS[@]} )) ||
		{ msg="Invalid option: $num"; continue; }
		((num--)); # msg="${OPTIONS[num]} was ${choices[num]:+un}checked"
		[[ "${choices[num]}" ]] && choices[num]="" || choices[num]="+"
done

for i in ${!OPTIONS[@]}; do 
		[[ "${choices[i]}" ]] && { CMDL+=" ${EXTRA[i]}"; }
done 

printf "\n${YELLOW} Executing: ${CYAN}make ${CMDL} -j${RESET}\n\n"
PATH=/mingw32/bin:/mingw64/bin:$PATH make $CMDL -j

if [ "${CMDL}" != " clean" ]; then

	printf "\n${GREEN}If all went well you should have a compiled .EXE in the 'builds/us_pc/' folder.\n"
	printf "${CYAN}Would you like to run the game? [y or n]: ${RESET}"
	read TEST

	if [ "${TEST}" = "y" ]; then
		exec ./build/us_pc/sm64.us.f3dex2e.exe
	fi 
else
	printf "\nYour build is now clean\n"
fi 