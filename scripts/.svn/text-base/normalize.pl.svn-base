#!/usr/bin/perl

use strict;
use warnings;
use open ':encoding(utf8)'; # input/output default encoding will be UTF-8

use Getopt::Std;
$Getopt::Std::STANDARD_HELP_VERSION = 1;
$main::VERSION = '0.13';

# Name of the system configuration file
my $confFileName = '/etc/clearpet.conf';

# This is the global reconstruction configuration hash
my %reconsConfig;
# This is the global reconstruction parameters hash
my %reconsParams;

my $reconsFolder = '';
my $binDir;
my $reconsParameterFile = '';
my $dataSet = '';
# The name of the directory for status output. If not specified, STDOUT will be used.
my $statusDir = '';
my @shares;	# only used if DataFrom is set to SinglesFromPrepros
my $numPrepro = 0;
my $ELow;
my $EHigh;
my $coincWindow;
my $angularOffset;

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
	system('/bin/umount', $unc) if $unc;
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
	my ($status, @msg) = @_;

	if ($statusDir)
	{
		my $statusFileName = "$statusDir/status$$";
		open(my $out, '>', $statusFileName) or die "Cannot write $statusFileName: $!";
		print $out "Type = Normalization\n" or die "Cannot print into $statusFileName: $!";
		print $out "Data = $dataSet\n";
		print $out "Output =\n";
		print $out "Status = $status\n";
		print $out "@msg\n";
		close $out;
	} else
	{
		print "$status: @msg\n";
	}
}


# Det the DAQ duration from an encoder file
# We get that from the first column in the second last line of that file.
#
sub GetDaqTime
{
	my $encFile = shift;

	open my $in, '<', $encFile or die "Cannot read $encFile: $!";

	my $line;
	my $prevLine;
	while (<$in>)
	{
		$prevLine = $line;
		$line = $_;
	}
	my ($reset,$dummy1, $dummy2) = split(/\s/, $prevLine, 3);
	return $reset;
}

# Rewrite paramters of a STIR parameter file
#
# Arguments:
# - name of the input parameter file (used as a template)
# - name of the output parameter file
# - a hash containing the rewrite information. The keys of the
#  hash contain the tag names, the values of the hash are the
#  new parameter xvalues for those tags.
#
sub RewriteParFile
{
  my $infile = shift;
  my $outfile = shift;
  my $sep = shift;
  my %rewriteInfo = @_;

  open my $in, '<', $infile or die "Cannot read $infile: $!";
  open my $out, '>', $outfile or die "Cannot write $outfile: $!";

  while (<$in>)
  {
	my ($tag, $dummy) = split(/$sep/);
	my $trimmedTag = trim($tag);
	if (defined($rewriteInfo{$trimmedTag}))
	{
	  print $out "$tag$sep $rewriteInfo{$trimmedTag}\n";
    } else
    {
      print $out $_;
    }
  }
}

# Delete a sinogram
#
# This function just deletes both files which make up a sinogram
#
# Arguments;
#  the sinogram name without suffix
#
sub DeleteSinogram
{
	my $name = shift;
	unlink $name . '.hs', unlink $name . '.s'
	  or die "Error deleting sinogram $name: $!";
}

sub CoincSorter
{
	my ($output, $coincWin, $fov, $firstReset, $lastReset, $randomsCorr) = @_;
	if ($reconsConfig{'UseOldSorter'})
	{
		system("$binDir/CorrectionOffset ./ $output.ccs $coincWin $numPrepro $ELow $EHigh $fov $firstReset $lastReset 0 >coinc.log 2>&1") == 0
			or die 'Error calling coincidence sorter';
		unlink 'Randoms.ccs';
	} else
	{
		my $opts = "-s -c=$coincWin -emin=$ELow -emax=$EHigh -f=$fov";
		$opts .= '-rfrom='.($firstReset+1)." -rto=$lastReset";
		$opts .= ' -n=' .  4*$numPrepro;
		$opts .=  " -o=$output";
		$opts .= " -A=$angularOffset";
		if ($randomsCorr)
		{
			$opts .= ' -t=' . 4*$coincWin;
		}
		my $files = '-a=encTime.txt';
		for (my $i = 1; $i <= $numPrepro; ++$i)
		{
			$files = $files . " file$i.bin";
		}
#old code:
#		system("$binDir/sorter2 $opts $files >coinc.log 2>&1") == 0
#			or die 'Error calling coincidence sorter';
        open(COINC_STATUS, "$binDir/sorter2 $opts $files 2>&1 | /usr/bin/tee coinc.log |")
            or die "Error calling coincidence sorter";
        while (my $line = <COINC_STATUS>)
        {
            UpdateStatus('Running', "Coincidence sorting: $line");
        }
        close COINC_STATUS;
	}
}

