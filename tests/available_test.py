# Test for is_available()

import subprocess

from mock_server import MockServer

# JOB OUTPUT
expected_cycles = 6

#def run_py_test(port=56789, run_real_server=False):
#    """Run the test
#
#    Args:
#        port: Port to use for server and client in testing
#        run_real_server: If True, we expect a real TCPB server and record a packet trace
#                         If False, run the test with MockServer and the recorded packet trace
#    Returns True if passed the tests, and False if failed the tests
#    """
#    # Set up MockServer for testing
#    if not run_real_server:
#        mock = MockServer(port, 'available/client_recv.bin', 'available/client_sent.bin')
#
#    with TCProtobufClient(host='localhost', port=port, trace=run_real_server, method='hf', basis='sto-3g') as TC:
#        count = 0
#        while not TC.is_available():
#            if run_real_server is True:
#                print('Not available')
#            count += 1
#
#        if count != expected_cycles:
#            print('Expected {} cycles, but only got {}'.format(expected_cycles, count))
#            return False
#
#    return True

def run_cpp_test(port=56789):
    """Run the test using an external C++ script in available/
    Note that the expected answer is hardcoded in C++
    If you change the test, make sure to update the C++ as well

    Args:
        port: Port to use for server and client in testing
    Returns True if passed the tests, and False if failed the tests
    """
    # Set up MockServer for testing
    mock = MockServer(port, 'available/client_recv.bin', 'available/client_sent.bin')

    # Subprocess out, expect a returncode of 1 for failure and 0 for success
    rc = subprocess.call("./available/available_test localhost {}".format(port), shell=True)

    if rc:
        return False

    return True

if __name__ == '__main__':
    #run_py_test(run_real_server=True)

    print("Running Python test...")
    run_py_test()

    print("Running C++ test...")
    run_cpp_test()


