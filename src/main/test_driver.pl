#!/usr/bin/perl

if(scalar(@ARGV) != 1){
    print "usage: test_driver <test manifest>\n";
    exit;
}

my $test_manifest = $ARGV[0];

open(TEST_MANIFEST,$test_manifest) or die "Error opening test manifest $test_manifest: $!";

while(<TEST_MANIFEST>){

    chomp;
    my $test = $_;

    print "Running test $test...";

    my $output = `$test`;
    my $ref = `cat $test.out`;

    if($output eq $ref){
        print "OK\n";
    }
    else{
        print "FAIL\n";
    }

}

close TEST_MANIFEST;
