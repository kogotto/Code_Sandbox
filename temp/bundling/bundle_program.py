#! /usr/bin/env python

import sys, os, time, tarfile

months  = ['JAN','FEB','MAR','APR','MAY','JUN','JUL','AUG','SEP','OCT','NOV','DEC']
cameras = ['cam1','cam2','cam3','cam4','cam5']


EO_camera_directories = 5
IR_camera_directories = 4

EO_images_per_step = 1
IR_images_per_step = 4


###########################################
#			Configuration Options		  #
###########################################
class ConfigOptions:
	"""
	Configuration Options for the Calibration Collection Program
	"""
	
	# List all of the command line options and assign initial values
	camera_type      = '_NONE_'
	camera_set       = False
	input_directory  = '_NONE_'
	output_directory = './image_output'
	num_bundles      = 2
	debug_level      = 0
	prefix_dir       = '.'

	def __init__( self ):
		"""
		Default Constructor for the Configuration Options 
		"""
		
		# Load the ini file
		inifilename = os.path.split(sys.argv[0])[0] + '/bundle_program.ini'

		# make sure it exists
		if os.path.exists( inifilename ) == False:
			print 'Error: could not find config file'
			return -1

		inifile=open(inifilename, 'r')
		inidata= inifile.read().split()	
		
		for str in inidata:
			hdr, dta = str.split('=')
			if   hdr == 'prefix_dir':
				self.prefix_dir = dta
			elif hdr == 'debug_level':
				self.debug_level = int(dta)

		
		# begin sweeping over items, looking for matching parameters
		command_args = sys.argv[1:]
		while( len(command_args) > 0 ):
			
			# input camera directory
			if command_args[0][:7] == '-input=':
				self.input_directory = command_args[0][7:]
				if self.input_directory[len(self.input_directory)-1] == '/':
					self.input_directory = self.input_directory[:len(self.input_directory)-1]
				command_args = command_args[1:]
			
			# output camera directory
			elif command_args[0][:8] == '-output=':
				self.output_directory = command_args[0][8:]
				command_args = command_args[1:]

			# debugging level
			elif command_args[0][:13] == '-debug_level=':
				self.debug_level = int(command_args[0][13:])
				command_args = command_args[1:]


			# input number of camera bundles
			elif command_args[0][:13] == '-num_bundles=':
				self.num_bundles   = int(command_args[0][13:])
				command_args = command_args[1:]
			
			# type of camera (EO or IR)
			elif command_args[0][:10] == '-cam_type=':
				
				if command_args[0][10:] == 'EO':
					self.camera_type = 'EO'
					self.camera_set  = True

				elif command_args[0][10:] == 'IR':
					self.camera_type = 'IR'
					self.camera_set  = True
				
				else:
					raise Exception('ERROR: unknown camera type: ' + command_args[0][10:])

				command_args = command_args[1:]
			
			# help
			elif command_args[0][:6] == '--help' or command_args[0][:5] == '-help' or command_args[0][:4] == 'help':
				self.usage()
				quit()

			# throw an exception if an option is not recognized
			else:
				self.usage()
				raise Exception('ERROR: Unknown parameter: ' + command_args[0]);
		
	
		# make sure everything we need is present
		if self.input_directory == '_NONE_':
			self.usage()
			raise Exception('ERROR: input camera directory must be defined')
		
		self.output_directory += '.tar.gz'
		
	
		print self
		raw_input("HOLDING")

		
	def usage( self ):
		"""
		Usage function for the program
		"""

		print sys.argv[0] + ' [options]'
		print ''
		print '   options:'
		print '       -input=<input file>'
		print '       -output=<output file>  (output will be tar.gz so don\'t add it to the string'
		print '       -num_bundles=<number of desired image bundles>'
		print '       -cam_type=<EO or IR>'
		print '       -debug_level=<0 default>  [0 - only fatal, 1 - everything]'
		print ''
		print '   .ini config options'
		print '       prefix_dir=<value>   -- Where you want to start with the input path'
		print '       input_dir=<value>    -- Name of input directory to bundle'
		print '       output_dir=<value>   -- Where to place the results with respect to the input
		print '       

	def __str__( self ):
		"""
		Print function for the Command Line option class
		"""

		strout  = 'Command-Line Options ' + '\n'
		strout += '     Prefix Directory : ' + self.prefix_dir      + '\n'
		strout += '     Input Directory  : ' + self.input_directory + '\n'
		strout += '     Output Directory : ' + self.output_directory + '\n'
		strout += '     Number Bundles   : ' + str(self.num_bundles) + '\n'
		strout += '     Camera Type      : ' + self.camera_type + '\n'
		strout += '     Camera Set       : ' + str(self.camera_set)  + '\n'
		strout += '     Debug Level      : ' + str(self.debug_level) + '\n'
		return strout


