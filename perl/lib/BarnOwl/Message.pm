use strict;
use warnings;

package BarnOwl::Message;

use BarnOwl::Message::Admin;
use BarnOwl::Message::AIM;
use BarnOwl::Message::Generic;
use BarnOwl::Message::Loopback;
use BarnOwl::Message::Zephyr;

use POSIX qw(ctime);

sub new {
    my $class = shift;
    my $time = time;
    my $timestr = ctime($time);
    $timestr =~ s/\n$//;
    my %args = (
        __meta    => {deleted => 0},
        time      => $timestr,
        unix_time => $time,
        login     => 'none',
        direction => 'none',
        @_);
    unless(exists($args{id})) {
        my $msglist = BarnOwl::message_list();
        $args{id} = $msglist->next_id;
    }
    if(exists $args{loginout}) {
        $args{login} = $args{loginout};
        delete $args{loginout};
    }
    if($class eq __PACKAGE__ && $args{type}) {
        $class = "BarnOwl::Message::" . ucfirst $args{type};
    }
    return bless {%args}, $class;
}

sub lock_message {
    my $self = shift;
    for my $k (keys %$self) {
        if($k !~ /^_/) {
            Internals::SvREADONLY $self->{$k}, 1;
        }
    }
}

sub __set_attribute {
    my $self = shift;
    my $attr = shift;
    my $val  = shift;
    $self->{$attr} = $val;
}

sub __format_attributes {
    my $self = shift;
    my %skip = map {$_ => 1} qw(unix_time fields id __meta __fmtext type);
    my $text = "";
    my @keys = sort keys %$self;
    for my $k (@keys) {
        my $v = $self->{$k};
        unless($skip{$k}) {
            $text .= sprintf("  %-15.15s: %-35.35s\n", $k, $v);
        }
    }
    return $text;
}

sub set_meta {
    my $self = shift;
    my $key = shift;
    my $val = shift;
    $self->{__meta}{$key} = $val;
    BarnOwl::View->consider_message($self);
}

sub get_meta {
    my $self = shift;
    my $key  = shift;
    return $self->{__meta}{$key};
}

sub type        { return shift->{"type"}; }
sub direction   { return shift->{"direction"}; }
sub time        { return shift->{"time"}; }
sub unix_time   { return shift->{"unix_time"}; }
sub id          { return shift->{"id"}; }
sub body        { return shift->{"body"}; }
sub sender      { return shift->{"sender"}; }
sub recipient   { return shift->{"recipient"}; }
sub login       { return shift->{"login"}; }
sub is_private  { return shift->{"private"}; }

sub is_login    { return shift->login eq "login"; }
sub is_logout   { return shift->login eq "logout"; }
sub is_loginout { my $m=shift; return ($m->is_login or $m->is_logout); }
sub is_incoming { return (shift->{"direction"} eq "in"); }
sub is_outgoing { return (shift->{"direction"} eq "out"); }

sub is_deleted  { return shift->get_meta("deleted"); }

sub is_admin    { return (shift->{"type"} eq "admin"); }
sub is_generic  { return (shift->{"type"} eq "generic"); }
sub is_zephyr   { return (shift->{"type"} eq "zephyr"); }
sub is_aim      { return (shift->{"type"} eq "AIM"); }
sub is_jabber   { return (shift->{"type"} eq "jabber"); }
sub is_icq      { return (shift->{"type"} eq "icq"); }
sub is_yahoo    { return (shift->{"type"} eq "yahoo"); }
sub is_msn      { return (shift->{"type"} eq "msn"); }
sub is_loopback { return (shift->{"type"} eq "loopback"); }

# These are overridden by appropriate message types
sub is_ping     { return 0; }
sub is_mail     { return 0; }
sub is_personal { return shift->is_private; }
sub class       { return undef; }
sub instance    { return undef; }
sub realm       { return undef; }
sub opcode      { return undef; }
sub header      { return undef; }
sub host        { return undef; }
sub hostname    { return undef; }
sub auth        { return undef; }
sub fields      { return undef; }
sub zsig        { return undef; }
sub zwriteline  { return undef; }
sub login_host  { return undef; }
sub login_tty   { return undef; }

# This is for back-compat with old messages that set these properties
# New protocol implementations are encourages to user override these
# methods.
sub replycmd         { return shift->{replycmd}};
sub replysendercmd   { return shift->{replysendercmd}};

sub pretty_sender    { return shift->sender; }
sub pretty_recipient { return shift->recipient; }

# Override if you want a context (instance, network, etc.) on personals
sub personal_context { return ""; }
# extra short version, for use where space is especially tight
# (eg, the oneline style)
sub short_personal_context { return ""; }

sub delete {
    my $self = shift;
    $self->set_meta(deleted => 1);
    BarnOwl::message_list()->set_attribute($self => deleted => 1);
}

sub undelete {
    my $self = shift;
    $self->set_meta(deleted => 0);
    BarnOwl::message_list()->set_attribute($self => deleted => 0);
}

sub is_question {
    my $self = shift;
    return $self->is_admin && defined($self->{question});
}

# Serializes the message into something similar to the zwgc->vt format
sub serialize {
    my ($this) = @_;
    my $s;
    for my $f (keys %$this) {
	my $val = $this->{$f};
	if (ref($val) eq "ARRAY") {
	    for my $i (0..@$val-1) {
		my $aval;
		$aval = $val->[$i];
		$aval =~ s/\n/\n$f.$i: /g;
		$s .= "$f.$i: $aval\n";
	    }
	} else {
	    $val =~ s/\n/\n$f: /g;
	    $s .= "$f: $val\n";
	}
    }
    return $s;
}

# Populate the annoying legacy global variables
sub legacy_populate_global {
    my ($m) = @_;
    $BarnOwl::direction  = $m->direction ;
    $BarnOwl::type       = $m->type      ;
    $BarnOwl::id         = $m->id        ;
    $BarnOwl::class      = $m->class     ;
    $BarnOwl::instance   = $m->instance  ;
    $BarnOwl::recipient  = $m->recipient ;
    $BarnOwl::sender     = $m->sender    ;
    $BarnOwl::realm      = $m->realm     ;
    $BarnOwl::opcode     = $m->opcode    ;
    $BarnOwl::zsig       = $m->zsig      ;
    $BarnOwl::msg        = $m->body      ;
    $BarnOwl::time       = $m->time      ;
    $BarnOwl::host       = $m->host      ;
    $BarnOwl::login      = $m->login     ;
    $BarnOwl::auth       = $m->auth      ;
    if ($m->fields) {
	@BarnOwl::fields = @{$m->fields};
	@main::fields = @{$m->fields};
    } else {
	@BarnOwl::fields = undef;
	@main::fields = undef;
    }
}

sub smartfilter {
    die("smartfilter not supported for this message\n");
}

# Display fields -- overridden by subclasses when needed
sub login_type {""}
sub login_extra {""}
sub long_sender {""}

# The context in which a non-personal message was sent, e.g. a chat or
# class
sub context {""}

# Some indicator of context *within* $self->context. e.g. the zephyr
# instance
sub subcontext {""}


1;
