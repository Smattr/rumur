import logging, sys

log = logging.getLogger('rumur')
log.addHandler(logging.StreamHandler(sys.stderr))
