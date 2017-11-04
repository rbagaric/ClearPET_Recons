#!/usr/bin/perl

use open ':encoding(utf8)'; # input/output default encoding will be UTF-8
use warnings;
use strict;
use CGI qw(:standard);
use Fcntl ':flock';	# import LOCK_* constants
use Encode;

my $configFile = "/etc/clearpet.conf";
my %reconsConfig;
my $statusDir;
my $binDir;

my $JSCRIPT=<<END;
function startTimer()
{
	setInterval(onTick, 30*1000); // every 30 seconds
}
function onTick()
{
	top.document.location = "status.html";
// Note: we do not use "document.location.reload();" here
// because it will create stupid messages in the browser
// if the page was called with POST arguments (when some
// close button has been clicked).
}
END

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
    my %params;
  
	open my $in, '<', $infile or return %params;
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
	
sub ExpectKey
{
	my ($expected, $line, $filename)= @_;

	chomp $line;

	my ($key, $value) = split(/=/, $line, 2);
	$key = trim($key);
	$value = trim($value);
	return ($key eq $expected) ? $value : "";
}

sub ReadStatusFile
{
	my $filename = shift;
	if (open my $in, '<', $filename)
	{
		flock($in, LOCK_EX);
		my $line = <$in>;
		my $type = ExpectKey("Type", $line, $filename);
		$line = <$in>;
		my $dataName = ExpectKey("Data", $line, $filename);
		$line = <$in>;
		my $imageName = ExpectKey("Output", $line, $filename);
		$line = <$in>;
		my $status = ExpectKey("Status", $line, $filename);

		my @messages = <$in>;
		flock ($in, LOCK_UN);
		return ($type, $dataName, $imageName, $status, @messages);
	} else
	{
		return ("unknown", "unkown", "unknown", "Error", "Cannot read status file $filename");
	}
}

sub AbortProcess
{
	my $file = shift;
	my $success  = 'The process has been aborted.<br>Return to the <a href="status.html" target="_top">Status page</a>.';


	# first check if the file is a status file of a running process,
	# or just a parameter file of a schduled process.
	$_ = $file;
	if (m/status([0-9]*)/)
	{
		my $pid = $1;
		my ($type, $dataName, $imageName, $status, @messages) = ReadStatusFile($file);
		if ($status eq 'Complete' || $status eq 'Error')
		{
			# The process has completed in the meantime. Do nothing.
			return 'The process has terminated. Go to the <a href="status.html" target="_top">Status page</a> to see the status.';
		} elsif ($status eq 'Running')
		{
			# Kill the running process.
			# Note: normally, we do not have the rights to kill a process owned by user clearpet.
			# Therefore, we use a copy of kill in the clearpet bin directory, which has been
			# "chmod +s" in order to run it as clearpet user.
			return (0 == system($binDir.'/kill', $pid))
				? $success : "Could not terminate process id $pid";
		} else
		{
			return "Unknow status code in status file!";
		}
	} else
	{
		# The process has not yet started, just remove the file.
		# Note: here we have a race condition. If the scheduler has just
		# started the process, the parameter file might already be parsed by
		# the program, but not yet be removed. This is a small risk, however,
		# and will not make real problems because the running process will
		# be seen after updating the status page.
		return unlink($file) ? $success : 'Could not delete file '.escapeHTML(Encode::decode_utf8($file));
	}
}

sub PrintFileInfo
{
	my ($file, $closeButton, $abortButton) = @_;
	
	$_ = $file;
	if (m/status[0-9]+/ || m/status_params[0-9]+/ )
	{
		my ($type, $dataName, $imageName, $status, @messages) = ReadStatusFile($file);

		print h3(escapeHTML("$type: ".$imageName)), "\n";
		print b("Data set: "), escapeHTML($dataName), br, "\n";
		print b("Status: "), escapeHTML($status);
		if ($status eq "Complete" || $status eq "Error")
		{
			if ($closeButton)
			{
				print "   ";
				print submit(escapeHTML(Encode::decode_utf8($file)), $closeButton);
			}
		} else
		{
			if ($abortButton)
			{
				print "   ";
				print submit(escapeHTML(Encode::decode_utf8($file)), $abortButton);
			}
		}
		print "\n", br;
		for my $line (@messages)
		{
			chomp $line;
			print escapeHTML($line), br, "\n";
		}
		return 1;
	}
	elsif (m/normalize.*/)
	{
		my %params = ReadParameters($file);
		$params{"DataSet"} or return 0;
		print h3("Normalization"), "\n";
		print b("Data set: "), escapeHTML(Encode::decode_utf8($params{"DataSet"})), br, "\n";
		print b("Status: "), "Scheduled";
		if ($abortButton)
		{
			print "   ";
			print submit(escapeHTML(Encode::decode_utf8($file)), $abortButton);
		}
		print "\n", br;
		return 1;
	}
	elsif (m/params.*/)
	{
		my %params = ReadParameters($file);
		$params{"ImageName"} or return 0;
		$params{"DataSet"} or return 0;
		print h3(escapeHTML(Encode::decode_utf8($params{"ImageName"}))), "\n";
		print b("Data set: "), escapeHTML(Encode::decode_utf8($params{"DataSet"})), br, "\n";
		print b("Status: "), "Scheduled";
		if ($abortButton)
		{
			print "   ";
			print submit(escapeHTML(Encode::decode_utf8($file)), $abortButton);
		}
		print "\n", br;
		return 1;
	}
	elsif (m/delete.*/)
	{
#		my %params = ReadParameters($file);
		print h3("Data set removal"), "\n";
		print b("Status: "), "Scheduled";
		if ($abortButton)
		{
			print "   ";
			print submit(escapeHTML(Encode::decode_utf8($file)), $abortButton);
		}
		print "\n", br;
		return 1;
	}
	return 0;
}

