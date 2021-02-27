#include "Pipeline.h"
#include "Stage.h"
#include <cassert>
#include <climits>
#include <cstddef>
#include <rumur/rumur.h>

using namespace rumur;

void Pipeline::add_stage(Stage &s) {

  // add this stage as the new head of the pipeline
  stages.insert(stages.begin(), &s);

  // inform all stages of the new head
  for (Stage *st : stages)
    st->attach(s);
}

void Pipeline::process(const Node &n) {

  assert(!stages.empty() && "starting processing with an empty pipeline");

  // pass this node to the head of the pipeline
  stages[0]->dispatch(n);
}

void Pipeline::finalise() {

  assert(!stages.empty() && "finalising an empty pipeline");

  // write the remaining contents of the input file
  stages[0]->sync_to(position(nullptr, UINT_MAX, UINT_MAX));

  // notify all stages that we are done
  for (Stage *s : stages)
    s->finalise();
}
