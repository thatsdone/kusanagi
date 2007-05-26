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
    printf("    -f FILE     : opreport log file\n");
    printf("    -M mode     : plain(default), edgesonly,...\n");
    printf("    -t target   : app name, 'null' means everything (default:null)\n");
    printf("    -m #topsyms : # of symbols in descent from top\n");
    printf("    -F #factor  : factor of diff1/diff2 below (default:10)\n");
    printf("    -C #cutoff  : cutoff # of diff1 (default:1000)\n");
    printf("    -u          : timestamps in UNIX time\n");
    printf("    -g          : format of graph to generate(gnuplot terminal)\n");
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
    printf("      '-M edgesonly' detects non-stationary changes of samples by tracking\n");
    printf("                 differences of samples on tnow-t1 and t1-t2 above.\n");
    printf("                 By default, if diff1/diff2 >= 10(factor) and\n");
    printf("                 diff1 >= 1000(cutoff), take it as an non-stationary change.\n");
    printf("                 Use '-F' and '-C' to change factor and cutoff.\n");
    printf("      '-M edgesplain' shows samples which have edges.\n");
    printf("      '-M edgesplot' generates a graph using gnuplot.\n");
    printf("      '-m' is valid only for '-M plain,'\n");
    printf("      '-F' and '-C' are valid only for '-M edges.'\n");
    printf("      '-g' is valid only for '-M edgesplot,'\n");
    printf("\n");
}
$unixtime_timestamp = 0;
$start_timestamp = 0;
$end_timestamp = 0;
$count = 0;
$header_parsed = 0;
#
# default parameter values
#
$quiet = 0;
$filename = "";
$target = "";
$regtarget = "";
$top_syms = 0;
$mode = "plain";
$factor = 10;
$cutoff = 1000;
$begin_time = "";
$end_time = "";
#
$appname_base = 4;
$symbol_base = 5;
#
$gp_terminal = "png";
$gp_size = "";
$gpkey = "png";
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
    } elsif ($ARGV[$i] eq "-R") {
	$i++;
	$regtarget = $ARGV[$i];
    } elsif ($ARGV[$i] eq "-M") {
	$i++;
	$mode = $ARGV[$i];
	if ($mode ne "plain" && $mode ne "edgesonly" && $mode ne "edgesplot" &&
	    $mode ne "edgesplain") {
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

    } elsif ($ARGV[$i] eq "-b") {
	$i++;
	$begin_time = $ARGV[$i];

    } elsif ($ARGV[$i] eq "-e") {
	$i++;
	$end_time = $ARGV[$i];

    } elsif ($ARGV[$i] eq "-gt") {
	$i++;
	$gp_terminal = $ARGV[$i];

    } elsif ($ARGV[$i] eq "-gs") {
	$i++;
	$gp_size = $ARGV[$i];

    } elsif ($ARGV[$i] eq "-gk") {
	$i++;
	$gp_key = $ARGV[$i];

    } elsif ($ARGV[$i] eq "-q") {
	$quiet = 1;
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
if ($mode eq "edgesplot") {
    $unixtime_timestamp = 1;
}

if ($begin_time ne "") {
# 2007/03/28 00:45:20
    ($year, $month, $date, $hour,$min,$sec) = split(/[\/ +:]/, $begin_time);
    $begin_time = &timegm($sec, $min, $hour,
			  $date, $month - 1, $year);
#    printf("%s/%s/%s %s:%s:%s\n", $year, $month, $date, $hour, $min, $sec);
    printf("%s\n", $begin_time);
}
if ($end_time ne "") {
    ($year, $month, $date, $hour, $min, $sec) = split(/[\/ +:]/, $end_time);
    $end_time = &timegm($sec, $min, $hour,
			  $date, $month - 1, $year);
    printf("%s\n", $begin_time);
}
#$check = 1;
#
# Print analysis parameters
#
if ($quiet == 0) {
    printf("#filename : %s\n", $filename);
    printf("#mode     : %s\n", $mode);
    if ($target eq "") {
	printf("#target   : %s\n", "everything");
    } else {
	printf("#target   : %s\n", $target);
    }
    printf("#topsyms  : %s\n", $top_syms);
    if ($mode =~ /edges/) {
	printf("#factor   : %s\n", $factor);
	printf("#cutoff   : %s\n", $cutoff);
    } else {
	printf("#factor   : %s (ignored)\n", $factor);
	printf("#cutoff   : %s (ignored)\n", $cutoff);
    }
    printf("#\n");
}
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

        # TZ is ignored at this moment using timegm()
#	if ($mode eq "edgesplot" || $mode eq "edgesplain") {
#	    ;
#	} els
	if ($unixtime_timestamp) {
	    ($day, $month, $date, $time, $tz, $year) = split(/ +/, $input);
	    ($hour, $min, $sec) = split(/:/, $time);
	    $timestamp = &timegm($sec, $min, $hour,
				 $date, $month_sym{$month}, $year);
	    $timestamp_str = $timestamp;
	    if ($start_timestamp == 0) {
		$start_timestamp = $timestamp;
	    }
	    $end_timestamp = $timestamp;
#	    printf("%s\n", $timestamp);
	} else {
#	    printf("%s\n", $input);
	    $timestamp_str = $input;
	}
        #
        # Ignore opreport header lines, check format and get some parameters
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
	# suppress detailed prototypes of C++ methods
	if ($input =~ /\(no symbols\)/) {
	    $input =~ s/\(.+\)/()/;
	    @str = split(/[ \t]+/, $input);
	    @str[$symbol_base + $samples_offset] = "(no_symbols)";
	} else {
	    $input =~ s/\(.+\)/()/;
	    @str = split(/[ \t]+/, $input);
	}
	$samples = $str[$samples_offset];

	$symbol_offset = $symbol_base + $samples_offset;
	if ($has_appname) {
	    $appname = $str[$appname_base + $samples_offset];
	} else {
	    $appname = "null";
	}
	$symbol = $str[$symbol_offset];
#print "HOGE: target = " . $target . " / " . $appname . "\n";
	if ($target eq "" || $appname eq $target) {
	    $key = $appname . " / " . $symbol;
#printf("DEBUG1: %s\n", $key);
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
# Shinagawa specific
	$trash = <IN>;

	# '-M plain' case
	if ($mode eq "plain") {
	    $num_syms = 0;
	    foreach(sort {$current{$b} <=> $current{$a}} keys %current) {
		if ($top_syms != 0 && $num_syms >= $top_syms) {
		    last;
		}
		if ($num_syms == 0) {
		    printf("%s\n", $timestamp_str);
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

        # '-M edges*' case
	} elsif ($mode =~ /^edges/) {
	    $num_syms = 0;

	    foreach(sort {$current{$b} <=> $current{$a}} keys %current) {
		if ($top_syms != 0 && $num_syms >= $top_syms) {
#printf("DEBUG7: %s / %s\n", $num_syms,$top_syms);
		    last;
		}
#		if ($_ =~ "signalHandler") {
#printf("DEBUG4! ");
#printf("%s %s %s\n", $current{$_}, $prev{$_}, $prev2{$_});
#		}
		if ((($current{$_} - $prev{$_}) >= 
		     ($prev{$_} - $prev2{$_}) * $factor) &&
		    (($current{$_} - $prev{$_}) >= $cutoff)) {
#printf("DEBUG5!\n");
		    if ($num_syms == 0 && $mode eq "edgesonly") {
			printf("%s\n", $timestamp_str);		    
			printf("%-45s   %7s   %7s   %7s\n",
			       "#app name / symbol", "current",
			       "diff1", "diff2");
		    }
		    if ($mode eq "edgesonly") {
			printf("%-45s , %7s , %7s , %7s\n",
			       substr($_, 0, 45),
			       $current{$_},
			       $current{$_} - $prev{$_},
			       $prev{$_} - $prev2{$_});
		    }
		    elsif ($mode eq "edgesplot" || $mode eq "edgesplain") {
## BUG!!!
			($appname, $slash, $symbol) = split(/[ +]/, $_);
#printf("DEBUG3: %s '%s'\n", $_, $appname);

			if ($target eq "" || $target eq $appname) {
			    $top{$_} = $current{$_};
			}
		    }
## DEBUG: really here?
		    $num_syms++;
		}
	    }
	}
	if ($mode eq "edgesonly" && $num_syms != 0) {
	    printf("\n");
	}
    }
}
#printf("HOGE8: %d\n", keys %top);
#exit;
close(IN);
if ($mode eq "edgesonly" || $mode eq "plain") {
    exit;
}
#
# -M edgesplot  for generating a graph by gnuplot 
# -M edgesplain for human readable time-series edges of sample counts
#
if ($quiet == 0) {
# print summary
# BUG!!
#    printf("%-45s   %8s   %8s   %8s\n",
#	   "#app name / symbol", "current", "diff1", "diff2");
    printf("%-45s   %7s\n",
	   "#app name / symbol", "diff1");
    $num_syms = 0;
    foreach(sort {$top{$b} <=> $top{$a}} keys %top) {
	if ($top_syms != 0 && $num_syms >= $top_syms) {
	    last;
	}
	printf("#%-45s %7s\n", $_, $top{$_});
	$num_syms++;
	$top_final{$_} = $top{$_};
    }
    printf("\n");
}

#
open(IN, $filename) || die "Failed to open" . $filename . "\n";
#
if ($mode eq "edgesplot") {
    $gp_cmdfile = $filename . ".gp";
    $gp_datafile = $filename . ".dat";
    open(CMD, ">" . $gp_cmdfile) || die "Failed to open" . $gp_cmdfile . "\n";
    open(D, ">" . $gp_datafile) || die "Failed to open" . $gp_datafile . "\n";
    printf(CMD "set terminal %s\n", $gp_terminal);
    if ($filename =~ /\.log$/) {
	$gp_output = $filename;
	$gp_output =~ s/log$/$gp_terminal/g;
    } else {
	$gp_output = $filename . "." . $gp_terminal;
    }
    if ($gp_size ne "") {
	printf(CMD "set size %s\n", $gp_size);
    }
    printf(CMD "set grid\n");
    printf(CMD "set output \"%s\"\n", $gp_output);
    $gp_title = "discontinuous changes in oprofile sample counts growth";
    printf(CMD "set title \"%s\"\n", $gp_title);
    $gp_xlabel = "\\ntime";
    printf(CMD "set xlabel \"%s\"\n", $gp_xlabel);
    $gp_ylabel = "samples";
    printf(CMD "set ylabel \"%s\"\n", $gp_ylabel);

    $gp_xtics = ($end_timestamp - $start_timestamp) / 10;
    if ($gp_xtics > 3600 && $gp_xtics % 3600) {
	$gp_xtics += (3600 - ($gp_xtics % 3600));
    } else {
	$gp_xtics = 900;
    }
#printf("DEBUG: gp_xtics = %s / %s %s\n", $gp_xtics, $start_timestamp, $end_timestamp);
#    $gp_xtics = 600;
    printf(CMD "set xtics %d\n", $gp_xtics);

    if ($gp_xtics >= 5 * 3600) {
	# more than 5 hours, mxtics=1hour
	$gp_mxtics = $gp_xtics / 3600;
    } elsif ($gp_xtics > 3600) {
	# more than 1 hour, mxtics=0.5hour
	$gp_mxtics = $gp_xtics / 3600 / 2;
    } elsif ($gp_xtics >= 1800) {
	$gp_mxtics = 6;
    } elsif ($gp_xtics >= 900) {
	$gp_mxtics = 3;
    } elsif ($gp_xtics >= 600) {
	$gp_mxtics = 5;
    } else {
	# else, anyway divided into 10 mxtics
	$gp_mxtics = 10;
    }
    printf(CMD "set mxtics %d\n", $gp_mxtics);
    $gp_xdata = "time";
    printf(CMD "set xdata %s\n", $gp_xdata);
    $gp_timefmt = "\"%s\"";
    printf(CMD "set timefmt %s\n", $gp_timefmt);
    printf(CMD "set format y \"10^%%L\"\n");
    printf(CMD "set logscale y\n");
    $gp_rmargin = 5;
    printf(CMD "set rmargin %d\n", $gp_rmargin);
    $gp_bmargin = 5;
    printf(CMD "set bmargin %d\n", $gp_bmargin);
    if ($gp_key eq "") {
	$gp_key = "right bottom";
    }
    printf(CMD "set key %s\n", $gp_key);
}
#
$count = 0;
while ($input = <IN>) {

    chop($input);

    if ($input =~ /^Linux/) {
	# ignore
	next;

    } elsif ($input =~ / [0-9][0-9]:[0-9][0-9]:[0-9][0-9] /) {
	if ($unixtime_timestamp) {
	    ($day, $month, $date, $time, $tz, $year) = split(/ +/, $input);
	    ($hour, $min, $sec) = split(/:/, $time);
	    $timestamp = &timegm($sec, $min, $hour,
				    $date, $month_sym{$month}, $year);
	    if ($mode eq "edgesplain") {
		printf("%s ", $timestamp);
		printf("\n",); # DEBUG
	    }
	} else {
	    printf("%s ", $input);
	    printf("\n",); # DEBUG
	}
	$count++;

    # process sampling data
    } elsif ($input =~ /^[0-9]/) {

	if ($input =~ /\(no symbols\)/) {
	    $input =~ s/\(.+\)/()/;
	    @str = split(/[ \t]+/, $input);
	    @str[$symbol_base + $samples_offset] = "(no_symbols)";
	} else {
	    $input =~ s/\(.+\)/()/;
	    @str = split(/[ \t]+/, $input);
	}
#	@str = split(/[ \t]+/, $input);
	$samples = $str[$samples_offset];

	$symbol_offset = $symbol_base + $samples_offset;
	if ($has_appname) {
	    $appname = $str[$appname_base + $samples_offset];
#	    $appname = $str[$#str - 1];
	} else {
	    $appname = "null";
	}

	$symbol = $str[$#str];
#	if ($symbol eq "\(no") {
#	    $symbol = "(no symbols)";
#	}
	if ($target eq "" || $appname eq $target) {
	    $key = $appname . " / " . $symbol;
#	    printf("DEBUG2: %s\n", $key);
	    if ($count >= 3) {
		$prev2{$key} = $prev{$key};
	    } 
	    if ($count >= 2) {
		$prev{$key} = $current{$key};
	    }
	    $current{$key} = $samples;
	    if ($top_final{$key} ne "") {
#	    printf("DEBUG: key=%-45s top=%s / %s\n", $key, $top{$key},
#		   $samples);
		$work{$key} = $samples;
	    }
	}

    } elsif ($input eq "") {
# Shinagawa specific
	$trash = <IN>;
	if ($count == 1 && $mode eq "edgesplot") {
	    $i = 1;
	    @num_top = keys %top_final;
	    printf(CMD "plot ");
	    foreach(sort {$top_final{$b} <=> $top_final{$a}} keys %top_final) {
#		printf("\nDEBUG: %-45s %7s\n", $_, $top{$_});
#		$key1 = $_;
#		$key1 =~ s/vmlinux \/ //;
## DEBUG
		($appname, $slash, $symbol) = split(/[ +]/, $_);
		if ($symbol eq "(no_symbols)") {
		    $key = $appname;
		} else {
		    $key = $symbol;
		}
#printf("DEBUG6: '%s' '%s' '%s'\n", $hoge, $key1, $_);
#		$key1 = $symbol;
## DEBUG
#		printf("%s\n", $key1);
#		printf("%s ", $key1);

#		printf("\"%s\" using 1:%s ti \"%s\" with lines \\ \n", "-", $i + 1, $key1);
		printf(CMD "\"%s\" using 1:%s ti \"%s\" with lines", $gp_datafile, $i + 1, $key);
		if ($i <= $#num_top) {
		    printf(CMD ", ");
		}
		$i++;

	    }
	    printf(CMD "\n");
	    printf(CMD "#\n");
	}
#
	if ($mode eq "edgesplot") {
	    printf(D "%s ", $timestamp);
	    foreach(sort {$top_final{$b} <=> $top_final{$a}} keys %top_final) {
		if ($top_final{$_} ne "") {
		    if ($work{$_} ne "") {
			printf(D "%s ", $work{$_});
		    } else {
			printf(D "%s ", "0");
		    }
		}
	    }
	    printf(D "\n");
	} elsif ($mode eq "edgesplain") {
# edgesplain
# $top{} contains current diff.
	    foreach(sort {$top_final{$b} <=> $top_final{$a}} keys %top_final) {
		if ($top_final{$_} ne "") {
		    if ($work{$_} ne "") {
#			printf("'%-45s' %s %s\n", $_, $top{$_}, $work{$_});
# $work{} contains total samples, not differences
			printf("'%-45s' %10s\n", $_, $work{$_});
		    } else {
			printf("'%-45s' %10s\n", $_, "0");
		    }
		}
	    }
	    printf("\n");
# edgesplain
	} else {
	    # impossible
	}
	%work = ();
    }
}
close(IN);
if ($mode eq "edgesplot") {
    close(CMD);
    close(D);
printf("Calling gnuplot...\n");
    system("gnuplot " . $gp_cmdfile);
#    unlink($gp_cmdfile);
#    unlink($gp_datafile);
}

