all: compile link

compile:
	g++ -c ./src/SupportFuncs.cpp \
		-Iexternal/spdlog/include -Iexternal/glm-master \
		-Iexternal/SDL3/include -Iexternal/SDL3_Image/include -Iexternal/nlohmann_json/include  \
		-o ./bin/SupportFuncs.o
	g++ -c ./src/Event.cpp \
		-Iexternal/spdlog/include -Iexternal/glm-master \
		-Iexternal/SDL3/include -Iexternal/SDL3_Image/include \
		-o ./bin/Event.o
	g++ -c ./src/EventBus.cpp \
		-Iexternal/spdlog/include -Iexternal/glm-master \
		-Iexternal/SDL3/include -Iexternal/SDL3_Image/include \
		-o ./bin/EventBus.o
	g++ -c ./src/Component.cpp \
		-Iexternal/spdlog/include -Iexternal/glm-master \
		-Iexternal/SDL3/include -Iexternal/SDL3_Image/include \
		-o ./bin/Component.o
	g++ -c ./src/Entity.cpp \
		-Iexternal/spdlog/include -Iexternal/glm-master \
		-Iexternal/SDL3/include -Iexternal/SDL3_Image/include \
		-o ./bin/Entity.o
	g++ -c ./src/ISystem.cpp \
		-Iexternal/spdlog/include -Iexternal/glm-master \
		-Iexternal/SDL3/include -Iexternal/SDL3_Image/include -Iexternal/nlohmann_json/include  \
		-o ./bin/ISystem.o
	g++ -c ./src/World.cpp \
		-Iexternal/spdlog/include -Iexternal/glm-master \
		-Iexternal/SDL3/include -Iexternal/SDL3_Image/include -Iexternal/nlohmann_json/include  \
		-o ./bin/World.o
	g++ -c ./src/Game.cpp \
		-Iexternal/spdlog/include -Iexternal/glm-master \
		-Iexternal/SDL3/include -Iexternal/SDL3_Image/include -Iexternal/nlohmann_json/include  \
		-o ./bin/Game.o
	g++ -c ./src/main.cpp \
		-Iexternal/spdlog/include -Iexternal/glm-master \
		-Iexternal/SDL3/include -Iexternal/SDL3_Image/include -Iexternal/nlohmann_json/include \
		-o ./bin/main.o

link:
	g++ ./bin/SupportFuncs.o ./bin/Event.o ./bin/EventBus.o ./bin/Component.o ./bin/Entity.o ./bin/ISystem.o ./bin/World.o ./bin/Game.o ./bin/main.o \
		-Lexternal/spdlog -lspdlog \
		-Lexternal/SDL3/lib -lSDL3 \
		-Lexternal/SDL3_Image/lib -lSDL3_image \
		-lwinmm -limm32 -lversion -lole32 -loleaut32 \
		-lsetupapi -lshell32 -luser32 -lgdi32 -luuid \
		-o ./bin/game.exe

run:
	./bin/game.exe

clean:
	rm -f ./bin/*.o ./bin/game.exe