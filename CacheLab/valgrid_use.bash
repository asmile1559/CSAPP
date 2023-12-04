# valgrind 输出文件  实用工具           追踪内存       程序
valgrind --log-fd=1 --tool=lackey -v --trace-mem=yes ls -l
