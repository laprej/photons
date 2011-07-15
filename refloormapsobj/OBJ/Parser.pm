#!/Users/manderso/.local/bin/perl

package OBJ::Parser;

use warnings;
use strict;

sub new {
  my($class, $filename) = @_;
  my $data = {
	      'filename' => $filename
	     };
  return bless($data, $class);
}


1;
