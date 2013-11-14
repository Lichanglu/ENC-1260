在当前目录下执行makefile clean,make即可
会自动把下级目录的.a文件以及与文件夹同名的.h文件拷贝到output下面，打包。（比如 TS目录，就是生成libts.a,拷贝ts.h和libts.a到output目录）
打包文件名为目录文件名 ，里面包含编译时间信息。


