#!/usr/bin/perl
# opanalyze.pl is a perl script for analyzing oprofile results:
# 
# Copyright (C) 2007  Masanori ITOH
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#	
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
# SYSNOPSIS:
#   opanalyze.pl -f file [OPTIONs]
#     
# DESCRIPTION:
#   opanalyze.pl is a helper perl script for analysis of time series opreport
#   (oprofile sampler) log files. As for details, see usage() or 
#   execute opanalyze.pl -h.
#
# AUTHOR:
#   Masanori ITOH  <masanori.itoh@gmail.com>
#
# $Id$
$version = "1.0";
use Time::Local;
#
%month_sym = (
          'Jan' => 0,
          'Feb' => 1,
          'Mar' => 2,
          'Apr' => 3,
          'May' => 4,
          'Jun' => 5,
          'Jul' => 6,
          'Aug' => 7,
          'Sep' => 8,
          'Oct' => 9,
          'Nov' => 10,
          'Dec' => 11
          );
#
sub usage () {
    printf("opanalyze.pl version %s\n", $version);
    printf("Usage: opanalyze.pl -f FILE [OPTION]\n");
    printf("    -f FILE     : a opreport log file\n");
    printf("    -M mode     : plain(default), edges,...\n");
    printf("    -t target   : app name, 'null' means everything (default:null)\n");
    printf("    -m #topsyms : # of symbols in descent from top\n");
    printf("    -F #factor  : factor of diff1/diff2 below (default:10)\n");
    printf("    -C #cutoff  : cutoff # of diff1 (default:1000)\n");
    printf("    -u          : timestamps in UNIX time\n");
    printf("Note:\n");
    printf("    First of all, take an oprofile report using 'opreport -la' without\n");
    printf("    specifying any binary files.\n");
    printf("    You can detect non stationary changes ('edges') of sample counts\n");
    printf("    out of a huge amount of time series opreport log file using '-M edges'\n");
    printf("    option.\n");
    printf("\n");
    printf("           t2       t1        tnow\n");
    printf("       ----+---------+---------+-----------> t\n");
    printf("           |<------->|<------->|\n");
    printf("           |  diff2  |  diff1  |\n");
    printf("         prev2     prev1    current\n");
    printf("\n");
    printf("         tnow,1,2,... : timestamps of opreport calls\n");
    printf("         current,prev,prev2,... : samples on each timestamp\n");
    printf("         diff1 = sample on tnow(current) - sample on t1(prev1)\n");
    printf("         diff2 = sample on t1(prev1) - sample on t2(prev2)\n");
    printf("\n");
    printf("      '-M plain' shows currnet, prev1 and prev2 as is.\n");
    printf("      '-M edges' detects non-stationary changes of samples by tracking\n");
    printf("                 differences of samples on tnow-t1 and t1-t2 above.\n");
    printf("                 By default, if diff1/diff2 >= 10(factor) and\n");
    printf("                 diff1 >= 1000(cutoff), take it as an non-stationary change.\n");
    printf("                 Use '-F' and '-C' to change factor and cutoff.\n");
    printf("      '-m' is valid only for '-M plain,'\n");
    printf("      '-F' and '-C' are valid only for '-M edges.'\n");
    printf("\n");
}
$unixtime_timestamp = 0;
$count = 0;
$header_parsed = 0;
#
# default parameter values
#
$filename = "";
$target = "";
$top_syms = 0;
$mode = "plain";
$factor = 10;
$cutoff = 1000;
#
$i = 0;
while ($i <= $#ARGV) {
    if ($ARGV[$i] eq "-f") {
	$i++;
	$filename = $ARGV[$i];
    } elsif ($ARGV[$i] eq "-m") {
	$i++;
	$top_syms = $ARGV[$i];
    } elsif ($ARGV[$i] eq "-t") {
	$i++;
	if ($ARGV[$i] ne "null") {
	    $target = $ARGV[$i];
	}
    } elsif ($ARGV[$i] eq "-M") {
	$i++;
	$mode = $ARGV[$i];
	if ($mode ne "plain" && $mode ne "edges") {
	    printf("mode %s not supported\n", $mode);
	    &usage();
	    exit;
	}
    } elsif ($ARGV[$i] eq "-C") {
	$i++;
	$cutoff = $ARGV[$i];

    } elsif ($ARGV[$i] eq "-F") {
	$i++;
	$factor = $ARGV[$i];
    } elsif ($ARGV[$i] eq "-u") {
	$unixtime_timestamp = 1;

    } elsif ($ARGV[$i] eq "-h") {
	&usage();
	exit;
    }
    $i++;
}
#
if ($filename ne "") {
    open(IN, $filename) || die "Failed to open" . $filename . "\n";
} else {
    printf("Specify log filename\n");
    &usage();
    exit;
}
#
# Print analysis parameters
#
printf("#filename : %s\n", $filename);
printf("#mode     : %s\n", $mode);
if ($target eq "") {
    printf("#target   : %s\n", "everything");
} else {
    printf("#target   : %s\n", $target);
}
printf("#topsyms  : %s\n", $top_syms);
if ($mode eq "edges") {
    printf("#factor   : %s\n", $factor);
    printf("#cutoff   : %s\n", $cutoff);
} else {
    printf("#factor   : %s (ignored)\n", $factor);
    printf("#cutoff   : %s (ignored)\n", $cutoff);
}
printf("#\n");
#
# Main loop
#
while ($input = <IN>) {

    chop($input);

    if ($input =~ /^Linux/) {
	# ignore
	next;

    } elsif ($input =~ / [0-9][0-9]:[0-9][0-9]:[0-9][0-9] /) {

	$count++;

        # TZ is ignored at this moment.
	if ($unixtime_timestamp) {
	    ($day, $month, $date, $time, $tz, $year) = split(/ +/, $input);
	    ($hour, $min, $sec) = split(/:/, $time);
	    $timestamp = &timelocal($sec, $min, $hour,
				    $date, $month_sym{$month}, $year);
	    printf("%s\n", $timestamp);
	} else {
	    printf("%s\n", $input);
	}
        #
        # Ignore opreport header lines
	while ($input = <IN>) {
	    chop($input);
	    if ($input =~ /^samples /) {
		if ($header_parsed == 0) {
		    if ($input =~ /app name/) {
			$has_appname = 1;
		    } else {
			$has_appname = 0;
		    }
		    $header_parsed = 1;
		}
		$samples_offset = 0;
		last;
	    } elsif ($input =~ /^vma /) {
		$samples_offset = 1;
		last;
	    }
	}

    # process sampling data
    } elsif ($input =~ /^[0-9]/) {

	@str = split(/[ \t]+/, $input);

	$samples = $str[$samples_offset];

	if ($has_appname) {
	    $appname = $str[$#str - 1];
	} else {
	    $appname = "null";
	}

	$symbol = $str[$#str];
	if ($symbol eq "\(no") {
	    $symbol = "(no symbols)";
	}

	if ($target eq "" || $appname eq $target) {
	    $key = $appname . " / " . $symbol;
	    if ($count >= 3) {
		$prev2{$key} = $prev{$key};
	    } 
	    if ($count >= 2) {
		$prev{$key} = $current{$key};
	    }
	    $current{$key} = $samples;
	}

    # a record terminator found
    } elsif ($input eq "") {
	# '-M plain' case
	if ($mode eq "plain") {
	    $num_syms = 0;
	    foreach(sort {$current{$b} <=> $current{$a}} keys %current) {
		if ($top_syms != 0 && $num_syms >= $top_syms) {
		    last;
		}
		if ($num_syms == 0) {
		    printf("%-45s   %7s   %7s   %7s\n",
			   "#app name / symbol",
			   "current", "prev1", "prev2");
		}
		printf("%-45s , %7s , %7s , %7s\n",
		       substr($_, 0, 45),
		       $current{$_},
		       $prev{$_},
		       $prev2{$_});
		$num_syms++;
	    }

        # '-M edges' case
	} elsif ($mode eq "edges") {
	    $num_syms = 0;
	    foreach(sort {$current{$b} <=> $current{$a}} keys %current) {
		if ($top_syms != 0 && $num_syms >= $top_syms) {
		    last;
		}
		if ((($current{$_} - $prev{$_}) >= 
		     ($prev{$_} - $prev2{$_}) * $factor) &&
		    (($current{$_} - $prev{$_}) >= $cutoff)) {
		    if ($num_syms == 0) {
			printf("%-45s   %7s   %7s   %7s\n",
			       "#app name / symbol", "current",
			       "diff1", "diff2");
		    }
		    printf("%-45s , %7s , %7s , %7s\n",
			   substr($_, 0, 45),
			   $current{$_},
			   $current{$_} - $prev{$_},
			   $prev{$_} - $prev2{$_});
		    $num_syms++;
		}
	    }
	}
	printf("\n");
    }
}
