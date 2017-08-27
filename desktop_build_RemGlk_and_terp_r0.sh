#!/bin/bash


# Makefile for RemGlk discussion
#   http://www.intfiction.org/forum/viewtopic.php?f=7&t=19868&start=10
#  TIP: "glulxer" means "glulxe remote"
#
# Recent work on Makefile integration with Z-terps for Plan 9 OS
#   https://bitbucket.org/mycroftiv/cursesifinterpreters
#


below_directory_name="remglk_desktop_src"
build_directory_name="remglk_desktop_build"

pwd
# confirm the path is where we expect it to be for relative work
if [ ! -f "Qt_Thunderquake_WR0.pro" ]; then
  echo "!!!! ERROR_CODE:A00 expected Qt_Thunderquake_WR0.pro does not exist, aborting"
  exit 1
else
  echo "!!!! Running from Thunderquake project folder"
fi


#
# *************************************************************
#
promptPause()
{
	read -rsp $'Press any key to continue...\n' -n1 key
}


prepBuild()
{
	echo "***"
	echo "*** Github checkouts"
	echo "****"
	pwd

	git clone https://github.com/erkyrath/remglk
	git clone https://github.com/erkyrath/glulxe

	echo "***"
	echo "*** RemGlk"
	echo "***"

	cd remglk
	pwd
	make

	cd ..

	if [ ! -f "../terpmake/Glulxe_RemGlk_Makefile" ]; then
	  echo "!!!! ERROR_CODE:A00 Glulxe_RemGlk_Makefile not found"

      echo "***"
      echo "*** Glulxe Makefile needs to be revised to pick RemGlk"
      echo "***"

      promptPause
	else
	  cp "../terpmake/Glulxe_RemGlk_Makefile" glulxe/Makefile
	fi

	cd glulxe

	make
}


#
# *************************************************************
# ** main starts here
#

if [ -d $below_directory_name ]
then
    echo "'$below_directory_name' directory is ready"
else
    mkdir $below_directory_name
fi

cd $below_directory_name
pwd

prepBuild

# check if output binary found
if [ -f "./glulxe" ]; then
    # make a copy of the binary executable file with more specific name
    cp ./glulxe remglk_glulxe
	pwd
	echo "***"
	echo "*** executable binary should be at: remglk_desktop_src/glulxe/remglk_glulxe"
	echo "***"
else
	pwd
	echo "***"
	echo "*** executable binary should be ?? at: remglk_desktop_src/glulxe/glulxe"
	echo "***"
fi
