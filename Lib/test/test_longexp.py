
REPS = 65580

import sys
if hasattr(sys, "original_platform") and sys.original_platform == "Pocket PC":
    REPS = 30000

l = eval("[" + "2," * REPS + "]")
print len(l)
