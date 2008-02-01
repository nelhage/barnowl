#!/usr/bin/env perl
use strict;
use warnings;

use DBI;
use DBIx::DBSchema;

=head1 DESCRIPTION

Creates the database tables for BarnOwl::MessageList::SQL

=cut

my $messages = DBIx::DBSchema::Table->new({
    name          => 'messages',
    primary_key   => 'id',
    columns       => [
        DBIx::DBSchema::Column->new({
            name => 'id',
            type => 'int',
            null => 0
           }),
        DBIx::DBSchema::Column->new({
            name => 'msg_time',
            type => 'int',
            null => 0,
           }),
        DBIx::DBSchema::Column->new({
            name   => 'protocol',
            type   => 'varchar',
            length => 30,
            null   => 0,
           }),
        DBIx::DBSchema::Column->new({
            name   => 'body',
            type   => 'text',
            null   => 0,
           }),
        DBIx::DBSchema::Column->new({
            name   => 'direction',
            type   => 'varchar',
            length => 10,
            null   => 0,
           }),
        DBIx::DBSchema::Column->new({
            name    => 'expunged',
            type    => 'bool',
            default => 'false'
           })
        ],
    'index'       => DBIx::DBSchema::ColGroup::Index->new([
        [qw(expunged)]
        ]),
   });

my $attrs = DBIx::DBSchema::Table->new({
    name        => 'attributes',
    columns     => [
        DBIx::DBSchema::Column->new({
            name => 'message_id',
            type => 'int',
            null => 0
           }),
        DBIx::DBSchema::Column->new({
            name   => 'key',
            type   => 'varchar',
            length => 30
           }),
        DBIx::DBSchema::Column->new({
            name   => 'value',
            type   => 'text',
            null   => 0,
           }),
       ],
    unique        => DBIx::DBSchema::ColGroup::Unique->new([
        [qw(message_id key)],
        ]),
    'index'       => DBIx::DBSchema::ColGroup::Index->new([
        [qw(message_id key)],
        [qw(key)],
        ]),
   });

my $schema = DBIx::DBSchema->new($messages, $attrs);

sub load_dbconfig {
    my %conf = ();
    open(my $fh, "<", "$ENV{HOME}/.owl/sql") or die("Unable to read $ENV{HOME}/.owl/sql: $!");
    while(my $line = <$fh>) {
        chomp($line);
        my ($k, $v) = split(/\s*:\s*/, $line, 2);
        $conf{$k} = $v;
    }
    close($fh);
    my $driver = delete $conf{driver} || 'SQLite';
    my $user = delete $conf{user};
    my $pass = delete $conf{password};
    $conf{dbname} ||= ($conf{database} || "$ENV{HOME}/.owl/messagedb");
    my $dsn = "dbi:$driver:" .
       join(';', map { $_ ."=".$conf{$_} } grep { defined $conf{$_} } keys %conf);
    return ($dsn, $user, $pass);
}

my ($dsn, $user, $pass) = load_dbconfig();

my $dbh = DBI->connect($dsn, $user, $pass, {RaiseError => 1, AutoCommit => 0});
my @sql = $schema->sql($dbh);

for my $sql (@sql) {
    $dbh->do($sql) or die($dbh->errstr);
}

$dbh->commit;
$dbh->disconnect;
