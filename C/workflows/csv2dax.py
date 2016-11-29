#!/usr/bin/python

import sys

if (len(sys.argv) != 2):
	print "Usage:",sys.argv[0], "<csv filename>"
	sys.exit(1)

with open(sys.argv[1]) as f:
	csv_content = f.readlines()

for i in range(0, len(csv_content)):
	csv_content[i] = csv_content[i].rstrip()




# Get the files and build a dictionary
file_dict = {}
for csv_line in csv_content:
	if csv_line.split(",")[0] == "File":
		file_dict[csv_line.split(",")[1]] = 1024 * 1024 * float(csv_line.split(",")[2])

# Process all the tasks
jobCount = 0
jobs_xml=""
for csv_line in csv_content:
	if csv_line.split(",")[0] != "Task":
		continue
	task_name = csv_line.split(",")[1]
	job_xml = ""
	jobCount += 1
	job_xml += "<job id=\"ID0000"+csv_line.split(",")[1] +"\" "
	job_xml += "namespace=\"NAMESPACE\" "
	job_xml += "name=\"" + csv_line.split(",")[0] +"\" "
	job_xml += "version=\"1.0\" "
	job_xml += "runtime=\"" + csv_line.split(",")[2] + "\">\n"

	for csv_line_file in csv_content:
		if csv_line_file.split(",")[0] != "Uses":
			continue
		if csv_line_file.split(",")[1] != task_name:
			continue
		filename = csv_line_file.split(",")[2]
		job_xml += "  <uses file=\""+filename+"\" "
		job_xml += "link=\""+csv_line_file.split(",")[3]+"\" "
		job_xml += "register=\"true\" "
		job_xml += "transfer=\"true\" "
		job_xml += "optional=\"false\" "
		job_xml += "type=\"data\" "
		job_xml += "size=\"" + str(file_dict[filename]) + "\" />\n"

	job_xml += "</job>\n"
	jobs_xml += job_xml;

	

# Process all the dependencies
childCount = 0
depends_xml=""
previously_seen_child = ""
for csv_line in csv_content:
	if csv_line.split(",")[0] != "Depends":
		continue
	child_name = csv_line.split(",")[1]
	parent_name = csv_line.split(",")[2]
	if previously_seen_child != child_name:
		if previously_seen_child != "":
			depends_xml += "</child>\n"
		previously_seen_child = child_name
		depends_xml += "<child ref=\"ID0000"+child_name+"\">\n"
		childCount += 1
	depends_xml += "  <parent ref=\"ID0000"+parent_name+"\" />\n"
depends_xml += "</child>\n"


preamble_xml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
preamble_xml += "<adag xmlns=\"http://pegasus.isi.edu/schema/DAX\" "
preamble_xml += "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
preamble_xml += "xsi:schemaLocation=\"http://pegasus.isi.edu/schema/DAX http://pegasus.isi.edu/schema/dax-2.1.xsd\" "
preamble_xml += "version=\"2.1\" count=\"1\" index=\"0\" name=\"NAME\" "
preamble_xml += "jobCount=\"" + str(jobCount) + "\" "
preamble_xml += "fileCount=\"0\" "
preamble_xml += "childCount=\"" + str(childCount) + "\">\n"

print preamble_xml
print jobs_xml
print depends_xml
print "</adag>\n"