#####################################
#             TACID Parser			#
#####################################
class TACID:
	input_string     = []
	raw_input        = []
	acquisition_date = []
	program_code     = []
	sortie_number    = []
	scene_number     = []
	producer_code    = []
	product_number	 = []
	project_code	 = []
	replay		     = []
	producer_sn		 = []
	production_datim = []


	def __init__(self, imgname ):
	
		instr = os.path.basename(imgname)
		self.input_string = instr
		self.raw_input = imgname

		# set the date
		day  = int(instr[:2])
		mon  = instr[2:5]
		year = int(instr[5:7]) + 2000
		for x in xrange(len(months)):
			if months[x] == mon:
				mon = x+1
				break
		t = time.mktime((year,mon,day, 0, 0, 0, 0, 0, -1))
		self.acquisition_date = time.gmtime(t)
		instr = instr[7:]

		# get program_code
		self.program_code = instr[0:2]
		instr = instr[2:]

		# get sortie_number
		self.sortie_number = instr[0:2]
		instr = instr[2:]

		# get scene number
		self.scene_number = int(instr[:5])
		instr = instr[5:]

		# get DoD producer code
		self.producer_code = instr[:2]
		instr = instr[2:]

		# get product number
		self.product_number = instr[:6]
		instr = instr[6:]
		
		# get the NGA Project Code
		self.project_code = instr[:2]
		instr = instr[2:]

		# get the replay (reprocessed or retransmitted state flag)
		self.replay = instr[:3]
		instr = instr[3:]

		# get the producer serial number
		self.producer_sn = instr[:3]
		instr = instr[3:]
	
		# production datim
		value = '0x' + instr[:8]
		self.production_datim = int(value, 16)
		instr = instr[8:]



	def __str__(self):
		output  = 'input string     : ' + self.input_string			 + '\n'
		output += 'acquisition date : ' + str(self.acquisition_date) + '\n'
		output += 'program code     : ' + str(self.program_code)     + '\n'
		output += 'sortie number    : ' + str(self.sortie_number)	 + '\n'
		output += 'scene number     : ' + str(self.scene_number)	 + '\n'
		output += 'producer code    : ' + str(self.producer_code)	 + '\n'
		output += 'product number   : ' + str(self.product_number)   + '\n'
		output += 'project code     : ' + str(self.project_code)     + '\n'
		output += 'replay           : ' + str(self.replay)			 + '\n'
		output += 'producer_sn      : ' + str(self.producer_sn)		 + '\n'
		output += 'production_datim : ' + str(self.production_datim) + '\n'

		return output


def nodesEqual( nodeA, nodeB ):
	
	if os.path.basename(nodeA.name) == os.path.basename(nodeB.name):
		if nodeA.type == nodeB.type:
			return True
	
	return False

def isValidTACID( node ):

	if len(os.path.basename(node.name)) != 44:
		return False
	
	return True


def validityCheck( directory ):
	
	# make sure you have a file
	if os.path.isdir(directory) == False:
		raise Exception(directory + ' is not a directory');

	
	return True

def find_camera_directory( directory ):
	
	# extract the contents of the directory
	contents = os.listdir(directory);

	# remove all non-directories
	directories = [elem for elem in contents if os.path.isdir(directory+'/'+elem) == True]
	
	# search to see if cam directories are present
	dir_stack = []
	for d in directories:
		try:
			idx = cameras.index(d) 
			dir_stack.append(directory+'/'+d)
		except ValueError:
			pass
	
	# if directory stack is empty or does not contain a sufficient number of cameras, then
	# we need to step into any directories in the current location
	if len(dir_stack) <= 0:
	
		# if there are no more directories, then we need to return a null list
		if directories == []:
			return []
		# Test each remaining directory
		else:
			for d in directories:
				dir_stack = find_camera_directory( directory + '/' + d )
				
				if dir_stack != []:
					return dir_stack
				else:
					pass
			
	else:
		return dir_stack





def sort_image_list( root_dir ):

	# set x to the starting point
	x = 1
	
	# iterate throught the entire list
	while x < (len(root_dir)):
		
		# create the test point
		item    = root_dir[x][0]
		item_pn = item.producer_sn
		item_sn = item.scene_number

		# iterate through the items before the test point
		y = 0
		i = -1
		FLG = False
		while y < x:
	    	
			# set anchor at where the test point become less than
			if item_pn < root_dir[y][0].producer_sn:
				i = y
				break
			elif item_pn == root_dir[y][0].producer_sn:
				if item_sn < root_dir[y][0].scene_number:
					i = y
					break
				if item_sn == root_dir[y][0].scene_number:
					root_dir[y] = root_dir[y] + root_dir[x]
					root_dir.pop(x)
					FLG = True
					break
			y += 1
		

		if ( FLG == True ) or (y == x):
			x += 1
			continue
		
		if y < x:
			temp = root_dir[x]

			z = x
			while z > i:
				root_dir[z] = root_dir[z-1]
				z = z - 1
			root_dir[i] = temp

		x += 1


