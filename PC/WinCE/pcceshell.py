#
#	A builtin Python Console for Windows/CE
#
#	David Kashtan, Validus Medical Systems
#
import sys
import _pcceshell_support

#
#	If we can't import traceback, use a local print_exception function
#
try:
    #
    #	Import traceback and use its print_exception function
    #
    import traceback
    print_exception = traceback.print_exception
except:
    #
    #	Local print_exception
    #
    def print_exception(Type, Value, Traceback, Limit=None):
	#
	#	Print a warning message (once)
	#
	if "_Import_Message_Printed" not in globals():
	    global _Import_Message_Printed
	    _Import_Message_Printed = True
	    print "Warning: Unable to import traceback, using internal (minimal) traceback"
	#
	#	If there is no limit on the traceback depth, make it sys.tracebacklimit
	#
	if Limit is None:
	    if hasattr(sys, "tracebacklimit"): Limit = sys.tracebacklimit
	    else: Limit = 100000
	#
	#	Work our way up the traceback stack and output a line for each frame (up to the depth limit)
	#
	if Traceback is not None and Limit > 0:
	    sys.stderr.write("Traceback (most recent call last):\n")
	while Traceback is not None and Limit > 0:
	    Code = Traceback.tb_frame.f_code
	    sys.stderr.write("  File \"%s\", line %d, in %s\n"%(Code.co_filename, Traceback.tb_lineno, Code.co_name))
	    Traceback = Traceback.tb_next
	    Limit -= 1
	#
	#	Output the exception information
	#
	Type = str(Type)
	if "." in Type: Type = Type[Type.index(".")+1:]
	sys.stderr.write("%s: %s\n"%(str(Type), str(Value)))

#
#	Add a writelines function to the PCCESHELL
#
def _writelines(File, Lines):
    for Line in Lines: File.write(Line)
_pcceshell_support.writelines = _writelines

#
#	Entry point from python.c: Wait for input, compile it and execute it
#
def main():
    #
    #	Attempt to load the telnet server
    #
    try:
      import telnetd
    except ImportError:
      pass
    try:
	#
	#	Initialize the session
	#
	Locals = {}
	sys.stdout.write("Python %s on %s\n%s" % (sys.version, sys.platform, sys.ps1))
	while 1:
	    #
	    #	Wait for input or exit
	    #
	    if not _pcceshell_support.Wait(): break
	    #
	    #	Get the text to execute and compile it
	    #
	    Code_Text = _pcceshell_support.Get_Input_Text()
	    if not Code_Text: continue
	    try:
		try:
		    Code_Object = compile(Code_Text, "<input>", "single")
		except SyntaxError, Error:
		    try:
			Code_Object = compile(Code_Text + "\n", "<input>", "single")
		    except SyntaxError, Error1:
			try:
			    Code_Object = compile(Code_Text + "\n\n", "<input>", "single")
			except SyntaxError, Error2:
			    try:
				e1 = Error1.__dict__
			    except AttributeError:
				e1 = Error1
			    try:
				e2 = Error2.__dict__
			    except AttributeError:
				e2 = Error2
			    if e1 == e2: raise SyntaxError, Error1
			    Code_Object = None
	    except SyntaxError:
		#
		#	Compiler reported a syntax error
		#
	 	sys.stdout.write("\n")
		Args = sys.exc_info() + (0,)
		print_exception(*Args)
		sys.stdout.write(sys.ps1)
		continue
	    except:
		#
		#	Other errors
		#
		Args = sys.exc_info()
		print_exception(*Args)
		continue
	    #
	    #	See if we got anything
	    #
	    if not Code_Object:
		#
		#	Just re-prompt
		#
		sys.stdout.write("\n%s" % sys.ps2)
		continue
	    #
	    #	Yes: Mark us busy and try to execute the code (with output queued)
	    #
	    sys.stdout.write("\n")
	    _pcceshell_support.Busy(1);
	    try:
		try:
		    exec Code_Object in Locals
		except SystemExit:
		    #
		    #	Exit: Just terminate
		    #
		    _pcceshell_support.Terminate()
		    return
		except:
		    #
		    #	Error: Get the exception information and generate a new exception at this frame level
		    #
		    Exception_Type, Exception_Value, Exception_Traceback = sys.exc_info()
                    try: 1/0
                    except:
			#
			#	We can use this new exception to peel off irrelevent frames from the REAL exception
			#
			Base_Traceback = sys.exc_traceback
			while Exception_Traceback is not None:
				Exception_Traceback = Exception_Traceback.tb_next
				if Base_Traceback is None: break
				Base_Traceback = Base_Traceback.tb_next
			#
			#	Now print the exception
			#
			print_exception(Exception_Type, Exception_Value, Exception_Traceback)
			#
			#	Clean up
			#
			Base_Traceback = None
			Exception_Type = None
			Exception_Value = None
			Exception_Traceback = None
	    finally:
		#
		#	Mark us as not busy and show and queued output (turning queueing off)
		#
		_pcceshell_support.Busy(0);
	    sys.stdout.write(sys.ps1)
    except:
	Args = sys.exc_info()
	print_exception(*Args)
    _pcceshell_support.Terminate()
