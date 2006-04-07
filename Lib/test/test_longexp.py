import unittest
from test import test_support

class LongExpText(unittest.TestCase):
    def test_longexp(self):
        REPS = 65580
        import os
        if os.name == 'ce':
            REPS = 30000        # Less memory on Windows CE
        l = eval("[" + "2," * REPS + "]")
        self.assertEqual(len(l), REPS)

def test_main():
    test_support.run_unittest(LongExpText)

if __name__=="__main__":
    test_main()
