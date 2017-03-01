#!/usr/bin/ruby
require 'mysql'

`nm busybox_unstripped > busybox.txt`
name='busybox_x86_32_FDLIST'

mydb=Mysql.connect('localhost', 'cgrtl', '9-410', 'callgraph')
mydb.query("drop table if exists `#{name}`")
mydb.query("CREATE TABLE IF NOT EXISTS `#{name}`(startaddr varchar(18),name varchar(60))")
mydb.query("BEGIN")

File.open("busybox.txt","r") do |file|  
	while line = file.gets  
		arr=line.split(" ")
		if(arr[1]=="t" or arr[1]=="T")
			mydb.query("insert into `#{name}` values(\"#{arr[0]}\",\"#{arr[2]}\")")
		end
	end  
end  
mydb.query("COMMIT")