sub Rebinner
{
	my ($paramFolder, $fov, $ringDiam, $coincData, $projData) = @_;
	# Note: in this subroutine, $fov must contain the FOV _and_ the diameter
	# in the form "7_Small" or "9_Large"

	# rewrite the seg_31.hs file for the current settings:
	my $outfile = "seg_31.hs";
	my %rewriteInfo;
	my $dim = $reconsConfig{"ImageSize_FOV$fov"};
	die "Configuration error: invalid or missing parameter ImageSize_FOV$fov" if !$dim;
	$rewriteInfo{"!matrix size [1]"} = $dim;
	$rewriteInfo{'inner ring diameter (cm)'} = $ringDiam;
	RewriteParFile("$paramFolder/$outfile", $outfile, ':=', %rewriteInfo);

	# rewrite the lm_to_projdata.par file for the current settings:
	$outfile = 'lm_to_projdata.par';
	%rewriteInfo = ();
	$rewriteInfo{'input file'} = $coincData;
	$rewriteInfo{'output filename prefix'} = $projData;
	RewriteParFile("$paramFolder/$outfile", $outfile, ':=', %rewriteInfo);

	# rewrite the Coincidence.cch file for the current settings:
	$outfile = "$coincData.cch";
	%rewriteInfo = ();
	$rewriteInfo{'ring diameter'} = "$ringDiam cm";
	if (!$reconsConfig{'ModuleAxialPitch'})
	{
		$rewriteInfo{'module axial pitch'} = '2.76 cm';
	} else
	{
		$rewriteInfo{'module axial pitch'} = $reconsConfig{'ModuleAxialPitch'}.' cm';
	}
	$rewriteInfo{'scan file name'} = $coincData;
	RewriteParFile("$paramFolder/Coincidences.cch", $outfile, ':', %rewriteInfo);

	system('/usr/bin/touch', 'seg_31.s');
	system("$binDir/lm_to_projdata lm_to_projdata.par > rebinner.log 2>&1") == 0
		or die 'Error calling rebinner';
}

sub BckTest
{
	my ($imageName, $paramFolder, $projdata, $fov) = @_;
	# Note: in this subroutine, $fov must contain the FOV _and_ the diameter
	# in the form "7_Small" or "9_Large"

	# Note: the bcktest program is special because it asks the user for some parameters.
	# I could not figure out how these parameters could be loaded from a parameter file.
	# Therefore, we feed the program vi stdin with the right answers. That's a bit
	# critical because it could fail with newer versions of the program. It would be
	# better to use expect or something like that to react more flexible to the questions
	# of the program.

	open my $in, '<', "$paramFolder/bcktest.in" or die "Cannot read $paramFolder/bcktest.in: $!";
	open my $out, '>', "bcktest.in" or die "Cannot write bcktest.in: $!";
	while (<$in>)
	{
		if (/^XYPIXELS.*/)
		{
			my $dim = $reconsConfig{"ImageSize_FOV$fov"};
			die "Configuration error: invalid or missing parameter ImageSize_FOV$fov" if !$dim;
			print $out $dim, "\n";
    	} else
    	{
      		print $out $_;
   	 	}
	}
	close $out;
	close $in;
	system("$binDir/bcktest $imageName.hv $projdata < bcktest.in >bcktest.log 2>&1") == 0
		or die 'Error calling bcktest';
}

