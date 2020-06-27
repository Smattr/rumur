#include <rumur/rumur.h>

// does this model have any put statements that involve scalarset types?
//
// A caller can use this to discriminate whether schedules (the chosen
// permutation of a scalarset during symmetry reduction) need to be available
// during verification even when counterexample traces are not required.
bool prints_scalarsets(const rumur::Model &m);
