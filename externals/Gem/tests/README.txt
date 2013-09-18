regression tests for Gem


tests are pd-patches living in subdirectories from here

naming conventions:
 subdirectories: 
	named after the (main) object to be tested
	e.g. gemframebuffer/ tests problems in [gemframebuffer]

 tests:
	tests starting with "crash_" are supposed to crash Pd
	tests starting with "fail_" are supposed to fail the unit-test
	all other tests are supposed to survive and pass the test


unit-tests:
 starting the test:
	each unit-test get's called with a uniq-ID as first argument
	it is supposed to create a receiver using this uniq-ID as follows
		[r $1-start]
	when a bang is received on this label, the test must start
 evaluating the test:
	each unit-test is supposed to create a sender using the uniq-ID:
		[s $1-result]
	the unit test can send 3 results to this label:
	 0: the test has failed
	 1: the test has passed
	-1: wait, not yet finished!
 running the test:
	when a "bang" is received at "$1-start", the test has to start and
	_immediately_ return one of the 3 results to "$1-result"
	if the test fails to immediately return a result, it is considered
	as FAILED and the test-run is stopped (the unit-test is deleted!)
	if the test needs some time to evaluate, it MUST send "-1" as a result
	in which case the test-engine is paused and waits for a proper result
	(0 or 1) in order to continue
 