sub MoveImage
{
	my ($name, $destDir) = @_;

	0 == system('/bin/mv', '--backup=simple', '--suffix=bak', $name.'.hv', $destDir)
		or die "Cannot move image $name to $destDir";
	0 == system('/bin/mv', '--backup=simple', '--suffix=bak', $name.'.v', $destDir)
		or die "Cannot move image $name to $destDir";
}

# Perl trim function to remove whitespace from the start and end of the string
sub trim
{
	my $string = shift;
	if (!$string)
	{
		return "";
	}
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
  param_file is the full path name of a normalization parameter file
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

sub VerifyProgram
{
	my ($execName) = @_;

	my $exec = "$binDir/$execName";
	-x $exec
	  or die "Configuration error: program $execName does not exist or is not executable";

	return $exec;
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

# Some parameter validations:
$numPrepro = $reconsConfig{'NumPrepros'};
if ($numPrepro < 1 || $numPrepro > 5)
{
	die 'Configuration error: missing or invalid value of parameter NumPrepros';
}
my @PreproID;
for (my $p = 1; $p <= $numPrepro; $p++)
{
	$PreproID[$p] = $reconsConfig{"PreproID$p"};
	$PreproID[$p] && length($PreproID[$p]) or die 'Missing or empty parameter "PreproID'.$p.'"';
}

$binDir = VerifyFolderParam('BinDir');
my $paramDir = VerifyFolderParam('ParamDir');
my $reconsRoot = VerifyFolderParam('StatusDir');

if ($reconsConfig{'UseOldSorter'})
{
	VerifyProgram('CorrectionOffset');
} else
{
	VerifyProgram('sorter2');
}
VerifyProgram('lm_to_projdata');
VerifyProgram('bcktest');

$ELow = $reconsConfig{'EnergyLow'};
($ELow >= 0) or die 'Configuration error: Invalid lower energy limit';
$EHigh = $reconsConfig{'EnergyHigh'};
($EHigh >= 0 && $EHigh < 1250) or die 'Configuration error: Invalid upper energy limit';
($ELow < $EHigh) or die  'Configuration error: lower energy limit is above upper energy limit';

$coincWindow = $reconsConfig{'CoincidenceWindow'};
($coincWindow >= 5 && $coincWindow <= 50) or die 'Configuration error: Invalid coincidence window';

$angularOffset = $reconsConfig{'AngularOffset'};
($angularOffset >= -180 && $angularOffset <= 180) or die 'Configuration error: Invalid angular offset';

UpdateStatus('Running', 'Preparing for normalization');

$dataSet = $reconsParams{'DataSet'};
$dataSet && length($dataSet) or die 'Missing or empty parameter "DataSet"';
if (substr($dataSet, 0, 1) ne '/')
{
	$dataSet = '/'.$dataSet;
}

my $ringDiameter = $reconsParams{'RingDiameter'};
($ringDiameter eq 'Small' || $ringDiameter eq 'Large')
	or die 'Parameter error: Invalid value for parameter "RingDiameter"';
my $ringDiamCM = $reconsConfig{"RingDiameter$ringDiameter"};
$ringDiamCM
	or die 'Configuration error: Missing or invalid value for parameter "RingDiameter'.$ringDiameter.'"';

#
# Prepare the reconstruction result folder, and change dir into it.
# From this point, we are working inside the result folder.
#
$reconsFolder = "$reconsRoot/recons$$";
mkdir $reconsFolder or die "Could not create folder $reconsFolder";
system('/bin/mv', $reconsParameterFile, $reconsFolder);
chdir $reconsFolder;

my $dataRoot;	# only used if DataFrom is set to SinglesFromLocal
my @daqFiles;

0 == system('/bin/cp', "$paramDir/lmf_header.db", '.')
	or die 'Could not copy file lmf_header.db';

if ($reconsConfig{'DataFrom'} eq 'SinglesFromPrepros')
{
	# we load singles from the preprocessors. We need to mount the samba shares.
	if ($reconsConfig{'MasterOnLocal'})
	{
		$dataRoot = VerifyFolderParam('DataRoot');
		$daqFiles[0] = "$dataRoot$dataSet".'_enc.txt';
	} else
	{
		$shares[0] = $reconsConfig{'MountPointMaster'};
		MountSMBShare($shares[0]);
		$daqFiles[0] = $shares[0].$dataSet.'_enc.txt';
	}
	for (my $p = 1; $p <= $numPrepro; $p++)
	{
		$shares[$p] = $reconsConfig{"MountPointPrepro$p"};
		MountSMBShare($shares[$p]);
		$daqFiles[$p] = $shares[$p].$dataSet."_".$PreproID[$p].".bin";
	}
}
elsif ($reconsConfig{'DataFrom'} eq 'SinglesFromLocal')
{
	# we load singles from the local disk. We get the data root, then we
	# check if the data files exist.
	$dataRoot = VerifyFolderParam('DataRoot');
	$daqFiles[0] = $dataRoot.$dataSet.'_enc.txt';
	for (my $p = 1; $p <= $numPrepro; $p++)
	{
		$daqFiles[$p] = $dataRoot.$dataSet."_".$PreproID[$p].".bin";
	}
}
else
{
	die 'Configuration error: missing or invalid value of parameter DataFrom';
}

# check if all the data files exist, and create symlinks to them in our recons folder
-r $daqFiles[0] or die "File $daqFiles[0] does not exist";
symlink($daqFiles[0],'encTime.txt')
  or die "Could not create symlink for $daqFiles[0]";
for (my $p = 1; $p <= $numPrepro; $p++)
{
	-r $daqFiles[$p] or die "File $daqFiles[$p] does not exist";
	symlink($daqFiles[$p],"file$p.bin")
	  or die "Could not create symlink for $daqFiles[$p]";
}

my $daqDuration = GetDaqTime('encTime.txt');
#$daqDuration = 1;

foreach my $fov (7, 9)
{
	my $FOV = $fov.'_'.$ringDiameter;
	# sort coincidences:
	UpdateStatus('Running', ("Coincidence sorting for FOV $fov"));
	CoincSorter('Coincidences', $coincWindow, $fov, 0, $daqDuration);

	# rebin the data
	UpdateStatus('Running', "Rebinning coincidences for FOV $fov");
	Rebinner($paramDir, $FOV, $ringDiamCM, 'Coincidences', 'proj_norm');
	# Note: the rebinner always appends "f1g1d0b0" to the sinogram name.
	my $sinoResult = 'proj_norm_f1g1d0b0';

	#
	# Reconstruction with bcktest
	#
	UpdateStatus('Running', ("Building sensitivity image for FOV $fov"));
	my $imageName = "norm_fov$FOV";
	BckTest($imageName, $paramDir, "$sinoResult.hs", $FOV);

	#
	# Finally, move the image to the output location
	#
	MoveImage($imageName, $paramDir);

} # end of fov loop

UpdateStatus('Running', 'Cleanup');

chdir('..');
system('/bin/rm', '-rf', $reconsFolder);
$reconsFolder = '';

UpdateStatus('Complete', '');

} # end of main subroutine

sub INT_Handler
{
	$SIG{INT} = 'IGNORE';
	kill("INT", -getpgrp());
	die "INT signal received";
}

sub TERM_Handler
{
	$SIG{TERM} = 'IGNORE';
	kill("TERM", -getpgrp());
	die "TERM signal received";
}


#
# Here, the real Perl main code begins
#
my %options;
getopts('p:s:', \%options);
die 'Missing required option -p' unless exists $options{'p'};
die 'Missing required value for option -p' unless defined $options{'p'};
$reconsParameterFile = $options{'p'};
if (exists $options{'s'})
{
	die 'Missing required value option -s' unless defined $options{'s'};
	$statusDir = $options{'s'};
}

$SIG{TERM} = \&TERM_Handler;
$SIG{INT} = \&INT_Handler;
my $result = 0;
eval { main(); };
if ($@)
{
	UpdateStatus('Error', $@);

	# Cleanup folder after error. This avoids pileup of reconstruction
	# folders, but makes debugging harder. You might want to comment
	# this line when debugging code!
	system('/bin/rm', '-rf', $reconsFolder) if $reconsFolder;

	$result = 1; #non-zero result code means error
}

#unmount the shares, if they had been mounted:
for my $s (@shares)
{
	UnmountSMBShare($s);
}

exit $result;
