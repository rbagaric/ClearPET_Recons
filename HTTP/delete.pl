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
my $dataRoot;
my $dataFrom;
my $numPrepro;

my $JSCRIPT=<<END;
function verifyForm()
{
  var i;
  var count = 0;
  for (i=0; i<document.my_form.DataSet.options.length; i++)
  {
	if (document.my_form.DataSet.options[i].selected)
	{
	  count++;
    }
  }
  if (count == 0)
  {
    alert("Please select at least one data set!");
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
	return $output ? "Cannot mount $unc:\n$output" : '';
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
	$statusDir = $reconsConfig{'StatusDir'};
	if (!$statusDir)
	{
		$msg = 'Missing value for configuration key "StatusDir"';
	} elsif (! -d $statusDir)
	{
		$msg = "Directory $statusDir does not exist";
	}
	$numPrepro = $reconsConfig{'NumPrepros'};
	if ($numPrepro < 1 || $numPrepro > 5)
	{
		$msg = 'Missing or invalid value for configuration key "NumPrepros"';
	}
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
		if ($reconsConfig{'MasterOnLocal'})
		{
			$dataRoot = $reconsConfig{'DataRoot'};
			if (!$dataRoot)
			{
				$msg = 'Missing value for configuration key "DataRoot"';
			} elsif (! -d $dataRoot)
			{
				$msg = "Directory $dataRoot does not exist";
			}
		} else
		{
			$dataRoot = $reconsConfig{'MountPointMasterWWW'};
			if (!$dataRoot)
			{
				$msg = 'Missing value for configuration key "MountPointMasterWWW"';
			} elsif (! -d $dataRoot)
			{
				$msg = "Directory $dataRoot does not exist";
			}
		}
	} else
	{
		$msg = 'Missing or invalid value for configuration key "DataFrom"';
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
	print header(-charset=>'UTF-8'),
	start_html(-title=>'Delete ClearPET Data Sets');

	if (!param('DataSet'))
  	{
    	print 'No data set had been selected. Nothing is deleted.<BR>';
	} else
	{
		print 'Selected data sets:', br;
		my $paramFile = $statusDir."/delete$$";
		if (open my $out, '>', $paramFile)
		{
			my @selection = param('DataSet');
			my $i = 0;
			foreach my $file (@selection)
			{
				++$i;
				$file = Encode::decode_utf8($file);
				print $out "DataSet$i = $file\n";
				print escapeHTML($file), br;
			}
			print $out "NumDataSets = $i\n";
			close $out;

			print p, "The removal job has been scheduled.\n", p;
			print 'Go to the <a href="status.html" target="_top">Status page</a> to monitor the progress.', p;

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
		print escapeHTML($msg), p if $msg;
	}

} else
{
	print header(-charset=>'UTF-8'),
	start_html(-title=>'Delete ClearPET Data Sets',
		   -encoding=>'UTF-8', -script=>$JSCRIPT),
	h2('Choose Data Sets to be deleted'), "\n",
	start_form(-name=>'my_form', -onSubmit=>'return verifyForm()');
 
	my @dataSets;
	if ($dataFrom eq 'SinglesFromLocal')
	{
		@dataSets = CollectDataSets($dataRoot);
	} else
	{
		if ($reconsConfig{'MasterOnLocal'})
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
	}

	print <<END;
  <p>You can select several data sets here, usually by holding down the Ctrl or Cmd key while clicking.</p>
  <p><b>Be sure that all reconstructions on these data are finished. The deleted data cannot be recovered!<b></p>
  <select name="DataSet" multiple size="10">
END
	foreach my $name (@dataSets)
	{
		my $encName = escapeHTML($name);
		print "   <option value=$encName>$encName</option>\n";
	}
	print <<END;
  </select>
 <p> 
 <input type="submit" value="Submit">
END

	print end_form, end_html;
}

