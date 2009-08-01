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
        ],
    'index'       => DBIx::DBSchema::ColGroup::Index->new([
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

my $dsn = shift;
my $user = shift;
my $pass = shift;

my $dbh = DBI->connect($dsn, $user, $pass, {RaiseError => 1, AutoCommit => 0});
my @sql = $schema->sql($dbh);

for my $sql (@sql) {
    $dbh->do($sql) or die($dbh->errstr);
}

$dbh->commit;
$dbh->disconnect;
