def lineno(stree):
    stree.calc_position()
    return stree.min_line

class RumurError(Exception):
    def __init__(self, message, node=None):
        if node is not None:
            message = '%d: %s' % (lineno(node), message)
        super(RumurError, self).__init__(message)
