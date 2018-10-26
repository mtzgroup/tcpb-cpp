"""Tests for the C++ client
"""

import sys
import unittest

class TestCppTCPB(unittest.TestCase):
    def test_available(self):
        self.assertTrue(available_test.run_cpp_test())

    def test_energy_grad_force(self):
       self.assertTrue(energy_grad_force_test.run_cpp_test())

if __name__ == '__main__':
    cpp_suite = unittest.TestLoader().loadTestsFromTestCase(TestCppTCPB)
    cpp_results = unittest.TextTestRunner(verbosity=2).run(cpp_suite)

    if len(cpp_results.errors) or len(cpp_results.failures):
        print("\n!!! Errors in C++ client !!!")
        sys.exit(1)
