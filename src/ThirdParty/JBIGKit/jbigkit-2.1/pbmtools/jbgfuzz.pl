#!/usr/bin/perl
# Simple fuzz tester for JBIG-KIT decoder -- Markus Kuhn
#
# Usage example:
#
# $ ../libjbig/tstcodec t.pbm
# $ ./pbmtojbg -f t.pbm | ./jbgfuzz.pl

use strict;

my $fntst = '/tmp/test.jbg';    # fuzz testing file to be generated
my $fntmp = $fntst . '~';       # temporary file (for atomic update)
my $fnvalid = '-';              # valid example BIE file
my $pbmtools = '.';             # location of jbgtopbm and jbgtopbm85
my $count = "inf";              # how many times shall we try?
my @decoders;

my $prefix_len = 2000;
my $rnd_suffix_len = 2000;
my $mutation_rate = 10; # percentage of bytes substituted in prefix

while ($_ = shift @ARGV) {
    if ($_ eq '-c') {
	$count = shift @ARGV;
    } elsif ($_ eq '-m') {
	$mutation_rate = shift @ARGV;
    } elsif ($_ eq '-p') {
	$prefix_len = shift @ARGV;
    } elsif ($_ eq '-r') {
	$rnd_suffix_len = shift @ARGV;
    } elsif ($_ eq '-d') {
	push @decoders, shift @ARGV;
    } elsif ($_ eq '-t') {
	$pbmtools = shift @ARGV;
    } else {
	$fnvalid = $_;
    }
}

@decoders = ('jbgtopbm', 'jbgtopbm85') unless @decoders;

# read some bytes from a valid BIE
my $valid_prefix;
my $in;
open($in, "<$fnvalid") || die("$fnvalid: $!\n");
read $in, $valid_prefix, $prefix_len;
close $in || die("$fnvalid: $!\n");

# open a source of random bytes
my $fn_rnd = '/dev/urandom';
my $rnd;
open($rnd, '<', $fn_rnd) || die;

for (my $i = 0; $i < $count; $i++) {
    my $out;
    open($out, '>', $fntmp) || die("$fntmp: $!\n");
    my $prefix;
    # randomly substitute some prefix bytes with random bytes
    $prefix = $valid_prefix;
    if (length($prefix) != $prefix_len) {
	warn("Truncating requested $prefix_len byte prefix to available ".
	     length($prefix)." bytes.\n");
	$prefix_len = length($prefix);
    }
    #print "\nB: ".join(',', unpack('C4N3C4', substr($prefix, 0, 20)))."\n";
    for (my $p = 0; $p < $prefix_len; $p++) {
	if (rand(100) < $mutation_rate) {
	    substr($prefix, $p, 1) = chr(int(rand(256)));
	}
    }
    #print "A: ".join(',', unpack('C4N3C4', substr($prefix, 0, 20)))."\n";
    # constrain header
    my ($dl,$d,$p,$res,$xd,$yd,$l0,$mx,$my,$order,$options,$rest) =
	unpack('C4N3C4a*', $prefix);
    redo if $xd * $yd > 1e9;    # eliminate excessive image sizes
    $prefix = pack('C4N3C4a*', $dl,$d,$p,$res,$xd,$yd,$l0,$mx,$my,
		$order,$options,$rest);
    print $out $prefix;
    # append random suffix
    my $data;
    read $rnd, $data, $rnd_suffix_len;
    print $out $data;
    close($out) || die("$fntmp: $!\n");
    rename($fntmp, $fntst) || die("mv $fntmp $fntst: $!\n");
    # now feed fuzz input into decoder(s)
    for my $jbgtopbm (@decoders) {
	printf "%5d: ", $i;
	$_ = `$pbmtools/$jbgtopbm $fntst /dev/null 2>&1`;
	my $r = $?;
	if ($r == 0) {
	    print "no error encountered\n";
	    next;
	} elsif ($r == 256) {
	    my $err;
	    if (/(\(error code.*\))/) {
		$err = $1;
		print $err, "\n";
	    } else {
		die("$_\nno error code found\n");
	    }
	} else {
	    die("$_\nreturn value: $r\n");
	}
    }
}
