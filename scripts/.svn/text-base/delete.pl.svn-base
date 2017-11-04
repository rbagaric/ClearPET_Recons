#!/usr/bin/perl

use strict;
use warnings;
use open ':encoding(utf8)'; # input/output default encoding will be UTF-8

use Getopt::Std;
$Getopt::Std::STANDARD_HELP_VERSION = 1;
$main::VERSION = "0.11";

# Name of the system configuration file
my $confFileName = "/etc/clearpet.conf";

# This is the global reconstruction configuration hash
my %reconsConfig;
# This is the global reconstruction parameters hash
my %reconsParams;

my $reconsParameterFile = "";
# The name of the directory for status output. If not specified, STDOUT will be used.
my $statusDir = "";
my $numPrepro;
my @shares;

#############################################################################
# Some helper functions
#

sub MountSMBShare
{
	my $unc = shift;
	my $output = qx(/bin/mount $unc 2>&1);
	die "Cannot mount $unc:\n$output" if $output;
}

sub UnmountSMBShare
{
	my $unc = shift;
	system("/bin/umount", $unc) if $unc;
	# Note: We do not check error codes here, because it does not really
	# matter for us if an umount does not work. Actually, we might call
	# this function for UNC names for which a mount failed.
}

# Update the status file for the current reconstruction process
# Arguments:
# - Status of the reconstruction
# - list of messages
sub UpdateStatus
{
	my $status = shift;

	if ($statusDir)
	{
		my $statusFileName = "$statusDir/status$$";
		open my $out, '>', $statusFileName or die "Cannot write $statusFileName: $!";
		print $out "Type = Removal of data sets\n";
		print $out "Data = see below\n";
		print $out "Output =\n";
		print $out "Status = $status\n";
		print $out @_;
	} else
	{
		print "$status: @_\n";
	}
}


# Perl trim function to remove whitespace from the start and end of the string
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
  my %result;

  open my $in, '<', $infile or die "Cannot read $infile: $!";
  while (<$in>)
  {
    chomp;
	next unless $_;
	next if /^#/;
	my ($name, $value) = split(/=/, $_, 2);
	$name = trim($name);
	$value = trim($value);
	$result{$name} = $value;
  }
  return %result;
}


sub usage
{
	my $err = shift;
	print $err, "\n";
	print <<END
Usage: $0 param_file [statusDir]
  param_file is the full path name of a parameter file
  statusDir is the name of a directory for status output. If not specified,
            status output is sent to standard output.
END
}

sub VerifyFolderParam
{
	my ($folderParam) = @_;

	my $folder = $reconsConfig{$folderParam};
	$folder && length($folder)
	  or die "Configuration error: missing or invalid value of parameter $folderParam";
	-d $folder
	  or die "Configuration error: directory specified in parameter $folderParam does not exist";

	return $folder;
}

sub DeleteFile
{
	my $file = shift;
	if (unlink $file)
	{
		return "Deleted $file\n";
	} else
	{
		return "Failed to delete $file:\n$!\n";
	}
}

#############################################################################
# Main program
#

# This function represents the main progam
# We use this as a function because we want to be able to catch any "die"
# events in this code from the caller.

sub main
{
# Read the system parameters
%reconsConfig = ReadParameters($confFileName);

# Read the DAQ parameters
%reconsParams = ReadParameters($reconsParameterFile);

unlink($reconsParameterFile);
 
# Some parameter validations:
$numPrepro = $reconsConfig{"NumPrepros"};
if ($numPrepro < 1 || $numPrepro > 5)
{
	die "Configuration error: missing or invalid value of parameter NumPrepros";
}
my @PreproID;
for (my $p = 1; $p <= $numPrepro; $p++)
{
	$PreproID[$p] = $reconsConfig{"PreproID$p"};
	$PreproID[$p] && length($PreproID[$p]) or die 'Missing or empty parameter "PreproID'.$p.'"';
}

UpdateStatus("Running", "Preparing for removal");

my @dirs;
if ($reconsConfig{"DataFrom"} eq "SinglesFromPrepros")
{
	# we load singles from the preprocessors. We need to mount the samba shares.
	$dirs[0] = $reconsConfig{"MountPointMaster"};
	MountSMBShare($dirs[0]);
	$shares[0] = $dirs[0];
	for (my $p = 1; $p <= $numPrepro; $p++)
	{
		$dirs[$p] = $reconsConfig{"MountPointPrepro$p"};
		MountSMBShare($dirs[$p]);
		$shares[$p] = $dirs[$p];
	}
}
elsif ($reconsConfig{"DataFrom"} eq "SinglesFromLocal")
{
	# we load singles from the local disk. We get the data root, then we
	# check if the data files exist.
	$dirs[0] = VerifyFolderParam("DataRoot");
	for (my $p = 1; $p <= $numPrepro; $p++)
	{
		$dirs[$p] = $dirs[0];
	}
}
else
{
	die "Configuration error: missing or invalid value of parameter DataFrom";
}


my @msgs;
my $numDataSets = $reconsParams{"NumDataSets"};
for (my $i = 1; $i <= $numDataSets; ++$i)
{
	my $dataSet = $reconsParams{"DataSet$i"};
	next unless $dataSet;

	if (substr($dataSet, 0, 1) ne '/')
	{
		$dataSet = '/'.$dataSet;
	}
	push @msgs, DeleteFile($dirs[0].$dataSet."_enc.txt");
	for (my $p = 1; $p <= $numPrepro; $p++)
	{
		push @msgs, DeleteFile($dirs[$p].$dataSet."_".$PreproID[$p].".bin");
	}
	UpdateStatus("Running", @msgs);
}

UpdateStatus("Complete", @msgs);

} # end af main subroutine

#
# Here, the real Perl main code begins
#
my %options;
getopts("p:s:", \%options);
die "Missing required option -p" unless exists $options{"p"};
die "Missing required value for option -p" unless defined $options{"p"};
$reconsParameterFile = $options{"p"};
if (exists $options{"s"})
{
	die "Missing required value option -s" unless defined $options{"s"};
	$statusDir = $options{"s"};
}

eval { main(); };
if ($@)
{
	UpdateStatus("Error", $@);
}

#unmount the shares, if they had been mounted:
for my $s (@shares)
{
	UnmountSMBShare($s);
}
