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
my $reconsParameterFile = '';
my $dataSet = '';
my $transmissionDataSet = '';
my $imageName = '';
# The name of the directory for status output. If not specified, STDOUT will be used.
my $statusDir = '';
my $binDir;
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
#	system('/bin/umount', $unc) if $unc;
	my $output = qx(/bin/umount $unc 2>&1);
	print $output if $output;
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
		open my $out, '>', $statusFileName or die "Cannot write $statusFileName: $!";
		print $out "Type = Reconstruction\n" or die "Cannot write into $statusFileName: $!";
		print $out "Data = $dataSet\n";
		print $out "Output = $imageName\n";
		print $out "Status = $status\n";
		print $out "@msg\n";
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
	unlink $name . ".hs", unlink $name . ".s"
	  or die "Error deleting sinogram $name: $!";
}

# Calculate with 2 sinograms
# This calls an external program in the list mode utilities folder in order
# to run some sinogram calculation. That program requires a parameter file,
# which is created here as well.
#
# Arguments:
# - operation, can be "add_sino", "subtract_sino" or multiply_sino
# - 1st sinogram
# - 2nd sinogram
# - resulting sinogram name
#
sub CalcSinograms
{
	my ($operation, $sino1, $sino2, $sinoresult) = @_;
	my $outfile = "calcsino.par";
	$sino1 = $sino1 . ".hs";
	$sino2 = $sino2 . ".hs";

	open my $out, '>', $outfile or die "Cannot write $outfile: $!";
{
	print $out <<END_PAR
$operation parameters:=
sino1 file := $sino1
sino2 file := $sino2
output filename prefix := $sinoresult
end:=
END_PAR
}
	close $out;

	system("$binDir/$operation",$outfile) == 0
	  or die "Error calling $operation";

	# clean up:
	unlink $outfile;
}

# Run a dead time correction on a sinogram
# This calls an external program in the list mode utilities folder in order
# to run the dead time correction. That program requires a parameter file,
# which is created here as well.
#
# Arguments:
# - input sinogram
# - length of DAQ, in minutes
# - resulting sinogram name
# - array with dead time correction parameters
#
sub DeadTimeCorrection
{
	my ($input, $daqTime, $output, @dtParams) = @_;
	my $paramfile = "deadtimecorr.par";
	$daqTime = $daqTime * 60;
	$input = $input . ".hs";

	open my $out, '>', $paramfile or die "Cannot write $paramfile: $!";
{
	print $out <<END_PAR
deadtimecorr parameters:=
input file := $input
DAQ duration in seconds := $daqTime
output filename prefix := $output
intrinsic true rate := $dtParams[0]
P2 := $dtParams[1]
P3 := $dtParams[2]
P4 := $dtParams[3]
P5 := $dtParams[4]
P6 := $dtParams[5]
P7 := $dtParams[6]
P8 := $dtParams[7]
end:=
END_PAR
}
	close $out;

	system("$binDir/deadtimecorr $paramfile > dtcorr.log 2>&1") == 0
	  or die "Error calling deadtimecorr";

	# clean up:
	unlink $paramfile;
}

# Run an operation on a sinogram
# This calls an external program in the list mode utilities folder in order
# to run the operation. That program requires a parameter file,
# which is created here as well.
#
# Arguments:
# - input sinogram header
# - resulting sinogram header name (without suffix)
#
sub ProcessSinogram
{
	my ($operation, $input, $output) = @_;
	my $outfile = "sino.par";
	$input = $input . ".hs";

	open my $out, '>', $outfile or die "Cannot write $outfile: $!";
{
	print $out <<END_PAR
$operation parameters:=
input file := $input
output filename prefix := $output
end:=
END_PAR
}
	close $out;

	system("$binDir/$operation",$outfile) == 0
	  or die "Error calling $operation";

	# clean up:
	unlink $outfile;
}

