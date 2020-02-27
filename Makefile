all: mainclient mainserver reverse upper lower ceasar identity yours

mainclient: mainclient.cpp
	g++ mainclient.cpp -o mainclient

mainserver: mainserver.cpp
	g++ mainserver.cpp -o mainserver

reverse: reverse.cpp
	g++ reverse.cpp -o reverse

upper: upper.cpp udp_serv.h
	g++ upper.cpp -o upper

lower: lower.cpp udp_serv.h
	g++ lower.cpp -o lower

ceasar: ceasar.cpp udp_serv.h
	g++ ceasar.cpp -o ceasar

identity: identity.cpp udp_serv.h
	g++ identity.cpp -o identity

yours: yours.cpp udp_serv.h
	g++ yours.cpp -o yours
