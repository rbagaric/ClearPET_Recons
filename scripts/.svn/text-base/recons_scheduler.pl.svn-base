#!/usr/bin/perl

=pod
=head1 ClearPET Reconstruction Scheduler
=head2 Overview
This program is intended to be running as a background process.
It watches the status folder once per minute and checks for new
parameter files. If such a file arrives, it starts a reconstruction
process by calling the reconstruction script and waiting for it to
complete. 

=head2 Access Rights
This process must be running with read/write access rights in the
status folder, and any file possibly created in there. To be sure
about that, it could be running as root. If so, be sure to remove
write access for anybody on file of this script for security reasons.

=head2 Used Files and Folders
The only fixed filename used in this script is F</etc/clearpet.conf>,
the ClearPET configuration file. In this file several other locations
can be configured. This script uses the I<StatusDir> key in order
to locate the status folder, and the I<ScriptDir> key to locate the
reconstruction scripts.
=cut

use strict;
use warnings;
use POSIX qw(EEXIST);
use Fcntl;
use Sys::Syslog;

# Name of the pid file
my $pidFile = '/var/run/recons_scheduler.pid';

# Name of the system configuration file
my $confFileName = '/etc/clearpet.conf';

my $statusDir = '';
my $pollInterval = 10;
my $normalize;
my $recons;
my $delete;

my $reloadConfig = 0;
my $terminate = 0;

#############################################################################
# Some helper functions
#

sub trim
{
	my $string = shift;
	$string =~ s/^\s+//;
	$string =~ s/\s+$//;
	return $string;
}

# Read a parameter file into the global params hash
#
# Parameter files have one parameter per line in the form name=value.
# Empty lines or lines starting with a # are ignored. White space
# before or after name and value is ignored as well. That means a line
# line "test=1" is the same as "  test  =  1  ".
#
# Arguments:
# - name of the parameter file
#
sub ReadParameters
{
  my ($infile) = @_;
  my %params;

  open my $in, '<', $infile or die "Cannot read $infile: $!";
  while (<$in>)
  {
    chomp;
	next unless $_;
	next if /^#/;
	my ($name, $value) = split(/=/, $_, 2);
	$name = trim($name);
	$value = trim($value);
	$params{$name} = $value;
  }
  return %params;
}



sub VerifyFolderParam
{
	my ($configRef, $folderParam) = @_;

	my $folder = $$configRef{$folderParam};
	$folder && length($folder)
	  or die "Configuration error: missing or invalid value of parameter $folderParam";
	-d $folder
	  or die "Configuration error: directory specified in parameter $folderParam does not exist";

	return $folder;
}

sub VerifyExec
{
	my ($binDir, $execName) = @_;

	my $exec = $binDir.'/'.$execName;
	-x $exec
	  or die "Configuration error: file $execName does not exist or is not executable";

	return $exec;
}

sub FindNextParamFile
{
    my ($prefix, $folder) = @_;
	my $cmd = "ls $folder/$prefix* 2>/dev/null";
	my @paramFiles = qx/$cmd/;
	if ($paramFiles[0])
	{
		chomp $paramFiles[0];
	}
	return $paramFiles[0];
}

sub ReloadConfig
{
	# Clear the reload flag before doing anything. Otherwise
	# we will end in endless reloads in case of config errors.
	$reloadConfig = 0;

	# Read the ClearPET configuration
	my %reconsConfig = ReadParameters($confFileName);

	my $dir = VerifyFolderParam(\%reconsConfig, 'StatusDir');
	-w $dir or die "Directory $dir is not writeable";
	my $scriptDir = VerifyFolderParam(\%reconsConfig, 'ScriptDir');
	VerifyExec($scriptDir, 'normalize.pl');
	VerifyExec($scriptDir, 'recons.pl');
	VerifyExec($scriptDir, 'delete.pl');

	$statusDir = VerifyFolderParam(\%reconsConfig, 'StatusDir');
	$normalize = $scriptDir.'/normalize.pl';
	$recons = $scriptDir.'/recons.pl';
	$delete = $scriptDir.'/delete.pl';
    $pollInterval = $reconsConfig{'SchedulerPollingInterval'};
	if (!$pollInterval || $pollInterval < 5 || $pollInterval > 60)
	{
		$pollInterval = 10;
	}
}

#############################################################################
# Main program
#

sub HUP_Handler
{
	$reloadConfig = 1;
}

