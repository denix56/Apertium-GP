#!/bin/bash

#script for executing all sudo commands required by Apertium-GP

PKGMOD=false

START="start apertium-apy"
STOP="stop apertium-apy"
RESTART="restart apertium-apy"

if [[ "$(which systemctl)" != "" ]]; then
   START="systemctl $START"
   STOP="systemctl $STOP"
   RESTART="systemctl $RESTART"
fi

while getopts "i:r:u:st" opt; do
	case $opt in
		i|r)
      	$OPTARG
      	PKGMOD=true
      	;;
   	u)
   		$OPTARG
   		;;
   	s)
   		$START
   		;;
   	t) 
   		$STOP
   		;;
    	\?)
      	echo "Invalid option: -$OPTARG" >&2
      	exit 1
      	;;
    	:)
      	echo "Option -$OPTARG requires an argument." >&2
      	exit 1
      	;;
  	esac
done

if $PKGMOD ; then
	$RESTART
fi 