sub RatioAttenSinogram
{
	my ($blank, $animal, $output) = @_;
	my $operation = "ratio_atten_sino";
	my $outfile = "sino.par";
	$blank = $blank . ".hs";
	$animal = $animal . ".hs";

	open my $out, '>', $outfile or die "Cannot write $outfile: $!";
{
	print $out <<END_PAR
$operation parameters:=
blank file := $blank
animal file := $animal
output filename prefix := $output
end:=
END_PAR
}
	close $out;

	system("$binDir/$operation",$outfile) == 0
	  or die "Error calling $operation";

	# clean up:
	unlink $outfile;
}


sub CoincSorter
{
	my ($output, $coincWin, $fov, $firstReset, $lastReset, $randomsCorr) = @_;
	if ($reconsConfig{"UseOldSorter"})
	{
		system("$binDir/CorrectionOffset ./ $output.ccs $coincWin $numPrepro $ELow $EHigh $fov $firstReset $lastReset 0 >coinc.log 2>&1") == 0
			or die "Error calling coincidence sorter";
		unlink "Randoms.ccs";
	} else
	{
		my $opts = "-c=$coincWin -emin=$ELow -emax=$EHigh -f=$fov";
		$opts .= "-rfrom=".($firstReset+1)." -rto=$lastReset";
		$opts .= " -n=" .  4*$numPrepro;
		$opts .= " -o=$output";
		$opts .= " -A=$angularOffset";
		if ($randomsCorr)
		{
			$opts .= " -t=" . 4*$coincWin;
		}
		my $files = "-a=encTime.txt";
		for (my $i = 1; $i <= $numPrepro; ++$i)
		{
			$files = $files . " file$i.bin";
		}
# old code:
#		system("$binDir/sorter2 $opts $files >coinc.log 2>&1") == 0
#			or die "Error calling coincidence sorter";
		open(COINC_STATUS, "$binDir/sorter2 $opts $files 2>&1 | /usr/bin/tee coinc.log |")
			or die "Error calling coincidence sorter";
		while (my $line = <COINC_STATUS>)
		{
			UpdateStatus('Running', "Coincidence sorting: $line");
		}
		close COINC_STATUS;
	}
}

