#
#	David Kashtan, Validus Medical Systems Inc.
#
#	A simple telnet server (using telnetlib) to allow
#	us to telnet into the python shell
#
import sys		# For access to stdout/stderr
import telnetlib	# For the telnet server
import thread		# For running the telnet server input in a separate thread
import socket		# For accepting telnet server connections
import string
import _pcceshell_support

#
#	Telnet server class (Need this for the "write" function)
#
class Telnetd:
    #
    #	Initialize the telnetd class with a telnetlib object from the supplied "accept" connection
    #
    def __init__(Self, New_Connection):
	#
	#	Get a telnetlib and set it up with the new connection
	#
	Self.t = telnetlib.Telnet()
	Self.t.sock = New_Connection[0]
	Self.t.host = New_Connection[1][0]
	Self.t.port = New_Connection[1][1]
	#
	#	Save the old stdout/stderr
	#
	Self.old_stdout = sys.stdout
	Self.old_stderr = sys.stderr
	#
	#	Change stdout/stderr to us
	#
	sys.stdout = Self
	sys.stderr = Self
	#
	#	Setup for us to echo and suppress Go-Aheads
	#
	Self.t.sock.sendall(telnetlib.IAC + telnetlib.DONT + telnetlib.ECHO +
			    telnetlib.IAC + telnetlib.DO   + telnetlib.SGA +
			    telnetlib.IAC + telnetlib.WILL + telnetlib.ECHO)
    #
    #	Write to the telnet client AND to the original stdout
    #
    def write(Self, String):
	#
	#	Do the telnet CRLF conversion and write to the telnet client
	#
	Self.t.write(string.replace(String, "\n", "\r\n"))
	#
	#	Also write to the original stdout
	#
	Self.old_stdout.write(String)
    #
    #	Implement writelines
    #
    def writelines(Self, Lines):
	for Line in Lines: Self.write(Line)
    #
    #	Flush to the telnet cliend AND to the original stdout
    #
    def flush(Self):
	Self.old_stdout.flush()
    #
    #	Close the telnet server connection
    #
    def close(Self):
        #
        #	Restore stdout/stderr
        #
        sys.stdout = Self.old_stdout
        sys.stderr = Self.old_stderr
        #
        #	Close the underlying telnetlib connection
        #
        Self.t.close()
        Self.t = None
    #
    #	On destruction make sure it is closed
    #
    def __del__(Self):
        if Self.t: Self.close()

#
#	A Null option negotiation routine (to keep telnetlib from responding to options)
#
def No_Option_Negotiation(s, cmd, opt): return

#
#	Wait for an incoming telnet connection and set it up
#
def Accept_Connection():
    #
    #	Setup the listener socket
    #
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM, 0)
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    s.bind(("0.0.0.0", 23))
    s.listen(1)
    #
    #	Wait for and accept the new connection (and get rid of the listener)
    #
    New_Connection = s.accept()
    s.close()
    #
    #	Put the socket into a telnetlib which we then put into a Telnetd
    #
    t = Telnetd(New_Connection)
    #
    #	Effectively disable the telnetlib option negotiation
    #
    t.t.set_option_negotiation_callback(No_Option_Negotiation)
    return t


#
#	Thread that reads data from the telnet connection and send characters to the edit window
#
def Telnetd_Thread(Ignore):
    while 1:
      #
      #	Wait for an incoming telnet connection
      #
      Telnetd = Accept_Connection()
      #
      #	Run through the edit window and send all the data to the telnet client
      #
      i = 0
      while 1:
	  Telnetd.t.write("\r\n")
          line = _pcceshell_support.Get_Edit_Window_Line(i)
	  if line is None: break
	  Telnetd.t.write(string.replace(line, "\n", "\r\n"))
          i += 1
      #
      #	Pretend that we got a <cr> so we get a new prompt
      #
      Data = chr(13)
      #
      #	Read telnet client data and send it to the edit window
      #
      while 1:
	#
	#	Process input data
	#
        for Character in Data:
	    #
	    #	Process the character (Turning <Del> into <Backspace>)
	    #
	    Character_Value = ord(Character) & 0x7f
	    if Character_Value == 127: Character_Value = 8
            if Character_Value >= 32:
		#
		#	Non-Control character: Echo it
		#
                Telnetd.t.write(Character)
            else:
		#
		#	Control character: Get the last line in the window
		#
		line = _pcceshell_support.Get_Edit_Window_Line(-1)
		if line is None: line = ""
		if Character_Value == 10:
			#
			#	Linefeed: Echo it
			#
			Telnetd.t.write(chr(10))
		elif Character_Value == 8:
			#
			#	Backspace: Check for prompt ">>> "
			#
			if len(line) <= 4:
				#
				#	At the beginning of the prompt: Ignore the backspace
				#
				continue
			#
			#	Erase the previous character on the client side and force the
			#	character to <backspace> then fall into the code that gives it
			#	to the shell edit window
			#
       		        Telnetd.t.write("\b \b")
			Character_Value = 8
		elif Character_Value == 13:
			#
			#	Carriage-Return: Check for the prompt ">>> "
			#
			if len(line) <= 4:
				#
				#	At the beginning of the prompt: Echo the new prompt
				#
				Telnetd.t.write("\r\n%s"%line)
	    #
	    #	Give this character to the Python Shell edit window
	    #
	    _pcceshell_support.Character_Input(Character_Value)
	#
	#	Get more telnet data
	# 
	Data=Telnetd.t.read_some()
        if len(Data) == 0: break
      #
      #	Close the connection
      #
      Telnetd.close()
      Telnetd=None
    

#
#	Start the listener thread
#
thread.start_new_thread(Telnetd_Thread, (None,))
