use File::Basename;
use lib (dirname($0) . '/../perl/lib');

package BarnOwl;

use strict;
use warnings;
use Carp;

sub bootstrap {}
sub get_data_dir {"."}
sub get_config_dir {"."}
sub create_style {}

our $ml = BarnOwl::MessageList->new();
sub message_list {$ml}

sub debug {
    warn "[DEBUG] ", shift, "\n" if $ENV{TEST_VERBOSE};
}

our %filters;

sub new_filter {
    my $name = shift;
    my $sub = shift;
    BarnOwl::View::invalidate_filter($name);
    $filters{$name} = $sub;
}

sub filter_message_match {
    my $filter = shift;
    my $m = shift;
    unless(exists $filters{$filter}) {
        die("Unknown filter: $filter\n");
    }
    return $filters{$filter}->($m);
}

sub BarnOwl::Internal::new_command {}
sub BarnOwl::Internal::new_variable_bool {}
sub BarnOwl::Internal::new_variable_int {}
sub BarnOwl::Internal::new_variable_string {}

use BarnOwl;

if($ENV{TEST_VERBOSE}) {
    $BarnOwl::View::DEBUG = 1;
}

sub is_prime {
    my $n = shift;
    if($n <= 1) {
        return 0;
    }
    for my $i (2 .. int(sqrt($n))) {
        if($n % $i == 0) {
            return 0;
        }
    }
    return 1;
}

# Define some filters for testing
BarnOwl::new_filter(all => sub {1});
BarnOwl::new_filter(one => sub {shift->{num} == 1});
BarnOwl::new_filter(none => sub {0});
BarnOwl::new_filter(even => sub {(shift->{num} % 2) == 0});
BarnOwl::new_filter(odd  => sub {(shift->{num} % 2) == 1});
BarnOwl::new_filter(prime  => sub {is_prime(shift->{num})});

# Use a smaller fill step for testing so we can test on small message
# lists.
$BarnOwl::View::FILL_STEP = 10;
