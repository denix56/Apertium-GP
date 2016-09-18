#!/usr/bin/perl

# Copyright (C) 2016, Denys Senkin <denisx9.0c@gmail.com>
#
# This file is part of apertium-gp
# apertium-gp is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# apertium-gp is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with apertium-gp.  If not, see <http://www.gnu.org/licenses/>.
# 
# script for executing all sudo commands required by Apertium-GP

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
sub _apt_search_package{ return `apt-cache search \"@_\"` };
sub _apt_get_info{ return `apt-cache show @_` };
sub _aptitude_get_info{ return `$Manager show @_` };

sub _dnf_yum_install{ return system("$Manager -y install @_") };
sub _dnf_yum_remove{ return system("$Manager -y remove @_") };
sub _dnf_yum_update{ return system("$Manager check-update") };
sub _dnf_yum_install_server{ return system("cd /usr/share/apertium-gp && $Manager -y install gcc python3-devel python-pycurl python-simplejson python3-pip zlib-devel subversion && pip3 install --upgrade tornado && svn co https://svn.code.sf.net/p/apertium/svn/trunk/apertium-tools/apertium-apy") };
sub _dnf_yum_remove_server{ return remove_tree('/usr/share/apertium-gp/apertium-apy') };
sub _dnf_yum_get_info{ return `$Manager info @_` };

sub _zypper_install{ return system("$Manager --non-interactive install @_") };
sub _zypper_remove{ return system("$Manager --non-interactive remove @_") };
sub _zypper_update{ return system("$Manager refresh") };
sub _zypper_install_server{ return system("cd /usr/share/apertium-gp && $Manager --non-interactive install gcc python3-devel python-pycurl python-simplejson python3-pip zlib-devel subversion && pip3 install --upgrade tornado && svn co https://svn.code.sf.net/p/apertium/svn/trunk/apertium-tools/apertium-apy") };
sub _zypper_remove_server{ return remove_tree('/usr/share/apertium-gp/apertium-apy') };
sub _zypper_get_info{ return `$Manager info @_` };

sub _other_search_package{ return `$Manager search \"@_\"` };

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
	my @args = @_;
	my %package_sizes;
	my @output = split /\n{2,}/, $get_info->($args[1]);
	for ($Manager) {
		when ("apt-get" || "aptitude") {
			foreach (@output) {
				my ($name, $size);
				if ($_ =~ /Package:\s(.*)\n/){
					$name = $1;
				} else {
					next;
				}
				if ($_ =~ /\nSize:\s(\d+)\n/) {
					$size = $1;
				} else {
					next;
				}
				if(exists $package_sizes{$name}){ 
				$package_sizes{$name} = max($size, $package_sizes{$name});
				} else{
					$package_sizes{$name} = $size;
				}
			}
		}
		when ("zypper") {
			shift @output;
			foreach (@output) {
				if (not($_ =~ /(noarch)/)){
					next;
				}
				my ($name, $size);
				if ($_ =~ /((apertium-(?!all-dev)[a-z]{2,3}-[a-z]{2,3})|(tesseract-ocr-(?!(all|dev))[a-z]{3}-?[a-z]*))\n/){
					$name = $1;
				} else {
					next;
				}
				if ($_ =~ /\s(\d+[,\.]?\d*)\s(([KM]i)?B)\n/) {
					$size = $1;		
					if ($2 eq "MiB"){
						$size =  int($size * 1024 * 1024 + 0.5);
					} elsif ($2 eq "KiB"){
						$size = int($size * 1024 + 0.5);
					}
				} else{
					next;
				}
				
				if(exists $package_sizes{$name}){ 
				$package_sizes{$name} = max($size, $package_sizes{$name});
				} else{
					$package_sizes{$name} = $size;
				}
			}	
		}
		when ("dnf" || "yum") {
			foreach (@output) {
				if (not($_ =~ /(noarch)/)){
					next;
				}
				my ($name, $size);
				if ($_ =~ /((apertium-(?!all-dev)[a-z]{2,3}-[a-z]{2,3})|(tesseract-ocr-(?!(all|dev))[a-z]{3}-?[a-z]*))\n/){
					$name = $1;
				} else {
					next;
				}
				if ($_ =~ /\s(\d+[,\.]?\d*)\s([KMB])\n/) {
					$size = $1;		
					if ($2 eq "M"){
						$size =  int($size * 1024 * 1024 + 0.5);
					} elsif ($2 eq "K"){
						$size = int($size * 1024 + 0.5);
					}
				} else {
					next;
				}
				
				if(exists $package_sizes{$name}){ 
				$package_sizes{$name} = max($size, $package_sizes{$name});
				} else{
					$package_sizes{$name} = $size;
				}
			}	
		}
	}
	foreach (keys %package_sizes){
				print "$_ $package_sizes{$_}\n";
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
my @output = $search_package->($args[1]) =~ /((apertium-(?!all-dev)[a-z]{2,3}-[a-z]{2,3})|(tesseract-ocr-(?!(all|dev|equ|osd))[a-z]{3}-?[a-z]*))[\s\n]/g;
print join(' ', grep {$_} uniq @output);
}

set_manager();

GetOptions ("install=s" => \&install_handler,
			"remove=s" => \&remove_handler,
			"update" => $update,
			"info=s" => \&get_size,
			"search=s" => \&search_handler,
			"start" => $start,
			"stop" => $stop,
			"restart" => $restart);
			
if ($Packages_Modified) {
	$restart->();
}

