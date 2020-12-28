CC = cl
CFLAGS = /nologo /dll

build: so_stdio.lib

so_stdio.lib: so_stdio.dll

so_stdio.dll: so_stdio.obj
		link $(CFLAGS) /out:$@ /implib:so_stdio.lib $**

so_stdio.obj: so_stdio.c
		$(CC) /nologo /W4 /D_CRT_SECURE_NO_WARNINGS /c /Foso_stdio.obj $**
		
clean:
		del so_stdio.dll *.obj
