use strict;
use warnings;

package BarnOwl::Style::Default;
use POSIX qw(strftime);

################################################################################
# Branching point for various formatting functions in this style.
################################################################################
sub format_message
{
    my $self = shift;
    my $m    = shift;
    my $fmt;

    if ( $m->is_loginout) {
        $fmt = $self->format_login($m);
    } elsif($m->is_ping && $m->is_personal) {
        $fmt = $self->format_ping($m);
    } elsif($m->is_admin) {
        $fmt = $self->format_admin($m);
    } else {
        $fmt = $self->format_chat($m);
    }
    $fmt = BarnOwl::Style::boldify($fmt) if $self->should_bold($m);
    return $fmt;
}

sub should_bold {
    my $self = shift;
    my $m = shift;
    return $m->is_personal && $m->direction eq "in";
}

sub description {"Default style";}

BarnOwl::create_style("default", "BarnOwl::Style::Default");

################################################################################

BEGIN {
    for my $field qw(body
                     context subcontext
                     personal_context short_personal_context
                     login login_type login_extra
                     pretty_sender long_sender
                     pretty_recipient ) {
        no strict 'refs';
        *{"BarnOwl::Style::Default::$field"} = sub {
            my $self = shift;
            my $m = shift;
            return $m->$field;
        }
    }
};

sub format_time {
    my $self = shift;
    my $m = shift;
    my $dateformat = BarnOwl::time_format('get_time_format');
    return strftime($dateformat, localtime($m->unix_time));
}

sub format_login {
    my $self = shift;
    my $m = shift;
    return sprintf(
        '@b<%s%s> for @b(%s) (%s) %s',
        uc( $self->login($m) ),
        $self->login_type($m),
        $self->pretty_sender($m),
        $self->login_extra($m),
        $self->format_time($m)
       );
}

sub format_ping {
    my $self = shift;
    my $m = shift;
    my $personal_context = $self->personal_context($m);
    $personal_context = ' [' . $personal_context . ']' if $personal_context;
    return "\@b(PING)" . $personal_context . " from \@b(" . $self->pretty_sender($m) . ")";
}

sub format_admin {
    my $self = shift;
    my $m = shift;
    return "\@bold(OWL ADMIN)\n" . $self->indent_body($m);
}

sub format_chat {
    my $self = shift;
    my $m = shift;
    my $header = $self->chat_header($m);
    return $header . "\n". $self->indent_body($m);
}

sub chat_header {
    my $self = shift;
    my $m = shift;
    my $header;
    if ( $m->is_personal ) {
        my $personal_context = $self->personal_context($m);
        $personal_context = ' [' . $personal_context . ']' if $personal_context;

        if ( $m->direction eq "out" ) {
            $header = ucfirst $m->type . $personal_context . " sent to " . $self->pretty_recipient($m);
        } else {
            $header = ucfirst $m->type . $personal_context . " from " . $self->pretty_sender($m);
        }
    } else {
        $header = $self->context($m);
        if(defined $self->subcontext($m)) {
            $header .= ' / ' . $self->subcontext($m);
        }
        $header .= ' / @b{' . $self->pretty_sender($m) . '}';
    }

    if($m->opcode) {
        $header .= " [" . $m->opcode . "]";
    }
    $header .= "  " . $self->format_time($m);
    $header .= $self->format_sender($m);
    return $header;
}

sub format_sender {
    my $self = shift;
    my $m = shift;
    my $sender = $self->long_sender($m);
    $sender =~ s/\n.*$//s;
    if (BarnOwl::getvar('colorztext') eq 'on') {
      return "  (" . $sender . '@color[default]' . ")";
    } else {
      return "  ($sender)";
    }
}

sub indent_body
{
    my $self = shift;
    my $m = shift;

    my $body = $self->body($m);
    if ($m->{should_wordwrap}) {
      $body = BarnOwl::wordwrap($body, BarnOwl::getnumcols()-9);
    }
    # replace newline followed by anything with
    # newline plus four spaces and that thing.
    $body =~ s/\n(.)/\n    $1/g;
    # Trim trailing newlines.
    $body =~ s/\n*$//;
    return "    ".$body;
}


1;
