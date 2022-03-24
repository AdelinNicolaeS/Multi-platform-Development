build:
	cl /Fohashmap.obj /c /MD /DEBUG hashmap.c 
	cl /Foso-cpp.obj /c /MD /DEBUG so-cpp.c 
	cl /Feso-cpp.exe /MD /DEBUG hashmap.obj so-cpp.obj 
clean:
	del hashmap.obj so-cpp.obj so-cpp.exe