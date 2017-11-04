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
function updateAtten()
{
//    alert("updateAtten called");
  var enabled = document.my_form.FOV.value == "7_Small"
    && document.my_form.quant.checked;
  document.my_form.atten.disabled = !enabled;
  enabled &= document.my_form.atten.checked;
  document.my_form.transData.disabled = !enabled;
}

function verifyForm()
{
//  alert("verifyForm called");
  if (document.my_form.DataSet.value == "")
  {
    alert("Please select a data set!");
    document.my_form.DataSet.focus();
    return false;
  }
  if (document.my_form.FOV.value == "")
  {
    alert("Please select a field of view!");
    document.my_form.FOV.focus();
    return false;
  }
  ElementList = document.getElementsByName("TransmissionDataSet");
  if (ElementList[0])
  {
    if (!document.my_form.TransmissionDataSet.disabled
   	  && document.my_form.TransmissionDataSet.value == "")
    {
      alert("Please select a data set for the transmission!");
      document.my_form.TransmissionDataSet.focus();
      return false;
    }
  }
  if (document.my_form.ImageName.value == "")
  {
    alert("Please specify the name of the final image!");
    document.my_form.ImageName.focus();
    return false;
  }
  if (document.my_form.FrameCount.value > 1
  && document.my_form.FrameLength.value == "")
  {
    alert("Please specify the duration of the frames!");
    document.my_form.FrameLength.focus();
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
	if (!param('FOV'))
  	{
    		$msg .= 'Please select a field of view!<BR>';
	}
	if (param('AttenuationCorrection'))
	{
		if (param('FOV') eq "7_Small")
		{
			$msg .= 'Attenuation Correction not required for small FOV!<BR>';
		} elsif (!param('Quantifications'))
		{
			$msg .= 'Attenuation Correction requires Quantifications to be enabled!<BR>';
		} elsif (!param('TransmissionDataSet'))
		{
			$msg .= 'Attenuation Correction requires a transmission data set to be selected!<BR>';
		}
	}
	if (!param('ImageName'))
  	{
    		$msg .= 'Please specifiy a name for the final image!<BR>';
	}
	if (param('FrameCount') > 1 && !param('FrameLength'))
  	{
    		$msg .= 'Please specifiy the duration of the frames!<BR>';
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
	start_html(-title=>'Starting ClearPET Reconstruction'),
	h2('Starting ClearPET Reconstruction');
	my $paramFile = $statusDir."/params$$";
	if (open my $out, '>', $paramFile)
	{
		my @names = param;
		foreach my $name (@names)
		{
			print $out "$name = ", Encode::decode_utf8(param($name)), "\n";
		}
		close $out;

		print "The reconstruction job has been scheduled.\n", p;
		print 'Go to the <a href="status.html" target="_top">Status page</a> to monitor the reconstruction progress.', p;
		print 'Or <a href="new.html" target="_top">Start another reconstruction</a>', p;

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
			print b('Warning: '), 'The reconstruction scheduler is not running. The reconstruction will not be processed until the scheduler is started';
		}
	} else
	{
		$msg = "Could not write parameter file $paramFile";
	}
	print escapeHTML(Encode::decode_utf8($msg)), p if $msg;
	print end_html;
} else
{
	print header(-charset=>'UTF-8'),
	start_html(-title=>'ClearPET Reconstruction Parameters',
		   -encoding=>'UTF-8',
                   -script=>$JSCRIPT, -onLoad=>'updateAtten();'),
	h2('ClearPET Reconstruction Parameters'), "\n",
	start_form(-name=>'my_form', -onSubmit=>'return verifyForm()');
 
	my @dataSets;
	if ($dataFrom eq 'SinglesFromLocal' )
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
  <td align="left">Field of view:</td>
  <td>
   <select name="FOV" onSelect="updateAtten();">
	<option value="">&lt;&lt; Select &gt;&gt;</option>
    <optgroup label="Small Gantry Diameter">
	 <option value="7_Small"> 77.5 mm</option>
	 <option value="9_Small"> 93.5 mm</option>
	</optgroup>
END
	if ($reconsConfig{'HasLargeDiameter'})
	{
print <<END;
	<optgroup label="Large Gantry Diameter">
	<option value="7_Large">135 mm</option>
	<option value="9_Large">163 mm</option>
	</optgroup>
END
	}
print <<END;
   </select>
  </td>
 </tr>

 <tr><td colspan="2"><hr></td></tr>

 <tr>
  <td align="left">Name of the final image: </td>
  <td><input name="ImageName" type="text" size="50" maxlength="50"></td>
 </tr>
 <tr>
  <td></td><td>
   <input type="checkbox" name="Save_CoincLog" value="1"> Save coincidence sorter log file
  </td>
 </tr>
 <tr>
  <td></td><td>
   <input type="checkbox" name="Save_CoincData" value="1"> Save coincidence data file
  </td>
 </tr>
 <tr>
  <td></td><td>
   <input type="checkbox" name="Save_Sinogram" value="1"> Save sinogram file
  </td>
 </tr>
 <tr><td colspan="2"><hr></td></tr>
 <tr>
  <td>
   <input id="quant" type="checkbox" name="Quantifications" value="1" onclick="updateAtten();"> Apply Corrections for Quantification
  </td>
  <td>
   <input id="mask" type="checkbox" name="MaskCylinder" value="1"> Mask image artefacts outside field of view
  </td>
END
	if ($reconsConfig{'EnableAttenuationCorrection'})
	{
		print <<END;
  <td>
   <input id="atten" type="checkbox" name="AttenuationCorrection" value="1" onclick="updateAtten();"> Attenuation correction
  </td>
 </tr>
 <tr>
  <td align="left">Data set for the transmission:</td>
  <td><select id="transData" name="TransmissionDataSet">
END
	  	print '   <option value="">', escapeHTML('<< Select >>'), "</option>\n";
		foreach my $name (@dataSets)
		{
			my $encName = escapeHTML($name);
			print "   <option value=$encName>$encName</option>\n";
		}
		print "</select></td>\n";
	}
	print <<END;
 </tr>

 <tr><td colspan="2"><hr></td></tr>
 <tr>
  <td align="left">Number of frames:</td>
  <td><input name="FrameCount" type="text" size="50" maxlength="3" value="1"></td>
 </tr>
 <tr>
  <td align="left">Duration of the frames in minutes:</td>
  <td><input name="FrameLength" type="text" size="50" maxlength="3"></td>
 </tr>
 <tr>
  <td align="left">Begin of first frame relative to start of DAQ [minutes]:</td>
  <td><input name="FirstFrameOffset" type="text" size="50" maxlength="3" value="0"></td>
 </tr>
 <tr>
   <td></td>
   <td><input type="checkbox" name="EquilibriumImage" value="1"> Calculate equilibrium image (only if at least 3 frames)
   </td>
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

