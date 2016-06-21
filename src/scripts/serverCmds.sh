#!/bin/bash
if [ "$1" = "-s" ] 
 then
   systemctl start apertium-apy;
else
   systemctl stop apertium-apy;
fi 
