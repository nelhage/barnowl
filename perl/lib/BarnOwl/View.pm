use warnings;
use strict;

package BarnOwl::View;

use BarnOwl::View::Iterator;
use BarnOwl::View::RangeList;

our $DEBUG = 0;

sub debug(&) {
    if($DEBUG) {
        BarnOwl::debug(shift->());
    }
}


our %view_cache;

sub message_deleted {
    my $class = shift;
    my $id = shift;
    for my $view (values %view_cache) {
        $view->message($id, 0);
    }
}

sub message {
    my $self = shift;
    my $idx = shift;
    my $val = shift;
    if(defined $val) {
        vec($self->{messages}, $idx, 1) = $val;
    }
    return vec($self->{messages}, $idx, 1);
};
sub get_filter {shift->{filter}};

sub is_empty      {
    my $self = shift;
    my $it = BarnOwl::View::Iterator->new();
    $it->initialize_at_start($self);
    return $it->is_at_end;
};

sub ranges        {shift->{ranges}};

sub new {
    my $class = shift;
    my $filter = shift;
    if(defined($view_cache{$filter})) {
        my $self = $view_cache{$filter};
        return $self;
    }

    my $self  = {messages  => "",
                 filter    => $filter,
                 ranges    => undef};
    bless $self, $class;
    $self->reset;
    $view_cache{$filter} = $self;
    return $self;
}

sub consider_message {
    my $self = shift;
    my $msg = shift;
    for (values %view_cache) {
        $_->_consider_message($msg);
    }
}

sub _consider_message {
    my $self = shift;
    my $msg  = shift;
    my $match;

    my $range = $self->ranges->find_or_insert($msg->{id});
    $range->expand_fwd($msg->{id} + 1);

    $match = BarnOwl::filter_message_match($self->get_filter, $msg);
    $self->message($msg->{id}, $match);
}

sub reset {
    my $self = shift;
    $self->{messages} = "";
    $self->{ranges} = BarnOwl::View::RangeList->new(-1, -1);
}

our $FILL_STEP = 100;

sub fill_back {
    my $self = shift;
    my $range = shift;
    my $pos  = $range->next_bk;
    my $ml   = BarnOwl::message_list();
    my $m;

    debug {"Fill back from $pos..."};

    $ml->iterate_begin($pos, 1);
    do {
        $m = $ml->iterate_next;

        $range->expand_fwd($m->{id} + 1) if $range->next_fwd < 0;

        unless(defined $m) {
            debug {"Hit start in fill_back."};
            goto loop_done;
        }

        $pos = $m->{id} if $pos < 0;

        if(BarnOwl::filter_message_match($self->get_filter, $m)) {
            $self->message($m->{id}, 1);
        }

        if($range->expand_bk($m->{id})) {
            $ml->iterate_done;
            $ml->iterate_begin($range->next_bk, 1);
        }
    } while(($pos - $m->{id}) < $FILL_STEP);

loop_done:
    $ml->iterate_done;
    debug {"[@{[$self->get_filter]}]" . $self->ranges->string_chain};
}

sub fill_forward {
    my $self = shift;
    my $range = shift;
    my $pos  = $range->next_fwd;
    my $ml   = BarnOwl::message_list();
    my $m;

    debug {"Fill forward from $pos..."};

    $ml->iterate_begin($pos, 0);
    do {
        $m = $ml->iterate_next;
        unless(defined $m) {
            debug {"Hit end in fill_forward."};
            goto loop_done;
        }

        $pos = $m->{id} if $pos < 0;

        if(BarnOwl::filter_message_match($self->get_filter, $m)) {
            $self->message($m->{id}, 1);
        }
        
        if($range->expand_fwd($m->{id} + 1)) {
            $ml->iterate_done;
            $ml->iterate_begin($range->next_fwd);
        }
    } while(($m->{id} - $pos) < $FILL_STEP);

loop_done:
    $ml->iterate_done;
    debug {"[@{[$self->get_filter]}]" . $self->ranges->string_chain};
}

sub invalidate_filter {
    my $filter = shift;
    delete $view_cache{$filter};
}

1;
