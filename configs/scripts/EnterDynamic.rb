require 'mysql'

$kernel_version="linux-3.5.4"
$directory_type="R"
$platform="x86_32"

$option=$kernel_version+"_"+$directory_type+"_"+$platform+"_"

module Enter
	def Enter.dynamic(dynamic_path)
		puts("#{Time.now}\tENTER DLIST BIGIN")
#		puts("dynamic_path1=#{dynamic_path}")
		dynamic_path=dynamic_path.gsub(/\/+/,"/")
#		puts("dynamic_path2=#{dynamic_path}")
		mydb=Mysql.connect('localhost', 'cgrtl', '9-410', 'callgraph')

		#Create Table DLIST
		mydb.query("DROP TABLE IF EXISTS `#{$option+"DLIST"}`")
		mydb.query("CREATE TABLE IF NOT EXISTS `#{$option+"DLIST"}`
			(F_point INT,
			C_point INT,
			C_id VARCHAR(20),
			Cd_address VARCHAR(20),
			PID VARCHAR(15),
			TID VARCHAR(15),
			C_time VARCHAR(15),
			R_time VARCHAR(15),
			RunTime INT,
			DLIST_id INT PRIMARY KEY AUTO_INCREMENT,
			INDEX(F_point),
			INDEX(C_point))")

		dlist_everyvalue=[]
		
		if File.directory? dynamic_path
			Dir.foreach(dynamic_path) do |filename|
				if filename!="." and filename!=".." and !(File.directory?(dynamic_path+"/"+filename)) and filename.index("result_time")
					puts("#{Time.now}\tfilename=#{filename} BEGIN")
					file=File.open(dynamic_path+"/"+filename,"r")
					while line=file.gets
						line=line.split(/[: ]/)
						if line[19]	#if the TIME field exists——this record is a complete recorde after protected mode
							pid=line[1]	#have existed in DLIST
							tid=line[3]	#have existed in DLIST
							calltime=line[5]	#have existed in DLIST
							returntime=line[7]	#have existed in DLIST
							fname=line[9]
							fdfile=line[10]
							fdline=line[11]
							cname=line[13]
							cdfile=line[14]
							cdline=line[15]
							cdaddress=line[17]
							time=line[19].to_i	#have existed in DLIST
							
							fpoint=0
							cpoint=0
							
							if fdfile=="null"
								rsF=mydb.query("SELECT f_id FROM `#{$option+"FDLIST"}` WHERE f_name='#{fname}'")	#Serch F function in the FDLIST by F_name
								rsF.each_hash do |rowF|
									fpoint=rowF['f_id']
									break	
								end
							else
								rsF=mydb.query("SELECT f_id FROM `#{$option+"FDLIST"}` WHERE f_name='#{fname}' AND f_dfile='#{fdfile}'")	#Serch F function in the FDLIST by F_name, F_dfile
								rsF.each_hash do |rowF|
									fpoint=rowF['f_id']
								end
							end
							
							if cdfile=="null"
								rsC=mydb.query("SELECT f_id FROM `#{$option+"FDLIST"}` WHERE f_name='#{cname}'")	#Serch C function in the FDLIST by C_name
								rsC.each_hash do |rowC|
									cpoint=rowC['f_id']
									break
								end
							else
								rsC=mydb.query("SELECT f_id FROM `#{$option+"FDLIST"}` WHERE f_name='#{cname}' AND f_dfile='#{cdfile}'")	#Serch C function in the FDLIST by C_name, C_dfile
								rsC.each_hash do |rowC|
									cpoint=rowC['f_id']
									break
								end
								
							end
							
							dlist_everyvalue.concat(["(#{fpoint}, #{cpoint}, '#{pid}', '#{tid}', '#{calltime}', '#{returntime}', #{time}),"])
							
							if dlist_everyvalue.size==1000	#insert into DLIST , 1000 as a group
								dlist_values=""
								dlist_everyvalue.each do |row|
									dlist_values += row
								end
								dlist_values=dlist_values.gsub(/,$/,'')
		 
								mydb.query("INSERT INTO `#{$option+"DLIST"}` (F_point, C_point, PID, TID, C_time, R_time, RunTime) VALUES#{dlist_values}")
								dlist_everyvalue.clear
							end							
							
#							mydb.query("INSERT INTO `#{$option+"DLIST"}` (F_point, C_point, PID, TID, C_time, R_time, RunTime) VALUES(#{fpoint}, #{cpoint}, '#{pid}', '#{tid}', '#{calltime}', '#{returntime}', #{time})")	#DLIST中的Cd_address字段在动态跟踪数据中对应着AT位置的数据，目前此数据还没有
						end
					end
				puts("#{Time.now}\tfilename=#{filename} END")
				end
			end
			
			if dlist_everyvalue.size>0	#insert into DLIST , 1000 as a group
				dlist_values=""
				dlist_everyvalue.each do |row|
					dlist_values += row
				end
				dlist_values=dlist_values.gsub(/,$/,'')

				mydb.query("INSERT INTO `#{$option+"DLIST"}` (F_point, C_point, PID, TID, C_time, R_time, RunTime) VALUES#{dlist_values}")
				dlist_everyvalue.clear
			end
			
		end
		puts("#{Time.now}\tENTER DLIST END")
	end
	
	def Enter.dolist()
		puts("#{Time.now}\tDOLIST BEGIN")

		mydb=Mysql.connect('localhost', 'cgrtl', '9-410', 'callgraph')

		#Create Table DOLIST 
		mydb.query("DROP TABLE IF EXISTS `#{$option+"DOLIST"}`")
		mydb.query("CREATE TABLE IF NOT EXISTS `#{$option+"DOLIST"}`
			(F_path VARCHAR(150),
			C_path VARCHAR(150),
			COUNT INT,
			INDEX(F_path, C_path))")
		
		dolist_everyvalue=[]
		tempFname=""	#主调函数名
		tempCname=""	#被调函数名
		tempFdfile=""		#主调函数定义路径
		tempCdfile=""		#被调函数定义路径
		tempFnode=""	#主调函数节点
		tempCnode=""	#被调函数节点
		
		rs=mydb.query("SELECT DISTINCT F_point, C_point FROM `#{$option+"DLIST"}`")
		rs.each_hash do |row|
			
			rsF = mydb.query("SELECT f_name, f_dfile FROM `#{$option+"FDLIST"}` WHERE f_id=#{row['F_point']}") #search the F_dfile in the FDLIST by F_point
			rsF.each_hash do |rowF|
				tempFname=rowF['f_name']
			        tempFdfile=rowF['f_dfile']
				tempFnode=tempFdfile+"/"+tempFname
			end
			
			rsC = mydb.query("SELECT f_name, f_dfile FROM `#{$option+"FDLIST"}` WHERE f_id=#{row['C_point']}") #search the C_dfile in the FDLIST by C_point
			rsC.each_hash do |rowC|
				tempCname=rowC['f_name']
				tempCdfile=rowC['f_dfile']
				tempCnode=tempCdfile+"/"+tempCname
			end

			count=0	#用于记录函数与函数间的调用次数
			if tempFnode!="" and tempCnode!="" and tempFnode!=tempCnode	#若不为空且不是自己调用自己
				rscount=mydb.query("SELECT count(*) FROM `#{$option+"DLIST"}` WHERE F_point=#{row['F_point']} AND C_point=#{row['C_point']}")	#统计函数间调用了多少次
				rscount.each_hash do |rowcount|
					count=rowcount['count(*)'].to_i
					dolist_everyvalue.concat(["(\"#{tempFnode}\",\"#{tempCnode}\",#{count}),"])
				end
				
				#2. the file's DOLIST
				if  tempFdfile!=tempCdfile	#若主调函数与被调函数不在一个文件中
					dolist_everyvalue.concat(["(\"#{tempFdfile}\",\"#{tempCnode}\",#{count}),"])
				end
				
				#3. the directory's DOLIST
				dir=File.dirname(tempFdfile).gsub(/\s+/,"").concat("/")
				while dir and dir!="/"
					dpath=" "+dir
					spath=" "+tempCnode
					if !(spath.index(dpath))
						dolist_everyvalue.concat(["(\"#{dir}\",\"#{tempCnode}\",#{count}),"])
						if dir.rindex("/",dir.rindex("/")-1)
							dir=dir.slice(0..dir.rindex("/",dir.rindex("/")-1))
						else
							break
						end
					else
						break
					end
				end				
			end
			
			if dolist_everyvalue.size>1000	#insert into DLIST , 1000 as a group
				dolist_values=""
				dolist_everyvalue.each do |row|
					dolist_values += row
				end
				dolist_values=dolist_values.gsub(/,$/,'')

				mydb.query("INSERT INTO `#{$option+"DOLIST"}` (F_path, C_path, COUNT) VALUES#{dolist_values}")
				dolist_everyvalue.clear
			end
		end
		
		if dolist_everyvalue.size>0
			dolist_values=""
			dolist_everyvalue.each do |row|
				dolist_values += row
			end
			dolist_values=dolist_values.gsub(/,$/,'')
			mydb.query("INSERT INTO `#{$option+"DOLIST"}` (F_path, C_path, COUNT) VALUES#{dolist_values}")
			dolist_everyvalue.clear
		end
		puts("#{Time.now}\tDOLIST END")
	end
end

if ARGV[0]
	directory_path=(ARGV[0]+"/").gsub(/\/+/,"/")
else
	directory_path="/mnt/freenas/dyn-trace-log/"
end
Enter.dynamic(directory_path)
Enter.dolist()
