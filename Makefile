all:
	make default
	make optimized

ifeq ($(MODE), TEST)
default:
	@g++ src/colorTheme.cpp customWarning/colorPrint/colorPrint.cpp src/mandelbrot.cpp -I include/ -I customWarning/ -I customWarning/colorPrint/  -lsfml-graphics -lsfml-window -lsfml-system -D_test -o default -march=native -O0 -Wall

optimized:
	@g++ src/colorTheme.cpp customWarning/colorPrint/colorPrint.cpp src/avxMandelbrot.cpp -I include/ -I customWarning/ -I customWarning/colorPrint/  -lsfml-graphics -lsfml-window -lsfml-system -D_test -o optimized -march=native -O0 -Wall -mavx2

else
default:
	@g++ src/colorTheme.cpp customWarning/colorPrint/colorPrint.cpp src/mandelbrot.cpp -I include/ -I customWarning/ -I customWarning/colorPrint/  -lsfml-graphics -lsfml-window -lsfml-system -o default -march=native -O0 -Wall

optimized:
	@g++ src/colorTheme.cpp customWarning/colorPrint/colorPrint.cpp src/avxMandelbrot.cpp -I include/ -I customWarning/ -I customWarning/colorPrint/  -lsfml-graphics -lsfml-window -lsfml-system -o optimized -march=native -O0 -Wall -mavx2
endif

clean:
	rm default optimized