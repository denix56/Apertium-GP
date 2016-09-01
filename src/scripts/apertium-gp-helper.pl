#!/usr/bin/perl

#script for executing all sudo commands required by Apertium-GP

use v5.18;
use strict;
no warnings 'all';
use Getopt::Long;
use List::Util qw( min max );
use List::MoreUtils qw(uniq);
use File::Path qw ( remove_tree );

sub _systemctl_start{ system("systemctl start apertium-apy"); };
sub _systemctl_stop{ system("systemctl stop apertium-apy"); };
sub _systemctl_restart{ system("systemctl restart apertium-apy"); };

sub _upstart_start{ system("start apertium-apy"); };
sub _upstart_stop{ system("stop apertium-apy"); };
sub _upstart_restart{ system("restart apertium-apy"); };

#server management
my ($start, $stop, $restart);

sub set_server_cmds {
	if (system("which systemctl >/dev/null") == 0) {
		$start = \&_systemctl_start;
		$stop = \&_systemctl_stop;
		$restart = \&_systemctl_restart;
   	} elsif (system("which upstart >/dev/null") == 0) { 
   		$start = \&_upstart_start;
		$stop = \&_upstart_stop;
		$restart = \&_upstart_restart;
	} else {
		say "No systemd or upstart installed.";
		exit 3;
	}
}

my $Manager;

sub _apt_install{ return system("$Manager -y install @_") };
sub _apt_remove{ return system("$Manager -y remove @_") };
sub _apt_update{ return system("$Manager update") };
sub _apt_install_server{ return system("$Manager -y install apertium-apy") };
sub _apt_remove_server{ return system("$Manager -y remove apertium-apy") };
sub _apt_search_package{ return `apt-cache search @_` };
sub _apt_get_info{ return `apt-cache show @_` };
sub _aptitude_get_info{ return `$Manager show @_` };

sub _dnf_yum_install{ return system("$Manager --y install @_") };
sub _dnf_yum_remove{ return system("$Manager --y remove @_") };
sub _dnf_yum_update{ return system("$Manager check-update") };
sub _dnf_yum_install_server{ return system("cd /usr/share/apertium-gp && $Manager --y install python3-devel python3-pip zlib-devel subversion \
	&& pip3 install --upgrade tornado && svn co https://svn.code.sf.net/p/apertium/svn/trunk/apertium-tools/apertium-apy") };
sub _dnf_yum_remove_server{ return remove_tree('/usr/share/apertium-gp/apertium-apy') };
sub _dnf_yum_get_info{ return `$Manager info @_` };

sub _zypper_install{ return system("$Manager --non-interactive install @_") };
sub _zypper_remove{ return system("$Manager --non-interactive remove @_") };
sub _zypper_update{ return system("$Manager refresh") };
sub _zypper_install_server{ return system("cd /usr/share/apertium-gp && $Manager --non-interactive install python3-devel python3-pip zlib-devel subversion \
	&& pip3 install --upgrade tornado && svn co https://svn.code.sf.net/p/apertium/svn/trunk/apertium-tools/apertium-apy") };
sub _zypper_remove_server{ return remove_tree('/usr/share/apertium-gp/apertium-apy') };
sub _zypper_get_info{ return `$Manager info @_` };

sub _other_search_package{ return `$Manager search @_` };

my ($install, $remove, $update, $install_server, $remove_server, $get_info, $search_package);

sub set_manager_commands {
	for ($Manager) {
		when ("apt-get" || "aptitude") {
			$install = \&_apt_install;
			$remove = \&_apt_remove;
			$update = \&_apt_update;
			$install_server = \&_apt_install_server;
			$remove_server  = \&_apt_remove_server;
			if ($_ eq "apt-get"){
				$get_info = \&_apt_get_info;
			} else{
				$get_info = \&_aptitude_get_info;
			}
			set_server_cmds();
		}
		when ("dnf" || "yum") {
			$install = \&_dnf_yum_install;
			$remove = \&_dnf_yum_remove;
			$update = \&_dnf_yum_update;
			$install_server = \&_dnf_yum_install_server;
			$remove_server  = \&_dnf_yum_remove_server;
			$get_info = \&_dnf_yum_get_info;
			$start = sub{ exit 2;};
			$stop = sub{ exit 2;};
			$restart = sub{ exit 2;};
			}
		when("zypper") {
			$install = \&_zypper_install;
			$remove = \&_zypper_remove;
			$update = \&_zypper_update;
			$install_server = \&_zypper_install_server;
			$remove_server  = \&_zypper_remove_server;
			$get_info = \&_zypper_get_info;
			$start = sub{ exit 2;};
			$stop = sub{ exit 2;};
			$restart = sub{ exit 2;};
		}
		default { 
			say "No supported package managers found.";
			exit 1;
		}
	}	
	if ($Manager eq "apt-get") {
		$search_package = \&_apt_search_package;
	} else { 
		$search_package = \&_other_search_package;
	}
}

sub get_size {
	for ($Manager) {
		when ("apt-get" || "aptitude") {
			my @output = $get_info->(@_) =~ /\nSize:\s(\d+)\n/g;
			print max(@output);
		}
		when ("zypper") {
			my @output = $get_info->(@_) =~ /\s(\d+[,\.]?\d*)\s([KM])?i?B\n/g;
			if ($2 eq "K"){
				print int(max(@output)+0.5)*1024;
			} elsif ($2 eq "M"){
				print int(max(@output)+0.5)*1024*1024;
			} else{
				print int(max(@output)+0.5);
			}
		}
		when ("dnf" || "yum") {
			my @output = $get_info->(@_) =~ /\nSize: (\d+)\n/g;
			print max(@output);
		}
	}
}

sub set_manager {
	my @manager_array = qw/zypper dnf yum apt-get aptitude/;
	foreach my $temp_manager (@manager_array) {
		if(system("which $temp_manager >/dev/null") == 0){
			$Manager = $temp_manager;
			set_manager_commands();
			return;
		}
	}
	say "No supported package managers found.";
	exit 1;
}

#main part
my $Packages_Modified = 0;

sub install_handler {
	my @args = @_;
	my $packages = join(' ', $args[1]);
	if (index($packages, "apertium-apy") != -1) {
    	$packages =~ s/\s?apertium-apy\s?/ /;
    	$install_server->();
	}
	$install->($packages);
	$Packages_Modified = 1;

}

sub remove_handler {
	my @args = @_;
	my $packages = join(' ', $args[1]);
	if (index($packages, "apertium-apy") != -1) {
    	$packages =~ s/\s?apertium-apy\s?/ /;
    	$remove_server->();
	}
	$remove->($packages);
	$Packages_Modified = 1;
}

sub search_handler {
my @args = @_;
	my @output = $search_package->($args[1]) =~ /apertium-((?!all-dev)[a-z]{2,3}-[a-z]{2,3})\s/g;
	print join(' ', uniq @output);
}

set_manager();

GetOptions ("install=s{1,}" => \&install_handler,
			"remove=s{1,}" => \&remove_handler,
			"update" => $update,
			"info=s" => \&get_size,
			"search=s" => \&search_handler,
			"start" => $start,
			"stop" => $stop,
			"restart" => $restart);

if ($Packages_Modified) {
	$restart->();
}