sub CoincSorterTrans
{
	my ($output, $coincWin, $fov, $lastReset, $paramsFolder) = @_;
	if ($reconsConfig{"UseOldSorter"})
	{
		system("$binDir/CorrectionOffsetTrans ./ $output.ccs $coincWin $numPrepro $ELow $EHigh $fov 0 $lastReset 0 >coinc.log 2>&1") == 0
			or die "Error calling coincidence sorter";
		unlink "Randoms.ccs";
	} else
	{
		my $opts = "-s -c=$coincWin -emin=$ELow -emax=$EHigh";
		$opts .= " -m=$paramsFolder/matrix_trans_fov$fov";
		$opts .= " -n=" .  4*$numPrepro;
		$opts .= " -o=$output";
		$opts .= " -A=$angularOffset";
		my $files = "-a=encTime.txt";
		for (my $i = 1; $i <= $numPrepro; ++$i)
		{
			$files = $files . " file$i.bin";
		}
# old code:
#		system("$binDir/sorter2 $opts $files >coinc.log 2>&1") == 0
#			or die "Error calling coincidence sorter";
		open(COINC_STATUS, "$binDir/sorter2 $opts $files 2>&1 |")
			or die "Error calling coincidence sorter";
		open(my $log, '>', "coinc.log");
		$log->autoflush(1);
		while (my $line = <COINC_STATUS>)
		{
			print $log, $line;
			UpdateStatus('Running', "Coincidence sorting for transmission data: $line");
		}
		close $log;
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
	$rewriteInfo{"inner ring diameter (cm)"} = $ringDiam;
	RewriteParFile("$paramFolder/$outfile", $outfile, ":=", %rewriteInfo);

	# rewrite the lm_to_projdata.par file for the current settings:
	$outfile = "lm_to_projdata.par";
	undef %rewriteInfo;
	$rewriteInfo{"input file"} = $coincData;
	$rewriteInfo{"output filename prefix"} = $projData;
	RewriteParFile("$paramFolder/$outfile", $outfile, ":=", %rewriteInfo);

	# rewrite the Coincidence.cch file for the current settings:
	$outfile = "$coincData.cch";
	undef %rewriteInfo;
	$rewriteInfo{"ring diameter"} = "$ringDiam cm";
	if (!$reconsConfig{"ModuleAxialPitch"})
	{
		$rewriteInfo{"module axial pitch"} = "2.76 cm";
	} else
	{
		$rewriteInfo{"module axial pitch"} = $reconsConfig{"ModuleAxialPitch"}." cm";
	}
	$rewriteInfo{"scan file name"} = $coincData;
	RewriteParFile("$paramFolder/Coincidences.cch", $outfile, ":", %rewriteInfo);

	system('/usr/bin/touch', 'seg_31.s');
	system("$binDir/lm_to_projdata lm_to_projdata.par > rebinner.log 2>&1") == 0
		or die "Error calling rebinner";
}

# Run OSMAPOSL on a sinogram and create an image
#
# Arguments:
# - name of the input parameter file (used as a template)
# - name of the sinogram (no suffix)
# - name of the sensitivity image (no suffix)
# - name of the final image (no suffix)
#
sub OSMAPOSL
{
	my ($infile, $sinogram, $sensiImage, $image) = @_;
	my $outfile = "osem.par";
	my %rewriteInfo;
	$rewriteInfo{"input file"} = "$sinogram.hs";
	$rewriteInfo{"output filename prefix"} = $image;
	$rewriteInfo{"sensitivity image"} = "$sensiImage.hv";

	RewriteParFile($infile, $outfile, ":=", %rewriteInfo);

	system("$binDir/OSMAPOSL $outfile > osmaposl.log 2>&1") == 0
	  or die "Error calling OSMAPOSL";

	# clean up:
	unlink $outfile;
}

sub MoveImage
{
	my ($name, $destDir, $sinogram) = @_;

	# Test if the file already exists. If so, then append "_1" to the name
	# and try again with increasing numbers until whe find a free file name.
	my $name1 = "$name";
	my $dest1 = "$destDir/$name";
	my $count = 0;
	while (-e "$dest1.hv")
	{
		$count++;
		$name1 = "$name"."_$count";
		$dest1 = "$destDir/$name1";
	}
	if (0 == $count)
	{
		0 == system('/bin/mv', "$name.hv", "$dest1.hv")
			or die "Cannot move $name.hv to $dest1.hv:\n$!";
	} else
	{
		my %rewriteInfo;
		$rewriteInfo{"name of data file"} = $name."_$count.v";
		RewriteParFile("$name.hv", "$dest1.hv", ":=", %rewriteInfo);
	}
	0 == system('/bin/mv', "$name.v", "$dest1.v")
		or die "Cannot move $name.v to $dest1.v:\n$!";
	if ($reconsParams{'Save_CoincLog'})
	{
		0 == system('/bin/mv', 'coinc.log', $dest1.'_coinc.log')
			or die "Cannot move coinc.log to $destDir:\n$!";
	}
	if ($reconsParams{'Save_CoincData'})
	{
		0 == system('/bin/mv', 'Coincidences.ccs', $dest1.'.ccs')
			or die "Cannot move Coincidences.ccs to $destDir:\n$!";
		my %rewriteInfo;
		$rewriteInfo{"scan file name"} = $name1;
		RewriteParFile("Coincidences.cch", $dest1.".cch", ":", %rewriteInfo);
	}
	if ($reconsParams{'Save_Sinogram'})
	{
		0 == system('/bin/cp', "$sinogram.s", $dest1.'.s')
			or die "Cannot move $sinogram to $destDir:\n$!";
		my %rewriteInfo;
		$rewriteInfo{"name of data file"} = $name1.".s";
		RewriteParFile($sinogram.".hs", $dest1.".hs", ":=", %rewriteInfo);
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
  param_file is the full path name of a reconstruction parameter file
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

$binDir = VerifyFolderParam("BinDir");
my $paramDir = VerifyFolderParam("ParamDir");
my $reconsRoot = VerifyFolderParam("StatusDir");
my $imageDir = VerifyFolderParam("ImageDir");

my $coincSorter = '';
my $coincSorterTrans = '';
if ($reconsConfig{'UseoldSorter'})
{
	$coincSorter = VerifyProgram("CorrectionOffset");
	$coincSorterTrans = VerifyProgram("CorrectionOffsetTrans");
}
VerifyProgram("lm_to_projdata");
VerifyProgram("OSMAPOSL");

$ELow = $reconsConfig{"EnergyLow"};
($ELow >= 0) or die "Configuration error: Invalid lower energy limit";
$EHigh = $reconsConfig{"EnergyHigh"};
($EHigh >= 0 && $EHigh < 1250) or die "Configuration error: Invalid upper energy limit";
($ELow < $EHigh) or die  "Configuration error: lower energy limit is above upper energy limit";

$coincWindow = $reconsConfig{"CoincidenceWindow"};
($coincWindow >= 5 && $coincWindow <= 50) or die "Configuration error: Invalid coincidence window";

$angularOffset = $reconsConfig{"AngularOffset"};
($angularOffset >= -180 && $angularOffset <= 180) or die "Configuration error: Invalid angular offset";

UpdateStatus("Running", "Preparing for reconstruction");

$dataSet = $reconsParams{"DataSet"};
$dataSet && length($dataSet) or die 'Missing or empty parameter "DataSet"';
if (substr($dataSet, 0, 1) ne '/')
{
	$dataSet = '/'.$dataSet;
}
$imageName = $reconsParams{"ImageName"};
$imageName && length($imageName) or die 'Missing or empty parameter "ImageName"';

my $sensiFactor;
my $applyAttenuationCorrection = 0;
my $quantificationApplied = $reconsParams{"Quantifications"};

my $FOV = $reconsParams{'FOV'};
# Extract the FOV value from the FOV name:
my $nfov;
if ($FOV eq '7_Small' || $FOV eq '7_Large')
{
	$nfov = 7;
} elsif ($FOV eq '9_Small' || $FOV eq '9_Large')
{
	$nfov = 9;
} else
{
	die 'Parameter error: Invalid value for parameter "FOV"';
}
# Extract the "Small" or "Large" from the FOV name:
my $ringDiameter = substr($FOV, 2, 5);
my $ringDiamCM = $reconsConfig{"RingDiameter$ringDiameter"};
$ringDiamCM
	or die 'Configuration error: Missing or invalid value for parameter "RingDiameter'.$ringDiameter.'"';

if ($FOV ne '7_Small')
{
	if ($quantificationApplied)
	{
		$applyAttenuationCorrection = $reconsParams{"AttenuationCorrection"};
	}
}
$sensiFactor = $reconsConfig{"Sensitivity_FOV$FOV"};
if (!$sensiFactor)
{
	die "Configuration error: Invalid sensitivity factor for selected FOV value and ring diameter";
}
my $normalImage = "$paramDir/norm_fov$FOV";
-r "$normalImage.v" && -r "$normalImage.hv"
	or die "Cannot read normalization image $normalImage";

my @dtParams;
$dtParams[0] = $reconsConfig{"IntrinsicTrueRate"};
for (my $i = 2; $i <= 8; $i++)
{
	$dtParams[$i-1] = $reconsConfig{"P$i"};
}
if ($nfov == 7)
{
	# intrinsic rate in config file is for FOV 9, we recalculate it for FOV 7:
	$dtParams[0] = $dtParams[0] * 7.0 / 9.0;
}

if ($applyAttenuationCorrection)
{
	$transmissionDataSet = $reconsParams{"TransmissionDataSet"};
	$transmissionDataSet && length($transmissionDataSet)
		or die 'Missing or empty parameter "TransmissionDataSet"';
}

my $maskCylinder = $reconsParams{"MaskCylinder"};

my $frameCount = $reconsParams{"FrameCount"};
if (!$frameCount)
{
	$frameCount = 1;
} elsif ($frameCount < 0 || $frameCount > 60)
{
	die  "Parameter error: Invalid frame count";
}
my $frameLength = $reconsParams{"FrameLength"};
my $frameOffset = $reconsParams{"FirstFrameOffset"};

my $buildEquilibriumImage = $reconsParams{"EquilibriumImage"};
if ($buildEquilibriumImage && $frameCount < 3)
{
	# we cannot build an equilibrium image for less than 3 frames!
	$buildEquilibriumImage = 0;
}

#
# Prepare the reconstruction result folder, and change dir into it.
# From this point, we are working inside the result folder.
#
$reconsFolder = "$reconsRoot/recons$$";
mkdir $reconsFolder or die "Could not create folder $reconsFolder";
system('/bin/mv', $reconsParameterFile, $reconsFolder);
$reconsParameterFile = '';
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
	$daqFiles[0] = "$dataRoot$dataSet".'_enc.txt';
	for (my $p = 1; $p <= $numPrepro; $p++)
	{
		$daqFiles[$p] = "$dataRoot$dataSet"."_".$PreproID[$p].".bin";
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
if (!$frameLength)
{
	$frameLength = $daqDuration;
	$frameCount = 1;
}

#
# If attenuation correction is applied, calculate the transmission data.
# This is identical for all frames.
#
if ($applyAttenuationCorrection)
{
	mkdir 'transmission' or die "Could not create folder $reconsFolder/transmission";
	chdir 'transmission';

	if ($reconsConfig{'DataFrom'} eq 'SinglesFromPrepros')
	{
		$daqFiles[0] = $shares[0].$transmissionDataSet.'_enc.txt';
		for (my $p = 1; $p <= $numPrepro; $p++)
		{
			$daqFiles[$p] = $shares[$p].$transmissionDataSet."_".$PreproID[$p].".bin";
		}
	}
	elsif ($reconsConfig{'DataFrom'} eq 'SinglesFromLocal')
	{
		$daqFiles[0] = $dataRoot/$transmissionDataSet.'_enc.txt';
		for (my $p = 1; $p <= $numPrepro; $p++)
		{
			$daqFiles[$p] = $dataRoot/$transmissionDataSet."_".$PreproID[$p].".bin";
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

	my $transDaqTime = GetDaqTime('encTime.txt');

	# Sort coincidences
	# Note that this uses a different version of the coinc sorter, because
	# it uses only the coincidences for the two modules where the transmission
	# sources are installed!.
	UpdateStatus('Running', 'Coincidence sorting for transmission data');
	CoincSorterTrans('Coincidences', $coincWindow, $nfov, $transDaqTime, $paramDir);

	# rebin the data
	UpdateStatus('Running', 'Rebinning coincidences for transmission data');
	Rebinner($paramDir, $FOV, $ringDiamCM, 'Coincidences', 'projdata');

	UpdateStatus('Running', 'Applying corrections on transmission data');

	# Smooth transmission
	ProcessSinogram('smooth_sino', 'projdata_f1g1d0b0', 'sino_E_smoothed');
	DeleteSinogram('projdata_f1g1d0b0');

	# Ratio B/E
	RatioAttenSinogram('sino_B_smoothed', 'sino_E_smoothed', 'sino_A_smoothed');
	DeleteSinogram('sino_E_smoothed');

	# Apply emission mask
	CalcSinograms('multiply_sino', 'sino_A_smoothed', 'mask_sino_emission', 'sino_A');
	DeleteSinogram('sino_A_smoothed');

	system('/bin/mv', 'sino_A.hs', 'sino_A.s', '..') or die 'Cannot move sino_A';

	chdir('..');
	system('/bin/rm', '-rf', 'transmission');
} # end of transmission calulations


#
# Frame loop
#
my $resetLast = $frameOffset + $frameCount * $frameLength;
if ($resetLast > $daqDuration)
{
	$resetLast = $daqDuration;
}

my $resetFrom = $frameOffset;
my $resetTo = $resetFrom + $frameLength;
my $frameIndex = 1;
my @saveSinos;	# only used for equilibrium image

while ($frameIndex <= $frameCount)
{
	my $frameMsg = '';
	if ($frameCount > 1)
	{
		$frameMsg = " for frame $frameIndex of $frameCount";
	}

	my $frameName = ($frameCount > 1) ? $imageName.$resetFrom.'to'.$resetTo : $imageName;

	# sort coincidences:
	UpdateStatus('Running', 'Coincidence sorting'.$frameMsg);
	CoincSorter('Coincidences', $coincWindow, $nfov, $resetFrom, $resetTo, $quantificationApplied);

	# rebin the data
	UpdateStatus('Running', 'Rebinning coincidences'.$frameMsg);
	Rebinner($paramDir, $FOV, $ringDiamCM, 'Coincidences', 'projdata');
	# Note: the rebinner always adds "f1g1d0b0" to the name of the sinogram.
	my $sinoResult = 'projdata_f1g1d0b0';

	# If quantification is used, we have to apply the corrections:
	if ($quantificationApplied)
	{
		#
		# Randoms Correction
		#
		if ($reconsConfig{'UseOldSorter'})
		{
			# sort coincidences with double coinc window
			UpdateStatus('Running', 'Coincidence sorting on randoms'.$frameMsg);
			CoincSorter($coincSorter, 'Coincidences', 2*$coincWindow, $nfov, $resetFrom, $resetTo);
			# and rebin that again
			UpdateStatus('Running', 'Rebinning random coincidences'.$frameMsg);
			Rebinner($paramDir, $FOV, $ringDiamCM, 'Coincidences', 'projdata2');
			UpdateStatus('Running', 'Random correction'.$frameMsg);
			# Subtract the 2 sinograms to get randoms:
			CalcSinograms('subtract_sino', 'sino1', 'projdata2_f1g1d0b0', 'sino_Randoms');
			DeleteSinogram('projdata2_f1g1d0b0');
			# Subtract the randoms from the original sinogram
			CalcSinograms('subtract_sino', 'projdata_f1g1d0b0', 'sino_Randoms', 'sino_E_corrR');
			DeleteSinogram('sino_Randoms');
		} else
		{
			# and rebin the random coincidences:
			UpdateStatus('Running', 'Rebinning random coincidences'.$frameMsg);
			Rebinner($paramDir, $FOV, $ringDiamCM, 'Coincidences_randoms', 'sino_Randoms');
			# Subtract the randoms from the original sinogram
			CalcSinograms('subtract_sino', 'projdata_f1g1d0b0', 'sino_Randoms_f1g1d0b0', 'sino_E_corrR');
			DeleteSinogram('sino_Randoms_f1g1d0b0');
		}
		DeleteSinogram('projdata_f1g1d0b0');

		#
		# Dead Time Correction
		#
		UpdateStatus('Running', 'Dead time correction'.$frameMsg);
		DeadTimeCorrection('sino_E_corrR', $frameLength, 'sino_E_corrRDT', @dtParams);
		DeleteSinogram('sino_E_corrR');

		#
		# Scatter correction
		#
		UpdateStatus('Running', 'Scatter correction'.$frameMsg);
		ProcessSinogram('scatters_by_gaussian_fit', 'sino_E_corrRDT', 'sino_E_corrRDTS');
		DeleteSinogram('sino_E_corrRDT');


		if ($applyAttenuationCorrection)
		{
			#
			# Attenuation Correction
			#
			UpdateStatus('Running', 'Attenuation correction'.$frameMsg);
			CalcSinograms('multiply_sino', 'sino_A', 'sino_E_corrRDTS', 'sino_E_corrRDTSA');
			DeleteSinogram('sino_E_corrRDTS');
			$sinoResult = 'sino_E_corrRDTSA';
		} else
		{
			$sinoResult = 'sino_E_corrRDTS';
		}

	} # end of corrections on the sinogram

	#
	# Reconstruction with OSMAPOSL
	#
	UpdateStatus('Running', 'OSMAPOSL reconstruction'.$frameMsg);
	OSMAPOSL("$paramDir/osem.par", $sinoResult, $normalImage, $frameName);
	# Note: OSMAPOSL always appends "_20" to the filename, at least with current parameters

	#
	# Write the quantification factor into the file:
	# In addition, we remove the stupid "_20" suffix from the name
	#
	my %rewriteQuant;
	$rewriteQuant{'quantification units'} = ($quantificationApplied) ? $sensiFactor / ($frameLength * 60) : 1;
	$rewriteQuant{'name of data file'} = $frameName.'.v';
	RewriteParFile($frameName.'_20.hv', $frameName.'.hv', ':=', %rewriteQuant);
	rename($frameName.'_20.v', $frameName.'.v');

	if ($maskCylinder)
	{
		my $fovDiam = (($nfov == 7) ? 77.5 : 93.5) * $ringDiamCM / 13.56;
		# Mask areas outside the FOV cylinder
		system("$binDir/mask_cyl", "-R=$fovDiam", $frameName.'hv')
			or die "Error calling masking operation mask_cyl";
	}

	#
	# Finally, move the image to the output location
	#
	MoveImage($frameName, $imageDir, $sinoResult);

	#
	# Save sinograms of last 3 frames for equilibrium image,
	# delete all other sinograms.
	#
	if ($buildEquilibriumImage)
	{
		if ($frameIndex <= 3)
		{
			$saveSinos[$frameIndex] = $sinoResult;
		} else
		{
			DeleteSinogram($saveSinos[1]);
			$saveSinos[1] = $saveSinos[2];
			$saveSinos[2] = $saveSinos[3];
			$saveSinos[3] = $sinoResult;
		}
	} else
	{
		DeleteSinogram($sinoResult);
	}

	$resetFrom = $resetFrom + $frameLength;
	$resetTo = $resetTo + $frameLength;
	$frameIndex++;
	last if $resetFrom >= $daqDuration;
	if ($resetTo > $daqDuration)
	{
		$resetTo = $daqDuration;
	}
} # end of frame loop

#
# Calculate the equilibrium image if needed
#
if ($buildEquilibriumImage)
{
	# Add the 3 last sinograms
	UpdateStatus('Running', 'Adding sinograms for equilibrium image');
	CalcSinograms('add_sino', $saveSinos[1], $saveSinos[2], 'sino_sum1');
	CalcSinograms('add_sino', 'sino_sum2', $saveSinos[3], 'sino_sum2');
	DeleteSinogram($saveSinos[1]);
	DeleteSinogram($saveSinos[2]);
	DeleteSinogram($saveSinos[3]);
	DeleteSinogram('sino_sum1');
	# reconstruct the sum:
	UpdateStatus('Running', 'OSMAPOSL reconstruction for equilibrium image');
	OSMAPOSL("$paramDir/osem.par", 'sino_sum2', $normalImage, $imageName.'_eq');
	DeleteSinogram('sino_sum2');
	# and requantify it
	my %rewriteQuant;
	$rewriteQuant{'quantification units'} = ($quantificationApplied) ? $sensiFactor / ($frameLength * 60) : 1;
	$rewriteQuant{'name of data file'} = $imageName.'_eq.v';
	RewriteParFile($imageName.'_eq__20.hv', $imageName.'_eq.hv', ':=', %rewriteQuant);
	rename($imageName.'_eq_20.hv', $imageName.'_eq.hv');
	# move the image to the output location
	MoveImage($imageName.'_eq', $imageDir);

}

UpdateStatus('Running', 'Cleanup');

chdir('..');
if (!$reconsConfig{'Debug'})
{
	system('/bin/rm', '-rf', $reconsFolder);
}
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
	if (!$reconsConfig{'Debug'})
	{
		system('/bin/rm', '-rf', $reconsFolder) if $reconsFolder;
	}

	$result = 1; #non-zero result code means error
}

#unmount the shares, if they had been mounted:
if ($shares[1])
{
	# Sometimes, the shares are still busy. This happens when
	# we have been terminated by a signal. We just wait a moment,
	# to give the child processes time to terminate.
	sleep(2);
}
for (my $p = 0; $p <= $numPrepro; $p++)
{
	if ($shares[$p])
	{
		print "Unmounting $shares[$p]\n";
		UnmountSMBShare($shares[$p]);
	}
}

exit $result;