sub TERM_Handler
{
	$terminate = 1;
}

# This function represents the main progam
# We use this as a function because we want to be able to catch any "die"
# events in this code from the caller.

sub main
{
	ReloadConfig();

	while (!$terminate)
	{
		if ($reloadConfig)
		{
			eval { ReloadConfig(); };
			if ($@)
			{
				syslog('err', "Error reloading configuration:\n$@\n");
			} else
			{
				syslog('info', "Reloaded configuration after receiving HUP signal\n");
			}
		}
		my $type;
		my $program;
		my $paramFile = FindNextParamFile('normalize', $statusDir);
		if ($paramFile)
		{
			$program = $normalize;
			$type = 'Normalization';
		} else
		{
			$paramFile = FindNextParamFile('params', $statusDir);
			if ($paramFile)
			{
				$program = $recons;
				$type = 'Reconstruction';
			} else
			{
				$paramFile = FindNextParamFile('delete', $statusDir);
				$program = $delete if $paramFile;
				$type = 'Removal of data sets';
			}
		}

		if (!$paramFile)
		{
			sleep($pollInterval);
		} else
		{
			syslog('info', "start processing on $paramFile.\n");
			system('sudo', '-u', 'clearpet', $program, "-p$paramFile", "-s$statusDir");

			my $msg;
			if ($? == -1)
			{
				$msg = "Failed to execute $program\n";
			} elsif ($? & 127)
			{
				my $sig = $? & 127;
				$msg = "$program died with signal $sig\n";
			} elsif ($? != 0)
			{
				my $code = $? >> 8;
				$msg = "$program exited with value $code\n"; 
			}
			if ($msg)
			{
				syslog('err', $msg);
			}
			
			if (-r $paramFile)
			{
				# If the parameter file still exists, there was something
				# seriously wrong with the reconstruction. We create a status
				# file in order to inform the user about the problem. Then
				# we delete the parameter file.
				my %params = ReadParameters($paramFile);
				my $statusFile = $paramFile;
				$statusFile =~ s/^$statusDir//;
				$statusFile =~ s/^\/*//;
				$statusFile = "$statusDir/status_$statusFile";
				if (open my $out, '>', $statusFile)
				{
					print $out 'Type = ', ($type) ? $type : 'Unknown', "\n";
					print $out 'Data = ', ($params{'DataSet'}) ? $params{'DataSet'} : 'Unknown', "\n";
					print $out 'Output = ', ($params{'ImageName'}) ? $params{'ImageName'} : 'Unknown', "\n";
					print $out "Status = Error\n";
					print $out "The reconstruction script did not run properly.\n";
					if ($msg)
					{
						print $out $msg;
					}
					close $out;
					chmod 0666, $out;
				}

				unlink($paramFile);
			}
		}
	}

} # end af main subroutine


#
# Here, the real Perl main code begins
#

# Daemonize the process, that means we chdir to root directory
# and change the std output streams to /dev/null.
chdir '/' or die "Cannot chdir to /: $!";
open STDIN, '/dev/null' or die "Cannot read /dev/null: $!";
open STDOUT, '>/dev/null' or die "Cannot write /dev/null: $!";

openlog 'ClearPET reconstruction', '', 'daemon';
syslog('info', "Scheduler started.\n");

my $pf;
if (!sysopen $pf, $pidFile, O_WRONLY|O_CREAT|O_EXCL, 0644)
{
	$! == EEXIST or die "Error opening pid file $pidFile: $!";
	# pid file already exists. Check if that process is still valid
	sysopen $pf, $pidFile, O_RDWR or die 'Cannot read pid file';
	my $id = <$pf>;
	$id or die 'Invalid contents of pid file';
	chomp $id;
	my $cmd = "ps -p $id --no-header";
	if (`$cmd`)
	{
		# Ok, seems that this process is already running. We stop now
		exit;
	} else
	{
		# No process of that process is running, seems to be an orphaned pid file;
		# We overwrite the pid file with our id
		seek($pf, 0, 0);
	}	
}
# write our process id to the pid file
print $pf $$;
truncate($pf, tell($pf));
close $pf;

$SIG{TERM} = \&TERM_Handler;
$SIG{HUP} = \&HUP_Handler;
eval { main(); };
if ($@)
{
	syslog('err', "Fatal error:\n$@\n");
} elsif ($terminate)
{
	syslog('info', "Terminated after receiving TERM signal\n");
}

unlink $pidFile;

