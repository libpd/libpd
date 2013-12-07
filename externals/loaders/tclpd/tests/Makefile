all:
	@# check that test-system itself is functional
	@# TODO: check that this check is functional? :-D
	@sh runtest.sh helloworld
	@# basic output test - outputting correct selector/atoms
	@sh runtest.sh basic_output
	@# basic input test - dispatching to correct methods
	@sh runtest.sh basic_input
	@# atoms conversion test & binbuf retrieval
	@sh runtest.sh binbuf

clean:
	rm -fv *.out runtest-*.pd