def build_image_list( root_dir ):

	# create an empty list
	images = []

	# find the contents of the directory
	contents = os.listdir( root_dir )

	# iterate through each item to either add or enter
	for item in contents:
		
		# check if item is file or directory
		if os.path.isdir( root_dir + '/' + item ) == True:

			# enter directory recursively
			images = images + build_image_list( root_dir + '/' + item )

		# otherwise, check if file matches
		elif os.path.isfile( root_dir + '/' + item ) == True:
			
			# check if the extension is a nitf
			if os.path.splitext( item )[1] == ".ntf" or os.path.splitext( item )[1] == ".NTF":
				
				images = images +  [[TACID(root_dir + '/' + item)]]

		else:
			raise Exception("ERROR: must be file or directory")

		
	return images



def image_tuple_match( cam_lists ):
	""" 
	This function determines if the top items in each camera array are from the same 
	time step.  If they are, then it returns true.  Otherwise, it returns false.
	"""

	# iterate over each camera file listing and see if the top of each list matches
	top_item = cam_lists[0][0][0]
	
	for lst in cam_lists:
		if top_item.scene_number != lst[0][0].scene_number:
			return False

		if top_item.product_number[:3] != lst[0][0].product_number[:3]:
			return False

		if top_item.producer_sn != lst[0][0].producer_sn:
			return False
	
	return True

def pop_earliest_image( cam_lists ):
	
	top_item = cam_lists[0][0][0]
	top_sn   = cam_lists[0][0][0].scene_number
	top_pn   = cam_lists[0][0][0].producer_sn
	top_idx  = 0

	for idx in range( 1, len(cam_lists)):

		# compare the top item with the current list head item
		if top_pn == cam_lists[idx][0][0].producer_sn:
			
			if top_sn <= cam_lists[idx][0][0].scene_number:
				continue
			elif top_sn > cam_lists[idx][0][0].scene_number:
				top_item = cam_lists[idx][0][0]
				top_sn   = cam_lists[idx][0][0].scene_number
				top_pn   = cam_lists[idx][0][0].producer_sn
				top_idx  = idx
			else:
				raise Exception("EROR")
		
		elif top_pn < cam_lists[idx][0][0].producer_sn:
			continue
		
		elif top_pn > cam_lists[idx][0][0].producer_sn:
			top_item = cam_lists[idx][0][0]
			top_sn   = cam_lists[idx][0][0].scene_number
			top_pn   = cam_lists[idx][0][0].producer_sn
			top_idx  = idx
			continue

		else:
			raise Exception("ERROR")

	# remove oldest image
	cam_lists[top_idx].pop(0)



def prune_camera_list( cam_lists ):
	
	# find the length of the shortest camera directory
	min_len = len( cam_lists[0] )
	for x in range( 1, len( cam_lists )):
		if min_len > len( cam_lists[x] ):
			min_len = len( cam_lists[x] )
	
	# iterate through each list, looking for matching images
	breakNow = False

	output = []
	
	while True:
		
		for x in range( 0, len( cam_lists )):
			if len( cam_lists[x] ) <= 0:
				breakNow = True
				break
		if breakNow == True:
			break

		# if all cam top images don't match, then pop the youngest until they do
		if  image_tuple_match( cam_lists ) == False:
			pop_earliest_image( cam_lists )
		else:
			st = []
			for x in range( 0, len(cam_lists)):
				st = st + cam_lists[x][0]
				cam_lists[x] = cam_lists[x][1:]
			output.append(st)

	return output


def main():

	# Parse command-line options
	options = ConfigOptions()
	
	# check that directory conforms to expected format
	validityCheck( options.input_directory )

	# do recursive traversal to find the camera directory location
	camera_roots = find_camera_directory( options.input_directory )
	
	# make sure we got something useful
	if camera_roots == None or len(camera_roots) <= 0:
		raise Exception('Error: directory contains no camera directories')	

	# we need to build an array which contains the list of files for each camera folder
	cam_lists = []
	if options.debug_level >= 1:
		print 'building camera array'
	
	for x in range( 0, len(camera_roots)):
		
		# create image list
		cam_lists.append( build_image_list( camera_roots[x] ))
		sort_image_list( cam_lists[x] )
	
	# next, we need to ensure that each camera folder contains only matching image pairs
	if options.debug_level >= 1:
		print 'pruning the list'
	image_tuples = prune_camera_list( cam_lists )

	# now that we have a list of images, we need to start dividing image sets
	bundle_step = len(image_tuples) / options.num_bundles
	
	# create output data
	tarball = tarfile.open( name=options.output_directory, mode='w:gz' )

	# create current position pointer
	current_pos = 0
	for x in range(0, options.num_bundles):
		
		# increment position
		current_pos = bundle_step * x
		for y in range( 0, len(image_tuples[current_pos])):
			
			# add file to tarball
			if options.debug_level >= 1:
				print image_tuples[current_pos][y].raw_input
			tarball.add( image_tuples[current_pos][y].raw_input)


	

if __name__ == "__main__":
	main()