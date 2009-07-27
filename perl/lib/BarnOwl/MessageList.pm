use strict;
use warnings;

package BarnOwl::MessageList;

sub binsearch {
    my $list = shift;

    return 0 unless @$list;
    
    my $val  = shift;
    my $key  = shift || sub {return $_[0]};
    my $left = 0;
    my $right = scalar @{$list} - 1;
    my $mid;

    while($left < $right) {
        $mid = int(($left + $right)/2);
        my $k = $key->($list->[$mid]);
        if($k == $val) {
            return $mid;
        } elsif ($k < $val) {
            $left = $mid + 1;
        } else {
            $right = $mid - 1;
        }
    }
    if($key->($list->[$left]) < $val) {
        $left++;
    }
    return $left;
}

my $__next_id = 0;

sub next_id {
    return $__next_id++;
}

sub new {
    my $class = shift;
    my $self = {messages => {}, keys => []};
    return bless $self, $class;
}

sub set_attribute {
    
}

sub get_size {
    my $self = shift;
    return scalar keys %{$self->{messages}};
}

=head2 iterate_begin ID [REVERSE]

Start an iteration over the message list. Once an iteration has
started, you can retrieve messages using C<iterate_next>, and stop the
iteration using C<iterate_done>.

Conceptually, a message list iterator points between two
messages. C<iterate_next> advances it in the direction of iteration,
and returns the message it passed over, or C<undef> if it has reached
the end.

The C<ID> parameter indicates which message to start iterating
from. The iterator starts out pointing immediately before that ID. A
negative ID starts the iterator past the end of the message list.

If the C<REVERSE> parameter is true, the iteration will procede in
direction of decreasing message ID.

=cut

sub iterate_begin {
    my $self = shift;
    my $id   = shift;
    my $rev  = shift;
    if($id < 0) {
        $self->{iterator} = scalar @{$self->{keys}};
    } else {
        $self->{iterator} = binsearch($self->{keys}, $id);
    }
    if($rev) {
        $self->{iterator}--;
    }
    
    $self->{iterate_direction} = $rev ? -1 : 1;
}

=head2 iterate_next

Return the next message in the message list, or C<undef> if the
iteration has completed. See C<iterate_begin> for more information
about iterating over a message list.

=cut

sub iterate_next {
    my $self = shift;
    if($self->{iterator} >= scalar @{$self->{keys}}
       || $self->{iterator} < 0) {
        return undef;
    }

    my $msg = $self->get_by_id($self->{keys}->[$self->{iterator}]);
    $self->{iterator} += $self->{iterate_direction};
    return $msg;
}

=head2 iterate_done

Finish iterating over a message list. This must be called once for
every call to C<iterate_begin>. Only one iteration can be performed at
a time; It is not at present allowed to do multiple iterations in
parallel.

=cut

sub iterate_done {
    # nop
}

sub get_by_id {
    my $self = shift;
    my $id = shift;
    return $self->{messages}{$id};
}

sub add_message {
    my $self = shift;
    my $m = shift;
    $self->{messages}->{$m->id} = $m;
    push @{$self->{keys}}, $m->id;
}

sub expunge {
    my $self = shift;
    for my $message (values %{$self->{messages}}) {
        if($message->is_deleted) {
            delete $self->{messages}->{$message->id};
            BarnOwl::View->message_deleted($message->id);
        }
    }
    $self->{keys} = [sort {$a <=> $b} keys %{$self->{messages}}];
}

1;
