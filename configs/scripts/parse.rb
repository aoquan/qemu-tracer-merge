#!/usr/bin/ruby
require 'mysql'

kernel=ARGV[0]+'_R_'+ARGV[1]+'_FDLIST'
mydb=Mysql.connect('localhost', 'cgrtl', '9-410', 'callgraph')

fh = File.new("#{ARGV[3]}", "w")

File.open("#{ARGV[2]}","r") do |file|  
	while line = file.gets 
		arr=line.chomp.split(",")
		tid=(arr[2].to_i(16)&0xffffe000).to_s(16)
		if(arr[3])
			if(arr[3]=="")
				fh.puts arr[0]+","+arr[1]+",0x"+tid+",0x"+arr[2]+","+arr[3]+","+arr[4]
			else
				if(arr[3]=="kernel")
					res=mydb.query("select f_name,f_dfile,f_dline from `#{kernel}` where f_saddress=\"#{arr[4]}\"").fetch_row
					if(res)
						fh.puts arr[0]+","+arr[1]+",0x"+tid+",0x"+arr[2]+","+arr[3]+","+res[0]+":"+res[1]+":"+res[2]
					else
						fh.puts arr[0]+","+arr[1]+",0x"+tid+",0x"+arr[2]+","+arr[3]+","+arr[4]
					end
				else
					res=mydb.query("select name from `#{arr[3]}_#{ARGV[1]}_FDLIST` where startaddr=\"#{arr[4]}\"").fetch_row
					if(res)
						fh.puts arr[0]+","+arr[1]+",0x"+tid+",0x"+arr[2]+","+arr[3]+","+res[0]
					else
						fh.puts arr[0]+","+arr[1]+",0x"+tid+",0x"+arr[2]+","+arr[3]+","+arr[4]
					end
				end
			end
		else
			fh.puts arr[0]+","+arr[1]+",0x"+tid+",0x"+arr[2]
		end
	end  
end  
