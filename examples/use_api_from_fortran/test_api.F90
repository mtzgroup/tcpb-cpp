!****************************************************
! Program for testing TCPB working as an API library
! Implemented by: Vinicius Wilian D. Cruzeiro
!****************************************************

program test_api

implicit none

character(len=80) :: host
integer :: port, status

! Set information about the server
host = "localhost"
port = 12345

! Attempts to connect to the TeraChem server
write (*,*) "Attempting to connect to TeraChem server using host ", &
  trim(host)//CHAR(0), " and port ", port, "."
call tc_connect(trim(host)//CHAR(0), port, status)
if (status == 0) then
  write (*,*) "Successfully connected to TeraChem server."
else if (status == 1) then
  write (*,*) "ERROR: Connection to TeraChem server failed!"
  STOP
else if (status == 2) then
  write (*,*) "ERROR: Connection to TeraChem server succeed, but the &
    server is not available!"
  STOP
else
  write (*,*) "ERROR: Status on tc_connect function is not recognized!"
  STOP
end if

! Finalizes variables on the TeraChem side
call tc_finalize()

end program test_api
