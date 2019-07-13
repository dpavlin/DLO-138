#!/usr/bin/perl
use warnings;
use strict;

mkdir 'new';
open(my $metadata, '>', 'new/metadata');

print $metadata qq{
[global]
sigrok version=0.5.1

[device 1]
capturefile=logic-1
total probes=8
total analog=2
analog9=CH1
analog10=CH2
probe1=D_CH1
probe2=D_CH2
unitsize=1
};

open(my $ch1, '>', 'new/analog-1-9-1');
open(my $ch2, '>', 'new/analog-1-10-1');
open(my $logic, '>', 'new/logic-1-1');

my $hz = 0;

# 1 ms = 1000 Hz

while(<>) {
	if ( m/Per Sample \(us\): (\d+\.\d+)/ ) {
		print $metadata "samplerate=",1000 / $1," kHz\n";
	} elsif ( m/^(\d+\.\d+)\s+(\d+\.\d+)\s+(-?\d+)\s+(\d+)\s+(\d+)/ ) {
		my ($time,$a1,$a2,$d1,$d2) = ($1,$2,$3,$4,$5);
		warn "XXX $a1 | $a2 | $d1 $d2";
		print $ch1 pack('f', $a1);
		print $ch2 pack('f', $a2);
		print $logic pack('C', ( $d2 << 1 | $d1 ) ); # LE
	} else {
		warn "IGNORE: $_\n";
	}
}

close($metadata);
close($ch1);
close($ch2);
close($logic);

open(my $version, '>', 'new/version');
print $version '2';
close($version);

system "cd new ; zip ../new.sr *";
