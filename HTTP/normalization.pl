#!/usr/bin/perl

use encoding 'utf8';
use open ':encoding(utf8)'; # input/output default encoding will be UTF-8
use warnings;
use strict;
use CGI qw(:standard -no_xhtml);
use Encode;

my $configFile = '/etc/clearpet.conf';
my %reconsConfig;

my $statusDir;
my $binDir;
my $dataRoot;
my $dataFrom;

my $JSCRIPT=<<END;
function verifyForm()
{
//    alert("verifyForm called");
  if (document.my_form.DataSet.value == "")
  {
    alert("Please select a data set!");
    document.my_form.DataSet.focus();
    return false;
  }
  if (document.my_form.RingDiameter.value == "")
  {
    alert("Please select a gantry diameter!");
    document.my_form.DataSet.focus();
    return false;
  }
  return true;
}

END

sub MountSMBShare
{
	my $unc = shift;
	my $output = qx(mount $unc 2>&1);
	return $output ? "Cannot mount $unc:\n$output" : "";
}

sub UnmountSMBShare
{
	my $unc = shift;
	system('umount', $unc);
	# Note: We do not check error codes here, because it does not really
	# matter for us if an umount does not work. Actually, we might call
	# this function for UNC names for which a mount failed.
}

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
  
	open my $in, '<', $infile or return 0;
	while (<$in>)
	{
	    chomp;
		next unless $_;
		next if /^#/;
		my ($name, $value) = split(/=/, $_, 2);
		$name = trim($name);
		$value = trim($value);
		$reconsConfig{$name} = $value;
	 }
	 return 1;
}
	
sub CollectDataSets
{
	my $dir = shift;
	my @dataSets;
	@dataSets = `find -L $dir -name *_enc.txt`; 
	for my $i (@dataSets)
	{
		chomp $i;
		$i =~ s/^$dir//;
		$i =~ s/_enc\.txt$//;
	}
	return @dataSets;
}

binmode(STDOUT, ':utf8');

my $msg;
if (ReadParameters($configFile))
{
	$dataFrom = $reconsConfig{'DataFrom'};
	if ($dataFrom eq 'SinglesFromLocal')
	{
		$dataRoot = $reconsConfig{'DataRoot'};
		if (!$dataRoot)
		{
			$msg = 'Missing value for configuration key "DataRoot"';
		} elsif (! -d $dataRoot)
		{
			$msg = "Directory $dataRoot does not exist";
		}
	} elsif ($dataFrom eq 'SinglesFromPrepros')
	{
		$dataRoot = $reconsConfig{'MountPointMasterWWW'};
		if (!$dataRoot)
		{
			$msg = 'Missing value for configuration key "MountPointMasterWWW"';
		} elsif (! -d $dataRoot)
		{
			$msg = "Directory $dataRoot does not exist";
		}
	} else
	{
		$msg = 'Missing or invalid value for configuration key "DataFrom"';
	}

	$statusDir = $reconsConfig{'StatusDir'};
	if (!$statusDir)
	{
		$msg = 'Missing value for configuration key "StatusDir"';
	} elsif (! -d $statusDir)
	{
		$msg = "Directory $statusDir does not exist";
	}

	$binDir = $reconsConfig{'BinDir'};
	if (!$binDir)
	{
		$msg = 'Missing value for configuration key "BinDir"';
	} elsif (! -d $binDir)
	{
		$msg = "Directory $binDir does not exist";
	}
} else
{
	$msg = "Error reading configuration file $configFile";
}

if ($msg)
{
	print header(-charset=>'UTF-8'),
	start_html(-title=>'ClearPET Configuration Error'),
	h2('ClearPET Configuration Error');
	print escapeHTML(Encode::decode_utf8($msg)), p, end_html;
	exit;
}


