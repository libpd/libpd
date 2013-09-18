#!/usr/bin/perl

#    Pure Data Packet - scafc: scaf compiler.
#    Copyright (c) by Tom Schouten <tom@zwizwa.be>
# 
#    This program is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.
# 
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
# 
#    You should have received a copy of the GNU General Public License
#    along with this program; if not, write to the Free Software
#    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#

# set this if you want to enable/disable optimizing

$optimize = 1;


# this parses a single scaf line
# it is not very intelligent. only looks for 1 def on a line
# todo: change later so it can read multiple lines


sub remove_illegal_characters {
    my $line = shift;
    $$line =~ s/\+/_PLUS_/g;
    $$line =~ s/-/_MINUS_/g;
    $$line =~ s/\@/_AT_/g;
    $$line =~ s/:/_COLON_/g;
    $$line =~ s/\?/_QMARK_/g;
    $$line =~ s/<</_SHIFT_/g;
    $$line =~ s/</_ST_/g;
    $$line =~ s/>/_GT_/g;
    $$line =~ s/=/_EQ_/g;
    $$line =~ s/\(/_OPEN_/g;
    $$line =~ s/\)/_CLOSE_/g;
}

sub parse_scaf_line {
    my $word, $def, $sub;
    shift;

    # this transforms the source into a parsed assembly like form
    # a word label: "<word>:<ret>"
    # a word definition line "<tab><word><ret>"
    # last def = <ret><ret>

    # dont process if line doesn't have a def

    # first remove comments
    s/\(\s+(\S+\s+)*?\)//g;

    if (m/:\s+/){

	# separate word and definition
	m/:\s+(\S+)\s+(.*)/;
	$word = $1;
	$def = $2;

	# remove illegal characters;
	remove_illegal_characters \$word;
	remove_illegal_characters \$def;

	# format definition in asm style
	$def =~ s/(\S+)(\s*)/\t$1\n/g;

	# replace ; by r
	$def =~ s/\s+;\s*/\n\tr\n/;

	# put word: def into one string
	$sub = "$word:\n$def\n";

	# debug
	#$sub =~ s/\t/<tab>/g;
	#$sub =~ s/\n/<ret>\n/g;
	#print "$sub";

	return $sub;

    }

};



# load and parse scaf source file
sub load_source {
    my $filename = shift;
    open(SOURCE, $filename) or die "Can't locate source module $filename\n";
    my @parsedsource;
    while (<SOURCE>){
	my $sub = parse_scaf_line $_;
	if ($sub) {
	    push @parsedsource, ($sub);
	}

    }
    close(SOURCE);
    return @parsedsource;

}

# this routine parses the optimization rules
sub load_optim {
    my $filename = shift;
    open(OPTIM, $filename) or die "Can't locate optimization rule file $filename\n";
    my @parsedoptim;
    while (<OPTIM>){
	unless (m/\A\#/){

	    if (m/\"\s*(.*?)\s*\".*?\"\s*(.*?)\s*\"/)
	      {
		  my $source = $1;
		  my $dest = $2;

		  $source =~ s/\s+/\n\t/;
		  $dest =~ s/\s+/\n\t/;
		  $source = "\t$source\n";
		  $dest = "\t$dest\n";

		  remove_illegal_characters \$source;
		  remove_illegal_characters \$dest;

		  push @parsedoptim, ("$source:$dest");
	      }
	}
    }
    close(OPTIM);

    return @parsedoptim;


}



# inline one parsed source's definitions into another parsed source's
sub inline_defs {
    my $dest = shift;
    my $source = shift;

    #print @$dest;
    #print @$source;


    # loop over file with inline defs
    foreach (@$source) {
	#print "<SUB>$_</SUB>\n";
	m/(\S+):\n(.*)\tr\n/s;

	my $def = "\t$1\n";
	my $body = $2;

	#print "<DEF>$def</DEF>\n";
	#print "<BODY>$body</BODY>\n";

	foreach (@$dest) {
	    s/$def/$body/g;
	}
	
    }

}

# this changes <WORD> to c <WORD> or j <WORD> all defined words
# the undefined words are supposed to be asm macros
sub call_defs {
    my $dest = shift;

    foreach (@$dest){
	m/(\S+):\n/s;
	my $word = $1;
	foreach (@$dest){
	    s/\t$word\n\tr\n/\tj $word\n/sg;
	    s/\t$word\n/\tc $word\n/sg;
	}
    }
}

# substitue word sequences in dest using optim table
sub subst_optim {
    my $dest = shift;
    my $optim = shift;
    foreach (@$optim){
	m/(.*?):(.*)/s;
	my $key = $1;
	my $subst = $2;
	
	foreach (@$dest){
	    s/$key/$subst/sg;
	}
    }
}
	
# add directives to produce global symbols
# global symbols need to start with carule_
sub global_syms {
    my $source = shift;
    foreach (@$source){
	s/rule_(\S+):\n/.globl\trule_$1\n.type\trule_$1,\@function\nrule_$1:\n/sg;
    }
}

# create an array with names for bookkeeping
sub name_array {
    my @namearray;
    my $source = shift;
    push @namearray, (".globl rulenames\nrulenames:\n");
    foreach (@$source){
	if (m/rule_(\S+):/s){
	    push @namearray, (".asciz\t\"$1\"\n");
	}
    }
    push @namearray, (".byte\t0\n");
    return @namearray;

}

# main program body

$dir=".";

$source = "-";


# parse command line
foreach (@ARGV){
    if (m/-I(.*)/) {
	$dir = $1;
    }
    else {
	$source = $_;
    }
}

$kernel = "$dir/kernel.scaf";
$macro  = "$dir/scafmacro.s";
$rules  = "$dir/optim.rules";



# load files
@psource = load_source $source;
@pkernel = load_source $kernel;
@poptim = load_optim $rules;


# substitute kernel defs in source
if ($optimize) {subst_optim \@psource, \@poptim;}
inline_defs \@psource, \@pkernel;

if ($optimize) {subst_optim \@psource, \@poptim;}

call_defs \@psource;
global_syms \@psource;
@pnames = name_array \@psource;

# print out asm file
print ".include \"$macro\"\n\n";
print @psource;
print @pnames;

