-- rumur_flags: ['--threads', '1', '--max-errors', '2']
-- checker_exit_code: 1
-- checker_output: None if xml else re.compile(r'^Startstate 1 fired\.\nx:0\n----------\n\nRule "foo" fired\.\nx:1\n----------\n\nEnd of the error trace\.(.|\n)*^Startstate 1 fired\.\nx:0\n----------\n\nRule "bar" fired\.\nx:2\n----------\n\nEnd of the error trace\.', re.MULTILINE)

/* This model is designed to trigger a bug that was observed on commit
 * 0f66cb1de6e4eb359fc4aa1ab4d55c8dd951a04d. When attempting to report multiple
 * errors and finding a deadlock, the same deadlock would simply be repeated for
 * until the error count had been reached. If this bug has been re-introduced,
 * model will produce the same error trace containing "foo" twice, instead of
 * the two possible deadlocks in this model.
 *
 * The option tweaking above is intended to...
 *  --threads 1: run single threaded to ensure the deadlocks are found in a
 *    deterministic order.
 *  --max-errors 2: look for more than one error, a necessary condition for
 *    triggering the bug.
 *  checker_output: match both different traces.
 */

var
  x: 0 .. 10;

startstate begin
  x := 0;
end;

rule "foo" x = 0 ==> begin
  x := x + 1;
end;

rule "bar" x = 0 ==> begin
  x := x + 2;
end;
