# -AbstractFS
Abstract file system for Unix OS family represented by singl file.  
FS consists of: Super block, map of index descriptors, array of index descriptors, map of data block and array of data block.
## Compiling
`gcc -c Flash_FS.c -o f.o -lm`  
`ar -r libFS.a f.o`  
`ranlib libFS.a`  
`gcc main.c -L. -lFS -lm -o FS`    
## Usage
- `-f 'fsSize' 'blockSize'`    format and create file system, all parameters are in bytes  
- `-c 'fileName' ` create   file in FS with name 'fileName', if file exist do not rewrite him  
- `-w 'fileName' 'text' `   write 'text' in file 'fileName', if file does not exist or overflow - ERROR occurred  
- `-r 'fileName' `   read all information from file 'fileName'  
- `-d 'fileName' `   delete file 'fileName' from file system  
- `-l `   show all files in file system  
- `-h `   help  
