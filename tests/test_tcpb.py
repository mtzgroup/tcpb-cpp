"""Tests for the C++ client
"""

import sys
import unittest

from available import available_test
from energy_grad_force import energy_grad_force_test

class TestCppTCPB(unittest.TestCase):
    def test_available(self):
        self.assertTrue(available_test.run_cpp_test())

    def test_energy_grad_force(self):
       self.assertTrue(energy_grad_force_test.run_cpp_test())

if __name__ == '__main__':
    cpp_suite = unittest.TestLoader().loadTestsFromTestCase(TestCppTCPB)
    cpp_results = unittest.TextTestRunner(verbosity=2).run(cpp_suite)
