require 'mysql'

$kernel_version="linux-3.5.4"
$directory_type="R"
$platform="x86_32"
$option=$kernel_version+"_"+$directory_type+"_"+$platform+"_"

#$totaltime_dir="/mnt/freenas/dyn-trace-log/"
$totaltime_file="/home/crdong/main/x86_32/result_TotalTime.dat"

mydb=Mysql.connect('localhost', 'cgrtl', '9-410', 'callgraph') #connect to mysql database

#Create Table S2ETimeLIST 
mydb.query("DROP TABLE IF EXISTS `#{$option+"S2ETimeLIST"}` ")
mydb.query("CREATE TABLE IF NOT EXISTS `#{$option+"S2ETimeLIST"}` 
	(f_point INT,
	f_time INT)")

file=File.open($totaltime_file,"r")
timelist_everyvalue=[]

while line=file.gets
	funinfo=line.split(/[\s:]/)
	tempfname=funinfo[0]
	tempfdfile=funinfo[1]
	tempfdline=funinfo[2]
	tempftime=funinfo[3].to_i
	rs=mydb.query("SELECT f_id FROM `#{$option+"FDLIST"}` WHERE f_name=\"#{tempfname}\" AND f_dfile=\"#{tempfdfile}\"")
	rs.each_hash do |row|
		tempfid=row['f_id'].to_i
		timelist_everyvalue.concat(["(#{tempfid},#{tempftime}),"])
		if timelist_everyvalue.size==1000	#insert into SLIST , 1000 as a group
			timelist_values=""
			timelist_everyvalue.each do |row|
				timelist_values += row
			end
			timelist_values=timelist_values.gsub(/,$/,'')
			mydb.query("INSERT INTO `#{$option+"S2ETimeLIST"}`(f_point, f_time) VALUES#{timelist_values}")
			timelist_everyvalue.clear
		end
		#mydb.query("INSERT INTO S2ETimeLIST(f_point, f_time) VALUES(#{tempfid}, #{tempftime})")
	end
end

if timelist_everyvalue.size>0	#insert into SLIST , 1000 as a group
	timelist_values=""
	timelist_everyvalue.each do |row|
		timelist_values += row
	end
	timelist_values=timelist_values.gsub(/,$/,'')
	mydb.query("INSERT INTO `#{$option+"S2ETimeLIST"}`(f_point, f_time) VALUES#{timelist_values}")
	timelist_everyvalue.clear
end
