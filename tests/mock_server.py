#!/usr/bin/env python
# mock_server.py
# Written by Stefan Seritan on July 16th, 2017
#
# A mock server for TCPB client testing
# Communication is really over sockets, but responses are feed into the mock server beforehand
# Threading model from https://stackoverflow.com/questions/23828264/how-to-make-a-simple-multithreaded-socket-server-in-python-that-remembers-client

import numpy as np
import socket
import struct
from threading import Thread
from Queue import Queue

from tcpb import terachem_server_pb2 as pb


class MockServer(object):
    """Mock server for TCPB client testing

    Intended testing workflow:
    - Response state of MockServer is set
    - Client call is executed and communicated to mock server over socket
    - Response is sent back to client over socket (on listening thread)
    - Output of client function is tested for correctness
    """
    def __init__(self, port, intracefile, outtracefile):
        """Initialize the MockServer object.

        Args:
            port: Integer of port number
            intracefile: Binary file containing packets recv'd by client
            outtracefile: Binary file containing packets sent by client
        """
        # Sanity checks
        if not isinstance(port, int):
            raise TypeError("Port number must be an integer")
        if port < 1023:
            raise ValueError("Port number is not allowed to below 1023 (system reserved ports)")

        self.header_size = 8 #Expect exactly 2 ints of 4 bytes each
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.sock.bind(('localhost', port))
        self.listen_thread = Thread(target=self.listen)

        # Expected messages from client (out for client, in for server)
        self.expected_msgs = self.load_trace(outtracefile)

        # Response message for client (in for client, out for server)
        self.response_msgs = self.load_trace(intracefile)

        # Start listening thread
        self.listen_thread.start()

    def load_trace(self, tracefile):
        """Load a packet trace from a TCPB client job run with trace=True
        
        Args:
            tracefile: If True, append to outfile. If False, overwrite outfile.
        Returns:
            msgs: List of (msg_type, msg_pb) from tracefile
        """
        f = open(tracefile, 'rb')
        data = f.read()
        f.close()

        msgs = []
        packet_start = 0
        while packet_start < len(data):
            header_mid = packet_start + 4
            header_end = packet_start + 8
            msg_type = struct.unpack_from('>I', data[packet_start:header_mid])[0]
            msg_size = struct.unpack_from('>I', data[header_mid:header_end])[0]
            msg_end = header_end + msg_size

            if msg_type == pb.STATUS:
                msg_pb = pb.Status()
            elif msg_type == pb.MOL:
                msg_pb = pb.Mol()
            elif msg_type == pb.JOBINPUT:
                msg_pb = pb.JobInput()
            elif msg_type == pb.JOBOUTPUT:
                msg_pb = pb.JobOutput()
            else:
                raise RuntimeError("PBHelper: Unknown message type {} for received message.".format(msg_type))

            if len(data) < msg_end:
                raise RuntimeError("PBHelper: Ran out of trace.")

            msg_pb.ParseFromString(data[header_end:msg_end])

            msgs += [(msg_type, msg_pb)]
            
            packet_start = msg_end

        return msgs

    def listen(self):
        self.sock.listen(1)

        client, address = self.sock.accept()
        client.settimeout(5)
        self.testClient(client)

    def testClient(self, client):    
        # Handle getting message from client
        # Very brittle, but should be fine for testing because I control communication
        while len(self.expected_msgs) > 0:
            try:
                header = self._recv_header(client)
            except socket.error as msg:
                print("MockServer: Problem receiving header from client. Error: {}".format(msg))
                return

            if header is None:
                print("MockServer: Problem receiving header from client")
                return

            msg_type = header[0]
            msg_size = header[1]

            expected_type, expected_pb = self.expected_msgs[0]
            if msg_type != expected_type:
                print("MockServer: Did not receive expected msg type from client")
                return

            try:
                msg_str = self._recv_message(client, msg_size)
            except socket.error as msg:
                print("MockServer: Problem receiving message from client. Error: {}".format(msg))
                return

            if msg_type == pb.STATUS:
                recvd_pb = pb.Status()
            elif msg_type == pb.MOL:
                recvd_pb = pb.Mol()
            elif msg_type == pb.JOBINPUT:
                recvd_pb = pb.JobInput()
            elif msg_type == pb.JOBOUTPUT:
                recvd_pb = pb.JobOutput()
            else:
                print("MockServer: Unknown protobuf type")
                return

            recvd_pb.ParseFromString(msg_str)

            # Compare to expected protobuf
            if recvd_pb.SerializeToString() != expected_pb.SerializeToString():
                print("MockServer: Expected protobuf did not match received protobuf")
                print("EXPECTED PROTOBUF:")
                print(expected_pb)
                print("\nRECEIVED PROTOBUF:")
                print(recvd_pb)
                return

            del self.expected_msgs[0]

            # Send response (if one)
            response_type, response_pb = self.response_msgs[0]
            try:
                self._send_header(client, response_type, response_pb.ByteSize())
            except socket.error as msg:
                print("MockServer: Problem sending header to client. Error: {}".format(msg))
                return

            try:
                self._send_message(client, response_pb.SerializeToString())
            except socket.error as msg:
                print("MockServer: Problem sending message to client. Error: {}".format(msg))
                return

            if response_type == pb.STATUS and response_pb.WhichOneof("job_status") == 'completed':
                # Also need to send joboutput, which should be next message
                response_type, response_pb = self.response_msgs[1]
                try:
                    self._send_header(client, response_type, response_pb.ByteSize())
                except socket.error as msg:
                    print("MockServer: Problem sending header to client. Error: {}".format(msg))
                    return

                try:
                    self._send_message(client, response_pb.SerializeToString())
                except socket.error as msg:
                    print("MockServer: Problem sending message to client. Error: {}".format(msg))
                    return

                del self.response_msgs[1]

            del self.response_msgs[0]

        client.shutdown(2)
        client.close()

    # Private send/recv functions
    def _send_header(self, client, msg_type, msg_size):
        """Sends a header to the TCPBClient

        Args:
            client: Client socket
            msg_size: Size of following message (not including header)
            msg_type: Message type (defined as enum in protocol buffer)
        Returns True if send was successful, False otherwise
        """
        # This will always pack integers as 4 bytes since I am requesting a standard packing (big endian)
        # Big endian is convention for network byte order (because IBM or someone)
        header = struct.pack('>II', msg_type, msg_size)
        try:
            client.sendall(header)
        except socket.error as msg:
            print("MockServer: Could not send header. Error: {}".format(msg))
            return False

        return True

    def _send_message(self, client, msg_str):
        """Sends a header to the TCPBClient

        Args:
            client: Client socket
            msg_str: String representation of binary message
        Returns True if send was successful, False otherwise
        """
        try:
            client.sendall(msg_str)
        except socket.error as msg:
            print("MockServer: Could not send message. Error: {}".format(msg))
            return False

        return True

    def _recv_header(self, client):
        """Receive a header from the TCPBClient

        Args:
            client: Client socket
        Returns (msg_type, msg_size) on successful recv, None otherwise
        """
        header = ''
        nleft = self.header_size
        while nleft:
            data = client.recv(nleft)
            if data == '':
                break
            header += data
            nleft -= len(data)

        # Check we got full message
        if nleft == self.header_size and data == '':
            print("MockServer: Could not recv header because socket was closed from client")
            return None
        elif nleft:
            print("MockServer: Got {} of {} expected bytes for header".format(nleft, self.header_size))
            return None

        msg_info = struct.unpack_from(">II", header)
        return msg_info

    def _recv_message(self, client, msg_size):
        """Receive a message from the TCPBClient

        Args:
            client: Client socket
            msg_size: Integer of message size
        Returns a string representation of the binary message if successful, None otherwise
        """
        if msg_size == 0:
            return ""

        msg_str = ''
        nleft = msg_size
        while nleft:
            data = client.recv(nleft)
            if data == '':
                break
            msg_str += data
            nleft -= len(data)

        # Check we got full message
        if nleft == self.header_size and data == '':
            print("MockServer: Could not recv message because socket was closed from client")
            return None
        elif nleft:
            print("MockServer: Got {} of {} expected bytes for header".format(nleft, self.header_size))
            return None

        return msg_str
