if [ ! -d "bin" ]; then
    mkdir bin
else
	rm bin/*
fi

g++ -g -O0 -I . -o bin/interrupts_EP interrupts_101232958_101232020_EP.cpp
g++ -g -O0 -I . -o bin/interrupts_RR interrupts_101232958_101232020_RR.cpp
g++ -g -O0 -I . -o bin/interrupts_EP_RR interrupts_101232958_101232020_EP_RR.cpp