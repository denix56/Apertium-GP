#!/bin/bash

#script for executing all sudo commands required by Apertium-GP

function set_server_cmds() {
	PKGMOD=false
	
	if [[ -n "$(which systemctl)" ]]; then
		readonly START="systemctl start apertium-apy"
   	readonly STOP="systemctl stop apertium-apy"
   	readonly RESTART="systemctl restart apertium-apy" 
	elif [[ -n "$(which upstart)" ]]; then
		readonly START="start apertium-apy"
		readonly STOP="stop apertium-apy"
		readonly RESTART="restart apertium-apy"
	else
		echo "No systemd or upstart installed."
		exit 3
	fi
}

function get_manager() {
	mngr_array=( [1]=apt-get [2]=aptitude [3]=dnf [4]=yum [5]=zypper )
	for MNGR in "${mngr_array[@]}"; do
		which $MNGR >/dev/null
		ret_code=$?
		if [[  $ret_code -eq 0 ]]; then
			break
		fi
	done
	
	readonly MNGR

	case $MNGR in
	"apt-get" | "aptitude" )
			INSTALL="$MNGR -y"
			REMOVE=$INSTALL
			readonly UPDATE="$MNGR update"
			readonly SERVERINST="$MNGR -y install apertium-apy"
			readonly SERVERRM="$MNGR -y remove apertium-apy"
			INFO="$MNGR show"
			set_server_cmds
			;;
	"yum" | "dnf" )
			INSTALL="$MNGR --y"
			REMOVE=$INSTALL
			readonly UPDATE="$MNGR check-update"
			INFO="$MNGR info"
			readonly SERVERINST="cd /usr/share/apertium-gp && $MNGR --non-interactive install python3-devel python3-pip zlib-devel subversion \
			&& pip3 install --upgrade tornado && svn co https://svn.code.sf.net/p/apertium/svn/trunk/apertium-tools/apertium-apy"
			readonly SERVERRM="$MNGR -y remove apertium-apy"
			;;
	"zypper" )
			INSTALL="$MNGR --non-interactive"
			REMOVE=$INSTALL
			readonly UPDATE="$MNGR refresh"
			INFO="$MNGR info"
			readonly SERVERINST="cd /usr/share/apertium-gp && $MNGR --non-interactive install python3-devel python3-pip zlib-devel subversion
			&& pip3 install --upgrade tornado && svn co https://svn.code.sf.net/p/apertium/svn/trunk/apertium-tools/apertium-apy"
			readonly SERVERRM="$MNGR -y remove apertium-apy"
			;;
		* )
			echo "No supported package manager found"
			exit 1
			;;
	esac
	
	if [[ "$MNGR" = "apt-get" ]]; then
		readonly SEARCH="apt-cache search"
		INFO="apt-cache show"
	else
		readonly SEARCH="$MNGR search"
	fi
	
	readonly INSTALL="$INSTALL install"
	readonly REMOVE="$REMOVE remove"
	readonly INFO
}

function get_size() {
	case $MNGR in
	"apt-get" | "aptitude" )
		sed -n -e 's/^Size\: //p' <<< "$($INFO $OPTARG)"
   	;;
   "zypper" )
   	local output=$(sed -n -e 's/.* \([0-9]\+.*\[KM]iB)/\1/; s/,/\./p' <<< "$($INFO $OPTARG)")
   	if [[ "$output" == *"KiB"* ]]; then
   		echo "${output// *}*1024" | bc -l
   	elif [[ "$output" == *"MiB"* ]]; then
   		echo "${output// *}*1048576" | bc -l
   	else
   		echo "${output// *}"
   	fi
   	;;
   "yum" | "dnf" )
   	;;
   esac	
}

get_manager

while getopts "i:r:I:S:ust" opt; do
	case $opt in		
		#install packages
		i)
			if [[ "$OPTARG" == *"apertium-apy"* ]]; then
				$SERVERINST
				OPTARG=${OPTARG//apertium-apy}
			fi
      	$INSTALL $OPTARG 
      	PKGMOD=true
      	;;
      
      #remove packages
      r)
      	if [[ "$OPTARG" == *"apertium-apy"* ]]; then
				$SERVERRM
				OPTARG=${OPTARG//apertium-apy}
			fi
      	$REMOVE $OPTARG
      	PKGMOD=true
      	;;
      
      #update repositories
   	u)
   		$UPDATE
   		;;
   	
   	#get info (size) about package
   	I)
   		get_size
   		;;
   	
   	#search packages
   	S)
   		$SEARCH $OPTARG
   		;;
   		
   	#start server
   	s)
   		if [[ ! -v START ]]; then
   			exit 2
   		fi
   		$START
   		;;
   	
   	#stop (terminate) server
   	t)
   		if [[ ! -v STOP ]]; then
   			exit 2
   		fi
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