if (param())
{
# For sure, we validate the parameters here (again), because the browser
# might have JaveScript disabled and cannot validate them.
	if (!param('DataSet'))
  	{
   		$msg .= 'Please select a data set!<BR>';
	}

	if ($msg)
	{
		print header(-charset=>'UTF-8'),
		start_html(-title=>'Parameter Validation Error'),
		h2('Parameter validation has found these errors:');
		print $msg, p;
		print 'Go back to the previous page and correct the errors.', p;
		print 'Here is a list of the parameters you have submitted:<BR>', Encode::decode_utf8(Dump);
		print end_html;
		exit;
	}

	print header(-charset=>'UTF-8'),
	start_html(-title=>'Starting ClearPET Normalization'),
	h2('Starting ClearPET Normalization');
	my $paramFile = $statusDir."/normalize$$";
	if (open my $out, '>', $paramFile)
	{
		my @names = param;
		foreach my $name (@names)
		{
			print $out "$name = ", Encode::decode_utf8(param($name)), "\n";
		}
		close $out;

		print "The normalization job has been scheduled.\n", p;
		print 'Go to the <a href="status.html" target="_top">Status page</a> to monitor the normalization progress.', p;

		my $pidfilename = '/var/run/recons_scheduler.pid';
		my $scheduler_ok = -r $pidfilename;
		if ($scheduler_ok)
		{
			open my $in, '<', $pidfilename;
			my $pid = <$in>;
			chomp $pid;
			if ($pid)
			{
				$scheduler_ok = `/bin/ps -p $pid --no-header`;
			} else
			{
				$scheduler_ok = 0;
			}
		}
		if (!$scheduler_ok)
		{
			print b('Warning: '), 'The reconstruction scheduler is not running. The normalization will not be processed until the scheduler is started.';
		}
	} else
	{
		$msg = "Could not write parameter file $paramFile";
	}
	print escapeHTML(Encode::decode_utf8($msg)) if $msg;
	print end_html;
} else
{
	print header(-charset=>'UTF-8'),
	start_html(-title=>'ClearPET Normalization Parameters',
		   -encoding=>'UTF-8', -script=>$JSCRIPT),
	h2('ClearPET Normalization Parameters'), "\n",
	start_form(-name=>'my_form', -onSubmit=>'return verifyForm()');
 
	my @dataSets;
	if ($dataFrom eq 'SinglesFromLocal')
	{
		@dataSets = CollectDataSets($dataRoot);
	} else
	{
		my $msg = MountSMBShare($dataRoot);
		if ($msg && length($msg))
		{
			print '<font color=#FF0000>', b(escapeHTML($msg)), '</font>', p;
		} else
		{
			@dataSets = CollectDataSets($dataRoot);
			UnmountSMBShare($dataRoot);
		}
	}

	print <<END;
<b>Warning! Read this:</b>
<ul>
  <li>The data set for normalization must be a measurement of a full cylinder.</li>
  <li>The normalization image will be installed immediately after completing the calculations.</li>
  <li>The old normalization image will be lost.</li>
</ul>
<hr>
<table border="0" cellpading="0" cellspacing="4">
 <tr>
END
	print '  <td align="left">Loading raw data from ';
	if ($dataFrom eq 'SinglesFromLocal')
	{
		print 'local hard disk.</td><td>Be sure to copy the data there before reconstruction!</td>';
	} else
	{
		print 'preprocessors.</td><td>Be sure that the preprocessors are online during reconstruction!</td>';
	}
	print <<END;
 </tr>
 <tr>
  <td align="left">Data set to be reconstructed:</td>
  <td><select name="DataSet">
END
	print '   <option value="">', escapeHTML('<< Select >>'), "</option>\n";
	foreach my $name (@dataSets)
	{
		my $encName = escapeHTML($name);
		print "   <option value=$encName>$encName</option>\n";
	}
	print <<END;
  </select></td>
 </tr>

 <tr><td colspan="2"><hr></td></tr>

 <tr>
  <td align="left">Gantry diameter:</td>
  <td><select name="RingDiameter">
	<option value="">&lt;&lt; Select &gt;&gt;</option>
	<option value="Small">Small</option>
END
	if ($reconsConfig{'HasLargeDiameter'})
	{
		print '   <option value="Large">Large</option>\n';
	}
print <<END;
  </select></td>
 </tr>

 <tr><td colspan="2"><hr></td></tr>
 <tr>
  <td></td>
  <td><input type="submit" value="Submit"></td>
 </tr>

</table>
END

	print end_form, end_html;
}

