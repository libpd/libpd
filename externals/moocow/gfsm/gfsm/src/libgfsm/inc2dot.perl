#!/usr/bin/perl -w

%inc = qw();
%nod = qw();
foreach $file (@ARGV) {
  open(F,"<$file") or die("$0: open failed for '$file': $!");
  @incs = qw();
  while (<F>) {
    chomp;
    if ($_ =~ m/^\s*\#\s*include\s*[\<\"\']\s*(\S+)[\>\"\']/) { push(@incs,$1); }
  }
  close(F);
  $inc{$file} = [@incs];
  @nod{$file,@incs} = undef;
}

##-- subs
sub safestr {
  my $str = shift;
  $str =~ s/[\.\,\+\-\=]/_/g;
  return $str;
}

##-- write dot file
print
  ("digraph include {\n",
   "  rankdir = LR;\n",
   "  rotate = 90;\n",
  );

##-- all nodes
foreach $f (sort keys(%nod)) {
  $f_str = safestr($f);
  if (exists($inc{$f})) { $attrs = "[ label=\"$f\", shape=box  ]"; }
  else                  { $attrs = "[ label=\"$f\", shape=box, style=filled, fill=gray ]"; }
  print " $f_str $attrs;\n";

  ##-- arcs
  if (defined($incs=$inc{$f})) {
    foreach $i (@$incs) {
      $i_str = safestr($i);
      if (exists($inc{$i})) { $e_attrs = "[ color=black ]"; }
      else                  { $e_attrs = "[ color=red, style=dashed ]" }
      print "\t$f_str -> $i_str $e_attrs;\n"
    }
  }
}
print "}\n";