binmode(STDOUT, ":utf8");

my $msg;
%reconsConfig = ReadParameters($configFile);
if (!%reconsConfig)
{
	$msg = "Error reading configuration file $configFile";
} else
{
	$statusDir = $reconsConfig{"StatusDir"};
	if (!$statusDir)
	{
		$msg = 'Missing value for configuration key "StatusDir"';
	} elsif (! -d $statusDir)
	{
		$msg = "Directory $statusDir does not exist";
	}
	$binDir = $reconsConfig{"BinDir"};
	if (!$binDir)
	{
		$msg = 'Missing value for configuration key "BinDir"';
	} elsif (! -d $binDir)
	{
		$msg = "Directory $binDir does not exist";
	}
}

if ($msg)
{
	print header(-charset=>'UTF-8'),
	start_html(-title=>'ClearPET Configuration Error'),
	h2('ClearPET Configuration Error');
	print escapeHTML(Encode::decode_utf8($msg)), p, end_html;
	exit;
}

print header(-charset=>'UTF-8'),
	start_html(-title => 'ClearPET Reconstruction Status',
		-meta => {'robots' => 'noindex,nofollow', 'expires' => '0'},
		-script=>$JSCRIPT, -onLoad=>"startTimer();"
		);

# If we called ourselves with parameters, handle this before creating HTML output.
if (param())
{
	my @names = param;
	if (param($names[0]) eq 'Abort')
	{
		print h2('Please confirm abortion of a process:'), "\n";
	} elsif (param($names[0]) eq 'Really Abort')
	{
		print h2('Result of process abortion'), "\n";
	} else
	{
		print h2('Status of running or recently terminated reconstructions:'), "\n";
	}
	print start_form();

	foreach my $name (@names)
	{
		if (param($name) eq 'Close')
		{
			if (!unlink($name))
			{
				$name = Encode::decode_utf8($name);
				print b(escapeHTML("Could not remove file $name")), p;
			}
		} elsif (param($name) eq 'Abort')
		{
			if (! -e $name)
			{
				print 'This process cannot be aborted in the moment',br;
				print 'Most probably this is because the process status has changed in the meantime.',br;
				print 'Please go back to the status page in order to get the current process status.',br;
			} else
			{
				PrintFileInfo($name, '', 'Really Abort');
			}
			print end_form, end_html;
			exit;
		} elsif (param($name) eq 'Really Abort')
		{
			if (! -e $name)
			{
				print 'This process cannot be aborted in the moment',br;
				print 'Most probably this is because the process status has changed in the meantime.',br;
				print 'Please go back to the status page in order to get the current process status.',br;
			} else
			{
				my $msg = AbortProcess($name);
				print $msg, p;
			}
			print end_form, end_html;
			exit;
		}
	}
} else
{
	print h2('Status of running or recently terminated reconstructions:'), "\n",
	start_form();
}

my $pidfilename = '/var/run/recons_scheduler.pid';
my $scheduler_ok = -r $pidfilename;
if ($scheduler_ok)
{
	open my $in, '<', $pidfilename;
	my $pid = <$in>;
	chomp $pid;
	if ($pid)
	{
		$scheduler_ok = `ps -p $pid --no-header`;
	} else
	{
		$scheduler_ok = 0;
	}
}
if (!$scheduler_ok)
{
	print b('Warning: '), 'The reconstruction scheduler is not running. The scheduled reconstructions will not be processed until the scheduler is started.', p;
}


my @statusFiles = qx/ls $statusDir\/status* 2>\/dev\/null/;
my @normalizeParamFiles = qx/ls $statusDir\/normalize* 2>\/dev\/null/;
my @reconsParamFiles = qx/ls $statusDir\/params* 2>\/dev\/null/;
my @deleteParamFiles = qx/ls $statusDir\/delete* 2>\/dev\/null/;

my $found = 0;

foreach my $file (@statusFiles)
{
	chomp $file;
	if (PrintFileInfo($file, 'Close', 'Abort'))
	{
		$found = 1;
		hr;
	}
}

foreach my $file (@normalizeParamFiles)
{
	chomp $file;
	if (PrintFileInfo($file, '', 'Abort'))
	{
		$found = 1;
		hr;
	}
}

foreach my $file (@reconsParamFiles)
{
	chomp $file;
	if (PrintFileInfo($file, '', 'Abort'))
	{
		$found = 1;
		hr;
	}
}

foreach my $file (@deleteParamFiles)
{
	chomp $file;
	if (PrintFileInfo($file, '', 'Abort'))
	{
		$found = 1;
		hr;
	}
}


if (!$found)
{
	print "No reconstruction jobs are running or have been finished recently.\n";
}

print end_form, end_html;


