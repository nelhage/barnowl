use warnings;
use strict;

=head1 NAME

BarnOwl::MessageList::SQL

=head1 DESCRIPTION

A SQL-backed BarnOwl message list!

=cut

package BarnOwl::MessageList::SQL;
use base qw(Class::Accessor);

use DBIx::Simple;
use SQL::Abstract;
use JSON;
use POSIX qw(ctime);

my $MESSAGES = 'messages';
my $message_fields = [qw(id msg_time protocol body direction)];
my $ATTRS = 'attributes';
my $attr_fields    = [qw(message_id key value)];

__PACKAGE__->mk_ro_accessors(qw(db));
__PACKAGE__->mk_accessors(qw(msg_iter attr_iter attr_lookahead _next_id));

sub new {
    my $class = shift;
    my $db = DBIx::Simple->new("DBI:SQLite:dbname=$ENV{HOME}/.owl/messagedb",
                             {RaiseError => 1, AutoCommit => 1});
    my $self = {db => $db};
    bless($self, $class);

    my $maxq = $self->db->query("SELECT MAX(id) FROM $MESSAGES")
      or die($self->db->error);
    my $max = $maxq->fetch;
    my $nextid;

    if($max && defined($max->[0])) {
        $nextid = $max->[0] + 1;
    } else {
        $nextid = 0;
    }
    $self->_next_id($nextid);

    return $self;
}

sub next_id {
    my $self = shift;
    my $id = $self->_next_id;
    $self->_next_id($id+1);
    return $id;
}

sub get_size {
    my $self = shift;
    my $count = $self->db->query("SELECT COUNT(*) from $MESSAGES")
      or die("Can't SELECT COUNT:" . $self->db->error);
    my $cnt = $count->fetch->[0];
    return $cnt;
}

sub timestr {
    my $time = shift;
    my $str = ctime($time);
    $str =~ s/\n$//;
    return $str;
}

sub load_base {
    my $msg  = shift;
    my $ref  = shift;
    $msg->{id}        = $ref->[0];
    $msg->{_time}     = $ref->[1];
    $msg->{time}      = timestr($ref->[1]);
    $msg->{type}      = $ref->[2];
    $msg->{body}      = $ref->[3];
    $msg->{direction} = $ref->[4];
}

sub load_attr {
    my $msg = shift;
    my $ref = shift;
    my $key = $ref->[1];
    my $value = $ref->[2];

    if($key eq 'fields') {
        $value = from_json($value);
    }

    $msg->{$key} = $value;
}

sub start_iterate {
    my $self = shift;
    $self->msg_iter($self->db->select($MESSAGES, $message_fields,
                                    {deleted => 'false'}, [qw(id)]))
      or die("Unable to SELECT from messages: " . $self->db->error);
    
    $self->attr_iter($self->db->select($ATTRS, $attr_fields,
                                     {}, [qw(message_id)]))
      or die("Unable to SELECT from attrs: " . $self->db->error);
    $self->attr_lookahead($self->attr_iter->fetch);
}

sub iterate_next {
    my $self = shift;
    my %msg;
    my $next = $self->msg_iter->fetch;
    unless($next) {
        return undef;
    }
    load_base(\%msg, $next);
    my $attr = $self->attr_lookahead;
    while($attr && $attr->[0] < $msg{id}) {
        $attr = $self->attr_iter->fetch;
    }
    while($attr && $attr->[0] == $msg{id}) {
        load_attr(\%msg, $attr);
        $attr = $self->attr_iter->fetch;
    }
    return BarnOwl::Message->new(%msg);
}

sub get_by_id {
    my $self = shift;
    my $id = shift;
    my %msg;
    my $msg = $self->db->select($MESSAGES, $message_fields, {id => $id})
      or die($self->db->error);
    my $attrs = $self->db->select($ATTRS, $attr_fields, {message_id => $id})
      or die($self->db->error);
    my $base = $msg->fetch;
    unless($base) {
        return undef;
    }
    load_base(\%msg, $base);
    while(my $row = $attrs->fetch) {
        load_attr(\%msg, $row);
    }
    return BarnOwl::Message->new(%msg);
}

sub add_message {
    my $self = shift;
    my $msg  = shift;
    my %attrs = %$msg;
    $self->db->begin_work;
    $self->db->insert($MESSAGES, {
        id        => $msg->id,
        msg_time  => $msg->{_time},
        protocol  => $msg->type,
        body      => $msg->body,
        direction => $msg->direction
       })
      or die($self->db->error);
    delete $attrs{$_} for (qw(_time time id type body direction __fmtext));
    $attrs{fields} = to_json($attrs{fields}) if ref($attrs{fields});
    while(my ($k, $v) = each %attrs) {
        $self->db->insert($ATTRS, {
            message_id => $msg->id,
            key        => $k,
            value      => $v
           })
          or die($self->db->error);
    }
    $self->db->commit;
}

sub expunge {
    ## XXX TODO Y'ALL
}

1;
